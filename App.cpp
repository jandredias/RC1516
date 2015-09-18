#include "SocketTCP.h"
#include "SocketUDP.h"
#include <iostream>
#include <string>

#define __PORT__ 59023

int main(){
   try{
    SocketUDP socket("simpson", __PORT__);
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