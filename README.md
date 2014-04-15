## 运行环境 ##
* ubuntu13.04

## 所需库与软件 ##
* gcc (测试版本>=4.7)
* libnetfilter_queue (版本:0.0.17) [下载网址](http://ftp.netfilter.org/pub/libnetfilter_queue)
* libnfnetlink (版本:0.0.41) [下载网址](http://www.netfilter.org/projects/libnfnetlink/downloads.html)
* ip_queue.h (已经淘汰,但是这里要用) [下载网址]()

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