#pragma once

#include "Command.h"
#include "UserManager.h"

namespace RC_User{
  class Request : public UI::Command<UserManager>{
    /**
     * @param receiver
     */
  public:
    Request(UserManager *);
    void execute();
  };

}
