# IP Camera

## 修改历史

| 版本  | 作者  | 日期 | 修改说明 |
|---|---|---|---|
|   | colum.jin | 2025-03-13| 初始版本，代码整合完成 |
|   | colum.jin | 2025-03-17| HLS 串流可通过扫码访问 <br>**新增**: FAQ 板块——纪录常见问题 |
|   | colum.jin | 2025-03-28| **新增**: <br>支持芯片： Orangepi 3b <br> 编译： **依赖**章节 <br> 使用：**server 搭建**章节 <br> FAQ: **视频地址可访问但视频不能正常播放**章节 <br> |

## 支持芯片


| 芯片  | 系统  | Linux Kernel | 开发板 |
|---|---|---|---|
| RK3568 | Debian | 6.1.75 | Rockchip Developer Kit RK3568 EVB1 |
| RK3566 | Ubuntu 22.04 | 5.10.160 | Orangepi 3b |


## 编译

### 本地编译

#### 依赖

```
sudo apt install -y cmake
sudo apt install -y gstreamer-1.0 gstreamer-app-1.0 gstreamer-video-1.0
sudo apt install -y libgstrtspserver-1.0-dev gstreamer1.0-rtsp
sudo apt install -y libqrencode-dev
sudo apt install -y libcairo2-dev
```

#### 步骤

``` shell
cd ip-camera
mkdir build && cd build

# 使用默认配置
cmake ..

# 或者使用 Debug 配置
cmake -DCMAKE_BUILD_TYPE=Debug ..

make -j4
```

### 交叉编译

TODO

## 使用

### server 搭建

用于 RTMP 和 HLS 两种串流方式，server 是基于 nginx 实现的，具体流程在 server/install-nginx.sh 和 server/config-nginx.sh 中。

### streamer 使用

``` shell
./streamer -h
./streamer: option requires an argument -- 'h'
Usage: ./streamer [options]
Options:
  -t <type>     Stream type (rtsp, rtmp, hls)
  -o <url>      Output URL/path
  -d <device>   Video device (default: /dev/video0)
  -w <width>    Video width (default: 1920)
  -h <height>   Video height (default: 1800)
  -f <fps>      Framerate (default: 30)
  -a <addr>     RTSP server address (default: 0.0.0.0)
  -p <port>     RTSP server port (default: 8554)
  -m <mount>    RTSP mount point (default: /test)

Examples:
  RTSP: ./streamer -t rtsp
  RTMP: ./streamer -t rtmp -o rtmp://0.0.0.0:1935/live/stream
  HLS:  ./streamer -t hls -o /var/www/html/hls/
```

### RTSP 使用

视频地址：<rtsp://192.168.2.102:8554/test>

### RTMP 使用

视频地址：<rtmp://192.168.2.102:1935/live/stream>

### HLS 使用

视频地址：<http://192.168.2.102:8080/hls/playlist.m3u8>
网页地址：<http://192.168.2.102:8080/index.html>

## RTSP RTMP HLS 推流的区别

RTSP、RTMP 和 HLS 是三种常见的流媒体协议，它们在推流（直播）场景中的区别主要体现在协议设计、延迟、兼容性以及应用场景等方面。以下是具体对比：

---

#### **1. RTSP（Real-Time Streaming Protocol）**

- **协议类型**：基于 TCP/UDP 的实时控制协议，常用于摄像头、监控等场景。
- **延迟**：1-5 秒（取决于实现）。
- **推流特点**：
  - 主要用于**设备到服务器**的流传输（如摄像头推流到 NVR）。
  - 需要客户端与服务器建立会话控制（通过 `PLAY`、`PAUSE` 等指令）。
  - **不直接支持网页播放**（需依赖插件或转码）。
- **优势**：
  - 低延迟，适合实时监控。
  - 支持双向交互（如 PTZ 摄像头控制）。
- **缺点**：
  - 兼容性差（浏览器不支持原生播放）。
  - 防火墙/NAT 穿透能力弱。

---

#### **2. RTMP（Real-Time Messaging Protocol）**

- **协议类型**：基于 TCP 的私有协议，最初由 Adobe 开发，用于 Flash 直播。
- **延迟**：1-3 秒（低延迟场景）。
- **推流特点**：
  - **主流推流协议**：常用于直播平台（如 OBS 推流到 CDN）。
  - 支持音视频复用，数据分块传输。
  - **逐渐被淘汰**（依赖 Flash），但国内仍广泛使用（需转码为 HLS/DASH 分发）。
- **优势**：
  - 低延迟，适合互动直播（连麦、弹幕）。
  - 支持动态码率切换（Adaptive Bitrate）。
