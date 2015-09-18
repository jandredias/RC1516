#include "List.h"

#include <iostream>

namespace RC_User{
  List::List(UserManager *manager) :
       Command<UserManager>("List", manager){}
    void List::execute(){
      _receiver->list();
    }
}
