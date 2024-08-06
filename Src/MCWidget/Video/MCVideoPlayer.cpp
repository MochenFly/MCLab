#include "MCVideoPlayer.h"
USE_MCWIDGET_NAMESPACE

#include <QDebug>
#include <QByteArray>

#include <thread>

#include <Windows.h>

MCVideoPlayer::MCVideoPlayer(QObject* parent)
    : QObject(parent)
{
    qRegisterMetaType<std::shared_ptr<MCVideoFrame>>("std::shared_ptr<MCVideoFrame>");
    qRegisterMetaType<MCWidget::MCVideoPlayer::VideoState>("VideoState");

    setLogEnabled(true);
}

MCVideoPlayer::~MCVideoPlayer()
{
    stopVideo();
}

void MCVideoPlayer::setVideoFilePath(const QString& filePath)
{
    m_videoFilePath = filePath;
}

void MCVideoPlayer::playVideo()
{
    if (m_isReadThreadFinished)
    {
        std::thread(&MCVideoPlayer::readVideo, this).detach();
    }
    else
    {
        QString state = QString("video play failed, state %1, read thread state %2").arg(m_state).arg(m_isReadThreadFinished);
        printLog(state, true);
    }
}

void MCVideoPlayer::playOneFrame(int frameTime)
{
    // 视频停止状态下跳转视频，只解码一帧数据
    // 确保读取线程停止，否则会因为冲突导致崩溃
    if (m_isReadThreadFinished && !m_seekRequestFlag)
    {
        m_seekRequestFlag = true;
        m_seekTime = frameTime;

        m_decodeOneFrameRequsetFlag = true;
        playVideo();
    }
}

void MCVideoPlayer::stopVideo()
{
    m_isStopped = true;

    while (!m_isReadThreadFinished)
    {
        Sleep(1);
    }
}

void MCVideoPlayer::seekVideo(int seekTime)
{
    if (!m_seekRequestFlag)
    {
        m_seekRequestFlag = true;
        m_seekTime = seekTime;
    }
}

MCVideoPlayer::VideoState MCVideoPlayer::getState()
{
    return m_state;
}

int MCVideoPlayer::getCurrentTime()
{
    // 跳转阶段返回跳转时间
    if (m_seekRequestFlag || m_seekFrameFlag)
    {
        return m_seekTime;
    }
    return m_currentTime;
}

void MCVideoPlayer::setLogEnabled(bool enabled)
{
    m_isLogEnabled = enabled;
}

