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
    std::cout << "Message sent" << std::endl;
    socket.write(message);
    message = socket.read();
    std::cout << message << std::endl;
  }catch(std::string s){
    std::cout << s << std::endl;
  }
  return 0;
}
