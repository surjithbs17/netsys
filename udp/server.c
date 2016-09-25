
#include <stdio.h>
#include <stdlib.h>
#include </usr/include/x86_64-linux-gnu/sys/types.h>
#include </usr/include/x86_64-linux-gnu/sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>
#include <string.h>


#define MAX_LINE 256
#define MAX_SEQ_NUM 128
#define PACKET_SIZE 256

int doesFileExist(const char *filename) {
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

void main(int argc, char *argv[])
{
  struct sockaddr_in sin,remote;
  struct hostent *hostp;
  unsigned int remote_len;
  char buf[MAX_LINE],parsed_buf[MAX_LINE],value[MAX_LINE],recv_buf[MAX_LINE],recv_buf_w_seq[MAX_LINE+7];
  int len;
  int slen;
  int s,new_s,flag_ls,flag_put,flag_get,flag_exit;
  FILE *ls_fp;
  char parsed_char;
  int PORT_NUM;
  char *host;
  
  if(argc > 1) {
    int port = atoi(argv[1]);
    PORT_NUM = port;
    printf("IP Address = 127.0.1.1 \n Port Number = %d\n",port);
  }

  //Address data structure
  bzero((char *)&sin,sizeof(sin));
  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = htonl(INADDR_ANY);
  //inet_addr("127.0.1.1"); 
  //htonl(INADDR_ANY)//
  sin.sin_port = htons((unsigned short) PORT_NUM);

  //setup passive open
  if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("Error: Socket");
    exit(1);
  }

  int optval = 1;
  setsockopt(s, SOL_SOCKET, SO_REUSEADDR,(const void *)&optval , sizeof(int));

  printf("\nSocket Created!");
  if((bind(s,(struct sockaddr *)&sin, sizeof(sin))) < 0) {
    perror("Error: Bind");
    exit(1);
  }
  remote_len = sizeof(remote);
  printf("\nSocket Bind done!");


  slen = sizeof(sin);
  printf("Going to listen and %d",PORT_NUM);
  bzero(buf,sizeof(buf));

   while (len = recvfrom(s, buf, sizeof(buf), 0,(struct sockaddr *) &remote, &remote_len))
   {
   		if(len<0)
        continue;

      printf("Recieved Buffer %s\n",buf );
     	
     	if(strcmp(buf,"ls") == 0)
     	{
     		printf("\nls command recieved\n");
     		flag_ls = 0;
     	}
     	else
     	{
     		flag_ls = -1;
     	}

     	if(strcmp(buf,"exit") == 0)
     	{
     		printf("\nexit command recieved\n");
     		flag_exit = 0;
     	}
     	else
     	{
     		flag_exit = -1;
     	}
     	
    	if(strstr(buf, "get") != NULL)
    	{
      		printf("Get match");
      		flag_get = 0;
    	}
    	else
   	 	{
      		flag_get = -1;
    	}

    	if(strstr(buf, "put") != NULL)
    	{
      		printf("Put match");
      		flag_put = 0;
    	}
    	else
    	{
    	  flag_put = -1;
    	}

     	//flag_exit = strcmp(buf,"exit");
     	printf("\nflag_ls == %d  \nflag_get == %d \nflag_put == %d \nflag_exit = %d\n",flag_ls,flag_get,flag_put,flag_exit);


     	if( (flag_ls == 0) )
      	{
      		fputs("\nInside ls Command \n",stdout);
      		system("ls > 'ls_file.txt'");
      		ls_fp = fopen("ls_file.txt","r");
      		int i=0;

      		while(fscanf(ls_fp,"%s",parsed_buf) != EOF)
      		{
      			int status = sendto(s, parsed_buf, (strlen(parsed_buf) + 1),0,(struct sockaddr *) &remote, remote_len);
				    if(status == -1)
				    {
					   perror("Error: Send Failed");
				    }
				    else
				    {
				      printf("%s\n",parsed_buf );      			
      			}
      		}
      		int status = sendto(s, "end", (strlen("end") + 1),0,(struct sockaddr *) &remote, remote_len);
				  if(status == -1)
				  {
					 perror("Error: Send Failed");
				  }


      		fclose(ls_fp);
          system("rm ls_file.txt");
      		printf("ls completed\n");
      	}

      	if (flag_get == 0)
      	{

      		printf("\n get command recieved\n");
      		char filename[MAX_LINE];
       		strncpy(filename,buf+4,len-3);
       		printf("\nFilename - %s\n", filename);

      		if(filename != NULL)
      		{
      			int file_exists = doesFileExist(filename);
      		  printf("in file exists loop\n");
      			if(file_exists == 0)
      			{
      				FILE *get_file = fopen(filename,"r+");
              //FILE *put_file = fopen("putfile","w+");
      				if(get_file == NULL)
      				{
      					printf("\nError: File open %s\n",filename);
      					exit(1);
      				}
      				//printf("\n in main\n");
      				
      				fseek(get_file,0,SEEK_END);
      				long filesize = ftell(get_file);
      				rewind(get_file);
      				printf("\nFile Size = %d",filesize);
              char send_buf[MAX_LINE];
      				long bytes_read,bytes_sent,size_check =0;
              //long bytes_write;
                    
                    char send_buf_w_seq[PACKET_SIZE+7];

              int count = 0;
      				if(filesize > PACKET_SIZE)
      				{
      					printf("\n size greater than 255\n");
      					fseek(get_file, SEEK_SET, 0);
                struct timeval tv;
                tv.tv_sec = 0;
                tv.tv_usec = 100000;
                if (setsockopt(s, SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(tv)) < 0) 
                {
                perror("Sock Timeout Error");
                }
              
      					while (size_check < filesize)
      					{
                  count++;
                  bzero(send_buf,sizeof(send_buf));
                  bzero(send_buf_w_seq,sizeof(send_buf_w_seq));
      						bytes_read = fread(send_buf,1,PACKET_SIZE,get_file);

                  //bytes_write = fwrite(send_buf,sizeof(char),sizeof(send_buf),put_file);
                  int seq_num = count % MAX_SEQ_NUM;
                  printf("Seq Num - %d",seq_num);
                  sprintf(send_buf_w_seq,"SeQ%04d",seq_num);
                  //printf("\nBuf with Seq - \n%s\nLength - %d sz %d\n",send_buf_w_seq,strlen(send_buf_w_seq),sizeof(send_buf_w_seq));
                  // strcat(send_buf_w_seq,"fg");
                  //printf("Send Buf - \n%s\n%d",send_buf,strlen(send_buf));
                  memcpy(send_buf_w_seq+strlen(send_buf_w_seq),send_buf,sizeof(send_buf));
                  //printf("\nBuf with Seq added - %s\n",send_buf_w_seq);


                 void datasend()
                 {
      						bytes_sent = sendto(s,send_buf_w_seq, bytes_read+7, 0,(struct sockaddr *) &remote, remote_len);
                  bzero(buf,sizeof(buf));
                  if(recvfrom(s, buf, sizeof(buf), 0,(struct sockaddr *) &remote, &remote_len)<0)
                  {
                    printf("Ack not recieved\n");
                    datasend();
                  }

                  printf("Buffer - %s\n",buf);
                  char ack_buf[7];
                  bzero(ack_buf,sizeof(ack_buf));
                  sprintf(ack_buf,"ACK%04d",seq_num);
                  int ack_flag = strcmp(buf,ack_buf);
                  printf("Recieved Buffer - %s, ACK Buffer - %s\n",buf,ack_buf);

                  if(ack_flag == 0)
                  {
                    printf("\n Ack recieved, %d",count);
                  }
                  else
                  {
                    printf("\n Ack Missed,%d",count);
                    datasend();
                  }
                }

                datasend();
      						size_check = size_check + bytes_read;
                  //printf("%s",send_buf);
                  
                  bzero(send_buf,sizeof(send_buf));
                  printf("\nBytes Read - %d, Bytes sent - %d , size_check - %d , Count = %d\n",bytes_read,bytes_sent,size_check,count);
      					}
      					fclose(get_file);
                char md5command[256] = "md5sum ";
                strcat(md5command,filename);
                strcat(md5command," > md5value.txt");
                printf("%s\n",md5command );
                system(md5command);
                FILE* md5file = fopen("md5value.txt","r+");
                fscanf(md5file,"%s",value);
                printf("MD5 Value - %s",value);
                system("rm md5value.txt");
                char end_command[MAX_LINE] = "SEQ9999";
                strcat(end_command,value);

                sendto(s,end_command, (sizeof(end_command)),0,(struct sockaddr *) &remote, remote_len);
      				  
                //fclose(put_file);
              }
      				else
      				{
      					printf("\n size less than 255\n");
      					fseek(get_file, SEEK_SET, 0);
      					bytes_read = fread(send_buf,PACKET_SIZE,1,get_file);
      					int seq_num = 1;
                sprintf(send_buf_w_seq,"SeQ%04d",seq_num);
                memcpy(send_buf_w_seq+strlen(send_buf_w_seq),send_buf,sizeof(send_buf));
                fclose(get_file);
      					printf("\nRead successful");
      					bytes_sent = sendto(s,send_buf_w_seq, bytes_read+7, 0,(struct sockaddr *) &remote, remote_len);
      					sendto(s,"SEQ9999ENDOFFILE1234",(sizeof("SEQ9999ENDOFFILE1234")),0,(struct sockaddr *) &remote, remote_len);
      					printf("\nNumber of bytes read = %d \nNumber of bytes sent = %d",bytes_read,bytes_sent);
      				}

      				if(filesize == size_check)
      				{
      					printf("\nFile Sent!");
      				}

      			}
      			else
      			{
      				printf("\nFile doesnt exist anymore\n");
      			}

      		printf("\n get command completed\n");
      		}
    	}

    	if (flag_put == 0)
      {
      		printf("\n put command recieved");
          printf("\nInside put\n");

          //printf("\n get command recieved\n");
          char filename[MAX_LINE];
          strncpy(filename,buf+4,len-3);
          printf("\nFilename - %s\n", filename);
 
      printf("\n after strcpy - %s fone", filename);
      FILE *get_file = fopen(filename,"w+");
      bzero(recv_buf_w_seq,sizeof(recv_buf_w_seq));
      //printf("opened file pid %d\n",getpid());
      int filesize = 0;
      int count = 1; 
      int old_seq;
      int endflag;
      char ack_buf[7];
      bzero(ack_buf,sizeof(ack_buf));
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
              bzero(ack_buf,sizeof(ack_buf));
              sprintf(ack_buf,"ACK%04d",seq_num);
              sendto(s,ack_buf,sizeof(ack_buf),0,&sin, slen);       
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
              sendto(s,ack_buf,sizeof(ack_buf),0,&sin, slen);
              continue;
            }
            else if(atoi(SEQ) > seq_num )
            {
              sendto(s,ack_buf,sizeof(ack_buf),0,&sin, slen);
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
      printf("\nFile Closed! File Size = %d\n",filesize);




      }

    	if (flag_exit == 0)
    	{
    		printf("\n Exit command recieved");
    		break;
    	}

    	bzero(buf,sizeof(buf));
  	}
    
    close(s);
  }



     	







