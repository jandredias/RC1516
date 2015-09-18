#include "SocketTCP.h"
#include "SocketUDP.h"
#include <iostream>
#include <string>

#define __PORT__ 59023

int main(){

  std::cout << "creating socket" << std::endl;

  SocketUDP socket(__PORT__);

  std::cout << "listening on port " << __PORT__ << std::endl;

  while(1){
    std::string message = socket.receive();
    std::cout << message << std::endl;
    std::string output = std::string("your input has ") + std::to_string(message.size()) \
    + std::string(" characters") + "\0";


    socket.send(output);
  }
  return 0;
}
