#include "Submit.h"

#include <iostream>

#include "Dialog.h"

namespace RC_User{
  Submit::Submit(UserManager *manager) :
       Command<UserManager>("Submit", manager){}
    void Submit::execute(){
      std::string qid;
      UI::Dialog::IO->print("QID: ");
      qid = UI::Dialog::IO->readString();
      std::string answers = "";
      for(int i = 0; i < 5; i++){
          UI::Dialog::IO->print("Answer to question ");
          UI::Dialog::IO->print(std::to_string(i + 1));
          UI::Dialog::IO->print("? ");

      answers += UI::Dialog::IO->readString();
      if(i < 4) answers += " ";
      }
      _receiver->submit(qid, answers);
    }
}
