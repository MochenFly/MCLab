#pragma once

#include <QObject>
#include <QMutex>
#include "MCWidget_Global.h"

extern "C"
{
    #include "libavcodec/avcodec.h"
    #include "libavformat/avformat.h"
    #include "libavutil/avutil.h"
    #include <libavutil/time.h>
    #include "libswscale/swscale.h"
    #include "libavutil/imgutils.h"
}

MCWIDGET_BEGIN_NAMESPACE

class MCWIDGET_EXPORT MCVideoPlayer : public QObject
{
    Q_OBJECT

public:
    enum VideoState
    {
        StoppedState,
        PlayingState,
        PausedState
    };

    MCVideoPlayer(QObject *parent);
    ~MCVideoPlayer();

    void setVideoFilePath(const QString& filePath);
    void playVideo();

private:
    bool readVideo();
    void decodeVideo();
   
    bool addPacket(const AVPacket& pkt);
    void clearPacketList();

    // 视频总时长变化
    void durationChanged(qint64 duration);
    // 视频播放状态变化
    void stateChanged(VideoState state);

    void sleepMsec(int msec);

private:
    AVFormatContext*    m_pFormatContext            { nullptr };        // 视频格式 IO 上下文
    AVCodecContext*     m_pReadCodecContext         { nullptr };        // 读取线程解码器上下文
    AVStream*           m_pVideoStream              { nullptr };        // 视频流 avformat_free_context

    QString             m_videoFilePath             { "" };

    VideoState          m_state                     { VideoState::StoppedState};

    bool                m_isPlaying                 { false };          // 是否正在播放
    bool                m_isPause                   { false };          // 是否正在暂停
    bool                m_isReadFinished            { false };          // 是否读取完成
    bool                m_isReadThreadFinished      { false };          // 是否读取线程完成
    bool                m_isDecodeThreadFinished    { false };          // 是否解码线程完成

    bool                m_seekFlag                  { false };          // 跳转标志
    qint64              m_seekPosition              { 0 };              // 跳转位置

    qint64              m_videoStartTime;                               // 视频开始时间

    QList<AVPacket>     m_listVideoPackts;

    const char*         m_flushData                 { "FLUSH_DATA" };
    const int           m_maxVideoSize              { 500 };

    QMutex              m_mutex;                                        // 互斥锁      

    int                 index                       { 0 };
};

MCWIDGET_END_NAMESPACE
