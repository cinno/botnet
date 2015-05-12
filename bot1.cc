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
   int client_len;
   int client_sockfd;
   int i;
   int nread;
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
   char *temp;

   struct sockaddr_in clientaddr;
   struct utsname buf;

   uname(&buf);
   time_t now;
   client_sockfd = socket(AF_INET, SOCK_STREAM, 0);
   clientaddr.sin_family = AF_INET;
   clientaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
   clientaddr.sin_port = htons(atoi(argv[2]));

   client_len = sizeof(clientaddr);

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
       }

       else if(strlen(buf_get) >= 2)
       {
           strncpy(message,buf_get,sizeof(buf_get)-1);
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
                       write(client_sockfd,"file creation complete",22);
                   else
                       write(client_sockfd,"file creation fail",18);
               }
           }
       }
   }
   return 0;
}

