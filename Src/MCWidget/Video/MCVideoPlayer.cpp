#include "MCVideoPlayer.h"
USE_MCWIDGET_NAMESPACE

#include <QDebug>
#include <QEventLoop>
#include <QTimer>
#include <QImage>

#include <thread>

MCVideoPlayer::MCVideoPlayer(QObject* parent)
    : QObject(parent)
{
}

MCVideoPlayer::~MCVideoPlayer()
{
}

void MCVideoPlayer::setVideoFile(const QString& filePath)
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

    // 更新为正在播放状态，更新为未暂停状态
    m_isPlaying = true;
    m_isPause = false;

    // 创建新的线程，读取视频数据
    std::thread(&MCVideoPlayer::readVideo, this).detach();
}

bool MCVideoPlayer::readVideo()
{
    if (m_videoFilePath.isEmpty())
    {
        return false;
    }

    // 重置视频流指针
    m_pVideoStream = nullptr;

    // 重置读取完成状态，重置读取线程完成状态
    m_isReadFinished = false;
    m_isReadThreadFinished = false;

    // 分配视频格式 IO 上下文
    m_pFormatContext = avformat_alloc_context();

    // 打开视频文件
    int result = avformat_open_input(&m_pFormatContext, m_videoFilePath.toStdString().c_str(), nullptr, nullptr);
    if (result < 0)
    {
        qCritical() << "The video decoder error triggered: avformat_open_input failed";
        //free();
        return false;
    }

    // 读取视频流信息。
    result = avformat_find_stream_info(m_pFormatContext, nullptr);
    if (result < 0)
    {
        qCritical() << "The video decoder error triggered: avformat_find_stream_info failed";
        //free();
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
        qCritical() << "The video decoder error triggered: find video index failed";
        //free();
        return false;
    }

    // 计算视频总时长
    durationChanged(m_pFormatContext->duration);

    // 通过视频流索引读取视频流
    m_pVideoStream = m_pFormatContext->streams[videoIndex];

    // 查找视频解码器
    const AVCodec* pCodec = avcodec_find_decoder(m_pVideoStream->codecpar->codec_id);
    if (pCodec == nullptr)
    {
        qCritical() << "The video decoder error triggered: avcodec_find_decoder failed";
        //free();+
        return false;
    }

    // 创建解码器上下文，并设置默认值
    m_pCodecContext = avcodec_alloc_context3(pCodec);
    if (!m_pCodecContext)
    {
        qCritical() << "The video decoder error triggered: avcodec_alloc_context3 failed";
        //free();
        return false;
    }

    // 使用视频流的 codecpar 为解码器上下文赋值
    result = avcodec_parameters_to_context(m_pCodecContext, m_pVideoStream->codecpar);
    if (result < 0)
    {
        qCritical() << "The video decoder error triggered: avcodec_parameters_to_context failed";
        //free();
        return false;
    }

    // 设置解码线程数为 8
    m_pCodecContext->thread_count = 8;

    // 打开解码器
    result = avcodec_open2(m_pCodecContext, nullptr, nullptr);
    if (result < 0)
    {
        qCritical() << "The video decoder error triggered: avcodec_open2 failed";
        //free();
        return false;
    }

    // 解码器打开后，创建新的线程，解码视频数据包
    std::thread(&MCVideoPlayer::decodeVideo, this).detach();

    // 切换视频状态为正在播放状态
    m_state = VideoState::PlayingState;
    stateChanged(m_state);

    // 记录视频开始时间
    m_videoStartTime = av_gettime();

    // 读取视频数据循环
    while (m_isPlaying)
    {
        // todo：优化视频播放、暂停状态下的循环控制


        if (m_seekFlag)
        {
            AVRational rational = {1, AV_TIME_BASE};
            qint64 seekTarget = av_rescale_q(m_seekPosition, rational, m_pVideoStream->time_base);
            if (av_seek_frame(m_pFormatContext, videoIndex, seekTarget, AVSEEK_FLAG_BACKWARD) < 0)
            {
                qCritical() << "The video decoder error triggered: av_seek_frame failed";
            }
            else
            {
                // 跳转视频，在视频数据队列里添加一个新的 AVPacket，并把 data 赋值为 "FLUSH_DATA" 以做标记
                AVPacket packet;
                av_new_packet(&packet, 10);
                strcpy((char*)packet.data, m_flushData);
                clearPacketList();
                addPacket(packet);
            }

            m_videoStartTime = av_gettime() - m_seekPosition;
        }

        // 视频数据包队列数据包个数超过一定值，就暂停读取数据，等待数据解码，以防内存不足问题
        if (m_maxVideoSize < m_listVideoPackts.size())
        {
            sleepMsec(50);
            continue;
        }

        /**
         * 读取视频数据
         * 前几帧，av_read_frame 读取结果不为空，avcodec_receive_frame 读取报错，错误返回值为 AVERROR(EAGAIN)
         * 后几帧，av_read_frame 读取结果为空，avcodec_receive_frame 读取正常
         * 这是由于还没有发送足够的视频数据包来填满解码器的内部缓冲区，解码器需要更多数据才能产生完整帧
         */
        AVPacket packet;
        if (av_read_frame(m_pFormatContext, &packet) < 0)
        {
            // 读取完成
            m_isReadFinished = true;
            break;
        }

        // 读取到的视频数据包添加到视频数据队列
        if (videoIndex == packet.stream_index)
        {
            addPacket(packet);
        }
    }

    // 等待视频数据播放完成循环
    while (m_isPlaying)
    {
        sleepMsec(100);
    }

    clearPacketList();
}

