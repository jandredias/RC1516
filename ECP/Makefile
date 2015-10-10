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
	rm -rf ecp *.o
