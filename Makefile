all:	client server

server:	udpServer.c
	gcc udpServer.c -o server
client:	clientUDP.c
	gcc clientUDP.c -o client
clean:
	rm client server
