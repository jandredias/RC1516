#include "Request.h"

#include <iostream>



namespace RC_User{
  Request::Request(UserManager *manager) :
       Command<UserManager>("Request", manager){}
    void Request::execute(){
      _receiver->request();
      std::cout << "Request command not implemented" << std::endl;
    }
}
