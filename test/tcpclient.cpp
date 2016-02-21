#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>

#define MAXLINE 80
int port = 50000;

int main(int argc, char *argv[])
{
  struct sockaddr_in pin;
  int sock_fd;
  char buf[MAXLINE];
  char str[MAXLINE];
  int n;


  signal(SIGPIPE,SIG_IGN);

  bzero(&pin, sizeof(pin));
  pin.sin_family = AF_INET;
  inet_pton(AF_INET, "192.168.1.100", &pin.sin_addr);
  pin.sin_port = htons(port);
  
  sock_fd = socket(AF_INET, SOCK_STREAM, 0);
  n=connect(sock_fd, (sockaddr *)&pin, sizeof(pin));
  if (-1 == n)
  {
     perror("call connect");
     exit(1);
  }
  while(NULL != fgets(str,MAXLINE, stdin))
  {
    write(sock_fd, str, strlen(str)+1);
    n=read(sock_fd, buf, MAXLINE);
    if (0 == n)
      printf("the othere side has been closed.\n");
    else
      printf("receive from server:%s\n",buf);
  }
  close(sock_fd);
  return 0;
}
