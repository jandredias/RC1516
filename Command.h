#ifndef __INESC_UI_COMMAND_H__
#define __INESC_UI_COMMAND_H__

#include <string>
namespace UI{
  template <class T>
  class Command{
    bool _last;

  protected:
    std::string _title;
    T* _receiver;

  public:
    Command(bool last, std::string title, T* receiver) :
      _last(last), _title(title), _receiver(receiver){}

    Command(std::string title, T* receiver) :
      _last(false), _title(title), _receiver(receiver){}

    /**
     * @return the command's title
     */
    std::string title(){ return _title; }
    /**
     * Indicates whether, in a menu, this is the last command
     * @return true if, in a menu, this is the last command
     */
    const bool isLast(){ return _last; }

    /**
     * Executes the command: as defined in the Command pattern, the
     * command's execution should eventually invokes the receiver's
     * action.
     */
    virtual void execute() = 0;
  };

}


#endif /* end of include guard: __INESC_UI_COMMAND_H__ */
