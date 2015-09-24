#include "Submit.h"

#include <iostream>

#include "Dialog.h"

namespace RC_User{
  Submit::Submit(UserManager *manager) :
       Command<UserManager>("Submit", manager){}
    void Submit::execute(){
      int qid;
      char r[5];
      UI::Dialog::IO->print("QID: ");
      qid = UI::Dialog::IO->readInteger();

      for(int i = 0; i < 5; i++){
          UI::Dialog::IO->print("Answer to question ");
          UI::Dialog::IO->print(std::to_string(i + 1));
          UI::Dialog::IO->print("? ");

          r[i] = (UI::Dialog::IO->readString()).data()[0];
      }
      _receiver->submit(qid, r);
    }
}
