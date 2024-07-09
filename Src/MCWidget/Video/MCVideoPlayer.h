#pragma once

#include <QObject>
#include <QMutex>
#include <QTimer>
#include "MCWidget_Global.h"
#include "MCVideoFrame.h"

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

    void seek(qint64 seekTime);

    void startTimer();
    void stopTimer();

signals:
    void sigFrameChanged(std::shared_ptr<MCVideoFrame> frame);
    void sigTimeChanged(qint64 time);
    void sigStateChanged(VideoState state);

private:
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

    bool                m_isStopped                 { false };          // 是否正在播放
    bool                m_isPause                   { false };          // 是否正在暂停
    bool                m_isReadFinished            { false };          // 是否读取完成
    bool                m_isReadThreadFinished      { false };          // 是否读取线程完成
    bool                m_isDecodeThreadFinished    { false };          // 是否解码线程完成
    bool                m_seekRequestFlag           { false };          // 跳转请求标志
    bool                m_seekFrameFlag             { false };          // 跳转完成标志

    qint64              m_videoStartTime;                               // 视频开始时间
    qint64              m_seekTime                  { 0 };              // 跳转位置
    qint64              m_seekVideoStartTime                  { 0 };    // 跳转时视频开始时间位置
    qint64              m_currentTime               { 0 };

    QList<AVPacket>     m_listVideoPackets;

    QMutex              m_mutex;                                        // 互斥锁   

    QTimer*             m_pTimer;

    const int           m_maxVideoSize              { 500 };
    const char*         m_flushData                 { "FLUSH_DATA" };
};

MCWIDGET_END_NAMESPACE
