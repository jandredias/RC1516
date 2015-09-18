#include "SocketTCP.h"
#include "SocketUDP.h"
#include <iostream>
#include <string>

#define __PORT__ 59023
#define __PROTOCOL_TCP__

#ifndef __PROTOCOL_TCP__ //It will be tested UDP
int main(){
   try{
    SocketUDP socket("localhost", __PORT__);
    std::string message;

    char msg[1024];
    std::cin.getline(msg, 1024);

    message = msg;
    socket.send(message);

    message = socket.receive();
    std::cout << message << std::endl;
    socket.close();
  }catch(std::string s){
    std::cout << s << std::endl;
  }
  return 0;
}

#endif
#ifndef __PROTOCOL_UDP__ //It will be tested TCP

int main(){
   try{
      std::cout << "Creating socket" << std::endl;
      SocketTCP socket("localhost", __PORT__);
      std::cout << "Trying to connect" << std::endl;
      std::string msg;
      socket.connect();
      std::cout << "Connected" << std::endl;
      std::getline (std::cin,msg);

      msg += '\n';
      socket.write(msg);

      msg = socket.read();

      std::cout << msg << std::endl;

      socket.disconnect();

  }catch(std::string s){
    std::cout << s << std::endl;
  }
  return 0;
}
#endif
