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

    void setVideoFile(const QString& filePath);
    void playVideo();

private:
    bool readVideo();
    void decodeVideo();
   
    bool addPacket(const AVPacket& pkt);
    void clearPacketList();

    void sleepMsec(int msec);

private:
    AVFormatContext*    m_pFormatContext    { nullptr };        // 视频 IO 上下文
    AVCodecContext*     m_pCodecContext     { nullptr };        // 解码器上下文
    AVStream*           m_pVideoStream      { nullptr };        // 视频流

    QString             m_videoFilePath     { "" };
    VideoState          m_state             { VideoState::StoppedState};

    bool                m_isPlaying         { false };          // 是否正在播放
    bool                m_isPause           { false };          // 是否正在暂停

    bool                m_isReadFinished    { false };          // 是否读取完成

    bool                m_isReadThreadFinished      { false };  // 是否读取线程完成
    bool                m_isDecodeThreadFinished    { false };  // 是否解码线程完成


    bool                m_seekFlag          { false };          // 跳转标志
    qint64              m_seekPosition      { 0 };              // 跳转位置

    qint64              m_videoStartTime;

    QList<AVPacket>     m_listVideoPackts;

    const char*         m_flushData         { "FLUSH_DATA" };

    QMutex              m_mutex;                                // 互斥锁      


    int                 index               { 0 };
};

MCWIDGET_END_NAMESPACE
