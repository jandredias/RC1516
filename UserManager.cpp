#include "UserManager.h"

#include "SocketUDP.h"
#include "SocketTCP.h"
#include <sstream>
#include <unistd.h>
#include "Dialog.h"
#include <math.h>       /* log */
#include <fstream>      // std::ofstream
#include "Exception.h"
UserManager::UserManager(int sid, int port, std::string ecpname) : _sid(sid), _port(port),
_ecpname(ecpname){}

std::string UserManager::list(){
  #if DEBUG
  UI::Dialog::IO->println("[ UserManager::list            ] Creating socket");
  #endif

  SocketUDP ecp = SocketUDP(_ecpname.data(), _port);

  std::string message;

  #if DEBUG
  UI::Dialog::IO->println("[ UserManager::list            ] Socket created\n"
                          "[ UserManager::list            ] Sending message");
  #endif

  std::stringstream stream;

  /*
   * This for cycle tries to connect to ECP for X times and will "return" the
   * server message on std::stringstream stream
   */
  for(auto i = 0; i < TRIES; i++){
   ecp.send(std::string("TQR\n"));
    try{
        ecp.timeout(MS_BETWEEN_TRIES);
        stream << ecp.receive();

        #if DEBUG
        UI::Dialog::IO->println("[ UserManager::list            ] Message sent to ECP\n"
                                "[ UserManager::list            ] Receiving message from ECP\n"
                                "[ UserManager::list            ] Message received from ECP");
        #endif

        break;
    }catch(std::string s){
      if(errno == 11){
        UI::Dialog::IO->print(". ");
        UI::Dialog::IO->flush();
        if(i < TRIES - 1) continue;
        UI::Dialog::IO->println();
        UI::Dialog::IO->println();
      }else{
        throw WritingOnSocket();
      }
    }
    throw ECPOffline();
  }

  #if DEBUG
  UI::Dialog::IO->println("[ UserManager::list            ] Processing message");
  #endif

  std::string code;
  stream >> code;
  if(code == std::string("EOF")){
    char c = '\0';
    stream.get(&c,1);
    if(c == '\n'){
      throw NoQuestionnaire();
    }
    throw UnknownFormatProtocol();
  }else if(code != std::string("AWT")){
    throw UnknownFormatProtocol();
  }
  int nt;
  stream >> nt;

  if(nt <= 0){
    throw UnknownFormatProtocol();
  }

  #if DEBUG
    UI::Dialog::IO->println(code  + std::string(" ") + std::to_string(nt));
  #endif

  std::string topic;

  std::string text = "Topics:\n";
  for(int i = 1; i < nt + 1; i++){
    stream >> topic;
    if(i != nt && stream.eof()) throw WrongNumberofTopics();
    text += std::to_string(i);
    text += std::string(" - ");
    text += topic;
    text += std::string("\n");
  }
  if(!stream.eof()){
      throw UnknownFormatProtocol();
  }
  #if DEBUG
  UI::Dialog::IO->println("[ UserManager::list            ] Message processed");
  #endif

  return text;
}

