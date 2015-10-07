#include "Request.h"
#include "Debug.h"

#include <iostream>

#include "Dialog.h"
#include "Exception.h"
namespace RC_User{
  Request::Request(UserManager *manager) :
       Command<UserManager>("Request", manager){}
    void Request::execute(){
      try{
        UI::Dialog::IO->print("Which topic do you want to get? ");
        std::pair<std::string, int> pair = _receiver->request(UI::Dialog::IO->readInteger());

        UI::Dialog::IO->println("Questionnaire has an unique id: " + pair.first);
        UI::Dialog::IO->println("It is stored in " + pair.first + ".pdf file");
        UI::Dialog::IO->print("The questionnaire should be submitted before ");

        time_t t = pair.second;   // get time now
        struct tm * now = localtime( & t );

        std::string deadline;
        if(now->tm_hour < 9) deadline += "0";
        deadline += std::to_string(now->tm_hour + 1) + ":";
        if(now->tm_min < 9) deadline += "0";
        deadline += std::to_string(now->tm_min + 1) + ":";
        if(now->tm_sec < 9) deadline += "0";
        deadline += std::to_string(now->tm_sec + 1);

        UI::Dialog::IO->print(deadline + " on ");

        deadline = "";
        if(now->tm_mday < 9) deadline += "0";
        deadline += std::to_string(now->tm_mday + 1) + "-";
        if(now->tm_mon < 8) deadline += "0";
        deadline += std::to_string(now->tm_mon + 1) + "-";
        deadline += std::to_string(now->tm_year + 1900);

        UI::Dialog::IO->println(deadline);

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
      }
    }
}
