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
  FILE *fpout = {0};
  char line[MAX_LINE];
  FILE *fpin = {0};

  int fd[2];

  pipe(fd);

  if(argc == 3) {
    host = argv[1];
    port = atoi(argv[2]);
    printf("IP Address = %s \n Port Number = %d\n",host,port);
  }
  else {
    perror("Error: Formatting argument");
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
  if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("Error: Socket");
    exit(1);
  }

  //main Loop
  int slen = sizeof(sin);
  char command[MAX_LINE];
  printf("\n****************** Client Initiated *******************\n\n\t\tPossibile Commands\n\tls\n\tget <filename>\n\tput <filename>\n\texit\n\n");


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

    printf("\nflag_ls == %d\nflag_get == %d \nflag_put == %d \nflag_exit == %d\n",flag_ls,flag_get,flag_put,flag_exit);
    if(flag_ls == 0 )
    {
      printf("flag_ls %d\n", flag_ls);
      len = strlen(command);
      printf("\nInside ls\n command - %s",command);

      int status = sendto(s,command,len,0,&sin, slen);
      if(status == -1)
        {
          perror("Error: Send Failed");
        }
      else
        {
        printf("\n\nls command sent successfully - %s\n\n",command);           
        }

      	while(len = recvfrom(s, recv_buf, sizeof(recv_buf), 0,(struct sockaddr *) &sin, &slen))
      	{
        
        	int end_flag = strcmp(recv_buf,"end");
        	if(end_flag==0)
            {
              	printf("\n ls completed \n\n");
             	break;
            }
            else
            {
            	printf("%s\n",recv_buf);
            }
            
      	}

    }


    if(flag_get == 0 )
    {
      printf("\nInside get\n");
      int len = strlen(command);
      printf("after strlen");
      printf("%s",command);
      char filename[MAX_LINE];
      bzero(filename,sizeof(filename));
      strncpy(filename,command+4,len-4);
 
      printf("\n after strcpy - %s fone", filename);
      printf("\n filename - %s \n buffer - %s ",filename,command);
 
      int status = sendto(s,command,len,0,&sin, slen);
      if(status == -1)
        {
          perror("Error: Send Failed");
        }
      else
        {
        printf("sent buf - %s\n",command );           
        }
      FILE *get_file = fopen(filename,"w+");

      //printf("opened file pid %d\n",getpid());
      int filesize = 0;
      while(len = recvfrom(s, recv_buf, sizeof(recv_buf), 0,(struct sockaddr *) &sin, &slen))
      {
        
            int endflag = strcmp(recv_buf,"ENDOFFILE1234");
            if(endflag == 0)
            {
              printf("File Recieved");
              break;
            }
            else
            {  
              //fputs(recv_buf, get_file);
              fwrite(recv_buf,sizeof(char), sizeof(recv_buf),get_file);
              printf("\nprinting file! %s",recv_buf);
            }

            filesize = filesize+len;
          bzero(recv_buf,sizeof(recv_buf));
      }
      fclose(get_file);
      printf("\nFile Closed! File Size = %d",filesize);
      
    }


    }
}

      
