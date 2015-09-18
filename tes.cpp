#include "SocketTCP.h"
#include "SocketUDP.h"
#include <iostream>
#include <string>
#include <cstdlib>

#define __PORT__ 59000

class TesManager{
  int _port;
  SocketUDP _socket;
public:
  TesManager(int port) : _port(port), _socket(SocketUDP(port)){}
  /**
   * just for testing
   */
  void execute(){
    while(1){
    std::cout << _socket.receive() << std::endl;
    _socket.send("AQT 15\n");
    std::cout << _socket.ip() << std::endl;
    std::cout << "port:    " << _port << std::endl;
  }}
};


int main(int argc, char* argv[]){
  int port = __PORT__;
  if(!(argc == 1 || argc == 3)){ std::cout <<"wrong number of parameteres" << std::endl; return 1; }
  for(int i = 1; i < argc; i += 2){
    if(std::string(argv[i]) != std::string("-p")){
      std::cout << "wrong format of parameters" << std::endl;
    }
    else{
      //should redefine ecp port
      port = atoi(argv[i+1]);
    }
  }

  TesManager manager(port);
  manager.execute();
  return 0;
}
