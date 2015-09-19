
#===================================================================
#============= DO NOT CHANGE ANYTHING AFTER THIS LINE ==============
#===================================================================
CC = g++
FLAGS = -std=c++11 -O3 -ggdb -Wall -pedantic

COMP = $(CC) $(FLAGS) -g -c

all:	ecp tes user

#===================================================================
#===========   Central Evaluation Contact Point Server  ============
#===================================================================

ecp:	ecp.cpp
	$(CC) $(FLAGS) ecp.cpp -o ecp

#===================================================================
#=============         TOPIC EVALUATION SERVER        ==============
#===================================================================

tes:	tes.cpp SocketTCP.o SocketUDP.o Dialog.o RequestQuiz.o TesManager.o
	$(CC) $(FLAGS) tes.cpp SocketTCP.o SocketUDP.o Dialog.o RequestQuiz.o TesManager.o -pthread -o tes
	tar xvf quiz.tar.xz

RequestQuiz.o:	RequestQuiz.h RequestQuiz.cpp
	$(COMP) RequestQuiz.cpp -o RequestQuiz.o

TesManager.o:	TesManager.h TesManager.cpp
	$(COMP) TesManager.cpp -o TesManager.o


#===================================================================
#=============                   USER                 ==============
#===================================================================

user:	user.cpp MenuBuilder.o List.o Submit.o Request.o UserManager.o SocketTCP.o SocketUDP.o Dialog.o
	$(CC) $(FLAGS) user.cpp MenuBuilder.o List.o Submit.o Dialog.o Request.o UserManager.o SocketTCP.o SocketUDP.o -o user

UserManager.o:	UserManager.h UserManager.cpp
	$(COMP) UserManager.cpp -o UserManager.o

Dialog.o:	Dialog.h Dialog.cpp
	$(COMP) Dialog.cpp -o Dialog.o

MenuBuilder.o:	MenuBuilder.cpp MenuBuilder.h
	$(COMP) MenuBuilder.cpp -o MenuBuilder.o

List.o:	List.cpp List.h
	$(COMP) List.cpp -o List.o

Submit.o:	Submit.cpp Submit.h
	$(COMP) Submit.cpp -o Submit.o

Request.o:	Request.cpp Request.h
	$(COMP) Request.cpp -o Request.o


#===================================================================
#=============                 OTHERS                 ==============
#===================================================================

SocketTCP.o:	SocketTCP.h SocketTCP.cpp
	$(COMP) SocketTCP.cpp -o SocketTCP.o

SocketUDP.o:	SocketTCP.h SocketUDP.cpp
	$(COMP) SocketUDP.cpp -o SocketUDP.o



clean:
	rm -rf ecp tes user *.o

app:	App.cpp AppServer.cpp
	g++ -std=c++11 App.cpp -o App
	g++ -std=c++11 AppServer.cpp -o AppServer
