#include <stdio.h>
#include <sys/type.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define PORT_NUM 5432
#define MAX_LINE 256

void main(int argc, char * argv[])
{
  FILE *fp;
  struct hostent *hp;
  struct sockaddr_in sin;
  char *host;
  char buf[MAX_LINE];
  int s,len;

  if(argc==2) {
    host = argv[1];
  }
  else {
    perror("Error: Formatting/ no argument");
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
  sin.sin_addr.s_addr = INADDR_ANY;
  sin.sin_port = htons(PORT_NUM);

//active open
  if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
    perror("Error: Socket");
    exit(1);
  }

  if (connect.(s,(struct sockaddr *)&sin, sizeof(sin)) < 0) {
    perror("Error: Connect");
    exit(1);
  }

//main Loop
  while (fgets(buf,sizeof(buf),stdin)) {
    buf[MAX_LINE-1] = '\0';
    len = strlen(buf) + 1;
    send(s,buf,len,0);
  }

}
