#pragma once

#include "Command.h"
#include "UserManager.h"

namespace RC_User{
  class Submit : public UI::Command<UserManager>{
    /**
     * @param receiver
     */
  public:
    Submit(UserManager *);
    void execute();
  };

}
