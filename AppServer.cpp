#include "SocketTCP.h"
#include "SocketUDP.h"
#include <iostream>
#include <string>

#define __PORT__ 58023
#define __PROTOCOL_TCP__

#ifndef __PROTOCOL_TCP__ //It will be tested UDP
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

#endif
#ifndef __PROTOCOL_UDP__ //It will be tested TCP

int main(){
  std::cout << "creating socket" << std::endl;
  SocketTCP socket(__PORT__);
  std::cout << "listening on port " << __PORT__ << std::endl;
  socket.listen(10);
  while(1){
    std::cout << "waiting for client" << std::endl;
    SocketTCP newSocket = socket.accept();
    std::cout << "client accepted" << std::endl;
    if(fork() == 0){
      socket.disconnect();
      std::cout << "child" << std::endl;
      std::string message = newSocket.read();
      std::cout << message << std::endl;
      std::string output = std::string("your input has ") + std::to_string(message.size()) \
      + std::string(" characters") + '\n';


      newSocket.write(output);
      return 0;
    }
    std::cout << "parent closed socket" << std::endl;
    newSocket.disconnect();
  }
  return 0;
}

#endif
