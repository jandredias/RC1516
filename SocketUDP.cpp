#include "SocketUDP.h"

SocketUDP::SocketUDP() : _server(true){}

SocketUDP::SocketUDP(const char addr[], int port) : _server(false){

  _fd = socket(AF_INET, SOCK_DGRAM, 0);

  if(_fd < 0) throw std::string("couldn't create socket");

  _hostptr = gethostbyname(addr);

  if(__DEBUG__) std::cout << "official name: " << _hostptr->h_name << std::endl;
  if(__DEBUG__) std::cout << "internet address: "
                          << inet_ntoa(* (struct in_addr*) _hostptr->h_addr_list[0]) << " "
                          << ntohl(((struct in_addr *) _hostptr->h_addr_list[0])->s_addr) << std::endl;

  memset((void*) &_serverAddr,(int) '\0', sizeof(_serverAddr));
  _serverAddr.sin_family      = AF_INET;
  _serverAddr.sin_addr.s_addr = ((struct in_addr *) _hostptr->h_addr_list[0])->s_addr;
  _serverAddr.sin_port        = htons((u_short) port);

  _clientLen = sizeof(_clientAddr);
  _serverLen = sizeof(_serverAddr);
}

SocketUDP::SocketUDP(int port) : _server(true){
  _fd = socket(AF_INET, SOCK_DGRAM, 0);

  memset((void*) &_serverAddr,(int)'\0', sizeof(_serverAddr));
  _serverAddr.sin_family      = AF_INET;
  _serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
  _serverAddr.sin_port        = htons((u_short) port);

  bind(_fd, (struct sockaddr*) &_serverAddr, sizeof(_serverAddr));
}

void SocketUDP::send(std::string text){

  if(text[text.size() - 1] != '\n') text += '\n';

  sendto(_fd, text.data(), text.size(), 0, (struct sockaddr*) &_serverAddr, _serverLen);
}

std::string SocketUDP::receive(int flags){
  char buffer[BUFFER_SIZE];
  int n = recvfrom(_fd, buffer, BUFFER_SIZE, flags, (struct sockaddr*) &_serverAddr, &_serverLen);

  if(n == -1) throw std::string("error reading content from the socket");
  std::string msg(buffer);
  int pos = msg.find('\n');
  return msg.substr(0, pos);
}

void SocketUDP::close(){
  if(::close(_fd) == -1) throw std::string();
}

std::string SocketUDP::ip(){
  struct sockaddr_in *addr_in = (struct sockaddr_in *) &_serverAddr;
  char *s = inet_ntoa(addr_in->sin_addr);
  return std::string(s);

  char* ipString = inet_ntoa(_serverAddr.sin_addr);
  std::cout << ipString << std::endl;
  std::cout << errno << std::endl;
}

std::string SocketUDP::hostname(){
  return std::string();
}

void SocketUDP::timeout(int ms){
  struct timeval tv;
  tv.tv_usec = ms % 1000;
  tv.tv_sec = (ms - (ms % 1000)) / 1000;

  if(setsockopt(_fd, SOL_SOCKET, SO_RCVTIMEO, &tv,sizeof(tv)) < 0)
    throw std::string("SocketUDP::setTimeOut ") + std::string(strerror(errno));
}
