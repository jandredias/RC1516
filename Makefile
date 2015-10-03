#===================================================================
#============= DO NOT CHANGE ANYTHING AFTER THIS LINE ==============
#===================================================================
CC = g++

FLAGS = -std=c++11 -O3 -ggdb -Wall -pedantic -Wpedantic -pedantic-errors \
        -Wextra -w -Waggressive-loop-optimizations -Werror -pedantic-errors \
				-Wparentheses -Wunknown-pragmas
#				-static-libstdc++

COMP = $(CC) $(FLAGS) -g -c

all:	ecp tes user

#===================================================================
#===========   Central Evaluation Contact Point Server  ============
#===================================================================

ecp:	ecp.o ECPManager.o RequestECP.o SocketUDP.o Dialog.o
	$(CC) $(FLAGS) ecp.o ECPManager.o RequestECP.o SocketUDP.o Dialog.o -lrt -pthread -o ecp

ecp.o: ecp.cpp ECPManager.h RequestECP.h SocketUDP.h SocketTCP.h
	$(COMP) ecp.cpp -o ecp.o
RequestECP.o: RequestECP.cpp RequestECP.h
	$(COMP) RequestECP.cpp -o RequestECP.o

ECPManager.o: ECPManager.cpp ECPManager.h RequestECP.h SocketUDP.h \
	 SocketTCP.h Dialog.h
	$(COMP) ECPManager.cpp -o ECPManager.o

#===================================================================
#=============         TOPIC EVALUATION SERVER        ==============
#===================================================================

tes:	tes.o SocketTCP.o SocketUDP.o Dialog.o RequestTES.o TesManager.o 1.pdf Quiz.o
	$(CC) $(FLAGS) tes.o SocketTCP.o SocketUDP.o Dialog.o RequestTES.o Quiz.o TesManager.o -pthread -o tes

tes.o: tes.cpp SocketTCP.h SocketUDP.h TesManager.h RequestTES.h Dialog.h
	$(COMP) tes.cpp -o tes.o

RequestTES.o: RequestTES.cpp RequestTES.h SocketTCP.h SocketUDP.h \
	 Dialog.h
	$(COMP) RequestTES.cpp -o RequestTES.o

TesManager.o: TesManager.cpp TesManager.h RequestTES.h SocketTCP.h \
	 SocketUDP.h Dialog.h
	$(COMP) TesManager.cpp -o TesManager.o

Quiz.o:	Quiz.cpp Quiz.h
	$(COMP) Quiz.cpp -o Quiz.o

1.pdf:
	tar xvf quiz.tar.xz

#===================================================================
#=============                   USER                 ==============
#===================================================================

user:	user.o MenuBuilder.o List.o Submit.o Request.o UserManager.o SocketTCP.o SocketUDP.o Dialog.o
	$(CC) $(FLAGS) user.o MenuBuilder.o List.o Submit.o Dialog.o Request.o UserManager.o SocketTCP.o SocketUDP.o -o user

user.o: user.cpp MenuBuilder.h UserManager.h Dialog.h
	$(COMP) user.cpp -o user.o

UserManager.o: UserManager.cpp UserManager.h SocketUDP.h SocketTCP.h \
	 Dialog.h
	$(COMP) UserManager.cpp -o UserManager.o

Dialog.o: Dialog.cpp Dialog.h
	$(COMP) Dialog.cpp -lrt -o Dialog.o

MenuBuilder.o: MenuBuilder.cpp MenuBuilder.h UserManager.h Menu.h \
	 Command.h Dialog.h List.h Request.h Submit.h
	$(COMP) MenuBuilder.cpp -o MenuBuilder.o

List.o: List.cpp List.h Command.h UserManager.h
	$(COMP) List.cpp -o List.o

Submit.o: Submit.cpp Submit.h Command.h UserManager.h Dialog.h
	$(COMP) Submit.cpp -o Submit.o

Request.o: Request.cpp Request.h Command.h UserManager.h Dialog.h
	$(COMP) Request.cpp -o Request.o


#===================================================================
#=============                 OTHERS                 ==============
#===================================================================

SocketTCP.o: SocketTCP.cpp SocketTCP.h Dialog.h
	$(COMP) SocketTCP.cpp -o SocketTCP.o

SocketUDP.o: SocketUDP.cpp SocketUDP.h
	$(COMP) SocketUDP.cpp -o SocketUDP.o

enable:
	sed -i -- 's/DEBUG\ 0/DEBUG\ 1/g' *.cpp *.h
	make clean
	make

disable:
	sed -i -- 's/DEBUG\ 1/DEBUG\ 0/g' *.cpp *.h
	make clean
	make

clean:
	rm -rf ecp tes user *.o *.pdf

app:	App.cpp AppServer.cpp
	g++ -std=c++11 App.cpp -o App
	g++ -std=c++11 AppServer.cpp -o AppServer
