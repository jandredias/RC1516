// test.c
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <netdb.h>
#include <stdio.h>

int main(void) 
{
	int fd, n;
	struct sockaddr_in addr;
	struct hostent *h;
	struct in_addr *a;
	char c;
	if((h=gethostbyname("tejo.tecnico.ulisboa.pt"))==NULL) { printf("get host name\n"); exit(1);}//error
	
	a=(struct in_addr*)h->h_addr_list[0];
	printf("internet address: %s\n",inet_ntoa(*a));
	c = getchar();
	
	/* ... */
	fd= socket(AF_INET,SOCK_DGRAM,0);//UDP socket
	
	if(fd==-1)exit(1);//error

	memset((void*)&addr,(int)'\0',sizeof(addr));
	addr.sin_family      =AF_INET;
	addr.sin_addr.s_addr =a;
	addr.sin_port        =htons(58000);
	
	n=sendto(fd,"Hello!\n",7,0,(struct sockaddr*)&addr,sizeof(addr));

	if(n==-1) { printf("send went south\n"); exit(1);}//error
/* ... */
}
