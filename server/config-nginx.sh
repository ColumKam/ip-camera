#!/bin/bash

# 检查是否以root用户运行
if [ "$EUID" -ne 0 ]; then
    echo "请使用root用户运行此脚本"
    exit 1
fi

# 检查是否安装了nginx
# if ! command -v nginx &> /dev/null; then
#     echo "nginx未安装"
#     exit 1
# elif ! -f /usr/local/nginx/sbin/nginx && ! -f /usr/sbin/nginx; then
#     echo "nginx未安装"
#     exit 1
# fi

# 定义源文件和目标文件路径
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SRC_NGINX_CONF="${SCRIPT_DIR}/nginx.conf"
DST_NGINX_CONF="/usr/local/nginx/conf/nginx.conf"
SRC_HTML_FILE="${SCRIPT_DIR}/index.html"
DST_HTML_FILE="/usr/local/nginx/html/index.html"

# 检查源文件是否存在
if [ ! -f "$SRC_NGINX_CONF" ]; then
    echo "错误：源文件 ${SRC_NGINX_CONF} 不存在"
    exit 1
fi

if [ ! -f "$SRC_HTML_FILE" ]; then
    echo "错误：源文件 ${SRC_HTML_FILE} 不存在"
    exit 1
fi

# 确保目标目录存在
mkdir -p "$(dirname "$DST_NGINX_CONF")"
mkdir -p "$(dirname "$DST_HTML_FILE")"

# 配置 nginx.conf
echo "正在配置 nginx.conf..."
if [ -f "$DST_NGINX_CONF" ]; then
    if ! diff -q "$SRC_NGINX_CONF" "$DST_NGINX_CONF" > /dev/null; then
        echo "检测到 nginx.conf 文件不一致"
        echo "是否覆盖？(y/n)"
        read -p "请输入：" answer
        if [ "$answer" == "y" ]; then
            cp "$SRC_NGINX_CONF" "$DST_NGINX_CONF"
            echo "nginx.conf 已更新"
        else
            echo "nginx.conf 更新已取消"
            exit 1
        fi
    else
        echo "nginx.conf 无需更新"
    fi
else
    cp "$SRC_NGINX_CONF" "$DST_NGINX_CONF"
    echo "nginx.conf 已创建"
fi

# 配置 index.html
echo "正在配置 index.html..."
if [ -f "$DST_HTML_FILE" ]; then
    if ! diff -q "$SRC_HTML_FILE" "$DST_HTML_FILE" > /dev/null; then
        echo "检测到 index.html 文件不一致"
        echo "是否覆盖？(y/n)"
        read -p "请输入：" answer
        if [ "$answer" == "y" ]; then
            cp "$SRC_HTML_FILE" "$DST_HTML_FILE"
            echo "index.html 已更新"
        else
            echo "index.html 更新已取消"
            exit 1
        fi
    else
        echo "index.html 无需更新"
    fi
else
    cp "$SRC_HTML_FILE" "$DST_HTML_FILE"
    echo "index.html 已创建"
fi

# 检查nginx配置是否正确
echo "正在检查nginx配置..."
if ! nginx -t; then
    echo "nginx配置检查失败"
    exit 1
fi

# 重启 nginx
echo "正在重启nginx服务..."
if command -v systemctl &> /dev/null; then
    systemctl restart nginx
else
    /usr/local/nginx/sbin/nginx -s reload
fi

if [ $? -ne 0 ]; then
    echo "重启nginx失败"
    exit 1
fi

echo "配置完成！"
echo "nginx已成功重启并应用新配置"
