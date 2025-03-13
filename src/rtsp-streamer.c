#include <gst/gst.h>
#include <gst/rtsp-server/rtsp-server.h>

int main(int argc, char *argv[]) {
  GMainLoop *loop;
  GstRTSPServer *server;
  GstRTSPMountPoints *mounts;
  GstRTSPMediaFactory *factory;

  gst_init(&argc, &argv);

  loop = g_main_loop_new(NULL, FALSE);
  server = gst_rtsp_server_new();

  gst_rtsp_server_set_address(server, "0.0.0.0");
  gst_rtsp_server_set_service(server, "8554");

  mounts = gst_rtsp_server_get_mount_points(server);
  factory = gst_rtsp_media_factory_new();

  if (argc > 1) {
    gst_rtsp_media_factory_set_launch(factory, argv[1]);
  } else {
    gst_rtsp_media_factory_set_launch(factory,
                "( v4l2src device=/dev/video0 ! "
                "video/x-raw,format=NV12,width=3200,height=1800,framerate=30/1 ! "
                "mpph264enc gop=30 ! rtph264pay name=pay0 pt=96 )");
  }

  gst_rtsp_media_factory_set_shared(factory, TRUE);
  gst_rtsp_mount_points_add_factory(mounts, "/test", factory);
  g_object_unref(mounts);

  gst_rtsp_server_attach(server, NULL);
  g_print("RTSP server started at rtsp://0.0.0.0:8554/test\n");

  g_main_loop_run(loop);

  return 0;
}

