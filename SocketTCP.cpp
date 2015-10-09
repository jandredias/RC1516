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

  debug("official name: " + std::string(_hostptr->h_name));
  debug("internet address: " +
        std::string(inet_ntoa(* (struct in_addr*) _hostptr->h_addr_list[0])) + " " +
        std::to_string(ntohl(((struct in_addr*) _hostptr->h_addr_list[0])->s_addr)));

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
  debug("I'm writing using const char");
  const char *ptr = text;
  int left = size;

  if(!_connected) throw std::string("Socket is not connected");

  while(left > 0){
    timeout(100);
    int written = ::write(_fd, ptr, left);
    if(written != 1){ debug(std::to_string(errno)); }
    if(written == 0 || (written == -1 && errno == 104)){
       throw SocketClosed();
    }
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
  debug("I'm writing using char");
  char *ptr = text;
  int left = size;

  if(!_connected) throw std::string("SocketTCP::write: Socket is not connected");


  while(left > 0){

    timeout(500);
    int written = ::write(_fd, ptr, left);
    if(written < 0){
      debug("ERROR: " + std::to_string(errno) + "\n" + strerror(errno));
      if(errno == 11) throw SocketClosed();
      if(errno == EPIPE)
        throw std::string("DISCONNETED");
      throw std::string("SocketTCP::write ").append(strerror(errno));
    }

    left -= written;
    ptr += written;
  }
}
void SocketTCP::write(std::string text){ //Calls void SocketTCP::write(const char*, int)
  debug("I'm writing using std::string");
  write(text.data(),text.size());
}
void SocketTCP::write(const char c){
  debug("I'm writing using single char");write(&c, 1); }








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
std::string SocketTCP::read(){
  if(!_connected) throw std::string("Socket is not connected");
  std::string text = "";

  int n;
  char b;
  timeout(5000);
  bool timeOut = false;
  while(1){
    n = ::read(_fd, &b, 1);
    if(n == 1 && b != '\n') text += b;
    else if( n == 1 && b == '\n' ) break;
    else if( n == -1){
      if(errno == 11){
        throw ConnectionTCPTimedOut();
      }
      debug();
      debug();
      debug("\t\t\tSOCKETUDP::receive ERROR RECEIVING");
      debug(std::to_string(errno));
      debug(strerror(errno));
      debug();
      debug();
      perror("error reading from socket server ");
      sleep(10);
    }
    if(!timeOut){ timeout(500); timeOut = true; }
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

void SocketTCP::timeout(int ms){
  struct timeval tv;
  tv.tv_usec = ms % 1000;
  tv.tv_sec = (ms - (ms % 1000)) / 1000;

  if(setsockopt(_fd, SOL_SOCKET, SO_RCVTIMEO, &tv,sizeof(tv)) < 0)
    throw std::string("SocketTCP::setTimeOut ").append(strerror(errno));
  if(setsockopt(_fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv)) < 0)
    throw std::string("SocketTCP::setTimeOut Sender").append(strerror(errno));
}
