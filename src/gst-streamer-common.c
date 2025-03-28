#include "gst-streamer-common.h"

static GMainLoop *loop = NULL;

void on_stream_error(GstBus *bus, GstMessage *msg, gpointer data) {
    GError *err;
    gchar *debug_info;
    
    gst_message_parse_error(msg, &err, &debug_info);
    g_printerr("Error received from element %s: %s\n", 
               GST_OBJECT_NAME(msg->src), err->message);
    g_printerr("Debugging information: %s\n", debug_info ? debug_info : "none");
    
    g_clear_error(&err);
    g_free(debug_info);
    
    if (loop) g_main_loop_quit(loop);
}

void on_stream_eos(GstBus *bus, GstMessage *msg, gpointer data) {
    g_print("End of stream\n");
    if (loop) g_main_loop_quit(loop);
}

void init_stream_config(StreamConfig *config) {
    config->device = "/dev/video0";
    config->width = 1920;
    config->height = 1080;
    config->framerate = 30;
    config->output_url = NULL;
    config->type = STREAM_TYPE_RTSP;
    
    // 初始化 RTSP 配置
    config->rtsp.address = "0.0.0.0";
    config->rtsp.port = "8554";
    config->rtsp.mount_point = "/test";
}

GstElement* create_pipeline(StreamConfig *config) {
    GstElement *pipeline;
    gchar *pipeline_str = NULL;
    
    // 基础视频源配置
    const char *base_src = g_strdup_printf(
        "v4l2src device=%s ! "
        "video/x-raw,format=NV12,width=%d,height=%d,framerate=%d/1 ! "
        "mpph264enc gop=30",
        config->device, config->width, config->height, config->framerate);

    // 根据流类型构建不同的管道
    switch (config->type) {
        case STREAM_TYPE_RTSP:
            // RTSP 管道在 setup_rtsp_pipeline 中创建
            pipeline_str = NULL;
            break;
            
        case STREAM_TYPE_RTMP:
            pipeline_str = g_strdup_printf(
                "%s ! h264parse ! flvmux streamable=true ! "
                "rtmpsink location=\"%s\"",
                base_src, config->output_url);
            break;
            
        case STREAM_TYPE_HLS:
            pipeline_str = g_strdup_printf(
                "%s ! h264parse ! mpegtsmux ! "
                "hlssink location=%s/segment_%%05d.ts "
                "playlist-location=%s/playlist.m3u8 "
                "playlist-length=3 target-duration=2",
                base_src, config->output_url, config->output_url);
            break;
    }
    
    if (pipeline_str) {
        pipeline = gst_parse_launch(pipeline_str, NULL);
        g_free(pipeline_str);
        return pipeline;
    }
    
    return NULL;
}

void cleanup_stream(GstElement *pipeline, GMainLoop *loop) {
    if (pipeline) {
        gst_element_set_state(pipeline, GST_STATE_NULL);
        gst_object_unref(pipeline);
    }
    if (loop) {
        g_main_loop_unref(loop);
    }
}

// RTSP 相关函数实现
GstRTSPServer* create_rtsp_server(StreamConfig *config) {
    GstRTSPServer *server = gst_rtsp_server_new();
    
    gst_rtsp_server_set_address(server, config->rtsp.address);
    gst_rtsp_server_set_service(server, config->rtsp.port);
    
    return server;
}

void setup_rtsp_pipeline(GstRTSPServer *server, StreamConfig *config) {
    GstRTSPMountPoints *mounts;
    GstRTSPMediaFactory *factory;
    gchar *launch_str;
    
    mounts = gst_rtsp_server_get_mount_points(server);
    factory = gst_rtsp_media_factory_new();
    
    // 构建 RTSP 管道字符串
    launch_str = g_strdup_printf(
        "( v4l2src device=%s ! "
        "video/x-raw,format=NV12,width=%d,height=%d,framerate=%d/1 ! "
        "mpph264enc gop=30 ! rtph264pay name=pay0 pt=96 )",
        config->device, config->width, config->height, config->framerate);
    
    gst_rtsp_media_factory_set_launch(factory, launch_str);
    g_free(launch_str);
    
    gst_rtsp_media_factory_set_shared(factory, TRUE);
    gst_rtsp_mount_points_add_factory(mounts, config->rtsp.mount_point, factory);
    g_object_unref(mounts);
}

void start_rtsp_server(GstRTSPServer *server) {
    gst_rtsp_server_attach(server, NULL);
}

void stop_rtsp_server(GstRTSPServer *server) {
    if (server) {
        g_object_unref(server);
    }
} 