all : echoserver tcpclient

CFLAGS = -g

echoserver : echoserver.cpp InetAddress.h InetAddress.cpp Socket.h Socket.cpp Epoll.h Epoll.cpp Channel.h Channel.cpp EventLoop.h EventLoop.cpp TcpServer.h TcpServer.cpp Acceptor.h Acceptor.cpp Connection.h Connection.cpp Buffer.h Buffer.cpp EchoServer.h EchoServer.cpp ThreadPool.h ThreadPool.cpp Timestamp.h Timestamp.cpp
	g++ $(CFLAGS) -o echoserver echoserver.cpp InetAddress.cpp Socket.cpp Epoll.cpp Channel.cpp EventLoop.cpp TcpServer.cpp Acceptor.cpp Connection.cpp Buffer.cpp EchoServer.cpp ThreadPool.cpp Timestamp.cpp -lpthread

tcpclient : tcpclient.cpp InetAddress.h InetAddress.cpp
	g++ $(CFLAGS) -o tcpclient tcpclient.cpp InetAddress.cpp

clean:
	rm echoserver tcpclient