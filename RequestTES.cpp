#include "RequestTES.h"
#include "Debug.h"

#include <fstream>      // std::ifstream
RequestTES::RequestTES(SocketTCP client, int sid, std::string qid, int deadline) :
_client(client), _qid(qid), _sid(sid), _deadline(deadline), _fileSize(-1){}

RequestTES::RequestTES(std::string message) : _message(message) {}

std::string RequestTES::message(){ return _message; }
void RequestTES::answer(std::string answer){ _answer = answer; }
std::string RequestTES::answer(){ return _answer; }

void RequestTES::message(std::string text){ _message = text; }
void RequestTES::write(std::string text){ _client.write(text); }
void RequestTES::write(){
  debug(std::string("Writing to socket"));

  _client.write(_answer);

  debug(std::string("First part written"));
  debug(std::string("Writing file to socket"));

  if(_fileSize > 0){
    //It send byte by byte
    std::ifstream ifs(_fileName, std::ifstream::in);
    char c = ifs.get();
    while(ifs.good()){
      _client.write(c);
      c = ifs.get();
    }
    ifs.close();
  }

  if(_answer.substr(0,3) == "AQT")
    _client.write("\n");

  debug(std::string("File written"));
}
std::string RequestTES::read(){ return _client.read(); }
void RequestTES::disconnect(){ _client.disconnect(); }
bool RequestTES::finished(){ return !_client.connected(); }
int RequestTES::sid(){ return _sid; }
void RequestTES::sid(int x){ _sid = x;}
void RequestTES::qid(std::string x){ _qid = x;}
std::string RequestTES::qid(){ return _qid; }
int RequestTES::deadline(){ return _deadline; }
void RequestTES::deadline(int x){ _deadline = x;}
void RequestTES::fileSize(int x){ _fileSize = x;}
