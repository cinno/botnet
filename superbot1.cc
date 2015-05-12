#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h> 
#include <sys/poll.h>

#define MAX_SOCKET 100

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

int main(int argc, char **argv)
{
   socklen_t client_len, bot_len;
   int client_sockfd, server_sockfd, bot_sockfd, sockfd;
   int state,nread,mread;
   int i,max,mmax;
   char id[2];
   char bot_id[MAX_SOCKET][2];
   char bot_number[2];
   char instruction[255];
   char buf[255];
   char buf_in[255];
   char buf_get[255];
   char message[255];
   char message_from_master[255];
   char message_parse[255];
   char show_message[255];
   char show_message_temp[255];
   char search_message[255];
   char search_message_temp[255];
   char send_message[255];
   char ip_addr[255];
   char *temp;
   int bot_table[8] = {0,0,0,0,0,0,0,0};
   int search_num = 0;
   int packet_num = 0;

   struct sockaddr_in clientaddr, serveraddr, botaddr;
   struct pollfd bot[MAX_SOCKET];
   struct pollfd master[MAX_SOCKET];
   
   state = 0;

   memset(message,'\0',255);
   memset(instruction,'\0',255);

   //**************server
   if((server_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
       error("socket error");
   bzero(&serveraddr, sizeof(serveraddr));
   serveraddr.sin_family = AF_INET;
   serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
   serveraddr.sin_port = htons(atoi(argv[2])); //connect to child-bot

   state = bind(server_sockfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr));

   if(state == -1)
       error("bind error");

   state = listen(server_sockfd,10);
   if(state == -1)
       error("listen error");


   //**************client
   client_sockfd = socket(AF_INET, SOCK_STREAM, 0);
   clientaddr.sin_family = AF_INET;
   clientaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
   clientaddr.sin_port = htons(60000); //connect to master-bot

   client_len = sizeof(clientaddr);
   bot_len = sizeof(botaddr);

   

   if(connect(client_sockfd, (struct sockaddr *)&clientaddr, client_len) < 0)
   {
       perror("Connect error : ");
       exit(0);
   }

   strncpy(id,argv[1],2);

   write(client_sockfd,id,2);
  
   //poll start

   master[0].fd = client_sockfd;
   master[0].events = POLLIN;
   for(i=1;i<MAX_SOCKET;i++)
   {
       master[i].fd = -1;
   }
   mmax = 0;

   bot[0].fd = server_sockfd;
   bot[0].events = POLLIN;
   for(i=1;i<MAX_SOCKET;i++)
   {
       bot[i].fd = -1;
   }
   max = 0;

   for(i=0;i<MAX_SOCKET;i++)
   {
       bot_id[i][0] = '\0';
       bot_id[i][1] = '\0';
   }
   
   while(1)
   {
       nread = poll(bot,max+i,1000);

       strncpy(bot_number,"00",2);

       if(bot[0].revents & POLLIN)
       {
           bot_sockfd = accept(server_sockfd,(struct sockaddr *)&botaddr,&bot_len);
           for(i=1;i<MAX_SOCKET;i++)
           {
               if(bot[i].fd < 0)
               {
                   bot[i].fd = bot_sockfd;
                   bot[i].events = POLLIN;
                   read(bot[i].fd,bot_id[i],2);
                   printf("bot %c%c is connected\n",bot_id[i][0],bot_id[i][1]);
                   sprintf(message,"Child-bot %c%c is connected with Super-bot %c%c.",bot_id[i][0],bot_id[i][1],id[0],id[1]);
                   write(client_sockfd,message,sizeof(message));
                   bot_table[i] = 1;
                   break;
               }
           }

           if(i > max)
               max = i;

           if(--nread <= 0)
               continue;
       }

       mread = poll(master,1,1000);

       if(master[0].revents & POLLIN)
       {
           read(client_sockfd,message_from_master,sizeof(message_from_master));
           strncpy(message_parse,message_from_master,sizeof(message_from_master));
           temp = strtok(message_parse," ");
           strncpy(instruction,temp,sizeof(temp));

           if(strstr(instruction,"show"))
           {
               memset(show_message,'\0',255);
               memset(show_message_temp,'\0',255);
               sprintf(show_message,"Superbot #%c%c : [",id[0],id[1]);

               for(i=0;i<MAX_SOCKET;i++)
               {
                   if(bot_id[i][1] != '\0')
                   {
                       sprintf(show_message_temp," Childbot #%c%c ",bot_id[i][0],bot_id[i][1]);
                       strcat(show_message,show_message_temp);
                   }
               }
               strcat(show_message,"]");
               write(client_sockfd,show_message,sizeof(show_message));
           }
           else if(strstr(instruction,"send"))
           {
               for(i=1;i<MAX_SOCKET;i++)
               {
                   if(bot[i].fd >= 0)
                   {
                       write(bot[i].fd,message_from_master,sizeof(message_from_master));
                       break;
                   }
               }
           }
           else if(strstr(instruction,"search"))
           {
               memset(search_message,'\0',255);
               memset(search_message_temp,'\0',255);
               temp = strtok(NULL," ");
               strncpy(bot_number,temp,sizeof(temp));
               search_num = atoi(bot_number);

               sprintf(search_message,"Superbot #%c%c : ",id[0],id[1]);

               for(i=0;i<10;i++)
               {
                   if(bot_id[i][0] == bot_number[0] && bot_id[i][1] == bot_number[1])
                   {
                       sprintf(search_message_temp,"Childbot #%c%c -> Alived",bot_number[0],bot_number[1]);
                       break;
                   }
                   else
                   {
                       sprintf(search_message_temp,"Childbot #%c%c -> Not respond",bot_number[0],bot_number[1]);
                   }
               }
               strcat(search_message,search_message_temp);
               write(client_sockfd,search_message,sizeof(search_message));
           }
       }

       for(i=1;i<=max;i++)
       {
           if((sockfd = bot[i].fd) < 0)
               continue;
           if(bot[i].revents & (POLLIN | POLLERR))
           {
               memset(buf,'\0',255);
               if(read(sockfd,buf,255) <= 0)
               {
                   printf("bot %c%c is disconnected\n",bot_id[i][0],bot_id[i][1]);
                   sprintf(message,"Child-bot %c%c is disconnected from Super-bot %c%c.",bot_id[i][0],bot_id[i][1],id[0],id[1]);
                   write(client_sockfd,message,sizeof(message));
                   bot_table[i] = 0;
                   bot_id[i][0] = '\0';
                   bot_id[i][1] = '\0';
                   close(bot[i].fd);
                   bot[i].fd = -1;
                   break;
               }
               write(client_sockfd,buf,sizeof(buf));//send to master
           }
           if(!strstr(instruction,"send"))
               write(bot[i].fd,message_from_master,sizeof(message_from_master));//broadcast
       }
       memset(message_from_master,'\0',255);
   }
   return 0;
}






