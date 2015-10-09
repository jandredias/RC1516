#include "Request.h"
#include "Debug.h"

#include <iostream>

#include "Dialog.h"
#include "Exception.h"
namespace RC_User{
  Request::Request(UserManager *manager) :
       Command<UserManager>("Request", manager){}
    void Request::execute(){
      if(_receiver->qid() != std::string("")){
        UI::Dialog::IO->readBoolean("There is a questionnaire that you didn't submit.\n"
                                    "If you continue, you won't be able to answer this questionnaire.\n"
                                    "You still want to request a new questionnaire?");
      }
      try{
        UI::Dialog::IO->print("Which topic do you want to get? ");
        std::pair<std::string, std::string> pair = _receiver->request(UI::Dialog::IO->readInteger());

        UI::Dialog::IO->println("It has the unique id: " + pair.first);
        UI::Dialog::IO->print("The questionnaire should be submitted before " + pair.second);
      }catch(InvalidTID s){
        UI::Dialog::IO->println("Invalid topicID. Don't be an idiot and request a valid topicID.");
      }catch(ErrorOnMessage s){
        UI::Dialog::IO->println("There was an error in the communication with the server.");
      }catch(NoQuestionnaire s){
        UI::Dialog::IO->println("There is no such topic number.");
      }catch(UnknownFormatProtocol s){
        UI::Dialog::IO->println("The message received from the server does not \
respect the protocol.\nThe process will be aborted.\nTry again later.");
      }catch(ECPOffline s){
        UI::Dialog::IO->println("Could not connect to ECP Server!\nTry again later.");
      }catch(WritingOnSocket s){
        UI::Dialog::IO->println("Error sending message to ECP Server");
      }catch(InvalidHostname s){
        UI::Dialog::IO->println("The TES hostname is not valid. Contact the ECP administrator.");
      }catch(InvalidPort s){
        UI::Dialog::IO->println("The TES port is not valid. Contact the ECP administrator.");
      }catch(TCPCreating s){
        UI::Dialog::IO->println("Error creating socket.");
        UI::Dialog::IO->println(s.message());
      }catch(connectOnServer s){
        UI::Dialog::IO->println("You can't connect on server side");
      }catch(ErrorConnectingTCP s){
        UI::Dialog::IO->println("Error connecting to TCP server");
        UI::Dialog::IO->println(s.message());
      }catch(DisconnectingTCP s){
        UI::Dialog::IO->println("Error disconnecting to TCP server");
        UI::Dialog::IO->println(s.message());
      }catch(std::string s){
        UI::Dialog::IO->println(s);
      }catch(UDPCreating s){
        UI::Dialog::IO->println("ECP server not found");
      }
    }
}
