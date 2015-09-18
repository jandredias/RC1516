#include "Submit.h"

#include <iostream>



namespace RC_User{
  Submit::Submit(UserManager *manager) :
       Command<UserManager>("Submit", manager){}
    void Submit::execute(){
      std::cout << "Submit command not implemented" << std::endl;
    }
}
