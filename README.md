# 实验一 #
***文件: lib_1_package_filter_fireware.c***
## 运行环境 ##
* ubuntu13.04

## 所需库与软件 ##
* gcc (测试版本>=4.7)
* libnetfilter_queue (版本:0.0.17) [下载网址](http://ftp.netfilter.org/pub/libnetfilter_queue)
* libnfnetlink (版本:0.0.41) [下载网址](http://www.netfilter.org/projects/libnfnetlink/downloads.html)
* ip_queue.h (已经淘汰,但是这里要用) [下载网址](https://gist.github.com/shangbo/11027423)

## 执行步骤 ##
1. 编译安装libnfnetlink
    1. 解压libnfnetlink.tar.gz
    2. 进入解压的文件夹
    3. 执行./configure
    4. 执行 make
    5. 执行 sudo make install
  
2. copy ip_queue.h 到指定位置  
```
    cp ip_queue.h /usr/include/linux/netfilter_ipv4/
```
  
3. 编译安装libnetfilter_queue
    1. 解压libnetfilter.tar.gz
    2. 进入解压文件夹
    3. 执行./configure
    4. 执行 make
    5. 执行 sudo make install

4. 编译lib_1_package_filter_firware.c (书上的有错.请按照我的命令编译)
```
    gcc lib_1_package_filter_firware.c -lnetfilter_queue -lnfnetlink -o fireware.out 
```


5. 使用超级管理员权限执行
```
    sudo iptables -A OUTPUT -j QUEUE
```

6. 其余请按照教材《信息安全技术解析与开发实践》来执行

# 实验二 #
***文件: lib_2_app_proxy_fireware.c***

## 运行环境 ##
* ubuntu13.04

## 所需组件 ##
* linux下chrome浏览器
    sudo apt-get install chromium-browser
* 安装完成后，在teminal下面输入chromium-browser打开浏览器
* chrome插件- vpn.s HTTP Proxy(去chrome应用商店里面去搜索)



## 编译 ##

    gcc lib_2_app_proxy_fireware.c -lpthread -o lib_2.out
    //会报一些warning,但是问题不大

## 配置代理 ##
1. 打开vpn.s http proxy options
2. 在profile里面新建一个规则,名字为local agent
3. manual configuration下面的HTTP Proxy配置为 127.0.0.1 ，port 配置为8080
4. 勾选 use the same proxy server for all protocols
5. 保存
6. 使你刚刚新建的规则运行 

## 运行 ##

* 运行lib_2.out  
```
./lib2.out -p 8080
```

* 此时你将无法访问人人网(随便选的)

## 修改 ##

* 修改将要过滤的网站，打开源文件，修改 BLACKED_SERVER 宏定义 line14
* 修改允许通过的ip地址，修改ALLOWD_CLIENTIP 内容 line16

###Over###


    



