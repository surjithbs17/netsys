#include <stdio.h>
#include </usr/include/x86_64-linux-gnu/sys/types.h>
#include <stdlib.h>
#include </usr/include/x86_64-linux-gnu/sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>

#define PORT_NUM 5132
#define MAX_LINE 2048
#define MAX_SEQ_NUM 128

void main(int argc, char * argv[])
{
  FILE *fp;
  struct hostent *hp;
  struct sockaddr_in sin;
  char *host;
  char buf[MAX_LINE],recv_buf[MAX_LINE],recv_buf_w_seq[MAX_LINE+7];
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
      bzero(recv_buf_w_seq,sizeof(recv_buf_w_seq));
      //printf("opened file pid %d\n",getpid());
      int filesize = 0;
      int count = 1; 
      int old_seq;
      int endflag;
      char hash_value[MAX_LINE],value[MAX_LINE];
      while(len = recvfrom(s, recv_buf_w_seq, sizeof(recv_buf_w_seq), 0,(struct sockaddr *) &sin, &slen))
      {
            //
            int seq_num = count % MAX_SEQ_NUM; 
            //char SEQ[10];
            char *SEQ = (char *)malloc(sizeof(char)*(10+1));
            bzero(SEQ,sizeof(SEQ));
            strxfrm(SEQ,recv_buf_w_seq+3,4);
            memcpy(recv_buf,recv_buf_w_seq+7,MAX_LINE);
            printf("SEQ num - %s\n",SEQ );
            if(seq_num == atoi(SEQ))
            {
              printf("Sequence Number Match\n");
              count++;
              endflag = 1;
              sendto(s,"ACK",sizeof("ACK"),0,&sin, slen);       
              old_seq = seq_num;
            }
            else if(atoi(SEQ)==9999)
            {
              printf("End Seq match\n");
              endflag = 0;
            }
            else if(atoi(SEQ) == old_seq)
            {
              printf("OLd packet recieved\n");
              sendto(s,"ACK",sizeof("ACK"),0,&sin, slen);
              continue;
            }
            else
            {
              continue;
            }

            //printf("Recv Buf - %s\n",recv_buf );

            //sendto(s,"ACK",sizeof("ACK"),0,&sin, slen);       
            //int endflag = strcmp(recv_buf,"ENDOFFILE1234");
            if(endflag == 0)
            {
              printf("\nFile Recieved");
              if(atoi(SEQ) == 9999)
              {
                printf("End of file seq num match");
                
                strxfrm(hash_value,recv_buf_w_seq+7,128);
              }
              break;
            }
            else
            {  
              //fputs(recv_buf, get_file);
              fwrite(recv_buf,sizeof(char), len-7,get_file);
              //printf("\nprinting file! %s",recv_buf);
              printf("\nBytes Recieved = %d   Count = %d\n",len,count);
            }

            filesize = filesize+(len-7);
          bzero(recv_buf,sizeof(recv_buf));
          bzero(SEQ,sizeof(SEQ));

      }
      fclose(get_file);
      char *md5command = (char *)malloc(sizeof(char)*(MAX_LINE+1));
      //char md5command[256] = "md5sum ";
      strcpy(md5command,"md5sum ");
      strcat(md5command,filename);
      strcat(md5command," > md5value.txt");
      printf("%s\n",md5command );
      system(md5command);
      FILE* md5file = fopen("md5value.txt","r+");
      fscanf(md5file,"%s",value);
      printf("From Sender - %s\nFrom file Value - %s\n",hash_value,value);
      if(strcmp(value,hash_value) == 0)
      {
        printf("File Match Success!\n");
      }
      printf("\nFile Closed! File Size = %d",filesize);
      
    }


    }
}

      
