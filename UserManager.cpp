#include "UserManager.h"

#include "SocketUDP.h"
#include "SocketTCP.h"
#include <sstream>
UserManager::UserManager(int sid, int port, std::string ecpname) : _sid(sid), _port(port),
_ecpname(ecpname){
}

void UserManager::list(){
  SocketUDP s = SocketUDP(_ecpname.data(), _port);
  s.send("TQR\n");
  std::stringstream stream(s.receive());
  std::string code;
  stream >> code;
  int nt;
  stream >> nt;
  std::cout << code << " " << nt << std::endl;

}

void UserManager::request(){
  //TODO
  std::string hostname = "localhost";
  int port = 59000;

  std::cout << "Creating socket" << std::endl;

  SocketTCP tes(hostname.data(), port);

    std::cout << "Socket created" << std::endl;
    std::cout << "Connecting..." << std::endl;
    tes.connect();

      std::cout << "Connected!" << std::endl;

        std::cout << "Writing..." << std::endl;
  tes.write("TER 01\n");

    std::cout << "Written" << std::endl;

      std::cout << "Reading" << std::endl;
  std::cout << tes.read() << std::endl;

    std::cout << "Read" << std::endl;
  tes.disconnect();
}

void UserManager::submit(){
  //TODO
}
