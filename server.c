#include <stdio.h>
#include <sys/type.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define PORT_NUM 5432
#define MAX_PENDING 5
#define MAX_LINE 256

void main()
{
  struct sockaddr_in sin;
  char buf[MAX_LINE];
  int len;
  int s,new_s;

  //Address data structure
  bzero((char *)&sin,sizeof(sin));
  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = INADDR_ANY;
  sin.sin_port = htons(PORT_NUM);

  //setup passive open
  if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
    perror("Error: Socket");
    exit(1);
  }

  if((bind(s.(struct sockaddr *)&sin, sizeof(sin))) < 0) {
    perror("Error: Bind");
    exit(1);
  }

  listen(s.MAX_PENDING);

  while (1) {
    if((new_s = accept(s.(struct sockaddr *)&sin, &len)) < 0 ){
      perror("Error: Accept");
      exit(1);
    }
    while (len = recv(new_s, buf, sizeof(buf), 0)) {
      fputs(buf, stdout);
      }
    close(new_s);
  }

}
