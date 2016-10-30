#talks
talks is my own IM system, but I only finished the primary tcp part at present.
It can only run at the Linux operating system.<br>
##Build
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
##Test
To run the tcp server, just type:
```shell
./server
```
The default log files about the server is the `server.log` file, if you want to specify the log file, you can run as this:
```shell
./server your_log_file_name
```
And to run the client:
```shell
./client
```
<br>
#talks中文说明
talk是我自己做的一个IM系统，不过仅仅做到了基本的tcp部分，而且只能运行在Linux操作系统上。
##Build
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
##Test
启动tcp server，只需要运行：
```shell
./server
```
这会使用默认的日志文件`server.log`，如果你想指定自己想要的日志文件，你可以按照这样运行：
```shell
./server your_log_file_name
``` 
