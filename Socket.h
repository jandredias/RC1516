#pragma once

#include <sys/un.h>
#include <unistd.h>

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#include <iostream>   // std::cout
#include <string>     // std::string, std::to_string

class Socket{
  int _socketSize;                ///Struct size
  int _socketfd;                  ///Socket file descriptor
  struct sockaddr_un _socketName; ///Struct that represents socket name

  bool _connected;


  socklen_t _dim_cli;


  int _sockfd;                    ///File descriptor
  bool _internet;                 ///true if socket has internet domain

  ///struct will be used in unix sockets
  struct sockaddr_un _endServUn, _endCliUn;
  int _sizeofServUn;

  ///struct will be used in internet sockets
  struct sockaddr_in _endServIn, _endCliIn;
  struct hostent *_hostpointer;
  int _sizeofServIn;
  ///std::string filename: used if socket has unix domain
  std::string _fileName;

  ///port and ip used it socket has internet domain
  int _port;
  std::string _hostname;
  Socket(){}
  void fd(int x){ _sockfd = x; }
public:
  /**
   * @description  Socket has internet domain, and it will be connected to a
   *               computer in the network identified by ip and port
   * @param int port
   */
  Socket(int port) : _connected(false), _internet(true), _port(port) {
    _sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if(_sockfd < 0)
     throw std::string("error opening socket");

    bzero((char*) &_endServIn, sizeof(_endServIn));
    _endServIn.sin_family = AF_INET;
    _endServIn.sin_port = htons(_port);
    _endServIn.sin_addr.s_addr = htonl(INADDR_ANY);
    _sizeofServIn = sizeof(_endServIn);
    if(bind(_sockfd, (struct sockaddr *) &_endServIn, _sizeofServIn)){
      perror("error binding socket");
      throw std::string("error binding socket");
    }
  }
  /**
   * @description  Socket has internet domain, and it will be connected to a
   *               computer in the network identified by ip and port
   * @param int port
   * @param int ip
   */
  Socket(std::string hostname, int port) : _connected(false),
    _internet(true), _port(port), _hostname(hostname){

    _sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(_sockfd < 0) throw std::string("error opening socket");

    bzero((char*) &_endServIn, sizeof(_endServIn));
    //HOSTNAME VS IP
    //_hostpointer = gethostbyname(hostname.data());

    if(_hostpointer == NULL){
      perror("error getting host by name");
      throw std::string("error getting host by name");
    }
    _endServIn.sin_family = AF_INET;
    _endServIn.sin_port = htons((u_short) _port);

    //HOSTNAME VS IP
    //_endServIn.sin_addr.s_addr = ((struct in_addr *) (_hostpointer->h_addr_list[0]))->s_addr;
    _endServIn.sin_addr.s_addr = inet_addr(hostname.data());
    _sizeofServIn = sizeof(_endServIn);
  }

  /**
   * @description Socket has UNIX domain, and it will be connected to a process
                  in the same computer
   * @param std::string path
   * @param bool server true if Socket is being created in server side
   */
  Socket(std::string path, bool server = false) : _connected(false),
    _internet(false), _fileName(path){

    //Create stream socket
    _sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if(_sockfd < 0) throw std::string("Error creating socket");

    //Delete file or socket previously existent
    if(server) unlink(UNIXSTR_PATH);

    //cleaning struct that will contain socket name
    bzero ((char*) &_endServUn,sizeof(_endServUn));

    //config socket: name + type
    _endServUn.sun_family = AF_UNIX;
    strcpy(_endServUn.sun_path, _fileName.data());

    _sizeofServUn = strlen(_endServUn.sun_path) + sizeof(_endServUn.sun_family);
    if(server){
      if(::bind(_sockfd,(struct sockaddr *) &_endServUn, _sizeofServUn) < 0)
        throw std::string("error binding socket");
    }
  }
  /**
   * @return int file descriptor
   */
  int fd(){ return _sockfd; }
  /**
   * @description connects to socket
   */
  void connect(){
    if(_internet){
          if(::connect(_sockfd,(struct sockaddr *) &_endServIn, sizeof(_endServIn)) < 0){
      perror("error connectiong internet socket");
      throw std::string("error connecting internet socket");
      }
    }else{
      if(::connect(_sockfd,(struct sockaddr *) &_endServUn, _sizeofServUn) < 0)
        throw std::string("error connecting socket");
      }
    _connected = true;
  }
  void disconnect(){
    if(::close(_sockfd))
      perror("Error closing socket");
    _connected = false;
  }
  void send(std::string text){
    if(!_connected) throw std::string("Socket is not connected");
    ::write(_sockfd, text.data(), text.size() + 1);
  }

  void listen(int max = 5){ ::listen(_sockfd, max); }
  /**
   * @return int file descriptor
   */
  Socket accept(){
    int newSocket = -1;
    socklen_t sizeofClient;
    std::cout << "I'm waiting for client" << std::endl;
    if(_internet){
      newSocket = ::accept(_sockfd, (struct sockaddr*) &_endCliIn, &sizeofClient);
      if(newSocket < 0) throw std::string("error creating dedicate connection");
    }else{
      newSocket = ::accept(_sockfd, (struct sockaddr*) &_endCliUn, &sizeofClient);
      if(newSocket < 0){
        perror("error creating dedicate connection");
        throw std::string("error creating dedicate connection");
      }
    }
    std::cout << newSocket;
    Socket s = Socket();
    s.fd(newSocket);
    return s;
  }
  std::string read(){
    if(!_connected) throw std::string("Socket is not connected");
    std::string text = "";
    int n;
    char b;
    while(1){
      n = ::read(_sockfd, &b, 1);
      if(n == 1) text += b;
      else if(n == 0) break;
      else if(n == -1) perror("error reading from socket server");
    }
    return text;
  }
};
