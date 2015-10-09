#include "UserManager.h"
#include "Debug.h"

#include <boost/progress.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include "SocketUDP.h"
#include "SocketTCP.h"
#include <sstream>
#include <iostream>
#include <unistd.h>
#include "Dialog.h"
#include <math.h>       /* log */
#include <fstream>      // std::ofstream
#include "Exception.h"

#define TIMEOUT 10000

#include <boost/range/algorithm/count.hpp>
int counter = 0;
UserManager::UserManager(int sid, int port, std::string ecpname) : _sid(sid), _port(port),
_ecpname(ecpname), _qid(""){}
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
  int n = 0;
  for(auto i = 0; i < TRIES; i++){
   ecp.send(std::string("TQR\n"));
    try{
        ecp.timeout(MS_BETWEEN_TRIES);
        message = ecp.receive();
        stream << message;

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

  n = boost::count(message, ' ');
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

  if(nt <= 0){ throw UnknownFormatProtocol(); }
  if(n != 1 + nt) throw UnknownFormatProtocol();
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
std::string UserManager::qid(){ return _qid; }
std::pair<std::string, std::string> UserManager::request(int tnn){

  debug("[ UserManager::request            ] Creating socket");

  SocketUDP ecp = SocketUDP(_ecpname.data(), _port);


  debug("[ UserManager::request            ] Socket created\n"
        "[ UserManager::request            ] Sending message");

  std::stringstream stream;

 int n = 0;
 std::string message;

  for(auto i = 0; i < TRIES; i++){
    ecp.send("TER " + std::to_string(tnn) + "\n");
    try{
      ecp.timeout(MS_BETWEEN_TRIES);
      message = ecp.receive();
      stream << message;

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

  n = boost::count(message, ' ');
  std::string code;
  stream >> code;
  if(code == std::string("EOF")){
    throw NoQuestionnaire();
  }else if(code == std::string("ERR")){
    throw ErrorOnMessage();
  }else if(code != std::string("AWTES")){
    throw UnknownFormatProtocol();
  }else if(n != 2){
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
  tes.timeout(TIMEOUT);
  std::string time;
  std::string qid;
  try{
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
    qid = tes.readWord();
    debug("QID size:"+ std::to_string(qid.size()));
    debug(std::string("QID: ") + qid);
    if (qid.size()>24)
      throw UnknownFormatProtocol();

    debug(std::string("Reading Server deadline"));

    time = tes.readWord();
    debug(std::string("Checking Server deadline FORMAT"));
    if (time.size() != 18)
    	throw UnknownFormatProtocol();
    char month[4];
    for (int i=0;i<3;i++) month[i]=time.at(i+2);
    month[3]='\0';
    debug(month);
    if(!(boost::iequals(month,"jan") || boost::iequals(month,"feb") ||
  		   boost::iequals(month,"mar") ||	boost::iequals(month,"apr") ||
  		   boost::iequals(month,"may") ||	boost::iequals(month,"jun") ||
  		   boost::iequals(month,"jul") ||	boost::iequals(month,"aug") ||
  		   boost::iequals(month,"sep") ||	boost::iequals(month,"oct") ||
  		   boost::iequals(month,"nov") ||	boost::iequals(month,"dec")))
      throw UnknownFormatProtocol();

    for (int i=0;i<18;i++){
  	  if ( i >=0 && i < 2){
  		  if (time.at(i) < '0' || time.at(i) > '9' )
  			throw UnknownFormatProtocol();
  	  }
  	  else if ( i >=2 && i < 5){
  		  if ( !((time.at(i) >= 'A' && time.at(i) <='Z')||(time.at(i) >= 'a' && time.at(i) <= 'z')) )
  			throw UnknownFormatProtocol();
  	  }
  	  else if ( i >=5 && i < 9){
  		  if (time.at(i) < '0' || time.at(i) > '9')
  			throw UnknownFormatProtocol();
  	  }
  	  else if ( i==9){
  		  if (time.at(i) != '_')
  			throw UnknownFormatProtocol();
  	  }
  	  else if ( i ==10 || i == 11 || i == 13 || i==14 || i==16 || i==17){
  		  if (time.at(i) < '0' || time.at(i) > '9')
  			throw UnknownFormatProtocol();
  	  }
  	  else if ( i == 12 || i== 15){
  		  if (time.at(i) != ':')
  			throw UnknownFormatProtocol();
  	  }

    }
    debug(std::string("Server deadline [GREEN]OK[REGULAR]"));
    debug(std::to_string(time.size()));
    debug(std::string("Deadline: ") + time);
    debug(std::string("Reading Server size"));


    std::string size = tes.readWord();

    debug(std::string("Size: ") + size);


    char b;
    std::string str_tnn = std::to_string(tnn);
    if (str_tnn.size() == 1){
      str_tnn = "0" + str_tnn;
    }

    if(counter == 999)
      counter = 0;
    std::string std_counter = std::to_string(++counter);
    if (std_counter.size() == 1){
      std_counter = "00" + std_counter;
    }
    else if (std_counter.size() == 2){
      std_counter = "0" + std_counter;
    }
    std::string filename = std::string("T") + str_tnn + std::string("QF") + std_counter + std::string(".pdf");

    debug(std::string("Writting to file") + filename);


    std::ofstream pdfFile(filename, std::ofstream::out);

    int fd = tes.rawRead();
    tes.timeout(5000);
    boost::progress_display p(atoi(size.data()));
    int n = 0;
    for(int i = 0; i < atoi(size.data()); i++){
      while((n = ::read(fd, &b, 1)) == 0);
      if(n == -1) throw ErrorOnMessage();
      pdfFile << b;
      ++p;
    }
    pdfFile.close();

    debug(std::string("File written"));
    debug(std::string("Disconnecting"));

    debug(std::string("Checking for \\n"));
    /*If nothing is read b could have the value \n  */
    b = 's';
    read(fd, &b, 1);
    debug();debug();debug();
    debug("CHARATER:: " + std::to_string(b));
    debug();debug();debug();
    if(b != '\n')
      throw UnknownFormatProtocol();


    _qid = qid;
    UI::Dialog::IO->println("Questionnaire is stored in " + filename + " file");
  }catch(ConnectionTCPTimedOut s){
    UI::Dialog::IO->println("Connection timed out. Try again later.");
  }
  tes.disconnect();
  debug("Disconnected");
  return std::make_pair(qid,time);
}

std::pair<std::string, int> UserManager::submit(std::string answers){
  if(_tesname == std::string("") || _tesport == 0 || _qid == "") throw NoRequestAsked();

  SocketTCP tes(_tesname.data(), _tesport);

  debug("Socket created");
  debug("Connecting...");

  tes.connect();

  debug(std::string("Connected!"));
  debug(std::string("Writing..."));

  tes.write(std::string("RQS ") + std::to_string(_sid) + " " + _qid + " " + answers + "\n");
  std::string code = tes.readWord();

  if(code == "ERR") throw ErrorOnMessage();

  std::string qidstr = tes.readWord();

  std::string score = tes.readWord();
  if(code.size() == 0 || qidstr.size() == 0 || score.size() == 0)
    throw UnknownFormatProtocol();

  if (score == "-1") throw AfterDeadlineSubmit();
  else if(score == "-2") throw InvalidQIDvsSID();
  _qid = "";

  if(!tes.end()) throw ErrorOnMessage();
  int s = atoi(score.data());
  return std::make_pair(qidstr,s);
}
