#include "List.h"
#include "Exception.h"
#include <iostream>
#include "Dialog.h"

namespace RC_User{
  List::List(UserManager *manager) :
       Command<UserManager>("List", manager){}
    void List::execute(){
      try{
        _receiver->list();
      }catch(std::string s){
        UI::Dialog::IO->println(s);
      }catch(UnknownFormatProtocol s){
        UI::Dialog::IO->println("The message received from the server does not \
respect the protocol.\nThe process will be aborted.\nTry again later.");
      }catch(MessageTooLongUDP s){
        UI::Dialog::IO->println("The message received from the server is too \
large to be processed. It will be discarted!");
      }
    }
}
