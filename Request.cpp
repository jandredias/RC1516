#include "Request.h"

#include <iostream>

#include "Dialog.h"

namespace RC_User{
  Request::Request(UserManager *manager) :
       Command<UserManager>("Request", manager){}
    void Request::execute(){
      _receiver->request(UI::Dialog::IO->readInteger());
    }
}
