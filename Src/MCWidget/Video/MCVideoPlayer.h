#pragma once

#include "MCWidget_Global.h"
#include "MCVideoFrame.h"

#include <QObject>
#include <QMutex>
#include <QTimer>

#include <thread>

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

    MCVideoPlayer(QObject* parent);
    ~MCVideoPlayer();

    void setVideoFilePath(const QString& filePath);

    void playVideo();
    void pauseVideo(bool pause);
    void stopVideo();

    void seek(qint64 seekTime);

    void startTimer();
    void stopTimer();

signals:
    void sigDurationChanged(qint64 msecond);
    void sigFrameChanged(std::shared_ptr<MCVideoFrame> frame);
    void sigTimeChanged(qint64 time);
    void sigStateChanged(VideoState state);

private:
    void openVideo();

    bool readVideoStart();
    void readVideoExit();
    void freeReadData();

    void decodeVideoStart();
    void decodeVideoExit();

    bool addPacket(const AVPacket& pkt);
    void clearPacketList();

    // 视频总时长变化
    void durationChanged(qint64 duration);
    // 视频播放状态变化
    void stateChanged(VideoState state);

private slots:
    void timerTimeOut();

private:

    AVFormatContext*    m_pFormatContext            { nullptr };        // 视频格式 IO 上下文
    AVStream*           m_pVideoStream              { nullptr };        // 视频流
    AVCodecContext*     m_pCodecContext             { nullptr };        // 解码器上下文

    QString             m_videoFilePath             { "" };
    VideoState          m_state                     { VideoState::StoppedState};

    bool                m_isOpened                  { false };          // 打开视频状态
    bool                isOpenThreadFinished        { true };           // 打开视频线程状态

    bool                m_isReadFinished            { false };          // 读取视频状态
    bool                m_isReadThreadFinished      { true };           // 读取视频线程状态

    bool                m_isDecodeFinished          { false };          // 解码视频状态
    bool                m_isDecodeThreadFinished    { true };           // 解码视频线程状态

    bool                m_isStopped                 { false };          // 是否正在播放
    bool                m_isPause                   { false };          // 是否正在暂停
    bool                m_seekRequestFlag           { false };          // 跳转请求标志
    bool                m_seekFrameFlag             { false };          // 跳转完成标志
    bool                m_seekWaitFlag{false};

    qint64              m_videoStartTime;                               // 视频开始时间
    qint64              m_seekTime                  { 0 };              // 跳转位置
    qint64              m_seekVideoStartTime                  { 0 };    // 跳转时视频开始时间位置
    qint64              m_currentTime               { 0 };

    QList<AVPacket>     m_listVideoPackets;

    QMutex              m_mutex;                                        // 互斥锁   

    QTimer*             m_pTimer;

    const int           m_maxVideoSize              { 500 };
    const char*         m_flushData                 { "FLUSH_DATA" };

    const static qint64 s_msecondTimeBase;

    std::thread         m_readThread;
    std::thread         m_decodeThread;
};

MCWIDGET_END_NAMESPACE