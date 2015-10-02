#pragma once

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>

#include <sys/un.h>
#include <stdio.h>
#include <arpa/inet.h>

#include <iostream>   // std::cout
#include <string>     // std::string, std::to_string

#ifndef DEBUG
#define DEBUG 0
#endif

class SocketTCP{
  int _fd;
  struct hostent *_hostptr;
  struct sockaddr_in _serverAddr;
  struct sockaddr_in _clientAddr;
  int clientLen;
  bool _server;
  bool _connected;


public:

  /**
   * @description           constructor used to accept new clients while
                            listening on server side
   */
  SocketTCP() : _server(true), _connected(true){}
  
  /**
   * @description           this constructor will be used by clients to connect
   *                        to a server socket
   * @param const char[]    server's hostname
   * @param int             server's port
   */
  SocketTCP(const char[], int);

  /**
   * @description           this constructor will be used by servers to create
   *                        a socket and wait for clients to connect
   * @param int             server's port that will be listening
   */
  SocketTCP(int);

  /**
   * @description           set socket's file descriptor
   *                        this will be used when a client's connection
   *                        accepted
   * @param int             client's socket fd
   */
  void fd(int fd);

  /**
   * @return int file descriptor
   */
  int fd();

  /**
   * @description           connects to socket
   *                        if a server side socket tries to connect to some
   *                        other socket it will throw an exception
   *                        only client side should connect!
   * @throws std::string
   */
  void connect();

  /**
   * @description           closes the socket
   *                        on success returns void
   *                        on failure throws an exception
   * @throws std::string
   */
  void disconnect();

  /**
   * @return                boolean that states if socket is connected
   */
  bool connected();
  /**
   * @param std::string
   */
  void write(std::string);
  void write(char*, int);
  void write(const char*, int);

  std::string read();
  char * read(int);
  std::string readWord();
  int rawRead();
  void listen(int = 5);

  /**
   * @return int file descriptor
   */
  SocketTCP accept();


  std::string ip();

  std::string hostname();
};
