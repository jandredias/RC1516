#include "SocketTCP.h"

SocketTCP::SocketTCP(const char addr[], int port) : _server(false), _connected(false){
  _fd = socket(AF_INET, SOCK_STREAM, 0);
  if(_fd < 0) throw std::string("couldn't create socket");
  _hostptr = gethostbyname(addr);

  std::cout << "official name: " << _hostptr->h_name << std::endl;
  std::cout << "internet address: "
            << inet_ntoa(* (struct in_addr*) _hostptr->h_addr_list[0]) << " "
            << ntohl(((struct in_addr*) _hostptr->h_addr_list[0])->s_addr) << std::endl;

  memset((void *) &_serverAddr, (int) '\0', sizeof(_serverAddr));

  _serverAddr.sin_family = AF_INET;
  _serverAddr.sin_port = htons((u_short) port);
  _serverAddr.sin_addr.s_addr = ((struct in_addr *) (_hostptr->h_addr_list[0]))->s_addr;
}

SocketTCP::SocketTCP(int port) : _server(true), _connected(false){
  _fd = socket(AF_INET, SOCK_STREAM, 0);

  memset((void *) &_serverAddr, (int) '\0', sizeof(_serverAddr));

  _serverAddr.sin_family = AF_INET;
  _serverAddr.sin_port = htons((u_short) port);
  _serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

  bind(_fd, (struct sockaddr*) &_serverAddr, sizeof(_serverAddr));
}

void SocketTCP::fd(int fd){ _fd = fd; }

int SocketTCP::fd(){ return _fd; }

void SocketTCP::connect(){
  if(_server) throw std::string("error! you can't connect from a server side socket");
  int i = 100;
  while(i)
    if(::connect(_fd,(struct sockaddr *) &_serverAddr, sizeof(_serverAddr)) < 0){
      perror("connect");
      i--;
    }
    else{
      _connected = true;
      return;
    }
  throw std::string("error connecting internet socket") + std::string(strerror(errno));

}

void SocketTCP::disconnect(){
  if(::close(_fd))
    throw std::string("Error closing socket");
  _connected = false;
}

void SocketTCP::write(std::string text){
  const char *ptr = text.data();
  int left = text.size();

  if(!_connected) throw std::string("Socket is not connected");

  while(left > 0){
    int written = ::write(_fd, ptr, left);
    left -= written;
    ptr += written;
  }
}
std::string SocketTCP::read(){
  if(!_connected) throw std::string("Socket is not connected");
  std::string text = "";

  int n;
  char b;
  while(1){
    n = ::read(_fd, &b, 1);
    if(n == 1 && b != '\n') text += b;
    else if( n == 1 && b == '\n' ) break;
    else if( n == -1) perror("error reading from socket server");
  }
  return text;
}

void SocketTCP::listen(int max){ ::listen(_fd, max); }

SocketTCP SocketTCP::accept(){
  int newSocket = -1;

  socklen_t sizeofClient = sizeof(_clientAddr);

  newSocket = ::accept(_fd, (struct sockaddr*) &_clientAddr, &sizeofClient);
  if(newSocket < 0) throw std::string("error creating dedicate connection");

  SocketTCP s = SocketTCP();

  s.fd(newSocket);
  return s;
}
