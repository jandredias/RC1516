#include "SocketTCP.h"
#include <iostream>
#include <string>

int main(){
  SocketTCP socket("localhost", 59000);
  std::string message;
  try{
    char msg[1024];
    socket.connect();
    std::cin.getline(msg, 1024);

    message = msg;
    socket.write(message);

    message = socket.read();
    std::cout << message << std::endl;
    socket.disconnect();
  }catch(std::string s){
    std::cout << s << std::endl;
  }
  return 0;
}
 