- **缺点**：
  - 浏览器原生不支持（需 WebRTC 或 MSE 转码）。
  - 对网络抖动敏感。

---

#### **3. HLS（HTTP Live Streaming）**

- **协议类型**：基于 HTTP 的流媒体协议，由 Apple 提出。
- **延迟**：10-30 秒（常规 HLS）或 2-10 秒（低延迟 HLS/LL-HLS）。
- **推流特点**：
  - **非实时推流协议**：推流端需将视频切片为 TS 分段，通过 HTTP 分发。
  - 通常用于**服务器到客户端**的分发（如 CDN 到观众）。
  - 推流时需先转码为 HLS 格式（如通过 RTMP 转 HLS）。
- **优势**：
  - 高兼容性：所有现代浏览器和移动端均支持。
  - 适应弱网环境（分段加载，支持多码率适配）。
  - 天然支持 CDN 和 HTTPS。
- **缺点**：
  - 延迟高，不适合实时互动场景。
  - 推流流程复杂（需编码、切片、分发）。

---

### **对比总结**

| **特性**       | RTSP                  | RTMP                  | HLS                   |
|----------------|-----------------------|-----------------------|-----------------------|
| **协议基础**    | TCP/UDP               | TCP                   | HTTP                  |
| **延迟**        | 低（1-5 秒）          | 低（1-3 秒）           | 高（10-30 秒）         |
| **推流场景**    | 监控设备、视频会议     | 直播平台推流           | 点播/直播分发（非推流） |
| **兼容性**      | 依赖播放器/插件        | 依赖转码/WebRTC        | 全平台原生支持         |
| **适用场景**    | 实时监控、低延迟交互   | 低延迟直播、互动场景    | 高兼容性直播、点播     |

---

#### **如何选择？**

1. **需要超低延迟**（如游戏直播、连麦）：优先 RTMP 或 WebRTC。
2. **兼容性和自适应码率**（如大众直播）：用 HLS/DASH。
3. **监控或物联网设备**：选 RTSP。
4. **现代应用**：结合 LL-HLS（低延迟 HLS）或 WebRTC 优化体验。

实际应用中，常通过**混合方案**实现（如 RTMP 推流 + HLS 分发）。

## 开发思路

### 基于 Gstreamer 开发

### 基于 Rockchip 平台

## RTSP、RTMP 和 HLS 推送流程

主要展示推流器、服务器、客户端三者之间的关系

### RTSP 推送流程

``` shell
+----------------------+      RTSP控制流       +---------------------+
|                      |  ◄──────┬───────►    |                     |
|   RTSP推流器          |     (TCP:554)        |   RTSP媒体服务器     |
| (IP摄像头/编码器)     |                      | (如Wowza/专用NVR)    |
+----------------------+                      +----------+----------+
         │  ▲                                            │
         │  └────────RTP/RTCP数据流──────────────────────┘
         │     (UDP/TCP 动态端口)
         ▼
+---------------------+
|                     |
|   RTSP播放客户端     |
| (VLC/专业播放软件)   |
+---------------------+

```

### RTMP 推送流程

``` shell
+----------------------+        RTMP推流      +---------------------+
|                      |  ────────────────►   |                     |
|   推流器             |    (TCP:1935)        |   RTMP媒体服务器     |
| (rtmp-streamer)      |                      |   (Nginx-RTMP)      |
+----------------------+                      +----------+----------+
                                                         │
                                                         │ RTMP分发
                                                         ▼
                                              +---------------------+
                                              |                     |
                                              |    RTMP播放客户端    |
                                              | (Flash/专用播放器)   |
                                              +---------------------+
```

### HLS 推送流程

``` shell
+----------------------+       推流        +---------------------+
|                      |  ───────────────► |                     |
|   推流器              | (RTMP/其他协议)   |   媒体处理服务器     |
| (hls-streamer)       |                   |  (切片+转码)        |
+----------------------+                   +----------+----------+
                                                            │
                                                            │ 生成HLS分片
                                                            ▼
                                +---------------------+  HTTP请求    +---------------------+
                                |                     | ◄─────────── |                    |
                                |   HLS存储服务器      |              |   HLS播放客户端     |
                                | (存储.ts/.m3u8文件)  |              | (浏览器/移动端APP)  |
                                +---------------------+              +---------------------+
```

## TODO List

- [x] 整理代码到 main.c 中 (完成日期：2025-3-13)
- [] RTMP 支持 WebRTC
- [] RTMP 支持 网页访问
- [] 支持 WiFi AP 模式下功能
- [x] 二维码展示链接地址 (完成日期：2025-3-17)
- [] 交叉编译

