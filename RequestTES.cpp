#include "RequestTES.h"

RequestTES::RequestTES(SocketTCP client, int sid, int qid, int deadline) :
_client(client), _qid(qid), _sid(sid), _deadline(deadline), _fileSize(-1){}


std::string RequestTES::message(){ return _message; }
void RequestTES::answer(std::string answer){ _answer = answer; }
std::string RequestTES::answer(){ return _answer; }

void RequestTES::message(std::string text){ _message = text; }
void RequestTES::write(std::string text){ _client.write(text); }
void RequestTES::write(){
  if(__DEBUG__) UI::Dialog::IO->println(std::string("Writing to socket"));
  _client.write(_answer);
  if(__DEBUG__) UI::Dialog::IO->println(std::string("First part written"));

  if(__DEBUG__) UI::Dialog::IO->println(std::string("Writing file to socket"));
  _client.write(_file, _fileSize);
  if(__DEBUG__) UI::Dialog::IO->println(std::string("File written"));
  delete[] _file;
}
std::string RequestTES::read(){ return _client.read(); }
void RequestTES::disconnect(){ _client.disconnect(); }
bool RequestTES::finished(){ return !_client.connected(); }

void RequestTES::sid(int x){ _sid = x;}
void RequestTES::qid(int x){ _qid = x;}
int RequestTES::qid(){ return _qid; }
void RequestTES::deadline(int x){ _deadline = x;}
void RequestTES::fileSize(int x){ _fileSize = x;}