std::string UserManager::request(int tnn){

  #if DEBUG
  UI::Dialog::IO->println("[ UserManager::request            ] Creating socket");
  #endif

  SocketUDP ecp = SocketUDP(_ecpname.data(), _port);


  #if DEBUG
  UI::Dialog::IO->println("[ UserManager::request            ] Socket created\n"
                          "[ UserManager::request            ] Sending message");
  #endif

  std::stringstream stream;
  for(auto i = 0; i < TRIES; i++){
    ecp.send("TER " + std::to_string(tnn) + "\n");
    try{
      ecp.timeout(MS_BETWEEN_TRIES);
      stream << ecp.receive();

      #if DEBUG
      UI::Dialog::IO->println("[ UserManager::request            ] Message sent to ECP\n"
                              "[ UserManager::request            ] Receiving message from ECP\n"
                              "[ UserManager::request            ] Message received from ECP");
      #endif

      break;
    }catch(std::string s){
      if(errno == 11){
        UI::Dialog::IO->print(". ");
        UI::Dialog::IO->flush();
        if(i < TRIES - 1)
          continue;
        UI::Dialog::IO->println();
        UI::Dialog::IO->println();
      }else{
        throw WritingOnSocket();
      }
    }
    throw ECPOffline();
  }

  std::string code;
  stream >> code;

  if(code == std::string("EOF")){
    char c = '\0';
    stream.get(&c,1);
    if(c == '\n'){
      throw NoQuestionnaire();
    }
    throw UnknownFormatProtocol();
  }else if(code == std::string("ERR")){
    throw ErrorOnMessage();
  }else if(code != std::string("AWTES")){
    throw UnknownFormatProtocol();
  }

  #if DEBUG
  UI::Dialog::IO->println("Connecting to TES Server");
  #endif

  std::string hostname = "";
  int port = 0;

  stream >> hostname;
  stream >> port;

  #if DEBUG
  std::cout << "Creating socket" << std::endl;
  #endif

  if(hostname == std::string("")) throw InvalidHostname();
  if(port <= 0) throw InvalidPort();

  SocketTCP tes(hostname.data(), port);

  #if DEBUG
  std::cout << "Socket created" << std::endl;
  std::cout << "Connecting..." << std::endl;
  #endif

  tes.connect();

  _tesname = hostname;
  _tesport = port;

  #if DEBUG
  UI::Dialog::IO->println(std::string("Connected!"));
  UI::Dialog::IO->println(std::string("Writing..."));
  #endif

  tes.write(std::string("RQT ") + std::to_string(_sid) + std::string("\n"));

  #if DEBUG
  UI::Dialog::IO->println(std::string("TER ") + std::to_string(tnn) + std::string("\n"));
  UI::Dialog::IO->println(std::string("Written"));
  UI::Dialog::IO->println(std::string("Reading Server Answer"));
  UI::Dialog::IO->println(std::string("Reading AQT"));
  #endif
  std::string message;
  message = tes.readWord();

  #if DEBUG
  UI::Dialog::IO->println(std::string("CODE: ") + message);
  #endif

  if(message != std::string("AQT")){
     UI::Dialog::IO->println(std::string("Unknown format of information"));
     return message;
  }

  #if DEBUG
  UI::Dialog::IO->println(std::string("Reading Server QID"));
  #endif

  std::string qid = tes.readWord();

  #if DEBUG
  UI::Dialog::IO->println(std::string("QID: ") + qid);
  UI::Dialog::IO->println(std::string("Reading Server deadline"));
  #endif

  std::string time = tes.readWord();

  #if DEBUG
  UI::Dialog::IO->println(std::string("Deadline: ") + time);
  UI::Dialog::IO->println(std::string("Reading Server size"));
  #endif

  std::string size = tes.readWord();

  #if DEBUG
  UI::Dialog::IO->println(std::string("Size: ") + size);
  #endif

  char b;

  std::string filename = qid + std::string(".pdf");

  #if DEBUG
  UI::Dialog::IO->println(std::string("Writting to file") + filename);
  #endif

  std::ofstream pdfFile(filename, std::ofstream::out);
  int fd = tes.rawRead();
  for(int i = 0; i < atoi(size.data()); i++){

    while(::read(fd, &b, 1) == 0);
    pdfFile << b;

  }
  pdfFile.close();

  #if DEBUG
  UI::Dialog::IO->println(std::string("File written"));
  UI::Dialog::IO->println(std::string("Disconnecting"));
  #endif

  tes.disconnect();

  #if DEBUG
  UI::Dialog::IO->println(std::string("Disconnected"));
  #endif
  return message;
}

void UserManager::submit(std::string qid, std::string answers){
  if(_tesname == std::string("") || _tesport == 0){
    UI::Dialog::IO->println(std::string("You should first request a questionnaire."));
    return;
  }
  SocketTCP tes(_tesname.data(), _tesport);

  #if DEBUG
  UI::Dialog::IO->println("Socket created");
  UI::Dialog::IO->println("Connecting...");
  #endif

  try{
    tes.connect();
  }catch(std::string s){
    UI::Dialog::IO->println(s);
    return;
  }


  #if DEBUG
  UI::Dialog::IO->println(std::string("Connected!"));
  UI::Dialog::IO->println(std::string("Writing..."));
  #endif

  tes.write(std::string("RQS ") + std::to_string(_sid) + " " + qid + " " + answers + "\n");
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
