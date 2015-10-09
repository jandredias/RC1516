#pragma once
#include <string>

class invalidArguments{};
class SocketClosed{};
class ConnectionTCPTimedOut{};
class connectOnServer{};
class InvalidTID{};
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
class ErrorOpeningFile{};
class SocketAlreadyInUse{
  bool _tcp;  //True if Socket is TCP, false if it is UDP

public:
  SocketAlreadyInUse(std::string s){ _tcp = (s == "TCP") ? true : false; }
  std::string message(){
    return "Socket " + std::string(((_tcp) ? "TCP" : "UDP")) + " already in use";
  }
};
class TCPCreating{
  std::string _message;
  public:
    TCPCreating(std::string message) : _message(message) {}
    std::string message(){ return _message; }
};
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
