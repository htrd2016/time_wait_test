#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>

int main(int argc, char *argv[])
{
  int listen_port;
  int sock;
  struct sockaddr_in addr;
  int client_sock;
  unsigned int len = sizeof(addr);
  int client_count = 0;

  if(argc<2)
  {
    printf("use age<path><port to listen>\n");
    return -1;
  }

  listen_port = atoi(argv[1]);
  sock = socket(PF_INET, SOCK_STREAM, 0);
  if(sock<=0)
  {
    printf("%s\n", strerror(errno));
    return -1;
  }

  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  addr.sin_port = htons(listen_port);  

  if(bind(sock, (struct sockaddr*)&addr, sizeof(addr))<0)
  {
    printf("%s\n", strerror(errno));
    close(sock);
    return -1;
  }

  if(listen(sock, 1)<0)
  {
    printf("%s\n", strerror(errno));
    close(sock);
    return -1;
  }

  for(;;)
  {
    client_sock = accept(sock, (struct sockaddr*)&addr, &len);
    printf("new connection ...\n");
    if(client_sock<=0)
    {
      printf("%s\n", strerror(errno));
      close(sock);
      return -1;
    }
    client_count++;
    printf("client count =%d\n", client_count);
  }

  close(client_sock);
  close(sock);
  return 0;
}
