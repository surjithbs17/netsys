

#include <stdio.h>
#include <stdlib.h>
#include </usr/include/x86_64-linux-gnu/sys/types.h>
#include </usr/include/x86_64-linux-gnu/sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>
#include <string.h>

#define PORT_NUM 5120
#define MAX_PENDING 5
#define MAX_LINE 256




void main(int argc, char *argv[])
{
  struct sockaddr_in sin;
  char buf[MAX_LINE],parsed_buf[MAX_LINE];
  int len;
  int s,new_s,flag;
  FILE *ls_fp;
  char parsed_char;

  //Address data structure
  bzero((char *)&sin,sizeof(sin));
  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = inet_addr("127.0.1.1");
  sin.sin_port = htons(PORT_NUM);

  //setup passive open
  if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
    perror("Error: Socket");
    exit(1);
  }

  if((bind(s,(struct sockaddr *)&sin, sizeof(sin))) < 0) {
    perror("Error: Bind");
    exit(1);
  }

  printf("Going to listen and %d",PORT_NUM);

  listen(s,MAX_PENDING);
  printf("\nListening ");
  while (1) {

    if((new_s = accept(s,(struct sockaddr *)&sin, &len)) < 0 ){
      perror("Error: Accept");
      exit(1);
    }
    printf("Connection Accepted\n");

    while (len = recv(new_s, buf, sizeof(buf), 0)) 
    {  
    	fputs(buf,stdout);
     	//flag = strcmp(buf,"ls");
     	printf("flag = %s",buf);
     	flag = strcmp(buf,"ls");
     	printf("flag == %d",flag);
     	
     	if( flag >= 0)
      	{
      		fputs("Inside LS",stdout);
      		system("ls > 'ls_file.txt'");
      		ls_fp = fopen("ls_file.txt","r");
      		int i=0;
      		//char *check_buf;
      		//check_buf = "I am sending";
      		//send(new_s, check_buf, (strlen(check_buf) + 1),0);
      		while(fscanf(ls_fp,"%s",parsed_buf) != EOF)
      		{
      			int status = send(new_s, parsed_buf, (strlen(parsed_buf) + 1),0);
				if(status == -1)
				{
					perror("Error: Send Failed");
				}
				else
				{
				printf("%s\n",parsed_buf );      			
      			}
      		}

      		fclose(ls_fp);
      	}
     	else
      	{
      		fputs("Type Something",stdout);
    	}
    }
    close(new_s);
  }

}
