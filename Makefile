all:	server client

server:	AppServer.cpp
	g++ -std=c++11 AppServer.cpp -o AppServer

client:	App.cpp
	g++ -std=c++11 App.cpp -o App
