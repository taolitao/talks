# talks
talks is my own IM system, but I only finished the primary tcp part at present.
It can only run at the Linux operating system.

## Build
First, you need to clone the project:
```shell
git clone https://github.com/light8lee/talks.git
cd talks/tcp/
```
To make the file, make sure you have installed `gcc` and `make`. If you want to make the server, just run at command line:
```shell
make server
```
Or if you want to make the client:
```shell
make client
```
You can also do:
```shelll
make all
```
to get both server and client at one time.

## Test
To run the server, just type:
```shell
./server [-p LISTEN_PORT] [-l LOG_FILE_NAME] [-s MAX_SIZE]
```
And to run the client:
```shell
./client [-p SERVER_PORT] [-t RUN_TIMES] [-u USER_NAME]
```

## Example
```shell
./server -p 28888 -s 2
```
It means listen at port:28888, allow at most 2 client connecting, and use default log file 'server.log'. The default port is 26666 and the default MAX_SIZE is 2. Make sure you have the port enabled.
```shell
./client -t 5 -u "Tom"
```
It means use default server port:26666, client user name is `Tom` and run 5 times.
<br>
# talks中文说明
talk是我自己做的一个IM系统，不过仅仅做到了基本的tcp部分，而且只能运行在Linux操作系统上。

## Build
首先从github上获取该项目，
```shell
git clone https://github.com/light8lee/talks.git
cd talks/tcp/
```
在构建项目之前，请确保你已经安装了`gcc`和`make`。如果想获得server的可执行文件，使用如下命令：
```shell
make server
```
如果要得到client的可执行文件，使用如下命令：
```shell
make client
```
当然你也可以通过输入：
```shelll
make all
```
同时得到server和client。

## Test
启动server，只需要运行：
```shell
./server [-p LISTEN_PORT] [-l LOG_FILE_NAME] [-s MAX_SIZE]
```
运行client：
```shell
./client [-p SERVER_PORT] [-t RUN_TIMES] [-u USER_NAME]
```

## Example
```shell
./server -p 28888 -s 2
```
这个例子中，server监听端口28888（默认使用的是26666），最多同时允许两个client连接，并且使用默认的日志文件'server.log'。
```shell
./client -t 5 -u "Tom"
```
这个例子中，client连接的server监听的端口使用默认值26666， client用户名为Tom，连续向server发送5次消息（每次时间间隔递增）。
