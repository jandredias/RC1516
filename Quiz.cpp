#include "Quiz.h"

Quiz::Quiz(int sid, int deadline, std::string filename) : _sid(sid), _deadline(deadline), _filename(filename){}
int Quiz::deadline(){ return _deadline; }
std::string Quiz::filename(){ return _filename; }
void Quiz::deadline(int deadline){ _deadline = deadline; }
void Quiz::filename(std::string filename){ _filename = filename; }
int Quiz::sid(){ return _sid; }
