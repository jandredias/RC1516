#include "SocketTCP.h"
#include <iostream>
#include <string>

int main(){
  SocketTCP socket(59000);
  socket.listen(10);
  while(1){
    SocketTCP newSocket = socket.accept();

    std::string message = newSocket.read();
    std::cout << message << std::endl;
    std::string output = std::string("your input has ") + std::to_string(message.size()) \
    + std::string(" characters");
    std::cout << output << std::endl;
    newSocket.write(output);
    newSocket.disconnect();
  }
  return 0;
}
