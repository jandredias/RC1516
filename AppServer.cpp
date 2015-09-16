#include "SocketTCP.h"
#include <iostream>
#include <string>

int main(){
  SocketTCP socket(59800);
  socket.listen();
  while(1){
    SocketTCP newSocket = socket.accept();
    std::string message = newSocket.read();
    std::cout << message << std::endl;
  }
  return 0;
}
