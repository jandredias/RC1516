#include "RequestTES.h"

RequestTES::RequestTES(SocketTCP client, int sid, std::string qid, int deadline) :
_client(client), _qid(qid), _sid(sid), _deadline(deadline), _fileSize(-1){}

RequestTES::RequestTES(std::string message) : _message(message) {}

std::string RequestTES::message(){ return _message; }
void RequestTES::answer(std::string answer){ _answer = answer; }
std::string RequestTES::answer(){ return _answer; }

void RequestTES::message(std::string text){ _message = text; }
void RequestTES::write(std::string text){ _client.write(text); }
void RequestTES::write(){
  #if DEBUG
  UI::Dialog::IO->println(std::string("Writing to socket"));
  #endif

  _client.write(_answer);

  #if DEBUG
  UI::Dialog::IO->println(std::string("First part written"));
  #endif

  #if DEBUG
  UI::Dialog::IO->println(std::string("Writing file to socket"));
  #endif
  if(_fileSize > 0)
    _client.write(_file, _fileSize);

  #if DEBUG
  UI::Dialog::IO->println(std::string("File written"));
  #endif


  if(_file != NULL) delete[] _file;
}
std::string RequestTES::read(){ return _client.read(); }
void RequestTES::disconnect(){ _client.disconnect(); }
bool RequestTES::finished(){ return !_client.connected(); }

void RequestTES::sid(int x){ _sid = x;}
void RequestTES::qid(std::string x){ _qid = x;}
std::string RequestTES::qid(){ return _qid; }
int RequestTES::deadline(){ return _deadline; }
void RequestTES::deadline(int x){ _deadline = x;}
void RequestTES::fileSize(int x){ _fileSize = x;}
