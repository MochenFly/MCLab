#include "MCVideoPlayer.h"
USE_MCWIDGET_NAMESPACE

#include <QDebug>
#include <QImage>

#include <thread>
#include <Windows.h>

MCVideoPlayer::MCVideoPlayer(QObject* parent)
    : QObject(parent)
{
    qRegisterMetaType<std::shared_ptr<MCVideoFrame>>("std::shared_ptr<MCVideoFrame>");

    m_pTimer = new QTimer(this);
    connect(m_pTimer, &QTimer::timeout, this, &MCVideoPlayer::timerTimeOut);
    m_pTimer->setInterval(500);
}

MCVideoPlayer::~MCVideoPlayer()
{
}

void MCVideoPlayer::setVideoFilePath(const QString& filePath)
{
    m_videoFilePath = filePath;
}

void MCVideoPlayer::playVideo()
{
    // 播放视频，先停止视频
    if (VideoState::StoppedState != m_state)
    {
        return;
    }

    // 更新播放状态
    m_isStopped = false;
    m_isPause = false;

    // 创建新的线程，读取视频数据
    std::thread(&MCVideoPlayer::readVideoStart, this).detach();
}

void MCVideoPlayer::seek(qint64 seekTime)
{
    if (!m_seekRequestFlag)
    {
        m_seekRequestFlag = true;
        m_seekFrameFlag = true;
        m_seekTime = seekTime;
    }
}

void MCVideoPlayer::startTimer()
{
    m_pTimer->start();
}

void MCVideoPlayer::stopTimer()
{
    m_pTimer->stop();
}

