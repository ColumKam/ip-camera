#!/bin/bash

# 检查是否以root用户运行
if [ "$EUID" -ne 0 ]; then
    echo "请使用root用户运行此脚本"
    exit 1
fi

# 检查当前系统是ubuntu\Debian 还是CentOS\RedHat\Fedora
if [ -f /etc/os-release ]; then
    . /etc/os-release
    OS=$NAME
else
    echo "无法检测操作系统类型"
    exit 1
fi

# 检查安装 libpcre3 libpcre3-dev
if ! dpkg -l | grep -q "^ii  libpcre3 "; then
    if [[ "$OS" == *"Ubuntu"* ]] || [[ "$OS" == *"Debian"* ]]; then
        apt-get install -y libpcre3 libpcre3-dev
    elif [[ "$OS" == *"CentOS"* ]] || [[ "$OS" == *"Red Hat"* ]] || [[ "$OS" == *"Fedora"* ]]; then
        yum install -y pcre pcre-devel
    else
        echo "当前系统不支持"
        exit 1
    fi
else
    echo "libpcre3 libpcre3-dev已安装"
fi

# 检查安装 openssl libressl-dev
if ! dpkg -l | grep -q "^ii  openssl "; then
    if [[ "$OS" == *"Ubuntu"* ]] || [[ "$OS" == *"Debian"* ]]; then
        apt-get install -y openssl libssl-dev
    elif [[ "$OS" == *"CentOS"* ]] || [[ "$OS" == *"Red Hat"* ]] || [[ "$OS" == *"Fedora"* ]]; then
        yum install -y openssl openssl-devel
    fi
else
    echo "openssl libssl-dev已安装"
fi

# 检查是否安装了nginx
if ! command -v nginx &> /dev/null; then
    echo "nginx未安装，请先安装nginx"
    # 是否通过软件包管理器安装
    echo "是否通过软件包管理器安装"
    read -p "请输入y或n: " install_nginx
    if [ "$install_nginx" == "y" ]; then
        if [[ "$OS" == *"Ubuntu"* ]] || [[ "$OS" == *"Debian"* ]]; then
            apt-get update
            apt-get install -y nginx
        elif [[ "$OS" == *"CentOS"* ]] || [[ "$OS" == *"Red Hat"* ]] || [[ "$OS" == *"Fedora"* ]]; then
            yum install -y nginx
        else
            echo "当前系统不支持"
            exit 1
        fi
    else
        echo "通过源码安装"
        # 下载nginx源码
        wget http://nginx.org/download/nginx-1.27.4.tar.gz
        tar -zxvf nginx-1.27.4.tar.gz
        # 下载 nginx-rtmp-module
        git clone https://github.com/arut/nginx-rtmp-module.git
        cd nginx-1.27.4
        ./configure --with-http_ssl_module --add-module=../nginx-rtmp-module
        make -j$(nproc)
        make install
    fi
fi