## FAQ

### 视频地址无法访问

【**现象**】

1. VLC 打不开视频地址
```
您的输入无法被打开:
VLC 无法打开 MRL「http://192.168.2.102:8080/playlist.m3u8」。详情请检查日志。
```
2. 访问页面实现 404


【**解决方法**】

1. 是否在同一网段？
2. 摄像头是否正常出流？
  2.1. 增加前缀 GST_DEBUG=2 <Command> 执行，观察日志
  2.2. 可以通过 /etc/init.d/rkaiq_3A.sh stop /etc/init.d/rkaiq_3A.sh start 重启 rkaiq_3A_server 服务
3. Ngnix 是否正常？
  3.1 可以通过 /usr/local/nginx/sbin/nginx -s reload 重启 nginx 服务
4. 检查 nginx 错误日志
  sudo tail -n 50 /usr/local/nginx/logs/error.log

### 视频地址可访问但视频不能正常播放

【**现象**】

1. 多设备访问不同局域网内的 index.html 网页均出现网页可访问，但是视频不能播放（显示圆环转圈）
2. 检查 nginx 错误日志

```
sudo tail -n 50 /usr/local/nginx/logs/error.log
```

发现有类似日志（该日志从 Orangepi 3b 5.10 开发环境中获取）

```
2025/03/27 18:00:39 [notice] 33413#0: signal process started
2025/03/27 18:00:39 [error] 33413#0: open() "/usr/local/nginx/logs/nginx.pid" failed (2: No such file or directory)
2025/03/27 18:02:40 [notice] 33594#0: signal process started
2025/03/27 18:12:56 [error] 33595#0: *4 open() "/usr/local/nginx/html/playlist.m3u8" failed (2: No such file or directory), client: 192.168.3.30, server: 0.0.0.0, request: "GET /playlist.m3u8 HTTP/1.1", host: "192.168.3.33:8080"
2025/03/27 18:12:56 [error] 33595#0: *5 open() "/usr/local/nginx/html/playlist.m3u8" failed (2: No such file or directory), client: 192.168.3.30, server: 0.0.0.0, request: "GET /playlist.m3u8 HTTP/1.0", host: "192.168.3.33:8080"
2025/03/27 18:14:04 [error] 33595#0: *9 open() "/usr/local/nginx/html/favicon.ico" failed (2: No such file or directory), client: 192.168.3.30, server: 0.0.0.0, request: "GET /favicon.ico HTTP/1.1", host: "192.168.3.33:8080", referrer: "http://192.168.3.33:8080/index.html"
2025/03/27 18:18:55 [error] 33595#0: *13 open() "/usr/local/nginx/html/favicon.ico" failed (2: No such file or directory), client: 192.168.2.17, server: 0.0.0.0, request: "GET /favicon.ico HTTP/1.1", host: "192.168.3.33:8080", referrer: "http://192.168.3.33:8080/index.html"
2025/03/27 18:31:11 [error] 33595#0: *30 open() "/usr/local/nginx/html/favicon.ico" failed (2: No such file or directory), client: 192.168.2.155, server: 0.0.0.0, request: "GET /favicon.ico HTTP/1.1", host: "192.168.2.102:8080", referrer: "http://192.168.2.102:8080/index.html"
2025/03/27 18:32:52 [error] 33595#0: *31 open() "/usr/local/nginx/html/favicon.ico" failed (2: No such file or directory), client: 192.168.3.12, server: 0.0.0.0, request: "GET /favicon.ico HTTP/1.1", host: "192.168.2.102:8080", referrer: "http://192.168.2.102:8080/index.html"
2025/03/27 18:42:38 [error] 33595#0: *38 open() "/var/www/html/hls/playlist.m3u8" failed (2: No such file or directory), client: 192.168.3.30, server: 0.0.0.0, request: "GET /hls/playlist.m3u8 HTTP/1.1", host: "192.168.3.33:8080"
2025/03/27 18:42:39 [error] 33595#0: *40 open() "/var/www/html/hls/playlist.m3u8" failed (2: No such file or directory), client: 192.168.3.30, server: 0.0.0.0, request: "GET /hls/playlist.m3u8 HTTP/1.1", host: "192.168.3.33:8080"
2025/03/27 18:42:40 [error] 33595#0: *41 open() "/var/www/html/hls/playlist.m3u8" failed (2: No such file or directory), client: 192.168.3.30, server: 0.0.0.0, request: "GET /hls/playlist.m3u8 HTTP/1.1", host: "192.168.3.33:8080"
2025/03/27 18:42:41 [error] 33595#0: *42 open() "/var/www/html/hls/playlist.m3u8" failed (2: No such file or directory), client: 192.168.3.30, server: 0.0.0.0, request: "GET /hls/playlist.m3u8 HTTP/1.1", host: "192.168.3.33:8080"
2025/03/27 18:42:56 [error] 33595#0: *43 open() "/var/www/html/hls/playlist.m3u8" failed (2: No such file or directory), client: 192.168.3.30, server: 0.0.0.0, request: "GET /hls/playlist.m3u8 HTTP/1.1", host: "192.168.3.33:8080", referrer: "http://192.168.3.33:8080/index.html"
2025/03/27 18:43:05 [error] 33595#0: *45 open() "/usr/local/nginx/html/playlist.m3u8" failed (2: No such file or directory), client: 192.168.3.30, server: 0.0.0.0, request: "GET /playlist.m3u8 HTTP/1.1", host: "192.168.3.33:8080"
2025/03/27 18:43:05 [error] 33595#0: *46 open() "/usr/local/nginx/html/playlist.m3u8" failed (2: No such file or directory), client: 192.168.3.30, server: 0.0.0.0, request: "GET /playlist.m3u8 HTTP/1.0", host: "192.168.3.33:8080"
2025/03/27 18:43:15 [error] 33595#0: *47 open() "/var/www/html/hls/playlist.m3u8" failed (2: No such file or directory), client: 192.168.3.30, server: 0.0.0.0, request: "GET /hls/playlist.m3u8 HTTP/1.1", host: "192.168.3.33:8080"
2025/03/27 18:43:15 [error] 33595#0: *48 open() "/var/www/html/hls/playlist.m3u8" failed (2: No such file or directory), client: 192.168.3.30, server: 0.0.0.0, request: "GET /hls/playlist.m3u8 HTTP/1.0", host: "192.168.3.33:8080"
2025/03/27 18:44:04 [error] 33595#0: *49 open() "/var/www/html/hls/playlist.m3u8" failed (2: No such file or directory), client: 192.168.3.30, server: 0.0.0.0, request: "GET /hls/playlist.m3u8 HTTP/1.1", host: "192.168.3.33:8080", referrer: "http://192.168.3.33:8080/index.html"
```