bool MCVideoPlayer::openVideo()
{
    if (m_videoFilePath.isEmpty())
    {
        printLog("the video file path is empty", true);
        return false;
    }

    // 分配视频格式 IO 上下文
    m_pFormatContext = avformat_alloc_context();
    if (nullptr == m_pFormatContext)
    {
        printLog("open video failed to avformat_alloc_context", true);
        closeVideo();
        return false;
    }

    // 中文路径先转 utf-8 再转成 const char*，否则有一定概率 avformat_open_input 执行失败
    QByteArray videoFilePathUtf8 = m_videoFilePath.toUtf8();
    const char* videoFilePath = videoFilePathUtf8.constData();
    printLog(QString("video file path %1").arg(m_videoFilePath));

    // 打开视频文件
    int result = avformat_open_input(&m_pFormatContext, videoFilePath, nullptr, nullptr);
    if (result < 0)
    {
        printLog(QString("open video failed to avformat_open_input, error code %1").arg(result), true);
        closeVideo();
        return false;
    }

    // 读取视频流信息
    result = avformat_find_stream_info(m_pFormatContext, nullptr);
    if (result < 0)
    {
        printLog(QString("open video failed to avformat_find_stream_info, error code %1").arg(result), true);
        closeVideo();
        return false;
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
        return false;
    }

    // 计算视频总时长
    emit sigDurationChanged(m_pFormatContext->duration / m_msecondTimeBase);
    printLog(QString("video duration %1").arg(m_pFormatContext->duration / m_msecondTimeBase));

    // 通过视频流索引读取视频流
    m_pVideoStream = m_pFormatContext->streams[m_videoIndex];

    // 视频帧率
    m_videoFrameRate = 0.0;
    if (0 != m_pVideoStream->avg_frame_rate.den)
    {
        m_videoFrameRate = m_pVideoStream->avg_frame_rate.num * 1.0 / m_pVideoStream->avg_frame_rate.den;
    }
    printLog(QString("video frame rate %1").arg(m_videoFrameRate));

    m_oneFrameTime = 0;
    if (0 < m_videoFrameRate)
    {
        m_oneFrameTime = m_msecondTimeBase / m_videoFrameRate;
    }

    // 视频总帧数
    int totalFrames = m_pVideoStream->nb_frames;
    printLog(QString("video frame number %1").arg(totalFrames));

    // 查找视频解码器
    const AVCodec* pCodec = avcodec_find_decoder(m_pVideoStream->codecpar->codec_id);
    if (nullptr == pCodec)
    {
        printLog(QString("open video failed to avcodec_find_decoder, error code %1").arg(result), true);
        closeVideo();
        return false;
    }

    // 创建解码器上下文，并设置默认值
    m_pCodecContext = avcodec_alloc_context3(pCodec);
    if (nullptr == m_pCodecContext)
    {
        printLog(QString("open video failed to avcodec_alloc_context3, error code %1").arg(result), true);
        closeVideo();
        return false;
    }

    // 使用视频流的 codecpar 为解码器上下文赋值
    result = avcodec_parameters_to_context(m_pCodecContext, m_pVideoStream->codecpar);
    if (result < 0)
    {
        printLog(QString("open video failed to avcodec_parameters_to_context, error code %1").arg(result), true);
        closeVideo();
        return false;
    }

    // 打开解码器
    result = avcodec_open2(m_pCodecContext, nullptr, nullptr);
    if (result < 0)
    {
        printLog(QString("open video failed to avcodec_open2, error code %1").arg(result), true);
        closeVideo();
        return false;
    }

    // 使用 8 线程解码
    m_pCodecContext->thread_count = 8;

    printLog(QString("video open finished"));

    return true;
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
    if (!openVideo())
    {
        return false;
    }

    // 更新播放状态
    m_isStopped = false;

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
    m_videoStartTime = av_gettime() / m_msecondTimeBase;
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
            AVRational rational = { 1, m_msecondTimeBase };
            int seekTarget = av_rescale_q(m_seekTime, rational, m_pVideoStream->time_base);
            if (av_seek_frame(m_pFormatContext, m_videoIndex, seekTarget, AVSEEK_FLAG_BACKWARD) < 0)
            {
                printLog("read video failed to av_seek_frame", true);
            }
            else
            {
                printLog(QString("read video seek %1").arg(m_seekTime));

                // 跳转视频，在视频数据列表里添加一个新的 AVPacket，并把 data 赋值为 "FLUSH_FLAG" 以做标记
                AVPacket* pPacket = av_packet_alloc();
                av_new_packet(pPacket, strlen(m_flushFlagChar) + 1);
                strcpy((char*)pPacket->data, m_flushFlagChar);
                clearPacketList();
                addPacket(pPacket);

                // 记录跳转开始时间
                m_seekVideoStartTime = av_gettime() / m_msecondTimeBase - m_seekTime;

                // 重置读取帧计数
                m_readFrameCount = 0;

                // 更新跳转执行标志
                m_seekFrameFlag = true;

                // 更新读取视频状态为未完成状态
                m_isReadFinished = false;
            }
            m_seekRequestFlag = false;
        }

        // 视频数据包列表数据包个数超过一定值，就暂停读取数据，等待数据解码，以防内存不足问题
        if (m_videoReadLimitNumber < m_listVideoPackets.size())
        {
            Sleep(1);
            continue;
        }

        AVPacket* pPacket = av_packet_alloc();
        if (av_read_frame(m_pFormatContext, pPacket) < 0)
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
                av_packet_unref(pPacket);
                break;
            }

            // 前几帧数据 avcodec_receive_frame 报错，返回 EAGAIN 值，是因为数据太少，解码器需要足够的数据合成帧
            // 所以在读取完成但是未解码完成后，发送空包到解码器，否则最后几帧解析不到
            addPacket(pPacket);
        }
        else
        {
            // 读取到的视频数据包添加到视频数据列表
            if (m_videoIndex == pPacket->stream_index)
            {
                addPacket(pPacket);
                ++m_readFrameCount;
            }
            else
            {
                av_packet_unref(pPacket);
            }
        }
    }

    // 清除数据
    clearPacketList();

    // 确保解码线程结束，再停止线程
    while (!m_isDecodeThreadFinished)
    {
        Sleep(1);
    }

    // 停止播放
    if (VideoState::StoppedState != m_state)
    {
        // 切换视频状态为正在播放状态
        m_state = VideoState::StoppedState;
        emit sigStateChanged(m_state);
    }

    // 关闭视频，析构指针
    closeVideo();

    printLog("video read thread exit");

    // 读取线程结束
    m_isReadFinished = false;
    m_isReadThreadFinished = true;
    m_readFrameCount = 0;

    return true;
}

