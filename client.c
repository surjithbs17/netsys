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
    int flag_put,flag_get;

  if(argc > 1) {
    host = argv[1];
    port = atoi(argv[2]);
    printf("IP Address = %s \n Port Number = %d\n",host,port);
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
  char command[MAX_LINE];
  while ((gets(command))) 
  {
    printf("%s",command);
    len = strlen(command);
    printf("%slength %d",command,len );
    int flag_ls = strcmp(command,"ls");
    int flag_exit = strcmp(command,"exit");

    if(strstr(command, "get") != NULL)
    {
      printf("Get match");
      flag_get = 0;
    }
    else
    {
      flag_get = -1;
    }

    if(strstr(command, "put") != NULL)
    {
      printf("Put match");
      flag_put = 0;
    }
    else
    {
      flag_put = -1;
    }

    
    
  

    printf("\nflag_ls == %d  \nflag_get == %d \n flag_put == %d",flag_ls,flag_get,flag_put);
      
    
    if(flag_ls == 0 )
    {
      printf("flag_ls %d\n", flag_ls);
      len = strlen(command);
      printf("\nInside ls\n command - %s",command);

      int status = send(s,command,len,0);
      if(status == -1)
        {
          perror("Error: Send Failed");
        }
      else
        {
        printf("\nsent ls - %s\n",command);           
        //printf("\n Sent ls command");
        }

        printf("pid is %d\n", getpid());

        printf("\n pid is %d waiting for servers reply",getpid());

      while(len = recv(s,recv_buf,sizeof(recv_buf),0))
      {
        pid_t childPID = fork();
        printf("lsss");

        if(childPID >= 0) // fork was successful
        {
          if(childPID == 0) // child process
          {
            fputs(recv_buf, stdout);
            printf("\n");
            printf("Child pid is %d\n", getpid());
            int end_flag = strcmp(recv_buf,"end");
            if(end_flag)
              break;
            _Exit();
          }
        }
        else // fork failed
        {
          printf("\n Fork failed, quitting!!!!!!\n");
          return 1;
        }
      }
    }

    if(flag_get == 0 )
    {
      printf("\nInside get\n");
      int len1 = strlen(command);
      printf("after strlen");
      printf("%s",command);
      char filename[MAX_LINE];
      bzero(filename,sizeof(filename));
       strncpy(filename,command+4,len1-4);
 
      //*buf = "get server\0";
      //char *new_buf = "get ls_file";
      printf("\n after strcpy - %s fone", filename);
      printf("\n filename - %s \n buffer - %s ",filename,command);
 
      //len = strlen(new_buf);
      int status = send(s,command,len1,0);
      if(status == -1)
        {
          perror("Error: Send Failed");
        }
      else
        {
        printf("sent buf - %s\n",command );           
        }
      FILE *get_file = fopen(filename,"w");

      printf("opened file pid %d\n",getpid());

      while(len1 = recv(s,recv_buf,sizeof(recv_buf),0))
      {
                printf("\nhgRecieved Buffer");

        pid_t childPID = fork();
        if(childPID >= 0) // fork was successful
        {
          if(childPID == 0) // child process
          {
            int endflag = strcmp(recv_buf,"END_OF_FILE1234");
            if(endflag == 0)
            {
              printf("File Recieved");
              break;
            }
            else
            {  
              fputs("priniting",stdout);
              fputs(recv_buf, get_file);
              printf("\nprinting file!");
            }
          }
        }
        else //fork failed
        {
          printf("\n Fork failed\n");
          return 1;
        }
      }
      printf("\nExiting get");
    }

  }
}
