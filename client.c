#include <stdio.h>
#include </usr/include/x86_64-linux-gnu/sys/types.h>
#include <stdlib.h>
#include </usr/include/x86_64-linux-gnu/sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>

#define PORT_NUM 5132
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
  while (gets(buf)) 
  {
    len = strlen(buf);
    int flag = strcmp(buf,"ls");
    
    if(flag == 0 )
    {
      printf("Inside ls\n");
      int status = send(s,buf,len,0);
      if(status == -1)
        {
          perror("Error: Send Failed");
        }
      else
        {
        printf("%s\n",buf );           
        }

      while(len = recv(s,recv_buf,sizeof(recv_buf),0))
      {
        pid_t childPID = fork();

        if(childPID >= 0) // fork was successful
        {
          if(childPID == 0) // child process
          {
            fputs(recv_buf, stdout);
            printf("\n");
            int end_flag = strcmp(recv_buf,"end");
            if(end_flag)
              break;
          }
          /**
          else //Parent process
          {
            printf("\n Packet Missed - Parent process");
          }
          **/
        }
        else // fork failed
        {
          printf("\n Fork failed, quitting!!!!!!\n");
          return 1;
        }
      }
    }

  }
}
