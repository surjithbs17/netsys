#include <stdio.h>
#include </usr/include/x86_64-linux-gnu/sys/types.h>
#include <stdlib.h>
#include </usr/include/x86_64-linux-gnu/sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>

#define PORT_NUM 5123
#define MAX_LINE 256

void main(int argc, char * argv[])
{
  FILE *fp;
  struct hostent *hp;
  struct sockaddr_in sin;
  char *host;
  char buf[MAX_LINE],recv_buf[MAX_LINE];
  int s,len,port;

  if(argc > 1) {
    host = argv[1];
    port = atoi(argv[2]);
    printf("IP Address = %s \n Port Number = %d",host,port);
  }
  else {
    perror("Error: Formatting no argument");
    exit(1);
  }

//hostname to ip Address
  hp = gethostbyname(host);
  if(!hp) {
    perror("Error: unknown host");
    exit(1);
  }

//build address data struct
  bzero((char *)&sin,sizeof(sin));
  sin.sin_family = AF_INET;
  //sin.sin_addr = INADDR_ANY;
  sin.sin_addr.s_addr = inet_addr(host);
  sin.sin_port = htons(port);


//active open
  if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
    perror("Error: Socket");
    exit(1);
  }

  if (connect(s,(struct sockaddr *)&sin, sizeof(sin)) < 0) {
    perror("Error: Connect");
    close(s);
    exit(1);
  }

//main Loop
  while (fgets(buf,sizeof(buf),stdin)) {
    buf[MAX_LINE-1] = '\0';
    len = strlen(buf) + 1;
    send(s,buf,len,0);
    while(len = recv(s,recv_buf,sizeof(recv_buf),0))
    {
      fputs(recv_buf, stdout);
      if(strcmp(recv_buf,"Let's End it!"))
        break;
    }
  }

}
