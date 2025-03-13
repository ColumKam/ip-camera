#ifndef GST_STREAMER_COMMON_H
#define GST_STREAMER_COMMON_H

#include <gst/gst.h>
#include <gst/rtsp-server/rtsp-server.h>
#include <glib.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// 流类型枚举
typedef enum {
    STREAM_TYPE_RTSP,
    STREAM_TYPE_RTMP,
    STREAM_TYPE_HLS
} StreamType;

// RTSP 服务器配置
typedef struct {
    char *address;         // 服务器地址
    char *port;           // 服务器端口
    char *mount_point;    // 挂载点
} RTSPConfig;

// 流配置结构体
typedef struct {
    char *device;           // 视频设备路径
    int width;             // 视频宽度
    int height;            // 视频高度
    int framerate;         // 帧率
    char *output_url;      // 输出URL或路径
    StreamType type;       // 流类型
    RTSPConfig rtsp;       // RTSP 配置
} StreamConfig;

// 公共函数声明
void on_stream_error(GstBus *bus, GstMessage *msg, gpointer data);
void on_stream_eos(GstBus *bus, GstMessage *msg, gpointer data);
GstElement* create_pipeline(StreamConfig *config);
void init_stream_config(StreamConfig *config);
void cleanup_stream(GstElement *pipeline, GMainLoop *loop);

// RTSP 相关函数声明
GstRTSPServer* create_rtsp_server(StreamConfig *config);
void setup_rtsp_pipeline(GstRTSPServer *server, StreamConfig *config);
void start_rtsp_server(GstRTSPServer *server);
void stop_rtsp_server(GstRTSPServer *server);

#endif // GST_STREAMER_COMMON_H 