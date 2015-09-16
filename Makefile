all:	server client

server:	AppServer.cpp
	g++ AppServer.cpp -o AppServer

client:	App.cpp
	g++ App.cpp -o App
