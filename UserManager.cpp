#include "UserManager.h"

#include "SocketUDP.h"
#include "SocketTCP.h"
#include <sstream>
#include <unistd.h>
#include "Dialog.h"
#include <math.h>       /* log */
#include <fstream>      // std::ofstream

#define __MS_BETWEEN_TRIES__ 2000
#define __TRIES__ 10
UserManager::UserManager(int sid, int port, std::string ecpname) : _sid(sid), _port(port),
_ecpname(ecpname){
}

void UserManager::list(){
  if(DEBUG) UI::Dialog::IO->println("[ UserManager::list            ] Creating socket");
  SocketUDP ecp = SocketUDP(_ecpname.data(), _port);
  std::string message;
  if(DEBUG) UI::Dialog::IO->println("[ UserManager::list            ] Socket created");
  if(DEBUG) UI::Dialog::IO->println("[ UserManager::list            ] Sending message");
  std::stringstream stream;
  for(auto i = 0; i < __TRIES__; i++){
    ecp.send(std::string("TQR\n"));
    try{
      ecp.timeout(__MS_BETWEEN_TRIES__);
      stream << ecp.receive();
      if(DEBUG) UI::Dialog::IO->println("[ UserManager::list            ] Message sent to ECP");
      if(DEBUG) UI::Dialog::IO->println("[ UserManager::list            ] Receiving message from ECP");
      if(DEBUG) UI::Dialog::IO->println("[ UserManager::list            ] Message received from ECP");
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
    UI::Dialog::IO->println();
    UI::Dialog::IO->println("Could not connect to ECP Server!");
    UI::Dialog::IO->println("Try again later.");
    return;
  }
  if(DEBUG) UI::Dialog::IO->println("[ UserManager::list            ] Processing message");
  std::string code;
  stream >> code;
  if(code == std::string("EOF")){
    UI::Dialog::IO->println("There is no questionnaire available at the moment. Try again later.");
    UI::Dialog::IO->println();
    return;
  }
  int nt;
  stream >> nt;
  std::cout << code << " " << nt << std::endl;

  int numberOfChars = log10(nt) + 1;

  std::string topic;
  int i = 0;
  UI::Dialog::IO->println(std::string("Topics:"));
  while(stream >> topic){
    if(i == 0) UI::Dialog::IO->print(" ");
    UI::Dialog::IO->print(std::string(" ", 1 + numberOfChars - log10(i)));
    UI::Dialog::IO->print(std::to_string(i));
    UI::Dialog::IO->print(std::string(" - "));
    UI::Dialog::IO->println(topic);
    i++;
  }
  if(DEBUG) UI::Dialog::IO->println("[ UserManager::list            ] Message processed");
}

// if(DEBUG) UI::Dialog::IO->println("");

void UserManager::request(int tnn){
  SocketUDP ecp = SocketUDP(_ecpname.data(), _port);
  std::string message;
  if(DEBUG) UI::Dialog::IO->println("[ UserManager::request            ] Socket created");
  if(DEBUG) UI::Dialog::IO->println("[ UserManager::request            ] Sending message");
  std::stringstream stream;
  for(auto i = 0; i < __TRIES__; i++){
    ecp.send(std::string("TER ") + std::to_string(tnn) + std::string("\n"));
    try{
      ecp.timeout(__MS_BETWEEN_TRIES__);
      stream << ecp.receive();
      if(DEBUG) UI::Dialog::IO->println("[ UserManager::request            ] Message sent to ECP");
      if(DEBUG) UI::Dialog::IO->println("[ UserManager::request            ] Receiving message from ECP");
      if(DEBUG) UI::Dialog::IO->println("[ UserManager::request            ] Message received from ECP");
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
    UI::Dialog::IO->println();
    UI::Dialog::IO->println("Could not connect to ECP Server!");
    UI::Dialog::IO->println("Try again later.");
    return;
  }
  stream >> message;
  if(message == std::string("EOF")){
    UI::Dialog::IO->println("There is no questionnaire topics available at the moment.");
    UI::Dialog::IO->println("Try again later.");
    return;
  }else if(message == std::string("ERR")){
    UI::Dialog::IO->println("[RED][ERR][REGULAR] There was an error in the communication with the server.");
    UI::Dialog::IO->println("Try again.");
    return;
  }else if(message != std::string("AWTES")){
    UI::Dialog::IO->println("[RED][ERR][REGULAR] There was an error in the communication with the server.");
    UI::Dialog::IO->println("Try again.");
    return;
  }

  if(DEBUG) UI::Dialog::IO->println("Connecting to TES Server");

  std::string code;
  std::string hostname;
  int port;

  stream >> hostname;
  stream >> port;

  if(DEBUG) std::cout << "Creating socket" << std::endl;

  SocketTCP tes(hostname.data(), port);

  if(DEBUG) std::cout << "Socket created" << std::endl;
  if(DEBUG) std::cout << "Connecting..." << std::endl;

  try{
    tes.connect();
  }catch(std::string s){
    UI::Dialog::IO->println(s);
    return;
  }
  _tesname = hostname;
  _tesport = port;

  if(DEBUG) UI::Dialog::IO->println(std::string("Connected!"));
  if(DEBUG) UI::Dialog::IO->println(std::string("Writing..."));

  tes.write(std::string("RQT ") + std::to_string(_sid) + std::string("\n"));

  if(DEBUG) UI::Dialog::IO->println(std::string("TER ") + std::to_string(tnn) + std::string("\n"));

  if(DEBUG) UI::Dialog::IO->println(std::string("Written"));

  if(DEBUG) UI::Dialog::IO->println(std::string("Reading Server Answer"));


  if(DEBUG) UI::Dialog::IO->println(std::string("Reading AQT"));
  message = tes.readWord();
  if(DEBUG) UI::Dialog::IO->println(std::string("CODE: ") + message);
  if(message != std::string("AQT")){
     UI::Dialog::IO->println(std::string("Unknown format of information"));
     return;
  }
  if(DEBUG) UI::Dialog::IO->println(std::string("Reading Server QID"));
  std::string qid = tes.readWord();
  if(DEBUG) UI::Dialog::IO->println(std::string("QID: ") + qid);
  if(DEBUG) UI::Dialog::IO->println(std::string("Reading Server deadline"));
  std::string time = tes.readWord();
  if(DEBUG) UI::Dialog::IO->println(std::string("Deadline: ") + time);
  if(DEBUG) UI::Dialog::IO->println(std::string("Reading Server size"));
  std::string size = tes.readWord();

  if(DEBUG) UI::Dialog::IO->println(std::string("Size: ") + size);
  char b;

  std::string filename = std::string("T01Q") + qid + std::string(".pdf");
  if(DEBUG) UI::Dialog::IO->println(std::string("Writting to file") + filename);

  std::ofstream pdfFile(filename, std::ofstream::out);
  int fd = tes.rawRead();
  for(int i = 0; i < atoi(size.data()); i++){
    if(DEBUG) UI::Dialog::IO->println(std::string("I'm reading the socket"));
    while(::read(fd, &b, 1) == 0);
      pdfFile << b;
    if(DEBUG) UI::Dialog::IO->print(std::string("Read one byte"));
    if(DEBUG) UI::Dialog::IO->println(std::to_string(i));
  }
  pdfFile.close();

  if(DEBUG) UI::Dialog::IO->println(std::string("File written"));
  if(DEBUG) UI::Dialog::IO->println(std::string("Disconnecting"));
  tes.disconnect();

  if(DEBUG) UI::Dialog::IO->println(std::string("Disconnected"));

}

void UserManager::submit(int qid, char r[]){
  SocketTCP tes(_tesname.data(), _tesport);

  if(DEBUG) UI::Dialog::IO->println("Socket created");
  if(DEBUG) UI::Dialog::IO->println("Connecting...");

  try{
    tes.connect();
  }catch(std::string s){
    UI::Dialog::IO->println(s);
    return;
  }


  if(DEBUG) UI::Dialog::IO->println(std::string("Connected!"));
  if(DEBUG) UI::Dialog::IO->println(std::string("Writing..."));

  tes.write(std::string("RQS ") + std::to_string(_sid));
  for(int i = 0; i < 5; i++){
    tes.write(" ");
    tes.write(std::to_string(r[i]));
  }
  tes.write("\n");
  std::string code = tes.readWord();
  std::string qidstr = tes.readWord();
  std::string score = tes.readWord();

  if(code.size() == 0 || qidstr.size() == 0 || score.size() == 0){
    UI::Dialog::IO->println(std::string("Error during communication with the Server"));
    return;
  }
  UI::Dialog::IO->print("Score: ");
  UI::Dialog::IO->println(score);

}
