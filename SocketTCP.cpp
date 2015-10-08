#include "SocketTCP.h"
#include "Debug.h"
#include "Dialog.h"
#include "Exception.h"

SocketTCP::SocketTCP(){}
SocketTCP::SocketTCP(int fd, struct sockaddr_in client) : _fd(fd), _clientAddr(client),
  _server(true), _connected(true){}
SocketTCP::SocketTCP(const char addr[], int port) : _server(false), _connected(false){
  _fd = socket(AF_INET, SOCK_STREAM, 0);

  if(_fd < 0) throw TCPCreating(strerror(errno));

  _hostptr = gethostbyname(addr);
  if(_hostptr == NULL) throw TCPCreating("SocketTCP::SocketTCP error getting host by name");

  #if DEBUG
    std::cout << "official name: " << _hostptr->h_name << std::endl;
    std::cout << "internet address: "
              << inet_ntoa(* (struct in_addr*) _hostptr->h_addr_list[0]) << " "
              << ntohl(((struct in_addr*) _hostptr->h_addr_list[0])->s_addr) << std::endl;
  #endif

  memset((void *) &_serverAddr, (int) '\0', sizeof(_serverAddr));

  _serverAddr.sin_family = AF_INET;
  _serverAddr.sin_port = htons((u_short) port);
  _serverAddr.sin_addr.s_addr = ((struct in_addr *) (_hostptr->h_addr_list[0]))->s_addr;

  int ms = 5000;
  struct timeval tv;
  tv.tv_usec = ms % 1000;
  tv.tv_sec = (ms - (ms % 1000)) / 1000;

  if(setsockopt(_fd, SOL_SOCKET, SO_RCVTIMEO, &tv,sizeof(tv)) < 0)
    throw std::string("SocketUDP::setTimeOut ").append(strerror(errno));
}

SocketTCP::SocketTCP(int port) : _server(true), _connected(false){
  _fd = socket(AF_INET, SOCK_STREAM, 0);

  memset((void *) &_serverAddr, (int) '\0', sizeof(_serverAddr));

  _serverAddr.sin_family = AF_INET;
  _serverAddr.sin_port = htons((u_short) port);
  _serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
  int optval = 1;
  if(setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval) < 0) //THis line ensures that there is no SOCKETTCP connection refused after Server Crash.
	throw std::string("SocketUDP::setTimeOut ").append(strerror(errno));
  if(bind(_fd, (struct sockaddr*) &_serverAddr, sizeof(_serverAddr)) < 0){
    if(errno == EADDRINUSE) throw SocketAlreadyInUse("TCP");
    else throw std::string("SocketTCP::SocketTCP ").append(strerror(errno));
  }
}

void SocketTCP::fd(int fd){ _fd = fd; }

int SocketTCP::fd(){ return _fd; }

void SocketTCP::connect(){
  if(_server) throw connectOnServer();

  if(::connect(_fd,(struct sockaddr *) &_serverAddr, sizeof(_serverAddr)) < 0)
    throw ErrorConnectingTCP(strerror(errno));
  _connected = true;
}

void SocketTCP::disconnect(){
  if(::close(_fd))
    throw DisconnectingTCP(strerror(errno));
  _connected = false;
}
void SocketTCP::write(const char* text, int size){
  const char *ptr = text;
  int left = size;

  if(!_connected) throw std::string("Socket is not connected");

  while(left > 0){
    int written = ::send(_fd, ptr, left,MSG_NOSIGNAL);
    if(written < -1){
      if(errno == EPIPE)
        throw std::string("DISCONNETED");
      throw std::string("SocketTCP::write ").append(strerror(errno));
    }
    left -= written;
    ptr += written;
  }
}

void SocketTCP::write(char* text, int size){
  char *ptr = text;
  int left = size;

  if(!_connected) throw std::string("SocketTCP::write: Socket is not connected");

  #if DEBUG
  UI::Dialog::IO->print(std::string("LEFT: "));
  UI::Dialog::IO->println(std::to_string(left));
  #endif

  while(left > 0){

    #if DEBUG
    UI::Dialog::IO->println(std::string("Writing to socket"));
    #endif

    int written = ::send(_fd, ptr, left,MSG_NOSIGNAL);

    if(written < -1){
      if(errno == EPIPE)
        throw std::string("DISCONNETED");
      throw std::string("SocketTCP::write ").append(strerror(errno));
    }
    #if DEBUG
    UI::Dialog::IO->print(std::string("Chars written to socket: "));
    UI::Dialog::IO->println(std::to_string(written));
    #endif

    left -= written;
    ptr += written;
  }
}
void SocketTCP::write(std::string text){
  write(text.data(),text.size());
}
void SocketTCP::write(const char c){
  write(&c, 1);
}
char* SocketTCP::read(int x){
  if(!_connected) throw std::string("SocketTCP::read Socket is not connected");
  char *buffer = new char[x];
  int read = 0;

  while(x){
    read = ::read(_fd, buffer + read, x - read);
    if(read < 0) throw std::string("SOCKETTCP::read") + strerror(errno);
  }
  return buffer;

}

std::string SocketTCP::plainTextRead(){
  if(!_connected) throw std::string("Socket is not connected");
  std::string text = "";

  int n;
  char b;
  while(1){
    std::cout << "still reading\n";
    n = ::read(_fd, &b, 1);
    if(b == '\0') break;
    else if(n == 1) text += b;
    else if( n == -1) perror("error reading from socket server ");
  }
  return text;
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
    else if( n == -1) perror("error reading from socket server ");
  }
  return text;
}

std::string SocketTCP::readWord(){
  if(!_connected) throw std::string("Socket is not connected ");
  std::string text = "";

  int n;
  char b;
  while(1){
    n = ::read(_fd, &b, 1);
    if(n == 1 && (b != ' ' && b!= '\n' && b!='\t')) text += b;
    else if( n == 1 && (b == ' ' || b== '\n' || b=='\t') ) break;
    else if( n == -1 ) throw std::string("SOCKETTCP::readWord ") + strerror(errno);
  }
  return text;
}

int SocketTCP::rawRead(){
  return _fd;
}

void SocketTCP::listen(int max){ ::listen(_fd, max); }

SocketTCP SocketTCP::accept(){
  int newSocket = -1;

  socklen_t sizeofClient = sizeof(_clientAddr);

  newSocket = ::accept(_fd, (struct sockaddr*) &_clientAddr, &sizeofClient);
  if(newSocket < 0) throw std::string("SOCKETTCP::accept ") + strerror(errno);

  SocketTCP s(newSocket, _clientAddr);
  return s;
}

bool SocketTCP::connected(){ return _connected; }

std::string SocketTCP::ip(){
  struct sockaddr_in *addr_in = (struct sockaddr_in *) &_clientAddr;
  char *s = inet_ntoa(addr_in->sin_addr);
  return std::string(s);
}

std::string SocketTCP::hostname(){
  return std::string();
}
