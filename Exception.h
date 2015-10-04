#pragma once
#include <string>

class SocketAlreadyInUse{
  bool _tcp;  //True if Socket is TCP, false if it is UDP

public:
  SocketAlreadyInUse(std::string s){ _tcp = (s == "TCP") ? true : false; }
  std::string message(){
    return "Socket " + std::string(((_tcp) ? "TCP" : "UDP")) + " already in use";
  }
};
class AfterDeadlineSubmit{};
class InvalidQIDvsSID{};
class NoRequestAsked{};
class UnknownFormatProtocol{};
class MessageTooLongUDP{};
class NoQuestionnaire{};
class ECPOffline{};
class WrongNumberofTopics{};
class WritingOnSocket{};
class ErrorOnMessage{};
class InvalidHostname{};
class InvalidPort{};

class TCPCreating{
  std::string _message;
  public:
    TCPCreating(std::string message) : _message(message) {}
    std::string message(){ return _message; }
};
class connectOnServer{};
class ErrorConnectingTCP{
  std::string _message;
  public:
  ErrorConnectingTCP(std::string message) : _message(message) {}
  std::string message(){ return _message; }
};
class DisconnectingTCP{
  std::string _message;
public:
  DisconnectingTCP(std::string message) : _message(message){}
  std::string message(){ return _message; }
};
