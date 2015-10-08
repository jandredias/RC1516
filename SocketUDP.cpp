#include "SocketUDP.h"
#include "Debug.h"
#include "Exception.h"
SocketUDP::SocketUDP() : _server(true){}

SocketUDP::SocketUDP(const char addr[], int port) : _port(port), _server(false){

  _fd = socket(AF_INET, SOCK_DGRAM, 0);
  if(_fd < 0) throw std::string("SocketUDP::SocketUDP ").append(strerror(errno));

  _hostptr = gethostbyname(addr);

  debug("official name: " + std::string(_hostptr->h_name));
  debug("internet address: " +
        std::string(inet_ntoa(* (struct in_addr*) _hostptr->h_addr_list[0])) + " " +
        std::to_string(ntohl(((struct in_addr *) _hostptr->h_addr_list[0])->s_addr)));

  memset((void*) &_serverAddr,(int) '\0', sizeof(_serverAddr));
  _serverAddr.sin_family      = AF_INET;
  _serverAddr.sin_addr.s_addr = ((struct in_addr *) _hostptr->h_addr_list[0])->s_addr;
  _serverAddr.sin_port        = htons((u_short) port);

  _clientLen = sizeof(_clientAddr);
  _serverLen = sizeof(_serverAddr);
}

SocketUDP::SocketUDP(int port) : _port(port),  _server(true){
  _fd = socket(AF_INET, SOCK_DGRAM, 0);
  if(_fd < 0) throw std::string("SocketUDP::SocketUDP ").append(strerror(errno));

  memset((void*) &_serverAddr,(int)'\0', sizeof(_serverAddr));
  _serverAddr.sin_family      = AF_INET;
  _serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
  _serverAddr.sin_port        = htons((u_short) port);

  if(bind(_fd, (struct sockaddr*) &_serverAddr, sizeof(_serverAddr)) < 0){
    if(errno == EADDRINUSE) throw SocketAlreadyInUse("UDP");
    else throw std::string("SocketUDP::SocketUDP ").append(strerror(errno));
  }
}

void SocketUDP::send(std::string text){
  //if(text[text.size() - 1] != '\n') text += '\n';
  sendto(_fd, text.data(), text.size(), 0, (struct sockaddr*) &_serverAddr, sizeof(_serverAddr));
}
std::string SocketUDP::port(){
  return std::to_string(_port);

}
std::string SocketUDP::receive(int flags){
  char buffer[BUFFER_SIZE];
  _serverLen = sizeof(_serverAddr);

  int n = recvfrom(_fd, buffer, BUFFER_SIZE, flags, (struct sockaddr*) &_serverAddr, &_serverLen);
  if(n >= BUFFER_SIZE) throw MessageTooLongUDP();
  else if(n == -1) throw std::string("SocketUDP::receive ").append(strerror(errno));

  std::string msg(buffer);
  int pos = msg.find('\n');
  if(pos == std::string::npos) throw UnknownFormatProtocol();
  return msg.substr(0, pos);
}

void SocketUDP::close(){
  if(::close(_fd) == -1) throw std::string("SocketUDP::close ").append(strerror(errno));
}

std::string SocketUDP::ip(){
  return std::string(inet_ntoa(((struct sockaddr_in *) &_serverAddr)->sin_addr));
}

std::string SocketUDP::hostname(){
  return std::string();
}

void SocketUDP::timeout(int ms){
  struct timeval tv;
  tv.tv_usec = ms % 1000;
  tv.tv_sec = (ms - (ms % 1000)) / 1000;

  if(setsockopt(_fd, SOL_SOCKET, SO_RCVTIMEO, &tv,sizeof(tv)) < 0)
    throw std::string("SocketUDP::setTimeOut ").append(strerror(errno));
}


struct sockaddr_in SocketUDP::client(){ return _serverAddr; }
void SocketUDP::client(struct sockaddr_in client){ _serverAddr = client; }
