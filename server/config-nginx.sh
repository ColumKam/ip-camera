#!/bin/bash

# 检查是否以root用户运行
if [ "$EUID" -ne 0 ]; then
    echo "请使用root用户运行此脚本"
    exit 1
fi

# 检查是否安装了nginx
if ! command -v nginx &> /dev/null; then
    echo "nginx未安装"
    exit 1
fi

DST_NGINX_CONF="/usr/local/nginx/conf/nginx.conf"

# 迁移 nginx.conf 到 /usr/local/nginx/conf/nginx.conf
if [ -f "nginx.conf" ]; then
    # 检查 /usr/local/nginx/conf/nginx.conf 与 nginx.conf 是否一致
    if [ -f "$DST_NGINX_CONF" ]; then
        if ! diff -q nginx.conf $DST_NGINX_CONF; then
            echo "nginx.conf 文件不一致"
            echo "是否覆盖？(y/n)"
            read -p "请输入：" answer
            if [ "$answer" == "y" ]; then
                cp nginx.conf $DST_NGINX_CONF
            else
                echo "覆盖 nginx.conf 已取消"
                exit 1
            fi
        fi
    else
        echo "nginx.conf 文件不存在"
        exit 1
    fi
fi

DST_HTML_FILE="/usr/local/nginx/html/index.html"
# 拷贝 index.html 到 /usr/local/nginx/html/index.html
if [ -f "index.html" ]; then
    if [ -f "$DST_HTML_FILE" ]; then
        if ! diff -q index.html $DST_HTML_FILE; then
            echo "index.html 文件不一致"
            echo "是否覆盖？(y/n)"
            read -p "请输入：" answer
            if [ "$answer" == "y" ]; then
                cp index.html $DST_HTML_FILE
            else
                echo "覆盖 index.html 已取消"
                exit 1
            fi
        fi
    else
        echo "index.html 文件不存在"
        exit 1
    fi
else
    echo "index.html 文件不存在"
    exit 1
fi

# 重启 nginx
nginx -s reload
if [ $? -ne 0 ]; then
    echo "重启 nginx 失败"
    exit 1
fi

echo "配置完成"
