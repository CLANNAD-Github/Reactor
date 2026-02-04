all : client echoserver

CFLAGS = -g
lib = -l pthread

client : client.cpp InetAddress.cpp
	g++ $(CFLAGS) -o client client.cpp InetAddress.cpp

echoserver : echoserver.cpp EchoServer.cpp InetAddress.cpp Socket.cpp Epoll.cpp Channel.cpp EventLoop.cpp TcpServer.cpp Acceptor.cpp Connection.cpp Buffer.cpp ThreadPool.cpp TimeStamp.cpp
	g++ $(CFLAGS) -o echoserver echoserver.cpp EchoServer.cpp InetAddress.cpp Socket.cpp Epoll.cpp Channel.cpp EventLoop.cpp TcpServer.cpp Acceptor.cpp Connection.cpp Buffer.cpp ThreadPool.cpp TimeStamp.cpp $(lib)

clean:
	rm client echoserver
