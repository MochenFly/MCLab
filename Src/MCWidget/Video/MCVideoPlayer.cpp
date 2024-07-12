#include "MCVideoPlayer.h"
USE_MCWIDGET_NAMESPACE

#include <QDebug>
#include <QByteArray>

#include <thread>

#include <Windows.h>

const qint64    MCVideoPlayer::s_msecondTimeBase        = AV_TIME_BASE / 1000;
const char*     MCVideoPlayer::s_flushFlagChar          = "FLUSH_FLAG";
const int       MCVideoPlayer::s_videoReadLimitNumver   = 500;

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
    stopVideo();
}

void MCVideoPlayer::setVideoFilePath(const QString& filePath)
{
    m_videoFilePath = filePath;

    if (m_isOpenThreadFinished && m_isReadThreadFinished)
    {
        std::thread(&MCVideoPlayer::openVideo, this).detach();
    }
}

void MCVideoPlayer::playVideo()
{
    if (VideoState::StoppedState != m_state)
    {
        return;
    }

    // 等待打开视频
    int timeOut = 0;
    while (1)
    {
        if (m_isOpenFinished)
        {
            break;
        }
        else
        {
            if (1000 <= timeOut)
            {
                return;
            }
            timeOut += 1;
            Sleep(1);
            continue;
        }
    }

    // 更新播放状态
    m_isStopped = false;
    std::thread(&MCVideoPlayer::readVideo, this).detach();
}

void MCVideoPlayer::stopVideo()
{
    m_isStopped = true;

    if (!m_isReadThreadFinished)
    {
        Sleep(10);
    }
}

