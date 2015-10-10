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
#=============         TOPIC EVALUATION SERVER        ==============
#===================================================================

tes:	tes.o SocketTCP.o SocketUDP.o Dialog.o RequestTES.o TesManager.o Quiz.o
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
	rm -rf tes *.o
