socket
======
socket is my own im system, but I only finish the primary tcp part at present.
It can only run at the Linux operating system.<br>
##Build
First, you need to clone the project:
```shell
git clone https://github.com/light8lee/socket.git
cd socket/tcp/
```
To make the file, you need first install `gcc` and `make`. If you want to make the server, just run:
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
./server <your log file name>
```
And to run the client:
```shell
./client
```