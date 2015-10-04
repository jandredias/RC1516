#include "List.h"
#include "Exception.h"
#include <iostream>
#include "Dialog.h"
#include <vector>
namespace RC_User{
  List::List(UserManager *manager) :
       Command<UserManager>("List", manager){}
    void List::execute(){
      try{
        std::vector<std::string> topics = _receiver->list();
        int i = 1;
        UI::Dialog::IO->println("Topics: \n--------");
        for(auto e : topics){
          UI::Dialog::IO->print(std::to_string(i++) + " - ");
          UI::Dialog::IO->println(e);
        }

      }catch(std::string s){
        UI::Dialog::IO->println(s);
      }catch(UnknownFormatProtocol s){
        UI::Dialog::IO->println("The message received from the server does not \
respect the protocol.\nThe process will be aborted.\nTry again later.");
      }catch(MessageTooLongUDP s){
        UI::Dialog::IO->println("The message received from the server is too \
large to be processed. It will be discarted!");
      }catch(NoQuestionnaire s){
        UI::Dialog::IO->println("There is no questionnaire available in the server.");
      }catch(ECPOffline s){
        UI::Dialog::IO->println("Could not connect to ECP Server!\nTry again later.");
      }catch(WrongNumberofTopics s){
        UI::Dialog::IO->println("The number of topics sent by server does not match with data received.");
      }catch(WritingOnSocket s){
        UI::Dialog::IO->println("Error sending message to ECP Server");
      }
    }
}
