#include <stdio.h>
#include </usr/include/x86_64-linux-gnu/sys/types.h>
#include <stdlib.h>
#include </usr/include/x86_64-linux-gnu/sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#define PORT_NUM 5132
#define MAX_LINE 2048
#define MAX_SEQ_NUM 128
#define PACKET_SIZE 2048

#define RED   "\x1B[31m"
#define GRN   "\x1B[32m"
#define YEL   "\x1B[33m"
#define BLU   "\x1B[34m"
#define MAG   "\x1B[35m"
#define CYN   "\x1B[36m"
#define WHT   "\x1B[37m"
#define RESET "\x1B[0m"


int doesFileExist(const char *filename) 
{
    struct stat st;
    int result = stat(filename, &st);
    if(result == 0)
    {
    printf("\nfile does exist!\n");
    return 0;
    }
    else
      return -1;
}

void main(int argc, char * argv[])
{
  FILE *fp;
  struct hostent *hp;
  struct sockaddr_in sin;
  char *host;
  char buf[MAX_LINE],recv_buf[MAX_LINE],recv_buf_w_seq[MAX_LINE+7];
  int s,len,port;
  int flag_put,flag_get;
  int nofile_flag = 0;

  if(argc == 3) {
    host = argv[1];
    port = atoi(argv[2]);
    printf(GRN"IP Address = %s \n Port Number = %d\n"RESET,host,port);
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
  sin.sin_addr.s_addr = inet_addr(host);
  sin.sin_port = htons(port);


  //active open
  if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0) 
  {
    perror("Error: Socket");
    exit(1);
  }

  //main Loop
  int slen = sizeof(sin);
  char command[MAX_LINE];
  printf("\n****************** Client Initiated *******************\n\n\t\tPossibile Commands\n\tls\n\tget <filename>\n\tput <filename>\n\texit\n\n");
  bzero(command,sizeof(command));
  while ((gets(command))) //getting the command on  a while loop
  {
    
    //printf("%s",command);
    len = strlen(command);
    int flag_ls = strcmp(command,"ls");
    int flag_exit = strcmp(command,"exit");

    if(strstr(command, "get") != NULL)  //checking for command and setting the flags
    {
      printf(YEL"Get command\n"RESET);
      flag_get = 0;
    }
    else
    {
      flag_get = -1;
    }

    if(strstr(command, "put") != NULL)
    {
      printf(YEL"Put Command\n"RESET);
      flag_put = 0;
    }
    else
    {
      flag_put = -1;
    }

    if(flag_ls == 0 )
    {
      printf(GRN"ls command recieved \n"RESET);
      len = strlen(command);
      int status = sendto(s,command,len,0,&sin, slen);
      if(status == -1)
      {
        perror("Error: Send Failed");
      }

      while(len = recvfrom(s, recv_buf, sizeof(recv_buf), 0,(struct sockaddr *) &sin, &slen)) //recieving buffer back from server
      {
         	int end_flag = strcmp(recv_buf,"end");
        	if(end_flag==0)
            {
            
             	break;
            }
            else
            {
            	printf("%s\n",recv_buf);
            }
            
      	}

    }


    else if(flag_get == 0 )
    {
      printf(GRN"\n\tGet Command Activated\n"RESET);
      int len = strlen(command);
      char *filename = (char *)malloc(sizeof(char)*(MAX_LINE+1));
      bzero(filename,sizeof(filename));
 
      strncpy(filename,command+4,len-3);
 
      void commandsend()    //iterative commandsend until it is successful
      {
      int status = sendto(s,command,len,0,&sin, slen);
      if(status == -1)
        {
          perror("Error: Send Failed");
          commandsend();
        }
        else
        {
        printf("sent buf - %s  , count = %d, len = %d\n",command,status,len );           
        }
     }
     commandsend();

      FILE *get_file = fopen(filename,"w+");  //file operations
      bzero(recv_buf_w_seq,sizeof(recv_buf_w_seq));
      int filesize = 0;
      int count = 1; 
      int old_seq;
      int endflag;
      char ack_buf[7];
      bzero(ack_buf,sizeof(ack_buf));
      char hash_value[MAX_LINE],value[MAX_LINE];
      
      while(len = recvfrom(s, recv_buf_w_seq, sizeof(recv_buf_w_seq), 0,(struct sockaddr *) &sin, &slen))
      {
            int seq_num = count % MAX_SEQ_NUM; //seq num calculations round robin fashion
       
            char *SEQ = (char *)malloc(sizeof(char)*(10+1));
            bzero(SEQ,sizeof(SEQ));
            strxfrm(SEQ,recv_buf_w_seq+3,4);
            memcpy(recv_buf,recv_buf_w_seq+7,MAX_LINE);
            printf(".");
            if(seq_num == atoi(SEQ))  //checking for the seq numbers
            {
              //printf("Sequence Number Match,%d\n",count);
              printf(".");
              count++;
              endflag = 1;
              bzero(ack_buf,sizeof(ack_buf));
              sprintf(ack_buf,"ACK%04d",seq_num);
              sendto(s,ack_buf,sizeof(ack_buf),0,&sin, slen);       //sending ack
              old_seq = seq_num;
            }
            else if(atoi(SEQ)==9999)
            {
              printf("EOF\n");
              endflag = 0;
              nofile_flag = 0;
            }
            else if(atoi(SEQ) == old_seq)
            {
              printf("~");  //old seq
              sendto(s,ack_buf,sizeof(ack_buf),0,&sin, slen);
              continue;
            }
            else if(atoi(SEQ) > seq_num )
            {
              if(atoi(SEQ) == 8888) //file doesnt exist
              {
                printf("File doesnt exist! \n");
                //fclose(filename);
                
                char remove_command[MAX_LINE];
                bzero(remove_command,sizeof(remove_command));
                strcpy(remove_command,"rm ");
                strcat(remove_command,filename); 
                system(remove_command);

                nofile_flag = 1;

                break;
              }
              sendto(s,ack_buf,sizeof(ack_buf),0,&sin, slen); //send ack
              continue;
            }

            if(endflag == 0)
            {
              if(atoi(SEQ) == 9999)
              {
                printf("End of file seq num match");
                strxfrm(hash_value,recv_buf_w_seq+7,128);
              }
              break;
            }
            else
            {  
              fwrite(recv_buf,sizeof(char), len-7,get_file);  //writing onto a file
            }

            filesize = filesize+(len-7);
            bzero(recv_buf,sizeof(recv_buf));
            bzero(SEQ,sizeof(SEQ));

      }
      fclose(get_file);

    if(nofile_flag == 0)
    {                                                               //md5 routinre
      char *md5command = (char *)malloc(sizeof(char)*(MAX_LINE+1));
      printf("md5command\n");
      
      strcpy(md5command,"md5sum ");
      strcat(md5command,filename);
      strcat(md5command," > md5value.txt");
      //printf("%s\n",md5command );
      system(md5command);
      bzero(value,sizeof(value));
      FILE* md5file = fopen("md5value.txt","r+");
      fscanf(md5file,"%s",value);
      printf("MD5Sum from sender - %s\nMD5sum from local file - %s\n",hash_value,value);
      if(strcmp(value,hash_value) == 0)
      {
        printf("File Match Checksum Success!\n");
      }
      printf("\nFile Closed! File Size = %d\n",filesize);
    }  
    }

    else if(flag_put == 0)
    {

      printf("\nInside Put\n");     //file operations
      int len = strlen(command);
      char filename[MAX_LINE];
      bzero(filename,sizeof(filename));
      strncpy(filename,command+4,len-3);
 
      int status = sendto(s,command,len,0,&sin, slen);
      if(status == -1)
        {
          perror("Error: Send Failed");
        }
      if(filename != NULL)  //checking for file
      {
        int file_exists = doesFileExist(filename);
        if(file_exists == 0)
        {
          FILE *put_file = fopen(filename,"r+");
          if(put_file == NULL)
          {
            printf("\nError: File open %s\n",filename);
          }
              
          fseek(put_file,0,SEEK_END);     //checking filesize routine
          long filesize = ftell(put_file);
          rewind(put_file);
          printf("\nFile Size = %d",filesize);
          char send_buf[MAX_LINE];
          long bytes_read,bytes_sent,size_check =0;
                    
          char send_buf_w_seq[PACKET_SIZE+7];

          int count = 0;
          if(filesize > PACKET_SIZE)
          {
            fseek(put_file, SEEK_SET, 0);
            struct timeval tv;
            tv.tv_sec = 0;
            tv.tv_usec = 100000;
            if (setsockopt(s, SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(tv)) < 0) 
            {
              perror("Sock Timeout Error");
            }
            char hash_value[MAX_LINE],value[MAX_LINE];
              
            while (size_check < filesize) //filesize check
            {
              count++;
              bzero(send_buf,sizeof(send_buf));
              bzero(send_buf_w_seq,sizeof(send_buf_w_seq));
              bytes_read = fread(send_buf,1,PACKET_SIZE,put_file);  //reading the file

              int seq_num = count % MAX_SEQ_NUM;

              sprintf(send_buf_w_seq,"SeQ%04d",seq_num);
              memcpy(send_buf_w_seq+strlen(send_buf_w_seq),send_buf,sizeof(send_buf));
              void datasend() //sending the data until it recieves ack
              {
                bytes_sent = sendto(s,send_buf_w_seq, bytes_read+7, 0,(struct sockaddr *) &sin, slen);  //sending the file
                bzero(buf,sizeof(buf));
                if(recvfrom(s, buf, sizeof(buf), 0,(struct sockaddr *) &sin, &slen)<0)  
                {
                  printf("-");
                  datasend();
                }
                char ack_buf[7];
                bzero(ack_buf,sizeof(ack_buf));
                sprintf(ack_buf,"ACK%04d",seq_num);
                int ack_flag = strcmp(buf,ack_buf);
                if(ack_flag == 0)
                {
                  printf(".");
                }
                else
                {
                  printf("!");
                  datasend();
                }
              }

              datasend();
              size_check = size_check + bytes_read;
               
              bzero(send_buf,sizeof(send_buf));
            }
            fclose(put_file);
            char md5command[256] = "md5sum "; //md5sum routine
            strcat(md5command,filename);
            strcat(md5command," > md5value.txt");
            //printf("%s\n",md5command );
            system(md5command);
            FILE* md5file = fopen("md5value.txt","r+");
            fscanf(md5file,"%s",value);
            printf("\nMD5 Value - %s",value);
            system("rm md5value.txt");
            char end_command[MAX_LINE] = "SEQ9999";
            strcat(end_command,value);

            sendto(s,end_command, (sizeof(end_command)),0,(struct sockaddr *) &sin, slen);
                
          }
          else  //file size less than packet size
          {
            printf(".");
            fseek(put_file, SEEK_SET, 0);
            bytes_read = fread(send_buf,sizeof(char),PACKET_SIZE,put_file);
            int seq_num = 1;
            sprintf(send_buf_w_seq,"SeQ%04d",seq_num);
            memcpy(send_buf_w_seq+7,send_buf,sizeof(send_buf));

            fclose(put_file);
            void datasend1()  //sending it ttill it recieves an ack, reliability
            {
            bytes_sent = sendto(s,send_buf_w_seq, bytes_read+7, 0,(struct sockaddr *) &sin, slen);
            
            bzero(buf,sizeof(buf));
            if(recvfrom(s, buf, sizeof(buf), 0,(struct sockaddr *) &sin, &slen)<0)
            {
              printf("^");
              datasend1();
            }

            printf("Buffer - %s\n",buf);
            char ack_buf[7];
            bzero(ack_buf,sizeof(ack_buf));
            sprintf(ack_buf,"ACK%04d",seq_num);
            int ack_flag = strcmp(buf,ack_buf);
            //printf("Recieved Buffer - %s, ACK Buffer - %s\n",buf,ack_buf);
            printf(".");
            if(ack_flag == 0)
            {
              printf(".");
            }
            else
            {
              printf("~");
              datasend1();
            }
            }

            datasend1();

            //printf("\nNumber of bytes read = %d \nNumber of bytes sent = %d",bytes_read,bytes_sent);
            filesize = bytes_sent-7;
            char md5command[256] = "md5sum "; //md5 routine
            strcat(md5command,filename);
            strcat(md5command," > md5value.txt");
            //printf("%s\n",md5command );
            char value[MAX_LINE];
            system(md5command);
            FILE* md5file = fopen("md5value.txt","r+");
            fscanf(md5file,"%s",value);
            printf("\nMD5 Value - %s\n",value);
            system("rm md5value.txt");
            char end_command[MAX_LINE] = "SEQ9999";
            strcat(end_command,value);
            sendto(s,end_command, (sizeof(end_command)),0,(struct sockaddr *) &sin, slen);  //sending end command with md5 value
            
          }

          if(filesize == size_check)
          {
            printf("\nFile Sent!");
          }
        }
        else
        {
          printf("\nFile doesn't exist \n");
        }

      }

    }
    else if(flag_exit==0)
    {
      sendto(s,"EXIT",(sizeof("EXIT")),0,(struct sockaddr *) &sin, slen);  
      printf("\nExiting ...\n");
      exit(1);

    }
    else
    {
      printf(RED"%s - Cannot understand the command\n"RESET,command);
    }

    printf(GRN"\n\n\n\t\tPossibile Commands\n\tls\n\tget <filename>\n\tput <filename>\n\texit\n\n"RESET);
    bzero(command,sizeof(command));
    }
}

      
