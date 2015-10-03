#include "Quiz.h"

Quiz::Quiz(int deadline, std::string filename) : _deadline(deadline), _filename(filename){}
int Quiz::deadline(){ return _deadline; }
std::string Quiz::filename(){ return _filename; }
void Quiz::deadline(int deadline){ _deadline = deadline; }
void Quiz::filename(std::string filename){ _filename = filename; }
