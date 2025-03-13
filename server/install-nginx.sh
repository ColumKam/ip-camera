#!/bin/bash

# 检查是否以root用户运行
if [ "$EUID" -ne 0 ]; then
    echo "请使用root用户运行此脚本"
    exit 1
fi

# 检查当前系统是ubuntu\Debian 还是CentOS\RedHat\Fedora

os_name=$(lsb_release -i -s)

# 检查安装 libpcre3 libpcre3-dev
if [ ! command -v libpcre3 &> /dev/null || ! command -v libpcre3-dev &> /dev/null ]; then
    if [ "$os_name" == "Ubuntu" || "$os_name" == "Debian" ]; then
        apt-get install -y libpcre3 libpcre3-dev
    elif [ "$os_name" == "CentOS" || "$os_name" == "RedHat" || "$os_name" == "Fedora" ]; then
        yum install -y libpcre3 libpcre3-dev
    else
        echo "当前系统不支持"
        exit 1
    fi
else
    echo "libpcre3 libpcre3-dev已安装"
fi

# 检查安装 openssl libressl-dev
if [ ! command -v openssl &> /dev/null || ! command -v libressl-dev &> /dev/null ]; then
    if [ "$os_name" == "Ubuntu" || "$os_name" == "Debian" ]; then
        apt-get install -y openssl libressl-dev
    elif [ "$os_name" == "CentOS" || "$os_name" == "RedHat" || "$os_name" == "Fedora" ]; then
        yum install -y openssl libressl-dev
    fi
else
    echo "openssl libressl-dev已安装"
fi

# 检查是否安装了nginx
if ! command -v nginx &> /dev/null; then
    echo "nginx未安装，请先安装nginx"
    # 是否通过软件包管理器安装
    echo "是否通过软件包管理器安装"
    read -p "请输入y或n: " install_nginx
    if [ "$install_nginx" == "y" ]; then
        if [ "$os_name" == "Ubuntu" || "$os_name" == "Debian" ]; then
            apt-get update
            apt-get install -y nginx
        elif [ "$os_name" == "CentOS" || "$os_name" == "RedHat" || "$os_name" == "Fedora" ]; then
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
        git clone https://github.com/arut/nginx-rtmp-module/archive/master.zip
        cd nginx-1.27.4
        ./configure --with-http_ssl_module --add-module=../nginx-rtmp-module
        make -j$(nproc)
        make install
    fi
fi
