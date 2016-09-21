
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
  char buf[MAX_LINE],parsed_buf[MAX_LINE],value[MAX_LINE];
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
                  sprintf(send_buf_w_seq,"SeQ%04d",seq_num);
                  //printf("\nBuf with Seq - \n%s\nLength - %d sz %d\n",send_buf_w_seq,strlen(send_buf_w_seq),sizeof(send_buf_w_seq));
                  // strcat(send_buf_w_seq,"fg");
                  //printf("Send Buf - \n%s\n%d",send_buf,strlen(send_buf));
                  memcpy(send_buf_w_seq+strlen(send_buf_w_seq),send_buf,sizeof(send_buf));
                  //printf("\nBuf with Seq added - %s\n",send_buf_w_seq);


                 void datasend()
                 {
      						bytes_sent = sendto(s,send_buf_w_seq, bytes_read+7, 0,(struct sockaddr *) &remote, remote_len);
                  if(recvfrom(s, buf, sizeof(buf), 0,(struct sockaddr *) &remote, &remote_len)<0)
                  {
                    printf("Ack not recieved\n");
                    datasend();
                  }

                  int ack_flag = strcmp(buf,"ACK");

                  if(ack_flag == 0)
                  {
                    printf("\n Ack recieved, %d",count);
                  }
                  else
                  {
                    printf("\n Ack Missed,%d",count);
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
                char end_command[MAX_LINE] = "SEQ9999";
                strcat(end_command,value);

                sendto(s,end_command, (sizeof(end_command)),0,(struct sockaddr *) &remote, remote_len);
      				  
                //fclose(put_file);
              }
      				else
      				{
      					printf("\n size less than 255\n");
      					fseek(get_file, SEEK_SET, 0);
      					bytes_read = fread(send_buf,filesize+1,1,get_file);
      					int seq_num = 1;
                  sprintf(send_buf_w_seq,"SeQ%04d",seq_num);
                  memcpy(send_buf_w_seq+strlen(send_buf_w_seq),send_buf,sizeof(send_buf));

                fclose(get_file);
      					printf("\nRead successful");
      					bytes_sent = sendto(s,send_buf_w_seq, (sizeof(send_buf_w_seq)), 0,(struct sockaddr *) &remote, remote_len);
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



     	







