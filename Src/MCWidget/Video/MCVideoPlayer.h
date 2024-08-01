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
        PlayingState
    };

    MCVideoPlayer(QObject* parent);
    ~MCVideoPlayer();

    // 设置视频文件路径
    void setVideoFilePath(const QString& filePath);

    // 播放视频
    void playVideo();
    // 播放一帧视频
    void playOneFrame(int frameTime);
    // 停止视频
    void stopVideo();
    // 跳转视频
    void seekVideo(int seekTime);

    // 获取视频播放状态
    VideoState getState();

    int getCurrentTime();

    // 设置日志启用状态
    void setLogEnabled(bool enabled);

signals:
    // 视频总时长变化
    void sigDurationChanged(int msecond);
    // 视频帧变化
    void sigFrameChanged(std::shared_ptr<MCVideoFrame> frame);
    // 视频播放状态变化
    void sigStateChanged(VideoState state);

private:
    // 打开视频
    bool openVideo();
    // 关闭视频
    void closeVideo();
    // 读取视频
    bool readVideo();
    // 解码视频
    void decodeVideo();

    // 添加数据包到列表
    void addPacket(AVPacket* pPacket);
    // 清空数据包对流
    void clearPacketList();

    // 打印日志
    void printLog(const QString& log, bool isError = false);

private:
    QString             m_videoFilePath             { "" };             // 视频路径

    VideoState          m_state                     { StoppedState };   // 视频播放状态

    AVFormatContext*    m_pFormatContext            { nullptr };        // 视频格式 IO 上下文
    AVStream*           m_pVideoStream              { nullptr };        // 视频流
    AVCodecContext*     m_pCodecContext             { nullptr };        // 解码器上下文

    bool                m_isStopped                 { false };          // 视频播放状态

    bool                m_isReadFinished            { false };          // 读取视频状态
    bool                m_isReadThreadFinished      { true };           // 读取视频线程状态

    bool                m_isDecodeFinished          { false };          // 解码视频状态
    bool                m_isDecodeThreadFinished    { true };           // 解码视频线程状态

    bool                m_seekRequestFlag           { false };          // 跳转请求标志
    bool                m_seekFrameFlag             { false };          // 跳转执行标志

    bool                m_decodeOneFrameRequsetFlag { false };          // 解码一帧请求标志
    bool                m_decodeOneFrameFlag        { false };          // 解码一帧完成标志

    bool                m_isLogEnabled              { false };          // 日志启用状态

    int                 m_videoIndex                { -1 };             // 视频流索引
    int                 m_oneFrameTime              { 0 };              // 视频一帧时间
    double              m_videoFrameRate            { 0.0 };            // 视频帧率

    int                 m_videoStartTime            { 0 };              // 视频开始时间

    int                 m_currentFrameIndex         { 0 };              // 当前视频帧索引

    int                 m_currentTime               { 0 };              // 当前视频播放进度时间

    int                 m_seekTime                  { 0 };              // 跳转时间
    int                 m_seekVideoStartTime        { 0 };              // 跳转时视频开始时间位置

    int                 m_readFrameCount            { 0 };              // 读取视频帧数量

    QList<AVPacket*>    m_listVideoPackets;                             // 视频数据包列表

    QMutex              m_mutex;                                        // 互斥锁   
                     
    const int           m_msecondTimeBase           { 1000 };           // 单位转换，毫秒

    const char*         m_flushFlagChar             { "FLUSH_FLAG" };   // 视频 buffer 刷新标志字符

    const int           m_videoReadLimitNumber      { 500 };            // 视频读取限制数量  
};

MCWIDGET_END_NAMESPACE
