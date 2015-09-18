all:	ecp tes user

ecp:	ecp.cpp
	g++ -std=c++11 ecp.cpp -o ecp

tes:	tes.cpp
	g++ -std=c++11 tes.cpp -o tes

user:	user.cpp
	g++ -std=c++11 user.cpp -o user

app:	App.cpp AppServer.cpp
	g++ -std=c++11 App.cpp -o App
	g++ -std=c++11 AppServer.cpp -o AppServer

clean:
	rm ecp tes user
