#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

int main(int argc, char ** argv)
{
  char server_ip[64];
  int server_port;
  int sock = 0;
  char buffer[1024] = {0};
  int send_len = -1;

  struct sockaddr_in addr;
  if(argc<4)
  {
    printf("use age<path><server ip><server port><data to send>\n");
    return -1;
  }

  strcpy(server_ip, argv[1]);
  server_port = atoi(argv[2]);
  strcpy(buffer, argv[3]);

  memset(&addr , 0, sizeof(struct sockaddr));
  printf("%s %d %s\n", server_ip, server_port, buffer);

  sock = socket(AF_INET, SOCK_STREAM, 0);
  if(sock<=0)
  {
    printf("error:%s\n", strerror(errno));
    return -1;
  }
  
  addr.sin_family = AF_INET;
  addr.sin_port = htons(server_port);
  addr.sin_addr.s_addr = inet_addr(server_ip);
  if(connect(sock, (struct sockaddr*)&addr, sizeof(struct sockaddr_in)) <0)
  {
    printf("%s\n", strerror(errno));
    close(sock);
    return -1;
  }  

  if((send_len=send(sock, buffer, strlen(buffer)+1, 0))<=0)
  {
    printf("%s\n", strerror(errno));
    return -1;
  }
  printf("send buffer len:%d\n", send_len);
  close(sock);
  return 0;
}
