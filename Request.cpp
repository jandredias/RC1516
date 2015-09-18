#include "Request.h"

#include <iostream>



namespace RC_User{
  Request::Request(UserManager *manager) :
       Command<UserManager>("Request", manager){}
    void Request::execute(){
      std::cout << "Request command not implemented" << std::endl;
    }
}
