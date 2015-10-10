#include "RequestECP.h"
#include "Debug.h"

#include <string>

RequestECP::RequestECP(std::string message, struct sockaddr_in clientAddr):
  _message(message), _processed(false), _sent(false), _clientAddr(clientAddr){}

RequestECP::RequestECP(std::string message) : _message(message), _processed(false),
_sent(false){}

void RequestECP::answer(std::string answer){ _answer = answer; }
std::string RequestECP::answer(){ return _answer; }
std::string RequestECP::read(){ return _message; }

struct sockaddr_in RequestECP::client(){ return _clientAddr; }