void MCVideoPlayer::decodeVideo()
{
    // 解码线程开始
    m_isDecodeThreadFinished = false;

    // 视频宽度、高度
    int videoWidth = 0;
    int videoHeight = 0;

    // 当前视频的 pts
    double videoPts = 0;

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
        qCritical() << "The video decoder error triggered: av_frame_alloc failed";
        return;
    }

    while (m_isPlaying)
    {
        if (m_isPause)
        {
            sleepMsec(100);
            continue;
        }

        m_mutex.lock();

        if (m_listVideoPackts.isEmpty())
        {
            m_mutex.unlock();

            if (m_isReadFinished)
            {
                // 视频数据队列为空，并且读取完成，视频解码完成，退出循环
                break;
            }
            else
            {
                // 视频数据队列为空，但未读取完成，等待读取视频数据
                sleepMsec(10);
                continue;
            }
        }

        AVPacket pkt = m_listVideoPackts.first();
        m_listVideoPackts.removeFirst();

        m_mutex.unlock();

        AVPacket* pPacket = &pkt;

        // 遇到视频跳转的标志视频数据包，刷新数据缓冲，继续解码下一个视频数据包
        if(0 == strcmp((char*)pPacket->data, m_flushData) == 0)
        {
            avcodec_flush_buffers(m_pCodecContext);
            av_packet_unref(pPacket);
            continue;
        }

        if (avcodec_send_packet(m_pCodecContext, pPacket) < 0)
        {
           qCritical() << "The video decoder error triggered: avcodec_send_packet failed";
           av_packet_unref(pPacket);
           continue;
        }

        while (1)
        {
            if (avcodec_receive_frame(m_pCodecContext, pFrame) < 0)
            {
                break;
            }

            if (pPacket->dts == AV_NOPTS_VALUE && pFrame->opaque && *(uint64_t*) pFrame->opaque != AV_NOPTS_VALUE)
            {
                videoPts = *(uint64_t *) pFrame->opaque;
            }
            else if (pPacket->dts != AV_NOPTS_VALUE)
            {
                videoPts = pPacket->dts;
            }
            else
            {
                videoPts = 0;
            }
            videoPts *= av_q2d(m_pVideoStream->time_base);

            if (m_seekFlag)
            {
                if (videoPts < m_seekPosition)
                {
                    av_packet_unref(pPacket);
                    break;
                }
                else
                {
                    m_seekFlag = false;
                }
            }

            if (pFrame->width != videoWidth || pFrame->height != videoHeight)
            {
                videoWidth  = pFrame->width;
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

            // 渲染数据 (pFrameYUV, videoWidth, videoHeight)

            QImage image(pFrame->width, pFrame->height, QImage::Format_RGB888);
            // 创建指向目标图像数据的指针
            uint8_t* dest[4] = { image.bits(), nullptr, nullptr, nullptr };
            int destLinesize[4] = { image.bytesPerLine(), 0, 0, 0 };
            // 使用 sws_scale 转换帧数据到目标图像
            sws_scale(pSwsContext, pFrame->data, pFrame->linesize, 0, pFrame->height, dest, destLinesize);

            index++;
            QString fileame = "D:/Video/" + QString::number(index) + ".png";
            image.save(fileame);
        }

        av_packet_unref(pPacket);
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

    m_isPlaying = false;
}

bool MCVideoPlayer::addPacket(const AVPacket& pkt)
{
    m_mutex.lock();

    // 赋值一份数据到 packet
    AVPacket packet;
    if (av_packet_ref(&packet, &pkt) < 0)
    {
        return false;
    }
    m_listVideoPackts << packet;

    m_mutex.unlock();

    return true;
}

void MCVideoPlayer::clearPacketList()
{
    m_mutex.lock();

    for (AVPacket pkt : m_listVideoPackts)
    {
        av_packet_unref(&pkt);
    }
    m_listVideoPackts.clear();

    m_mutex.unlock();
}

void MCVideoPlayer::durationChanged(qint64 duration)
{
    // todo：实现视频总时间变化
}

void MCVideoPlayer::stateChanged(VideoState state)
{
    // todo：实现视频播放状态切换
}

void MCVideoPlayer::sleepMsec(int msec)
{
    if (msec <= 0)
    {
        return;
    }

    QEventLoop loop;
    QTimer::singleShot(msec, &loop, &QEventLoop::quit);
    loop.exec();
}