void MCVideoPlayer::seekVideio(qint64 seekTime)
{
    if (!m_seekRequestFlag)
    {
        m_seekRequestFlag = true;
        m_seekTime = seekTime;
    }

    if (VideoState::StoppedState == m_state)
    {
        m_decodeOneFrameRequsetFlag = true;
        playVideo();
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

void MCVideoPlayer::setLogEnabled(bool enabled)
{
    m_isLogEnabled = enabled;
}

void MCVideoPlayer::openVideo()
{
    m_isOpenFinished = false;
    m_isOpenThreadFinished = false;

    if (m_videoFilePath.isEmpty())
    {
        printLog("the video file path is empty", true);
        m_isOpenThreadFinished = true;
        return;
    }

    // 分配视频格式 IO 上下文
    m_pFormatContext = avformat_alloc_context();
    if (nullptr == m_pFormatContext)
    {
        printLog("open video failed to avformat_alloc_context", true);
        closeVideo();
        m_isOpenThreadFinished = true;
        return;
    }

    QByteArray videoFilePathUtf8 = m_videoFilePath.toUtf8();
    const char* videoFilePath = videoFilePathUtf8.constData();
    printLog(QString("video file path %1").arg(m_videoFilePath));

    // 打开视频文件
    int result = avformat_open_input(&m_pFormatContext, videoFilePath, nullptr, nullptr);
    if (result < 0)
    {
        printLog(QString("open video failed to avformat_open_input, error code %1").arg(result), true);
        closeVideo();
        m_isOpenThreadFinished = true;
        return;
    }

    // 读取视频流信息
    result = avformat_find_stream_info(m_pFormatContext, nullptr);
    if (result < 0)
    {
        printLog(QString("open video failed to avformat_find_stream_info, error code %1").arg(result), true);
        closeVideo();
        m_isOpenThreadFinished = true;
        return;
    }

    // 查找视频流索引
    m_videoIndex = -1;
    for (unsigned int index = 0; index < m_pFormatContext->nb_streams; ++index)
    {
        if (AVMEDIA_TYPE_VIDEO == m_pFormatContext->streams[index]->codecpar->codec_type)
        {
            m_videoIndex = index;
            break;
        }
    }
    if (m_videoIndex < 0)
    {
        printLog(QString("open video failed to find video index"), true);
        closeVideo();
        m_isOpenThreadFinished = true;
        return;
    }

    // 计算视频总时长
    emit sigDurationChanged(m_pFormatContext->duration / s_msecondTimeBase);
    printLog(QString("video duration %1").arg(m_pFormatContext->duration / s_msecondTimeBase));

    // 通过视频流索引读取视频流
    m_pVideoStream = m_pFormatContext->streams[m_videoIndex];

    // 视频帧率
    double frameRate = 0.0;
    if (0 != m_pVideoStream->avg_frame_rate.den)
    {
        frameRate = m_pVideoStream->avg_frame_rate.num * 1.0 / m_pVideoStream->avg_frame_rate.den;
    }
    printLog(QString("video frame rate %1").arg(frameRate));

    // 视频总帧数
    qint64 totalFrames = m_pVideoStream->nb_frames;
    printLog(QString("video frame number %1").arg(totalFrames));

    // 查找视频解码器
    const AVCodec* pCodec = avcodec_find_decoder(m_pVideoStream->codecpar->codec_id);
    if (nullptr == pCodec)
    {
        printLog(QString("open video failed to avcodec_find_decoder, error code %1").arg(result), true);
        closeVideo();
        m_isOpenThreadFinished = true;
        return;
    }

    // 创建解码器上下文，并设置默认值
    m_pCodecContext = avcodec_alloc_context3(pCodec);
    if (nullptr == m_pCodecContext)
    {
        printLog(QString("open video failed to avcodec_alloc_context3, error code %1").arg(result), true);
        closeVideo();
        m_isOpenThreadFinished = true;
        return;
    }
    
    // 使用视频流的 codecpar 为解码器上下文赋值
    result = avcodec_parameters_to_context(m_pCodecContext, m_pVideoStream->codecpar);
    if (result < 0)
    {
        printLog(QString("open video failed to avcodec_parameters_to_context, error code %1").arg(result), true);
        closeVideo();
        m_isOpenThreadFinished = true;
        return;
    }

    // 打开解码器
    result = avcodec_open2(m_pCodecContext, nullptr, nullptr);
    if (result < 0)
    {
        printLog(QString("open video failed to avcodec_open2, error code %1").arg(result), true);
        closeVideo();
        m_isOpenThreadFinished = true;
        return;
    }

    m_isOpenFinished = true;
    m_isOpenThreadFinished = true;
}

void MCVideoPlayer::closeVideo()
{
    if (nullptr != m_pCodecContext)
    {
        avcodec_close(m_pCodecContext);
        m_pCodecContext = nullptr;
    }

    avformat_close_input(&m_pFormatContext);

    // 析构 m_pFormatContext 的时候会同时析构 m_pVideoStream
    avformat_free_context(m_pFormatContext);

    m_pVideoStream = nullptr;
}

bool MCVideoPlayer::readVideo()
{
    // 状态重置
    m_isReadFinished = false;
    m_isReadThreadFinished = false;
    m_readFrameCount = 0;

    // 解码器打开后，创建新的线程，解码视频数据包
    std::thread(&MCVideoPlayer::decodeVideo, this).detach();

    if (!m_decodeOneFrameRequsetFlag)
    {
        // 切换视频状态为正在播放状态
        m_state = VideoState::PlayingState;
        emit sigStateChanged(m_state);
    }

    // 记录视频开始时间
    m_videoStartTime = av_gettime() / s_msecondTimeBase;
    m_seekVideoStartTime = m_videoStartTime;

    // 读取视频数据循环
    while (1)
    {
        if (m_isStopped)
        {
            // 视频停止，退出读取
            break;
        }

        if (m_seekRequestFlag)
        {
            // 根据时间，和一帧所用的时间，计算结果 seekTarget 即跳转目标帧的编号
            AVRational rational = {1, s_msecondTimeBase};
            qint64 seekTarget = av_rescale_q(m_seekTime, rational, m_pVideoStream->time_base);
            if (av_seek_frame(m_pFormatContext, m_videoIndex, seekTarget, AVSEEK_FLAG_BACKWARD) < 0)
            {
                printLog("read video failed to av_seek_frame", true);
            }
            else
            {
                printLog(QString("read video seek %1").arg(m_seekTime));

                // 跳转视频，在视频数据列表里添加一个新的 AVPacket，并把 data 赋值为 "FLUSH_FLAG" 以做标记
                AVPacket packet;
                av_new_packet(&packet, 10);
                strcpy((char*)packet.data, s_flushFlagChar);
                clearPacketList();
                addPacket(packet);
                m_seekVideoStartTime = av_gettime() / s_msecondTimeBase - m_seekTime;

                if (m_decodeOneFrameRequsetFlag)
                {
                    m_currentTime = m_seekVideoStartTime;
                }

                m_readFrameCount = 0;
            }
            m_seekRequestFlag = false;
            m_seekFrameFlag = true;

            // 跳转之后，更新读取视频状态为未完成状态
            m_isReadFinished = false;
        }

        // 视频数据包列表数据包个数超过一定值，就暂停读取数据，等待数据解码，以防内存不足问题
        if (s_videoReadLimitNumver < m_listVideoPackets.size())
        {
            Sleep(5);
            continue;
        }

        AVPacket packet;
        if (av_read_frame(m_pFormatContext, &packet) < 0)
        {
            if (!m_isReadFinished)
            {
                // 读取完成
                m_isReadFinished = true;
                printLog(QString("video read finished %1").arg(m_readFrameCount));
            }

            if (m_isStopped)
            {
                // 读取完成，并且停止，可以退出读取循环
                av_packet_unref(&packet);
                break;
            }

            // 前几帧数据 avcodec_receive_frame 报错，返回 EAGAIN 值，是因为数据太少，解码器需要足够的数据合成帧
            // 所以在读取完成但是未解码完成后，发送空包到解码器，否则最后几帧解析不到
            addPacket(packet);
        }
        else
        {
            // 读取到的视频数据包添加到视频数据列表
            if (m_videoIndex == packet.stream_index)
            {
                addPacket(packet);
                ++m_readFrameCount;
            }
            else
            {
                av_packet_unref(&packet);
            }
        }
    }

    // 清除数据
    clearPacketList();

    // 确保解码线程结束，再停止线程
    while (!m_isDecodeThreadFinished)
    {
        Sleep(50);
    }

    // 停止播放
    if (VideoState::StoppedState != m_state)
    {
        // 切换视频状态为正在播放状态
        m_state = VideoState::StoppedState;
        emit sigStateChanged(m_state);
    }

    // 读取线程结束
    m_isReadFinished = true;
    m_isReadThreadFinished = true;
    m_readFrameCount = 0;

    return true;
}

void MCVideoPlayer::decodeVideo()
{
    // 重置解码线程完成状态
    m_isDecodeFinished = false;
    m_isDecodeThreadFinished = false;

    // 视频宽度、高度
    int videoWidth = 0;
    int videoHeight = 0;

    // 当前视频帧编号
    qint64 currentFrameIndex = 0;
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

    // 解码视频数据循环
    while (1)
    {
        if (m_decodeOneFrameFlag)
        {
            m_decodeOneFrameFlag = false;
            break;
        }

        if (m_isStopped)
        {
            // 视频停止，退出解码
            break;
        }

        // 读取数据包
        m_mutex.lock();
        if (m_listVideoPackets.isEmpty())
        {
            m_mutex.unlock();

            // 视频数据列表为空，但未读取完成，等待读取视频数据
            Sleep(5);
            continue;
        }
        AVPacket pkt = m_listVideoPackets.first();
        m_listVideoPackets.removeFirst();
        m_mutex.unlock();

        AVPacket* pPacket = &pkt;

        // 遇到视频跳转的标志视频数据包，刷新数据缓冲，继续解码下一个视频数据包
        if(0 == strcmp((char*)pPacket->data, s_flushFlagChar))
        {
            printLog("decode video flush buffer");
            avcodec_flush_buffers(m_pCodecContext);
            av_packet_unref(pPacket);
            continue;
        }

        qDebug() << "avcodec_send_packet ready";
        // 将数据包传送到解码器
        if (avcodec_send_packet(m_pCodecContext, pPacket))
        {
            qDebug() << "avcodec_send_packet failed";
            if (m_isReadFinished)
            {
                // 视频数据列表为空，并且读取完成，视频解码完成，退出循环
                printLog("video decode finished");
                break;
            }

            av_packet_unref(pPacket);
            continue;
        }
        qDebug() << "avcodec_send_packet ok";

        if (avcodec_receive_frame(m_pCodecContext, pFrame) < 0)
        {
            // 视频数据包接收
            av_packet_unref(pPacket);
            continue;
        }
        qDebug() << "avcodec_receive_frame\tok";

        // 计算当前帧对应的时间
        if (AV_NOPTS_VALUE == pPacket->dts && pFrame->opaque && AV_NOPTS_VALUE != *((uint64_t*)pFrame->opaque))
        {
            currentFrameIndex = *((uint64_t*)pFrame->opaque);
        }
        else if (AV_NOPTS_VALUE != pPacket->dts)
        {
            currentFrameIndex = pPacket->dts;
        }
        else
        {
            currentFrameIndex = 0;
        }

        if (0 != m_pVideoStream->time_base.den)
        {
            /**
             * av_q2d 的计算算法是除法，判断分母不为 0
             * av_q2d(m_pVideoStream->time_base) 的计算结果为一帧数据的时间，单位为秒，假如帧率为 20，则计算结果为 0.05
             * 通过 currentFrameIndex 与一帧数据时间相乘，得到当前帧的对应时间，单位毫秒
             */
            currentFrameTime = currentFrameIndex * av_q2d(m_pVideoStream->time_base) * s_msecondTimeBase;
        }

        if (m_seekFrameFlag)
        {
            // 跳转触发时，在延时循环内，继续解码下一帧
            // 如果 m_seekVideoStartTime != m_videoStartTime，则表示当前还在读取跳转前的数据帧
            // 舍弃跳转前的帧，直接 continue
            if (m_seekVideoStartTime != m_videoStartTime)
            {
                m_videoStartTime = m_seekVideoStartTime;
                av_packet_unref(pPacket);
                continue;
            }

            // 跳转触发，刷新解码器缓冲区后解码的数据帧
            // 没到跳转时刻，就 continue，等到执行到跳转时刻
            if (currentFrameTime < m_seekTime)
            {
                // 跳转未完成，未到跳转时刻
                av_packet_unref(pPacket);
                continue;
            }
            else
            {
                // 跳转完成，清除跳转标志
                m_seekFrameFlag = false;
            }
        }

        // 等待循环，等待当前帧对应的时间到来
        while (1)
        {
            // 跳转触发时，在延时循环内，退出
            if (m_seekFrameFlag)
            {
                break;
            }

            // 根据开始时间，计算当前时间
            m_currentTime = av_gettime() / s_msecondTimeBase - m_videoStartTime;
            if (currentFrameTime <= m_currentTime)
            {
                // 跳转一帧，捕获到了这一帧，更新状态
                if (m_decodeOneFrameRequsetFlag)
                {
                    m_decodeOneFrameRequsetFlag = true;
                    m_decodeOneFrameFlag = true;
                }

                break;
            }

            // 当前时间未到当前视频帧对应的时间，等待
            Sleep(1);
            continue;
        }

        // 跳转触发时，在延时循环外，继续解码下一帧
        if (m_seekFrameFlag)
        {
            av_packet_unref(pPacket);
            continue;
        }

        if (pFrame->width != videoWidth || pFrame->height != videoHeight)
        {
            videoWidth = pFrame->width;
            videoHeight = pFrame->height;

            if (nullptr != pFrameYUV)
            {
                av_free(pFrameYUV);
            }

            if (nullptr != pYUVBuffer)
            {
                av_free(pYUVBuffer);
            }

            if (nullptr != pSwsContext)
            {
                sws_freeContext(pSwsContext);
            }

            pFrameYUV = av_frame_alloc();

            // 按 1 字节进行内存对齐，得到的内存大小最接近实际大小
            int yuvSize = av_image_get_buffer_size(AV_PIX_FMT_YUV420P, videoWidth, videoHeight, 1);
            unsigned int byteCount = static_cast<unsigned int>(yuvSize);
            pYUVBuffer = static_cast<uint8_t*>(av_malloc(byteCount * sizeof(uint8_t)));
            av_image_fill_arrays(pFrameYUV->data, pFrameYUV->linesize, pYUVBuffer, AV_PIX_FMT_YUV420P, videoWidth, videoHeight, 1);

            // 将解码后的数据转换成 AV_PIX_FMT_YUV420P
            pSwsContext = sws_getContext(videoWidth, videoHeight, (AVPixelFormat)pFrame->format,
                                         videoWidth, videoHeight, AV_PIX_FMT_YUV420P,
                                         SWS_BICUBIC, NULL, NULL, NULL);
        }

        sws_scale(pSwsContext, pFrame->data, pFrame->linesize, 0, videoHeight, pFrameYUV->data, pFrameYUV->linesize);

        std::shared_ptr<MCVideoFrame> pVideoFrame = std::make_shared<MCVideoFrame>();
        pVideoFrame.get()->setYUVData(pYUVBuffer, videoWidth, videoHeight);

        // 跳转触发时，在渲染信号前，继续解码下一帧
        if (m_seekFrameFlag)
        {
            av_packet_unref(pPacket);
            continue;
        }
        
        emit sigFrameChanged(pVideoFrame);

        av_packet_unref(pPacket);
    }
    av_free(pFrame);

    if (nullptr != pFrameYUV)
    {
        av_free(pFrameYUV);
    }

    if (nullptr != pYUVBuffer)
    {
        av_free(pYUVBuffer);
    }

    if (nullptr != pSwsContext)
    {
        sws_freeContext(pSwsContext);
    }

    if (!m_isStopped)
    {
        m_isStopped = true;
    }

    // 解码线程结束
    m_isDecodeFinished = true;
    m_isDecodeThreadFinished = true;
}

void MCVideoPlayer::addPacket(const AVPacket& pkt)
{
    // 复制一份数据到 packet
    m_mutex.lock();
    AVPacket packet;
    if (av_packet_ref(&packet, &pkt) < 0)
    {
        return;
    }
    m_listVideoPackets << packet;
    m_mutex.unlock();
    return;
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

void MCVideoPlayer::printLog(const QString& log, bool isError)
{
    if (m_isLogEnabled)
    {
        if (isError)
        {
            qCritical() << QString("The video palyer error: %1").arg(log);
        }
        else
        {
            qInfo() << QString("The video palyer: %1").arg(log);
        }
    }
}

void MCVideoPlayer::timerTimeOut()
{
    // 执行跳转，但是跳转还没完成时，暂停更新进度条，等待跳转完成
    // 否则会出现进度条两个数值之间，很快一闪而过的现象
    if (!m_seekFrameFlag)
    {
        emit sigTimeChanged(m_currentTime);
    }
}
