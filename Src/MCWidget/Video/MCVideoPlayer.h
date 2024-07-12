#pragma once

#include "MCWidget_Global.h"
#include "MCVideoFrame.h"

#include <QObject>
#include <QMutex>
#include <QTimer>

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

    // 设置视频文件路径
    void setVideoFilePath(const QString& filePath);

    // 播放视频
    void playVideo();
    // 停止视频
    void stopVideo();
    // 跳转视频
    void seekVideio(qint64 seekTime);

    // 开启定时器
    void startTimer();
    // 停止定时器
    void stopTimer();

    // 设置日志启用状态
    void setLogEnabled(bool enabled);

signals:
    // 视频总时长变化
    void sigDurationChanged(qint64 msecond);
    // 视频帧变化
    void sigFrameChanged(std::shared_ptr<MCVideoFrame> frame);
    // 视频时间变化
    void sigTimeChanged(qint64 time);
    // 视频播放状态变化
    void sigStateChanged(VideoState state);

private:
    // 打开视频
    void openVideo();
    // 关闭视频
    void closeVideo();
    // 读取视频
    bool readVideo();
    // 解码视频
    void decodeVideo();

    // 添加数据包到列表
    void addPacket(const AVPacket& pkt);
    // 清空数据包对流
    void clearPacketList();

    // 打印日志
    void printLog(const QString& log, bool isError = false);

private slots:
    void timerTimeOut();

private:
    QString             m_videoFilePath             { "" };             // 视频路径

    VideoState          m_state                     { StoppedState};    // 视频播放状态

    AVFormatContext*    m_pFormatContext            { nullptr };        // 视频格式 IO 上下文
    AVStream*           m_pVideoStream              { nullptr };        // 视频流
    AVCodecContext*     m_pCodecContext             { nullptr };        // 解码器上下文

    bool                m_isStopped                 { false };          // 视频播放状态

    bool                m_isOpenFinished            { false };          // 打开视频状态
    bool                m_isOpenThreadFinished      { true };           // 打开视频线程状态

    bool                m_isReadFinished            { false };          // 读取视频状态
    bool                m_isReadThreadFinished      { true };           // 读取视频线程状态

    bool                m_isDecodeFinished          { false };          // 解码视频状态
    bool                m_isDecodeThreadFinished    { true };           // 解码视频线程状态

    bool                m_seekRequestFlag           { false };          // 跳转请求标志
    bool                m_seekFrameFlag             { false };          // 跳转完成标志

    bool                m_decodeOneFrameRequsetFlag { false };
    bool                m_decodeOneFrameFlag        { false };

    bool                m_isLogEnabled              { false };          // 日志启用状态

    qint64              m_videoIndex                { -1 };             // 视频流索引

    qint64              m_videoStartTime;                               // 视频开始时间

    qint64              m_currentTime               { 0 };              // 当前视频播放进度时间

    qint64              m_seekTime                  { 0 };              // 跳转时间
    qint64              m_seekVideoStartTime        { 0 };              // 跳转时视频开始时间位置

    qint64              m_readFrameCount            { 0 };              // 读取视频帧数量

    QList<AVPacket>     m_listVideoPackets;                             // 视频数据包列表

    QMutex              m_mutex;                                        // 互斥锁   

    QTimer*             m_pTimer;                                       // 更新视频进度定时器

    const static qint64 s_msecondTimeBase;                              // 单位转换，毫秒

    const static char*  s_flushFlagChar;                                // 视频 buffer 刷新标志字符

    const static int    s_videoReadLimitNumver;                         // 视频读取限制数量
};

MCWIDGET_END_NAMESPACE
