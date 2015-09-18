#pragma once

#include "Command.h"
#include "UserManager.h"

namespace RC_User{
  class List : public UI::Command<UserManager>{
    /**
     * @param receiver
     */
  public:
    List(UserManager *);
    void execute();
  };

}
