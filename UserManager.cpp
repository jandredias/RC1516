#include "UserManager.h"

#include "SocketUDP.h"
#include "SocketTCP.h"
#include <sstream>
#include <unistd.h>
#include "Dialog.h"

#define __MS_BETWEEN_TRIES__ 1000
#define __TRIES__ 10
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

void UserManager::request(int tnn){
  SocketUDP ecp = SocketUDP(_ecpname.data(), _port);
  std::string message;
  for(auto i = 0; i < __TRIES__; i++){
    ecp.send(std::string("TER ") + std::to_string(tnn) + std::string("\n"));
    try{
      ecp.timeout(__MS_BETWEEN_TRIES__);
      message = ecp.receive();
      break;
    }catch(std::string s){
      if(errno == 11){
        UI::Dialog::IO->print(". ");
        UI::Dialog::IO->flush();
        if(i < __TRIES__ - 1)
          continue;
        UI::Dialog::IO->println();
        UI::Dialog::IO->println();
      }else{
        UI::Dialog::IO->println("error sending message to ECP server");
      }
    }
    UI::Dialog::IO->println("Could not connect to ECP Server!");
    UI::Dialog::IO->println("Try again later.");
    return;
  }
  if(message == std::string("EOF")){
    UI::Dialog::IO->println("There is no questionnaire topics available at the moment.");
    UI::Dialog::IO->println("Try again later.");
    return;
  }else if(message == std::string("ERR")){
    UI::Dialog::IO->println("[RED][ERR][REGULAR] There was an error in the communication with the server.");
    UI::Dialog::IO->println("Try again.");
    return;
  }

  std::stringstream stream(message);
  std::string code;
  std::string hostname;
  int port;

  stream >> code;
  if(code != std::string("AWTES")){
    UI::Dialog::IO->println("[RED][ERR][REGULAR] There was an error in the communication with the server.");
    UI::Dialog::IO->println("Try again.");
  }

  stream >> hostname;
  stream >> port;

  if(__DEBUG__) std::cout << "Creating socket" << std::endl;

  SocketTCP tes(hostname.data(), port);

  if(__DEBUG__) std::cout << "Socket created" << std::endl;
  if(__DEBUG__) std::cout << "Connecting..." << std::endl;

  try{
    tes.connect();
  }catch(std::string s){
    UI::Dialog::IO->println(s);
    return;
  }

  if(__DEBUG__) std::cout << "Connected!" << std::endl;
  if(__DEBUG__) std::cout << "Writing..." << std::endl;

  tes.write("TER " + std::to_string(tnn));

  if(__DEBUG__) std::cout << "Written" << std::endl;

  if(__DEBUG__) std::cout << "Reading" << std::endl;

  message = tes.read();

  if(__DEBUG__) std::cout << "Read" << std::endl;
  tes.disconnect();
}

void UserManager::submit(){
  //TODO
}
