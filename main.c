#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

extern int errno;

int main(){
  struct hostent *h;
  struct in_addr *a;
  char buffer[128];
  if(gethostname(buffer, 128) == -1)
    printf("error: %s\n", strerror(errno));
  else
    printf("hostname: %s\n", buffer);
  

  if((h=gethostbyname(buffer)) == NULL) exit(1); //error
  
  printf("official hostname: %s\n", h->h_name);

  a = (struct in_addr*) h->h_addr_list[0];
  printf("internet addr: %s (%08lX)\n", inet_ntoa(*a), ntohl(a->s_addr));

  return 0;
}
