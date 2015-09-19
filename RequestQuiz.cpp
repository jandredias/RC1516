#include "RequestQuiz.h"

RequestQuiz::RequestQuiz(SocketTCP client, int sid, int qid, int deadline) :
_client(client), _qid(qid), _sid(sid), _deadline(deadline), _fileSize(-1){}
