#pragma once

#include "UserManager.h"

namespace RC_User{
  class MenuBuilder{
  public:
    /**
     * @param receiver
     */
    static void menuFor(UserManager *);
  };
}