3. 检查 HLS 文件的访问权限和路径 

```
ls -l /var/www/html/hls/
```

```
% ls -l /var/www/html/hls/
total 42656
-rw-r--r-- 1 root root     209 Mar 27 18:40 playlist.m3u8
-rw-r--r-- 1 root root 2321424 Mar 27 18:39 segment_00004.ts
-rw-r--r-- 1 root root 2308828 Mar 27 18:39 segment_00005.ts
-rw-r--r-- 1 root root 2292096 Mar 27 18:39 segment_00006.ts
-rw-r--r-- 1 root root 2291532 Mar 27 18:39 segment_00007.ts
-rw-r--r-- 1 root root 2281568 Mar 27 18:39 segment_00008.ts
-rw-r--r-- 1 root root 2290028 Mar 27 18:40 segment_00009.ts
-rw-r--r-- 1 root root 2422944 Mar 27 18:40 segment_00010.ts
-rw-r--r-- 1 root root 2358648 Mar 27 18:40 segment_00011.ts
-rw-r--r-- 1 root root 2320672 Mar 27 18:40 segment_00012.ts
-rw-r--r-- 1 root root 1200128 Mar 27 18:40 segment_00013.ts
-rw-r--r-- 1 root root 2333080 Mar 27 18:39 segment_00485.ts
-rw-r--r-- 1 root root 2387224 Mar 27 18:39 segment_00486.ts
-rw-r--r-- 1 root root 2385344 Mar 27 18:39 segment_00487.ts
-rw-r--r-- 1 root root 2284952 Mar 27 18:39 segment_00488.ts
-rw-r--r-- 1 root root 2289276 Mar 27 18:39 segment_00489.ts
-rw-r--r-- 1 root root 2316724 Mar 27 18:39 segment_00490.ts
-rw-r--r-- 1 root root 2400948 Mar 27 18:39 segment_00491.ts
-rw-r--r-- 1 root root 2402264 Mar 27 18:39 segment_00492.ts
-rw-r--r-- 1 root root 2376696 Mar 27 18:39 segment_00493.ts
-rw-r--r-- 1 root root  376832 Mar 27 18:39 segment_00494.ts
```

【**可能错误原因**】

1. /usr/local/nginx/sbin/nginx 没有使用 root 权限执行，但是 streamer 的启动却是 root 权限执行
