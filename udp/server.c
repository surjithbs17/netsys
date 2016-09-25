
#include <stdio.h>
#include <stdlib.h>
#include </usr/include/x86_64-linux-gnu/sys/types.h>
#include </usr/include/x86_64-linux-gnu/sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>
#include <string.h>


#define MAX_LINE 2048
#define MAX_SEQ_NUM 128
#define PACKET_SIZE 2048

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
  int nofile_flag = 0;
  
  if(argc > 1) {
    int port = atoi(argv[1]);
    PORT_NUM = port;
    printf(" Port Number = %d\n",port);
  }

  //Address data structure
  bzero((char *)&sin,sizeof(sin));
  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = htonl(INADDR_ANY);
  //htonl(INADDR_ANY)//
  sin.sin_port = htons((unsigned short) PORT_NUM);

  //setup passive open
  if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("Error: Socket");
    exit(1);
  }

  int optval = 1;
  setsockopt(s, SOL_SOCKET, SO_REUSEADDR,(const void *)&optval , sizeof(int));

  //printf("\nSocket Created!");
  if((bind(s,(struct sockaddr *)&sin, sizeof(sin))) < 0) {
    perror("Error: Bind");
    exit(1);
  }
  remote_len = sizeof(remote);
  
  slen = sizeof(sin);
  
  bzero(buf,sizeof(buf));

  while (len = recvfrom(s, buf, sizeof(buf), 0,(struct sockaddr *) &remote, &remote_len))   //While loop for continous operation
  {
    if(len<0) 
      continue;

    printf("Recieved Buffer %s\n",buf );
                                                   	
    if(strcmp(buf,"ls") == 0)                            //Checking for commands and setting the flags
    {
      printf("ls command recieved\n");
     	flag_ls = 0;
    }
    else
    {
    	flag_ls = -1;
    }

    if(strcmp(buf,"EXIT") == 0)
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
      printf("Get command recieved\n");
      flag_get = 0;
    }
    else
   	{
    	flag_get = -1;
    }
   	if(strstr(buf, "put") != NULL)
   	{
   		printf("Put command recieved\n");
   		flag_put = 0;
    }
    else
    {
      flag_put = -1;
    }


   
     if( (flag_ls == 0) )     //Inside ls command
      {
      	
      	system("ls > 'ls_file.txt'");  //doing a system command to list the files and piping the output to a file
      	ls_fp = fopen("ls_file.txt","r");    
      	int i=0;                                         
     		while(fscanf(ls_fp,"%s",parsed_buf) != EOF)    //Reading the file
     		{
     			int status = sendto(s, parsed_buf, (strlen(parsed_buf) + 1),0,(struct sockaddr *) &remote, remote_len); //sending it through socket
			    if(status == -1)
			    {
				    perror("Error: Send Failed");
				  }
				
      	}
      	int status = sendto(s, "end", (strlen("end") + 1),0,(struct sockaddr *) &remote, remote_len);  //command to oend the operation
				if(status == -1)
				{
				  perror("Error: Send Failed");
				}

     		fclose(ls_fp);
        system("rm ls_file.txt"); //removing the ls file
       
      }

      if (flag_get == 0)
      {
     		printf("\n get command recieved\n");
      	char *filename = (char *)malloc(sizeof(char)*(MAX_LINE+1));
    		strncpy(filename,buf+4,len-3);
      	printf("\nFilename - %s\n", filename);

      	if(filename != NULL)
      	{
      		int file_exists = doesFileExist(filename);  //checking whether the file exists
      	  
    			if(file_exists == 0)
     			{
     				FILE *get_file = fopen(filename,"r+"); //opening it in reading mode
            
     				if(get_file == NULL)
     				{
     					printf("\nError: File open %s\n",filename);
     					
     				}
     				
     				fseek(get_file,0,SEEK_END);  //Routing to check the file size
     				long filesize = ftell(get_file);
     				rewind(get_file);
     				printf("\nFile Size = %d",filesize);
            char send_buf[MAX_LINE];
     				long bytes_read,bytes_sent,size_check =0;
            //long bytes_write;
            char send_buf_w_seq[PACKET_SIZE+7];   //Buffer with sequence numbeer added
            int count = 0;
     				if(filesize > PACKET_SIZE) //if file is bigger than packet size it has to be put in a loop
     				{
     					
     					fseek(get_file, SEEK_SET, 0); //setting the file pointer to origin
              struct timeval tv;
              tv.tv_sec = 0;
              tv.tv_usec = 100000;
              if (setsockopt(s, SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(tv)) < 0)  //timeout setup
              {
               perror("Sock Timeout Error");
              }
    					while (size_check < filesize)
    					{
                count++;  //count for file size calculations
                bzero(send_buf,sizeof(send_buf));
                bzero(send_buf_w_seq,sizeof(send_buf_w_seq));
    						bytes_read = fread(send_buf,1,PACKET_SIZE,get_file);  //reading the buffer from file
                int seq_num = count % MAX_SEQ_NUM;  //giving sequence number in round robin fashion

                sprintf(send_buf_w_seq,"SeQ%04d",seq_num);  //formatiing sequence buffer
                memcpy(send_buf_w_seq+strlen(send_buf_w_seq),send_buf,sizeof(send_buf));  //concatenatin two buffers
                

                void datasend() //function - inline, to send the buffer iteratively until it recieves ack
                {
      					  bytes_sent = sendto(s,send_buf_w_seq, bytes_read+7, 0,(struct sockaddr *) &remote, remote_len);
                  bzero(buf,sizeof(buf));
                  if(recvfrom(s, buf, sizeof(buf), 0,(struct sockaddr *) &remote, &remote_len)<0)
                  {
                    printf("-");
                    datasend();
                  }

                  
                  char ack_buf[7];
                  bzero(ack_buf,sizeof(ack_buf));
                  sprintf(ack_buf,"ACK%04d",seq_num);
                  int ack_flag = strcmp(buf,ack_buf);
                  printf("Recieved Buffer - %s, ACK Buffer - %s\n",buf,ack_buf);

                  if(ack_flag == 0)
                  {
                    printf(".");
                  }
                  else
                  {
                    printf("!\n");
                    datasend();
                  }
                }

                datasend();
      					size_check = size_check + bytes_read;
                
                  
                bzero(send_buf,sizeof(send_buf));
                
      				}
      				fclose(get_file);
              char md5command[256] = "md5sum "; //md5checksum routine
              strcat(md5command,filename);
              strcat(md5command," > md5value.txt");
              
              system(md5command);
              FILE* md5file = fopen("md5value.txt","r+");
              fscanf(md5file,"%s",value);
              printf("\nMD5 Value - %s\n",value);
              system("rm md5value.txt");
              char end_command[MAX_LINE] = "SEQ9999";
              strcat(end_command,value);
              sendto(s,end_command, (sizeof(end_command)),0,(struct sockaddr *) &remote, remote_len);
      				  
              
            }
      			else
      			{
      				printf("\n Size less than %d\n",PACKET_SIZE);
      				fseek(get_file, SEEK_SET, 0);     //file reading routine
              bzero(send_buf,sizeof(send_buf));
              bzero(send_buf_w_seq,sizeof(send_buf_w_seq));
      				bytes_read = fread(send_buf,sizeof(char),PACKET_SIZE,get_file);
              
      				int seq_num = 1;
              sprintf(send_buf_w_seq,"SeQ%04d",seq_num);
              memcpy(send_buf_w_seq+7,send_buf,sizeof(send_buf));
              fclose(get_file);
      			  
                              
                void datasend1()  //inline function for repetetive sending until it recieves ack
                {
                  bytes_sent = sendto(s,send_buf_w_seq, bytes_read+7, 0,(struct sockaddr *) &remote, remote_len);
                  bzero(buf,sizeof(buf));
                  if(recvfrom(s, buf, sizeof(buf), 0,(struct sockaddr *) &remote, &remote_len)<0)
                  {
                    printf("^");
                    datasend1();
                  }

                  printf("Buffer - %s\n",buf);
                  char ack_buf[7];
                  bzero(ack_buf,sizeof(ack_buf));
                  sprintf(ack_buf,"ACK%04d",seq_num);
                  int ack_flag = strcmp(buf,ack_buf);
                  printf("Recieved Buffer - %s, ACK Buffer - %s\n",buf,ack_buf);

                  if(ack_flag == 0)
                  {
                    printf("Ack match \n");
                  }
                  else
                  {
                    printf("Ack not match\n");
                    datasend1();
                  }
                }

                datasend1();

                nofile_flag = 0;
                size_check = bytes_sent-7;
                
              char md5command[256] = "md5sum ";     //md5 routine
              strcat(md5command,filename);
              strcat(md5command," > md5value.txt");
              
              system(md5command);
              FILE* md5file = fopen("md5value.txt","r+");
              fscanf(md5file,"%s",value);
              printf("\nMD5 Value - %s\n",value);
              system("rm md5value.txt");
              char end_command[MAX_LINE] = "SEQ9999";
              strcat(end_command,value);
              sendto(s,end_command, (sizeof(end_command)),0,(struct sockaddr *) &remote, remote_len);
      				
      			}
            
      			if(filesize == size_check)
      			{
      				printf("\nFile Sent!\n");
              nofile_flag = 0;
      			}

      		}
      		else  //file doesnt exist condition
      		{
      			printf("\nFile doesnt exist anymore\n");
            sendto(s,"SEQ8888ENDOFFILE1234",(sizeof("SEQ8888ENDOFFILE1234")),0,(struct sockaddr *) &remote, remote_len);
            nofile_flag = 1;
            continue;
      		}

      	}
    	}

    	if (flag_put == 0)
      {

        printf("\nInside put\n");

        char filename[MAX_LINE];      //file open routine
        strncpy(filename,buf+4,len-3);
        printf("\nPutting the file - %s\n", filename);
        FILE *get_file = fopen(filename,"w+");
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
            
          int seq_num = count % MAX_SEQ_NUM; 
          char *SEQ = (char *)malloc(sizeof(char)*(10+1));  //sequence term computations in round robin fashion
          bzero(SEQ,sizeof(SEQ));
          strxfrm(SEQ,recv_buf_w_seq+3,4);
          memcpy(recv_buf,recv_buf_w_seq+7,MAX_LINE);
          printf(".");
          if(seq_num == atoi(SEQ))
          {
            //Sequence Number Match
              count++;
              endflag = 1;
              bzero(ack_buf,sizeof(ack_buf));
              sprintf(ack_buf,"ACK%04d",seq_num);
              sendto(s,ack_buf,sizeof(ack_buf),0,&sin, slen);       
              old_seq = seq_num;  //having an account on old seq num
          }
          else if(atoi(SEQ)==9999)
          {
            //End Seq match
            endflag = 0;
          }
          else if(atoi(SEQ) == old_seq)
          {
            printf("~");
            sendto(s,ack_buf,sizeof(ack_buf),0,&sin, slen);
            continue;
          }
          else if(atoi(SEQ) > seq_num )
          {
            sendto(s,ack_buf,sizeof(ack_buf),0,&sin, slen);
            continue;
          }

          if(endflag == 0)
          {
            if(atoi(SEQ) == 9999)
            {
              //End of file seq num match
              strxfrm(hash_value,recv_buf_w_seq+7,128); //decoding md5hash value from end string
            }
            break;
          }
          else
          {  
            
            fwrite(recv_buf,sizeof(char), len-7,get_file);
            //writing onto a file
          }

          filesize = filesize+(len-7);
          bzero(recv_buf,sizeof(recv_buf));
          bzero(SEQ,sizeof(SEQ));

      }
      fclose(get_file);

    if(nofile_flag == 0)
    {
      char *md5command = (char *)malloc(sizeof(char)*(MAX_LINE+1)); //md5routine
      //char md5command[256] = "md5sum ";
      strcpy(md5command,"md5sum ");
      strcat(md5command,filename);
      strcat(md5command," > md5value.txt");
      //printf("%s\n",md5command );
      system(md5command);
      FILE* md5file = fopen("md5value.txt","r+");
      fscanf(md5file,"%s",value);
      system("rm md5value.txt");

      printf("MD5sum value from Sender - %s\nMD5sum value From local file - %s\n",hash_value,value);
      if(strcmp(value,hash_value) == 0)
      {
        printf("File Match Success!\n");
      }
      printf("\nFile Closed! File Size = %d\n",filesize);
    }
    }

  	if (flag_exit == 0)  //exit command recieved
  	{
   		printf("\n Exit command recieved\n");
    	exit(1);
    }

    bzero(buf,sizeof(buf));
  	}
    
    close(s);
    

  }