void MCVideoPlayer::decodeVideo()
{
    // 重置解码线程完成状态
    m_isDecodeFinished = false;
    m_isDecodeThreadFinished = false;
    m_currentFrameIndex = 0;

    // 视频宽度、高度
    int videoWidth = 0;
    int videoHeight = 0;

    // 当前视频帧编号
    int currentFrameIndex = 0;
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
        if (m_isStopped)
        {
            // 视频停止，退出解码
            break;
        }

        if (m_decodeOneFrameFlag)
        {
            // 一帧视频数据解码完成，退出循环
            printLog("video decode one frame finished");
            m_decodeOneFrameFlag = false;
            break;
        }

        // 读取数据包
        m_mutex.lock();
        if (m_listVideoPackets.isEmpty())
        {
            m_mutex.unlock();

            // 视频数据列表为空，但未读取完成，等待读取视频数据
            Sleep(1);
            continue;
        }
        AVPacket* pPacket = m_listVideoPackets.first();
        m_listVideoPackets.removeFirst();
        m_mutex.unlock();

        // 遇到视频跳转的标志视频数据包，刷新数据缓冲，继续解码下一个视频数据包
        if (nullptr != pPacket->data && 0 == strcmp((char*)pPacket->data, m_flushFlagChar))
        {
            printLog("decode video flush buffer");
            avcodec_flush_buffers(m_pCodecContext);
            av_packet_unref(pPacket);

            // 更新视频开始时间
            m_videoStartTime = m_seekVideoStartTime;

            // seek 之后，av_read_frame 读取内容依旧从第一帧开始，m_currentFrameIndex 重置为 0
            m_currentFrameIndex = 0;

            continue;
        }

        // 将数据包传送到解码器
        avcodec_send_packet(m_pCodecContext, pPacket);

        if (avcodec_receive_frame(m_pCodecContext, pFrame) < 0)
        {
            if (m_isReadFinished && nullptr == pPacket->data)
            {
                // 视频数据列表为空，并且读取完成，视频解码完成，退出循环
                printLog("video decode finished");
                break;
            }

            // 视频数据包接收
            av_packet_unref(pPacket);
            continue;
        }
        ++m_currentFrameIndex;

        // 计算当前帧对应的时间
        currentFrameTime = m_currentFrameIndex * m_oneFrameTime;

        if (m_seekFrameFlag)
        {
            // 跳转触发时，如果 m_seekVideoStartTime != m_videoStartTime，则表示当前还在读取跳转前的数据帧
            // 舍弃跳转前的帧，直接 continue
            if (m_seekVideoStartTime != m_videoStartTime)
            {
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
            m_currentTime = av_gettime() / m_msecondTimeBase - m_videoStartTime;
            if (currentFrameTime <= m_currentTime)
            {
                // 跳转一帧，捕获到了这一帧，更新状态
                if (m_decodeOneFrameRequsetFlag)
                {
                    m_decodeOneFrameRequsetFlag = false;
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
    printLog("video decode thread exit");

    // 解码线程结束
    m_isDecodeFinished = true;
    m_isDecodeThreadFinished = true;
}

void MCVideoPlayer::addPacket(AVPacket* pPacket)
{
    m_mutex.lock();
    m_listVideoPackets << pPacket;
    m_mutex.unlock();
    return;
}

void MCVideoPlayer::clearPacketList()
{
    m_mutex.lock();
    for (AVPacket* pPacket : m_listVideoPackets)
    {
        av_packet_unref(pPacket);
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
            qCritical() << QString("The video player error: %1").arg(log);
        }
        else
        {
            qInfo() << QString("The video player: %1").arg(log);
        }
    }
}
