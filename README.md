# TFTPClinet

完成一个TFTP客户端程序。TFTP是一种简单的文件传输协议。目标是在UDP之上上建立一个类似于FTP的但仅支持文件上传和下载功能的传输协议

+ [x] Socket

+ [x] 严格按照TFTP协议与标准TFTP服务器通信；

+ [x] 能够实现两种不同的传输模式netascii和octet；

+ [x] 能够将文件上传到TFTP服务器；

+ [x] 能够从TFTP服务器下载指定文件；

+ [x] 能够向用户展现文件操作的结果：文件传输成功/传输失败；

+ [ ] 针对传输失败的文件，能够提示失败的具体原因；

+ [ ] 能够显示文件上传与下载的吞吐量；

+ [ ] 能够记录日志，对于用户操作、传输成功，传输失败，超时重传等行为记录日志；

+ [x] 人机交互友好（图形界面/命令行界面均可）；

## TFTP协议

TFTP协议定义了五种类型的数据包

+ 读文件请求包：Read Request，简写RRQ

+ 写文件请求包：Write Request，简写WRQ

+ 文件数据包：Data，简写DATA

+ 回应包：Acknowledgement，简写ACK

+ 错误信息包：ERROR，简写ERROR

## TFTPD32

> https://blog.csdn.net/mango_girl/article/details/45332095

+ 注意把文件放到tftpd64.exe同目录下可上传/下载

## Socket

> https://www.cnblogs.com/skyfsm/p/6287787.html

## 实现

> https://github.com/ideawu/tftpx/blob/master/client.c

> https://github.com/ideawu/tftpx/blob/master/tftpx.h

> https://blog.csdn.net/dadizhiying1215/article/details/8546927

+ 对于packet部分字段以0结尾，不是'0'

  ```c
  sprintf(snd->req, "%s%c%s%c", tc->file_name, 0, tc->mode, 0);
  ```

  来构造request

+ 注意htons()和ntohs()对**网络字节序**(从高到低的顺序存储，网络上的统一约定)和本地字节序的转换

+ ack的blocknum与data 的 相对应

## 使用

+ 操作系统： Ubuntu 20.04   4.19.104-microsoft-standard

+ tftp服务端：tftpd64.exe

+ tftp客户端：

  + 头文件：Client.h

  + Client.cpp 

+ 对Client.cpp编译后，`./Client`按交互信息提示输入即可

+ Tips:
  + 默认传输端口:69，可以在Client.cpp的main()中的port4addr[]进行修改
  + 需要上传的文件需与可执行文件Client同目录，**下载文件需与tftpd64.exe同目录**