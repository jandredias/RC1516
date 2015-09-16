#include "SocketTCP.h"
#include <iostream>
#include <string>

int main(){
  SocketTCP socket("jdiastk.com", 59000);
  std::string message;
  try{
    char msg[1024];
    socket.connect();
    std::cin.getline(msg, 1024);

    message = msg;
    std::cout << "Sending message" << std::endl;
    socket.write(message);
    std::cout << "Message sent" << std::endl;
    std::cout << "Reading message" << std::endl;

    //std::cout << "Message Received" << std::endl;
    //std::cout << message << std::endl;

  }catch(std::string s){
    std::cout << s << std::endl;
  }
  message = socket.read();
  socket.disconnect();
  return 0;
}