bool MCVideoPlayer::readVideoStart()
{
    if (m_videoFilePath.isEmpty())
    {
        return false;
    }

    // 重置读取完成状态，重置读取线程完成状态
    m_isReadFinished = false;
    m_isReadThreadFinished = false;

    // 分配视频格式 IO 上下文
    m_pFormatContext = avformat_alloc_context();
    if (nullptr == m_pFormatContext)
    {
        qCritical() << "The video palyer error: read video failed to avformat_alloc_context";
        readVideoExit();
        return false;
    }

    const char* videoFilePath = m_videoFilePath.toStdString().c_str();
    qInfo() << "The video palyer: video file path" << videoFilePath;

    // 打开视频文件
    int result = avformat_open_input(&m_pFormatContext, videoFilePath, nullptr, nullptr);
    if (result < 0)
    {
        qCritical() << "The video palyer error: read video failed to avformat_open_input" << result;
        readVideoExit();
        return false;
    }

    // 读取视频流信息
    result = avformat_find_stream_info(m_pFormatContext, nullptr);
    if (result < 0)
    {
        qCritical() << "The video palyer error: read video failed to avformat_find_stream_info" << result;
        readVideoExit();
        return false;
    }

    // 查找视频流索引
    int videoIndex = -1;
    for (unsigned int index = 0; index < m_pFormatContext->nb_streams; ++index)
    {
        if (AVMEDIA_TYPE_VIDEO == m_pFormatContext->streams[index]->codecpar->codec_type)
        {
            videoIndex = index;
            break;
        }
    }
    if (videoIndex < 0)
    {
        qCritical() << "The video palyer error: read video failed to find video index";
        readVideoExit();
        return false;
    }

    // 计算视频总时长
    durationChanged(m_pFormatContext->duration);
    qInfo() << "The video palyer: video duration" << m_pFormatContext->duration;

    // 通过视频流索引读取视频流
    m_pVideoStream = m_pFormatContext->streams[videoIndex];

    // 视频帧率
    double frameRate = 0.0;
    if (0 != m_pVideoStream->avg_frame_rate.den)
    {
        frameRate = m_pVideoStream->avg_frame_rate.num * 1.0 / m_pVideoStream->avg_frame_rate.den;
    }
    qInfo() << "The video palyer: video frame rate" << frameRate;

    // 视频总帧数
    qint64 totalFrames = m_pVideoStream->nb_frames;
    qInfo() << "The video palyer: video frame number" << totalFrames;

    // 查找视频解码器
    const AVCodec* pCodec = avcodec_find_decoder(m_pVideoStream->codecpar->codec_id);
    if (nullptr == pCodec)
    {
        qCritical() << "The video palyer error: read video failed to avcodec_find_decoder";
        readVideoExit();
        return false;
    }

    // 创建解码器上下文，并设置默认值
    m_pCodecContext = avcodec_alloc_context3(pCodec);
    if (nullptr == m_pCodecContext)
    {
        qCritical() << "The video palyer error: read video failed to avcodec_alloc_context3";
        readVideoExit();
        return false;
    }

    // 使用视频流的 codecpar 为解码器上下文赋值
    result = avcodec_parameters_to_context(m_pCodecContext, m_pVideoStream->codecpar);
    if (result < 0)
    {
        qCritical() << "The video palyer error: read video failed to avcodec_parameters_to_context" << result;
        readVideoExit();
        return false;
    }

    // 设置解码线程数为 8
    m_pCodecContext->thread_count = 8;

    // 打开解码器
    result = avcodec_open2(m_pCodecContext, nullptr, nullptr);
    if (result < 0)
    {
        qCritical() << "The video palyer error: read video failed to avcodec_open2" << result;
        readVideoExit();
        return false;
    }

    // 解码器打开后，创建新的线程，解码视频数据包
    std::thread(&MCVideoPlayer::decodeVideoStart, this).detach();

    // 切换视频状态为正在播放状态
    stateChanged(VideoState::PlayingState);

    // 记录视频开始时间
    m_videoStartTime = av_gettime();

    // 读取视频数据循环
    while (1)
    {
        if (m_isStopped)
        {
            // 视频停止，退出读取
            break;
        }

        if (m_isPause)
        {
            // 视频暂停，等待恢复
            Sleep(50);
            continue;
        }

        if (m_isReadFinished)
        {
            if (m_isStopped)
            {
                // 读取完成，并且停止，可以退出读取循环
                break;
            }

            // 读取完成，并未停止，等待解码线程停止
            Sleep(50);
            continue;
        } 

        if (m_seekRequestFlag)
        {
            AVRational rational = {1, AV_TIME_BASE};
            qint64 seekTarget = av_rescale_q(m_seekTime, rational, m_pVideoStream->time_base);

            qInfo() << "---------- av_seek_frame" << m_seekTime << seekTarget;
            if (av_seek_frame(m_pFormatContext, videoIndex, m_seekTime, AVSEEK_FLAG_BACKWARD) < 0)
            {
                qCritical() << "The video palyer error: read video failed to av_seek_frame";  
            }
            else
            {
                // 跳转视频，在视频数据队列里添加一个新的 AVPacket，并把 data 赋值为 "FLUSH_DATA" 以做标记
                AVPacket packet;
                av_new_packet(&packet, 10);
                strcpy((char*)packet.data, m_flushData);
                clearPacketList();
                addPacket(packet);

                m_videoStartTime = av_gettime() - m_seekTime;
            }
            m_seekRequestFlag = false;

            if (m_isPause)
            {
                // todo 暂停状态下的 seek 处理
            }
        }

        // 视频数据包队列数据包个数超过一定值，就暂停读取数据，等待数据解码，以防内存不足问题
        if (m_maxVideoSize < m_listVideoPackets.size())
        {
            Sleep(10);
            continue;
        }

        AVPacket packet;
        if (av_read_frame(m_pFormatContext, &packet) < 0)
        {
            av_packet_unref(&packet);

            // 读取完成
            m_isReadFinished = true;
            qInfo() << "The video palyer: video read finished";

            if (m_isStopped)
            {
                // 读取完成，并且停止，可以退出读取循环
                break;
            }

            // 读取完成，并未停止，等待解码线程停止
            Sleep(50);
            continue; 
        }

        // 读取到的视频数据包添加到视频数据队列
        if (videoIndex == packet.stream_index)
        {
            addPacket(packet);
        }
        else
        {
            av_packet_unref(&packet);
        }
    }

    // 等待视频停止
    while (!m_isStopped)
    {
        Sleep(50);
    }

    readVideoExit();

    return true;
}

void MCVideoPlayer::readVideoExit()
{
    // 清除数据
    clearPacketList();

    // 停止播放
    if (VideoState::StoppedState != m_state)
    {

    }

    // 确保解码线程结束，再停止线程
    while (false)
    {
        Sleep(50);
    }

    // 指针析构
    freeReadData();

    // 切换视频状态为正在播放状态
    stateChanged(VideoState::StoppedState);

    // 读取线程结束
    m_isReadThreadFinished = true;
}

