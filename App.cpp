#include "SocketTCP.h"
#include <iostream>
#include <string>

int main(){
  try{
    char msg[1024];
    SocketTCP socket("jdiastk.com", 59000);
    socket.connect();
    std::cin.getline(msg, 1024);

    std::string message(msg);
    std::cout << "Sending message" << std::endl;
    socket.write(message);
    std::cout << "Message sent" << std::endl;
    std::cout << "Reading message" << std::endl;
    //message = socket.read();
    //std::cout << "Message Received" << std::endl;
    //std::cout << message << std::endl;
    socket.disconnect();
  }catch(std::string s){
    std::cout << s << std::endl;
  }
  return 0;
}
