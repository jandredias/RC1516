#===================================================================
#============= DO NOT CHANGE ANYTHING AFTER THIS LINE ==============
#===================================================================
CC = g++

FLAGS = -std=c++11 -O3 -ggdb -Wall -pedantic -Wpedantic -pedantic-errors \
        -Wextra -w -Waggressive-loop-optimizations -Werror -pedantic-errors \
				-Wparentheses -Wunknown-pragmas
#				-static-libstdc++

COMP = $(CC) $(FLAGS) -g -c

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

Dialog.o: Dialog.cpp Dialog.h
	$(COMP) Dialog.cpp -lrt -o Dialog.o

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