void MCVideoPlayer::freeReadData()
{
    if (m_pCodecContext != nullptr)
    {
        avcodec_close(m_pCodecContext);
        m_pCodecContext = nullptr;
    }

    avformat_close_input(&m_pFormatContext);

    // 析构 m_pFormatContext 的时候会同时析构 m_pVideoStream
    avformat_free_context(m_pFormatContext);

    m_pVideoStream = nullptr;
}

void MCVideoPlayer::decodeVideoStart()
{
    // 重置解码线程完成状态
    m_isDecodeThreadFinished = false;

    // 视频宽度、高度
    int videoWidth = 0;
    int videoHeight = 0;

    // 当前视频帧编号
    qint64 currentFrameNumber = 0;
    // 当前视频帧时间
    double currentFrameTime = 0;

    // 视频帧率
    AVFrame* pFrame = nullptr;
    AVFrame* pFrameYUV = nullptr;

    // 解码后的 YUV 数据 
    uint8_t* pYUVBuffer = nullptr;
    // 视频格式转换上下文
    SwsContext* pSwsContext = nullptr;

    // 分配 AVFrame，字段设置为默认值
    pFrame = av_frame_alloc();
    if (nullptr == pFrame)
    {
        qCritical() << "The video palyer error: decode video failed to av_frame_alloc";
        return;
    }

    // 解码视频数据循环
    while (1)
    {
        if (m_isStopped)
        {
            // 视频停止，退出解码
            break;
        }

        if (m_isPause)
        {
            // 视频暂停，等待解码
            Sleep(50);
            continue;
        }

        m_mutex.lock();

        if (m_listVideoPackets.isEmpty())
        {
            m_mutex.unlock();

            if (m_isReadFinished)
            {
                // 视频数据队列为空，并且读取完成，视频解码完成，退出循环
                qInfo() << "The video palyer: video decode finished";
                break;
            }
            else
            {
                // 视频数据队列为空，但未读取完成，等待读取视频数据
                qInfo() << "The video palyer: video decode finished";
                Sleep(10);
                continue;
            }
        }

        AVPacket pkt = m_listVideoPackets.first();
        m_listVideoPackets.removeFirst();

        m_mutex.unlock();

        AVPacket* pPacket = &pkt;

        // 遇到视频跳转的标志视频数据包，刷新数据缓冲，继续解码下一个视频数据包
        if(0 == strcmp((char*)pPacket->data, m_flushData))
        {
            qInfo() << "The video palyer: decode video flush buffer";
            avcodec_flush_buffers(m_pCodecContext);
            av_packet_unref(pPacket);
            continue;
        }

        // 将数据包传送到解码器
        int result = avcodec_send_packet(m_pCodecContext, pPacket);
        if (result < 0)
        {
            qCritical() << "The video palyer error: decode video failed to avcodec_send_packet" << result;
            av_packet_unref(pPacket);
            continue;
        }

        // 视频数据包接收循环
        while (1)
        {         
            qInfo() << "avcodec_receive_frame failed";
            if (avcodec_receive_frame(m_pCodecContext, pFrame) < 0)
            {
                // 当前视频数据包中的视频帧都被接收完成，继续接收下一个数据包
                av_packet_unref(pPacket);

                break;
            }

            qInfo() << "avcodec_receive_frame finished";

            // 计算当前帧对应的时间
            if (AV_NOPTS_VALUE == pPacket->dts && pFrame->opaque && AV_NOPTS_VALUE != *((uint64_t*)pFrame->opaque))
            {
                currentFrameNumber = *((uint64_t*)pFrame->opaque);
            }
            else if (AV_NOPTS_VALUE != pPacket->dts)
            {
                currentFrameNumber = pPacket->dts;
            }
            else
            {
                currentFrameNumber = 0;
            }

            if (0 != m_pVideoStream->time_base.den)
            {
                /**
                 * av_q2d 的计算算法是除法，判断分母不为 0
                 * av_q2d(m_pVideoStream->time_base) 的计算结果为一帧数据的时间，单位为秒，假如帧率为 20，则计算结果为 0.05
                 * 通过 currentFrameNumber 与一帧数据时间相乘，得到当前帧的对应时间
                 */
                currentFrameTime = currentFrameNumber * av_q2d(m_pVideoStream->time_base);
            }

            if (m_seekFrameFlag)
            {
                if (currentFrameTime * AV_TIME_BASE < m_seekTime)
                {
                    // 跳转未完成，并且当前视频帧时间小于跳转时间，跳过当前视频帧
                    av_packet_unref(pPacket);
                    qInfo() << "---------- seek frame wait";
                    continue;
                }
                else
                {
                    qInfo() << "---------- seek frame finished";

                    // 跳转完成，清除跳转标志
                    m_seekFrameFlag = false;
                }
            }

            m_mutex.lock();
            // 等待循环，等待当前帧对应的时间到来
            while (1)
            {
                double currentTime = (av_gettime() - m_videoStartTime) * 1.0 / AV_TIME_BASE;
                // 计算当前时间，更新进度条
                m_currentTime = currentTime * AV_TIME_BASE;
                if (currentFrameTime <= currentTime)
                {
                    qInfo() << "---------- wait time break";
                    break;
                }

                qInfo() << "---------- currentTime" << currentTime;
                qInfo() << "---------- currentFrameTime" << currentFrameTime;
                qInfo() << "---------- currentFrameNumber" << currentFrameNumber;
                qInfo() << "---------- wait time continue";
                Sleep(5);
                continue;
            }
            m_mutex.unlock();

            if (pFrame->width != videoWidth || pFrame->height != videoHeight)
            {
                videoWidth = pFrame->width;
                videoHeight = pFrame->height;

                if (pFrameYUV != nullptr)
                {
                    av_free(pFrameYUV);
                }

                if (pYUVBuffer != nullptr)
                {
                    av_free(pYUVBuffer);
                }

                if (pSwsContext != nullptr)
                {
                    sws_freeContext(pSwsContext);
                }

                pFrameYUV = av_frame_alloc();

                // 按 1 字节进行内存对齐，得到的内存大小最接近实际大小
                int yuvSize = av_image_get_buffer_size(AV_PIX_FMT_YUV420P, videoWidth, videoHeight, 1);

                unsigned int byteCount = static_cast<unsigned int>(yuvSize);
                pYUVBuffer = static_cast<uint8_t *>(av_malloc(byteCount * sizeof(uint8_t)));
                av_image_fill_arrays(pFrameYUV->data, pFrameYUV->linesize, pYUVBuffer, AV_PIX_FMT_YUV420P, videoWidth, videoHeight, 1);

                // 将解码后的数据转换成 AV_PIX_FMT_YUV420P
                pSwsContext = sws_getContext(videoWidth, videoHeight, (AVPixelFormat)pFrame->format,
                                             videoWidth, videoHeight, AV_PIX_FMT_YUV420P,
                                             SWS_BICUBIC, NULL, NULL, NULL);
            }

            sws_scale(pSwsContext, pFrame->data, pFrame->linesize, 0, videoHeight, pFrameYUV->data, pFrameYUV->linesize);

            std::shared_ptr<MCVideoFrame> pVideoFrame = std::make_shared<MCVideoFrame>();
            pVideoFrame.get()->setYUVData(pYUVBuffer, videoWidth, videoHeight);
            emit sigFrameChanged(pVideoFrame);
            
            av_packet_unref(pPacket);
        }
    }
    av_free(pFrame);

    if (pFrameYUV != nullptr)
    {
        av_free(pFrameYUV);
    }

    if (pYUVBuffer != nullptr)
    {
        av_free(pYUVBuffer);
    }

    if (pSwsContext != nullptr)
    {
        sws_freeContext(pSwsContext);
    }

    if (!m_isStopped)
    {
        m_isStopped = true;
    }

    m_isDecodeThreadFinished = true;
}

void MCVideoPlayer::decodeVideoExit()
{
}

bool MCVideoPlayer::addPacket(const AVPacket& pkt)
{
    m_mutex.lock();

    // 复制一份数据到 packet
    AVPacket packet;
    if (av_packet_ref(&packet, &pkt) < 0)
    {
        return false;
    }
    m_listVideoPackets << packet;

    m_mutex.unlock();

    return true;
}

void MCVideoPlayer::clearPacketList()
{
    m_mutex.lock();

    for (AVPacket pkt : m_listVideoPackets)
    {
        av_packet_unref(&pkt);
    }
    m_listVideoPackets.clear();

    m_mutex.unlock();
}

void MCVideoPlayer::durationChanged(qint64 duration)
{
    // todo：实现视频总时间变化
}

void MCVideoPlayer::stateChanged(VideoState state)
{
    m_state = state;
    emit sigStateChanged(state);
}

void MCVideoPlayer::timerTimeOut()
{
    emit sigTimeChanged(m_currentTime);
}
