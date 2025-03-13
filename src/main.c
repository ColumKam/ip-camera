#include "gst-streamer-common.h"

static void print_usage(const char *program_name) {
    g_print("Usage: %s [options]\n", program_name);
    g_print("Options:\n");
    g_print("  -t <type>     Stream type (rtsp, rtmp, hls)\n");
    g_print("  -o <url>      Output URL/path\n");
    g_print("  -d <device>   Video device (default: /dev/video0)\n");
    g_print("  -w <width>    Video width (default: 3200)\n");
    g_print("  -h <height>   Video height (default: 1800)\n");
    g_print("  -f <fps>      Framerate (default: 30)\n");
    g_print("  -a <addr>     RTSP server address (default: 0.0.0.0)\n");
    g_print("  -p <port>     RTSP server port (default: 8554)\n");
    g_print("  -m <mount>    RTSP mount point (default: /test)\n");
    g_print("\nExamples:\n");
    g_print("  RTSP: %s -t rtsp \n", program_name);
    g_print("  RTSP: %s -t rtsp -a 192.168.1.100 -p 8555 -m /camera1\n", program_name);
    g_print("  RTSP: %s -t rtsp -w 1920 -h 1080 -f 25\n", program_name);
    g_print("  RTMP: %s -t rtmp -o rtmp://0.0.0.0:1935/live/stream\n", program_name);
    g_print("  HLS:  %s -t hls -o /tmp/hls\n", program_name);
}

int main(int argc, char *argv[]) {
    GstElement *pipeline = NULL;
    GstRTSPServer *rtsp_server = NULL;
    GMainLoop *loop;
    GstBus *bus;
    StreamConfig config;
    int opt;
    
    // 初始化 GStreamer
    gst_init(&argc, &argv);
    
    // 初始化默认配置
    init_stream_config(&config);
    
    // 解析命令行参数
    while ((opt = getopt(argc, argv, "t:o:d:w:h:f:a:p:m:")) != -1) {
        switch (opt) {
            case 't':
                if (strcmp(optarg, "rtsp") == 0)
                    config.type = STREAM_TYPE_RTSP;
                else if (strcmp(optarg, "rtmp") == 0)
                    config.type = STREAM_TYPE_RTMP;
                else if (strcmp(optarg, "hls") == 0)
                    config.type = STREAM_TYPE_HLS;
                else {
                    g_printerr("Invalid stream type: %s\n", optarg);
                    print_usage(argv[0]);
                    return 1;
                }
                break;
            case 'o':
                config.output_url = optarg;
                break;
            case 'd':
                config.device = optarg;
                break;
            case 'w':
                config.width = atoi(optarg);
                break;
            case 'h':
                config.height = atoi(optarg);
                break;
            case 'f':
                config.framerate = atoi(optarg);
                break;
            case 'a':
                config.rtsp.address = optarg;
                break;
            case 'p':
                config.rtsp.port = optarg;
                break;
            case 'm':
                config.rtsp.mount_point = optarg;
                break;
            default:
                print_usage(argv[0]);
                return 1;
        }
    }
    
    // 验证必要参数
    if (!config.output_url && config.type != STREAM_TYPE_RTSP) {
        g_printerr("Error: Output URL/path is required for RTMP and HLS\n");
        print_usage(argv[0]);
        return 1;
    }
    
    // 创建主循环
    loop = g_main_loop_new(NULL, FALSE);
    
    // 根据流类型创建不同的管道
    if (config.type == STREAM_TYPE_RTSP) {
        // 创建和设置 RTSP 服务器
        rtsp_server = create_rtsp_server(&config);
        setup_rtsp_pipeline(rtsp_server, &config);
        start_rtsp_server(rtsp_server);
        
        // 打印 RTSP URL
        g_print("RTSP streaming started. Access at rtsp://%s:%s%s\n",
                config.rtsp.address, config.rtsp.port, config.rtsp.mount_point);
    } else {
        // 创建普通管道
        pipeline = create_pipeline(&config);
        if (!pipeline) {
            g_printerr("Error: Failed to create pipeline\n");
            return 1;
        }
        
        // 设置消息处理
        bus = gst_element_get_bus(pipeline);
        gst_bus_add_signal_watch(bus);
        g_signal_connect(bus, "message::error", G_CALLBACK(on_stream_error), NULL);
        g_signal_connect(bus, "message::eos", G_CALLBACK(on_stream_eos), NULL);
        gst_object_unref(bus);
        
        // 启动管道
        GstStateChangeReturn ret = gst_element_set_state(pipeline, GST_STATE_PLAYING);
        if (ret == GST_STATE_CHANGE_FAILURE) {
            g_printerr("Unable to set the pipeline to the playing state.\n");
            cleanup_stream(pipeline, loop);
            return 1;
        }
        
        // 打印启动信息
        switch (config.type) {
            case STREAM_TYPE_RTMP:
                g_print("RTMP streaming started. Access at %s\n", config.output_url);
                break;
            case STREAM_TYPE_HLS:
                g_print("HLS streaming started. Access at http://YOUR_IP:8080%s/playlist.m3u8\n", 
                       config.output_url);
                break;
        }
    }
    
    g_print("Press Ctrl+C to stop streaming.\n");
    
    // 运行主循环
    g_main_loop_run(loop);
    
    // 清理资源
    if (config.type == STREAM_TYPE_RTSP) {
        stop_rtsp_server(rtsp_server);
    } else {
        cleanup_stream(pipeline, loop);
    }
    
    return 0;
}
