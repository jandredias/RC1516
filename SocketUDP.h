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

#define BUFFER_SIZE 2048

#define __DEBUG__ 0

class SocketUDP {
  int _fd;
  struct hostent *_hostptr;
  struct sockaddr_in _serverAddr;
  struct sockaddr_in _clientAddr;
  socklen_t _serverLen;
  int _clientLen;

  bool _server;

public:

  /**
   * //FIXME
   */
  SocketUDP();

  /**
   * @description             SocketUDP for clientside
   * @param const char[]      Server hostname or ip dot notation
   * @param int               port where the socket will read and send data to
   */
  SocketUDP(const char[], int );

  /**
   * @description             SocketUDP for serverside
   * @param int               port where the socket will read and send data to
   */
  SocketUDP(int);
  /**
   * @param  std::string   string text that will be send to the socket
   */
  void send(std::string);

  /**
   * @return std::string  string read from the socket
   */
  std::string receive();

  /**
   * @description   closes the socket
   *                on success nothing will be return
   *                on failure it will throw a std::string exception
   */
  void close();

  /**
   * @return std::string    returns an ip on dot notation from the client
   */
  std::string ip();

  /**
   * @return std::string    returns client's hostname
   */
   std::string hostname();
};
