#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>


#define __PORT__ 59000

int main(void) {
	
	int fd, n;
	int addrStructLen;
	struct hostent *hostprt;

	struct sockaddr_in serverAddr, clientAddr;
	struct in_addr *a;
	char msg[] = "vamos ver se recebem isto";
	char buffer[1024];
 
	fd = socket(AF_INET, SOCK_DGRAM, 0);
	
	memset((void*) &serverAddr, (int) '\0', sizeof(serverAddr));
	serverAddr.sin_family      = AF_INET;
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddr.sin_port        = htons(__PORT__);

	bind(fd, (struct sockaddr*) &serverAddr, sizeof(serverAddr));
	addrStructLen = sizeof(clientAddr);
	while(1){
		recvfrom(fd, buffer, sizeof(buffer), 0, (struct sockaddr*) &clientAddr, &addrStructLen);
		hostprt = gethostbyaddr(&clientAddr.sin_addr, sizeof(clientAddr.sin_addr), AF_INET);
		if(hostprt == NULL)
			printf("internet addr: [%s:%hu]\n", inet_ntoa(clientAddr.sin_addr),ntohs(clientAddr.sin_port));
		else	printf("internet addr: [%s:%hu]\n", hostprt->h_name, ntohs(clientAddr.sin_port));
		printf("mensagem recebida: %s\n", buffer);

		sendto(fd, msg, strlen(msg) + 1, 0, (struct sockaddr*) &clientAddr, sizeof(clientAddr));
	}
	close(fd);

	return 0;
}
