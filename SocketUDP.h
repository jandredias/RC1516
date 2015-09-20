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

#ifndef __DEBUG__
#define __DEBUG__ 0
#endif

class SocketUDP {
  int _fd;
  int _port;
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
  SocketUDP(const char[], int);

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
  std::string receive(int = 0);

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

  /**
   * @reteurn std::string port where the socket is configured
   */
  std::string port();
  /**
   * @description          set timeout while reading messages
   */
  void timeout(int);

  /**
   * @return struct sockaddr_in will return struct that represents the request
   */
  struct sockaddr_in client();

  /**
   * @param struct sockaddr_in struct that represents the request
   */
  void client(struct sockaddr_in);

};
