#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/time.h>
#include <pthread.h>
#include <linux/tcp.h>

#define MAX_CLIENT 2048

int g_client_sockets[MAX_CLIENT];
int g_client_count = 0;
int g_command_server_socket = 0;

char g_server_ip[64];
int g_server_port = -1;

void init_client_sockets()
{
  int i=0;
  for(i=0;i<MAX_CLIENT; i++)
  {
    g_client_sockets[i] = 0;
  }
}

int is_client_disconnected(int sock)
{
  struct tcp_info info; 
  int len=sizeof(info); 
  getsockopt(sock, IPPROTO_TCP, TCP_INFO, &info, (socklen_t *)&len); 
  if((info.tcpi_state != 1))
  {
    //printf("%d\n", (int)info.tcpi_state);
    return 1;
  }
  return 0;
}

void *check_clients_connected_thread(void *argv)
{
  int i=0;
  while(1)
  {
     for(i=0;i<g_client_count;i++)
     {
        if(g_client_sockets[i]!=0 && is_client_disconnected(g_client_sockets[i]) == 1)
        {
           printf("index %d client is closed\n", i);
           close(g_client_sockets[i]);
           g_client_sockets[i] = 0;
        }
     }
     usleep(10000);
  } 
}

int client_connect_to_server(int *client, const char *server_ip, int server_port)
{
  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(struct sockaddr));
  *client = socket(AF_INET, SOCK_STREAM, 0);
  if(*client<=0)
  {
    printf("error:%s\n", strerror(errno));
    return -1;
  }

  addr.sin_family = AF_INET;
  addr.sin_port = htons(server_port);
  addr.sin_addr.s_addr = inet_addr(server_ip);
  
  //printf("server:%s,port:%d\n", server_ip, server_port);
  if(connect(*client, (struct sockaddr*)&addr, sizeof(struct sockaddr_in)) <0)
  {
    printf("%s\n", strerror(errno));
    close(*client);
    return -1;
  }
  return 0;
}

int multi_clients_connect_to_server(int count, const char *server_ip, int server_port)
{
  int i = 0;
  int max = 0;
  int dealed = 0;
  struct timeval start, end;
  long long time_tmp = 0;

  if(g_client_count<0)
  {
     g_client_count = 0;
  }  

  i = g_client_count;
  max = g_client_count+count;
  
  if(g_client_count<0)
  {
     g_client_count = 0;
  }

  gettimeofday(&start, NULL);
  for(; i<MAX_CLIENT&&i<max; i++)
  {
    if(-1 == client_connect_to_server(&(g_client_sockets[i]), server_ip, server_port))
    {
      return -1;
    }
    g_client_count++;
    dealed++;
  }
  gettimeofday(&end, NULL);
  time_tmp = 1000000*(long long)(end.tv_sec-start.tv_sec) + (long long)(end.tv_usec-start.tv_usec);

  printf("start at: %d.%d\n", (int)start.tv_sec, (int)start.tv_usec);
  printf("end   at: %d.%d\n", (int)end.tv_sec, (int)end.tv_usec);
  //printf("avg used time is %lf usec of %d,total used time is %d sec %d usec\n", (time_tmp)/(double)dealed, count, (int)(time_tmp/1000000), (int)(time_tmp%1000000));
  
  printf("the count of %d clients connect to %s:%d total used time is %d seconds and %d millimicro seconds\n", dealed,server_ip, server_port, (int)(time_tmp/1000000), (int)(time_tmp%1000000));
  printf("after connect:left client socket:%d\n", g_client_count);
  return 0;
}

int multi_clients_close(int count)
{
  int i = g_client_count - 1;
  int min = g_client_count-count-1;
  for(;i>min&&i>=0;i--)
  {
    close(g_client_sockets[i]);
    g_client_count--;
  }

  if(g_client_count<0)
  {
     g_client_count = 0;
  }

  printf("after close:left client socket:%d\n", g_client_count);
  return 0;
}

int loop_command_server(int *server, int listen_port)
{
  char buffer[1024];
  struct sockaddr_in addr;
  int client_sock;
  unsigned int len = sizeof(addr);
  size_t recv_len = 0;
  int count = 0;
  int on=1;

  *server = socket(PF_INET, SOCK_STREAM, 0);
  if(*server<=0)
  {
    printf("%s\n", strerror(errno));
    return -1;
  }

  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  addr.sin_port = htons(listen_port);  
  
  if((setsockopt(*server, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)))<0)  
  {  
     perror("setsockopt failed");  
     return -1; 
  }  

  if(bind(*server, (struct sockaddr*)&addr, sizeof(addr))<0)
  {
    printf("%s\n", strerror(errno));
    close(*server);
    return -1;
  }

  if(listen(*server, 1)<0)
  {
    printf("listen %s\n", strerror(errno));
    close(*server);
    return -1;
  }

  for(;;)
  {
    client_sock = accept(*server, (struct sockaddr*)&addr, &len);
    printf("new connection ...\n");
    if(client_sock<=0)
    {
      printf("%s\n", strerror(errno));
      close(*server);
      return -1;
    }

    memset(buffer, 0, 1024);
    if((recv_len=recv(client_sock, buffer, 1024, 0))<=0)
    {
      printf("%s\n", strerror(errno));
      close(client_sock);
      break;
    }

    printf("recv:%s\n", buffer);
    if (strstr(buffer, "open:") != 0)
    {
      printf("%s\n", buffer);
      count = atoi(buffer+strlen("open:"));
      printf("count:%d\n", count);
      multi_clients_connect_to_server(count, g_server_ip, g_server_port);
    }
    else if(strstr(buffer, "close:") != 0)
    {
      printf("%s\n", buffer);
      count = atoi(buffer+strlen("close:"));
      printf("count:%d\n", count);
      multi_clients_close(count);
    }
    else
    {
      printf("error command:%s\n", buffer);
      sprintf(buffer, "please send like \"open:10\" or \"close:10\"\n");
      write(client_sock, buffer, strlen(buffer)+1);
    }
    
    close(client_sock);
    //printf("socket closed!!!\n");
  }

  close(*server);
  return 0;
}

int main(int argc, char ** argv)
{
  int command_server_port;
  pthread_t ntid;
  int temp;

  g_client_count = 0;

  if(argc<4)
  {
    printf("use age<path><server ip><server port><command server port>\n");
    return -1;
  }
  strcpy(g_server_ip, argv[1]);
  g_server_port = atoi(argv[2]);
  //client_count = atoi(argv[3]);
  command_server_port = atoi(argv[3]);
  
  init_client_sockets();

  printf("max client count:%d,command server port is %d\n", MAX_CLIENT, command_server_port);

  if((temp=pthread_create(&ntid, NULL, check_clients_connected_thread, NULL))!= 0)
  {
    printf("can't create thread: %s\n",strerror(temp));
    return -1;
  }

  loop_command_server(&g_command_server_socket, command_server_port);
  //printf("max client count:%d, client count to connect:%d,server port is:%d\n", MAX_CLIENT, client_count, g_server_port);


  //multi_clients_connect_to_server(client_count, g_server_ip, g_server_port);

  //sleep(30);

  //multi_clients_close(client_count);

  return 0;
}
