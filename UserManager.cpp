#include "UserManager.h"

#include <boost/progress.hpp>
#include "SocketUDP.h"
#include "SocketTCP.h"
#include <sstream>
#include <unistd.h>
#include "Dialog.h"
#include <math.h>       /* log */
#include <fstream>      // std::ofstream
#include "Exception.h"

#ifndef DEBUG
#define DEBUG 0
#endif

#if DEBUG
#ifndef debug(S)
#define debug(S) UI::Dialog::IO->println(S)
#endif
#else
#ifndef debug(S)
#define debug(S)
#endif
#endif

UserManager::UserManager(int sid, int port, std::string ecpname) : _sid(sid), _port(port),
_ecpname(ecpname){}


std::pair<std::string, int> UserManager::tesLocation(int topicNumber){

}

std::vector<std::string> UserManager::list(){
  debug("[ UserManager::list            ] Creating socket");

  SocketUDP ecp = SocketUDP(_ecpname.data(), _port);

  std::string message;

  debug("[ UserManager::list            ] Socket created\n"
        "[ UserManager::list            ] Sending message");

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

        debug("[ UserManager::list            ] Message sent to ECP\n"
              "[ UserManager::list            ] Receiving message from ECP\n"
              "[ UserManager::list            ] Message received from ECP");
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

  debug("[ UserManager::list            ] Processing message");

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

  debug(code  + std::string(" ") + std::to_string(nt));

  std::vector<std::string> topics;
  std::string topic;

  for(int i = 1; i < nt + 1; i++){
    stream >> topic;
    if(i != nt && stream.eof()) throw WrongNumberofTopics();
    topics.push_back(topic);
  }
  if(!stream.eof()){
      throw UnknownFormatProtocol();
  }
  debug("[ UserManager::list            ] Message processed");

  return topics;
}

std::pair<std::string, int> UserManager::request(int tnn){

  debug("[ UserManager::request            ] Creating socket");

  SocketUDP ecp = SocketUDP(_ecpname.data(), _port);


  debug("[ UserManager::request            ] Socket created\n"
        "[ UserManager::request            ] Sending message");

  std::stringstream stream;
  for(auto i = 0; i < TRIES; i++){
    ecp.send("TER " + std::to_string(tnn) + "\n");
    try{
      ecp.timeout(MS_BETWEEN_TRIES);
      stream << ecp.receive();

      debug("[ UserManager::request            ] Message sent to ECP\n"
            "[ UserManager::request            ] Receiving message from ECP\n"
            "[ UserManager::request            ] Message received from ECP");

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
    throw NoQuestionnaire();
  }else if(code == std::string("ERR")){
    throw ErrorOnMessage();
  }else if(code != std::string("AWTES")){
    throw UnknownFormatProtocol();
  }

  debug("Connecting to TES Server");

  std::string hostname = "";
  int port = 0;

  stream >> hostname;
  stream >> port;

  debug("Creating socket");

  if(hostname == std::string("")) throw InvalidHostname();
  if(port <= 0) throw InvalidPort();

  SocketTCP tes(hostname.data(), port);

  debug("Socket created\nConnecting...");

  tes.connect();

  _tesname = hostname;
  _tesport = port;

  debug(std::string("Connected!\nWriting..."));

  tes.write(std::string("RQT ") + std::to_string(_sid) + std::string("\n"));

  debug("TER " + std::to_string(tnn) + "\n" "Written" "\n"
        "Reading Server Answer" "\n" "Reading AQT");

  std::string message;
  message = tes.readWord();

  debug(std::string("CODE: ") + message);
  if(message == std::string("EOF")) throw InvalidTID();
  if(message != std::string("AQT")) throw UnknownFormatProtocol();

  debug(std::string("Reading Server QID"));

  std::string qid = tes.readWord();

  debug(std::string("QID: ") + qid);
  debug(std::string("Reading Server deadline"));

  std::string time = tes.readWord();

  debug(std::string("Deadline: ") + time);
  debug(std::string("Reading Server size"));


  std::string size = tes.readWord();

  debug(std::string("Size: ") + size);


  char b;

  std::string filename = qid + std::string(".pdf");

  debug(std::string("Writting to file") + filename);


  std::ofstream pdfFile(filename, std::ofstream::out);

  int fd = tes.rawRead();

  boost::progress_display p(atoi(size.data()));
  for(int i = 0; i < atoi(size.data()); i++){
    while(::read(fd, &b, 1) == 0);
    pdfFile << b;
    ++p;
  }
  pdfFile.close();

  debug(std::string("File written"));
  debug(std::string("Disconnecting"));


  tes.disconnect();

  debug(std::string("Disconnected"));


  return std::make_pair(qid,atoi(time.data()));
}

std::pair<std::string, int> UserManager::submit(std::string qid, std::string answers){
  if(_tesname == std::string("") || _tesport == 0) throw NoRequestAsked();

  SocketTCP tes(_tesname.data(), _tesport);

  debug("Socket created");
  debug("Connecting...");


  tes.connect();


  debug(std::string("Connected!"));
  debug(std::string("Writing..."));


  tes.write(std::string("RQS ") + std::to_string(_sid) + " " + qid + " " + answers + "\n");
  std::string code = tes.readWord();

  if(code == "ERR") throw ErrorOnMessage();

  std::string qidstr = tes.readWord();
  std::string score = tes.readWord();
  if(code.size() == 0 || qidstr.size() == 0 || score.size() == 0)
    throw UnknownFormatProtocol();


  if (score == "-1") throw AfterDeadlineSubmit();
  else if(score == "-2") throw InvalidQIDvsSID();

  return std::make_pair(qid,atoi(score.data()));
}
