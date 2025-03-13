#include <gst/gst.h>
#include <glib.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static GMainLoop *loop;
static GstElement *pipeline;

static void on_eos(GstBus *bus, GstMessage *msg, gpointer data) {
    g_print("End of stream\n");
    g_main_loop_quit(loop);
}

static void on_error(GstBus *bus, GstMessage *msg, gpointer data) {
    GError *err;
    gchar *debug_info;

    gst_message_parse_error(msg, &err, &debug_info);
    g_printerr("Error received from element %s: %s\n",
               GST_OBJECT_NAME(msg->src), err->message);
    g_printerr("Debugging information: %s\n", debug_info ? debug_info : "none");

    g_clear_error(&err);
    g_free(debug_info);

    g_main_loop_quit(loop);
}

int main(int argc, char *argv[]) {
    GstBus *bus;
    GstStateChangeReturn ret;
    gchar *rtmp_url = "rtmp://0.0.0.0:1935/live/stream";

    /* Initialize GStreamer */
    gst_init(&argc, &argv);

    /* Create the main loop */
    loop = g_main_loop_new(NULL, FALSE);

    /* Parse command line parameters */
    if (argc > 1) {
        rtmp_url = argv[1];
    }

    g_print("RTMP server streaming to: %s\n", rtmp_url);

    /* Build the pipeline
     * We use v4l2src for camera capture with Rockchip hardware
     * Use hardware encoding with mpph264enc
     * flvmux for FLV container format (used by RTMP)
     * rtmpsink to stream to RTMP server
     */
    gchar *pipeline_str = g_strdup_printf(
        "v4l2src device=/dev/video0 ! "
        "video/x-raw,format=NV12,width=3200,height=1800,framerate=30/1 ! "
        "mpph264enc ! h264parse ! "
        "flvmux streamable=true ! "
        "rtmpsink location=\"%s\"", rtmp_url);

    pipeline = gst_parse_launch(pipeline_str, NULL);
    g_free(pipeline_str);

    /* Start playing */
    ret = gst_element_set_state(pipeline, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        g_printerr("Unable to set the pipeline to the playing state.\n");
        gst_object_unref(pipeline);
        return -1;
    }

    /* Add message handlers */
    bus = gst_element_get_bus(pipeline);
    gst_bus_add_signal_watch(bus);
    g_signal_connect(bus, "message::error", G_CALLBACK(on_error), NULL);
    g_signal_connect(bus, "message::eos", G_CALLBACK(on_eos), NULL);
    gst_object_unref(bus);

    /* Start the main loop */
    g_print("RTMP streaming started. Press Ctrl+C to stop.\n");
    g_main_loop_run(loop);

    /* Clean up */
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);
    g_main_loop_unref(loop);

    return 0;
}

