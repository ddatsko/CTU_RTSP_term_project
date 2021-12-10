#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <sockLib.h>
#include <string.h>

#define SERVER_PORT     80 /* Port 80 is reserved for HTTP protocol */
#define SERVER_MAX_CONNECTIONS  20

void www(int argc, char *argv[])
{
  int s;
  int newFd;
  struct sockaddr_in serverAddr;
  struct sockaddr_in clientAddr;
  int sockAddrSize;

  sockAddrSize = sizeof(struct sockaddr_in);
  bzero((char *) &serverAddr, sizeof(struct sockaddr_in));
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(SERVER_PORT);
  serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

  s=socket(AF_INET, SOCK_STREAM, 0);
  if (s<0)
  {
    printf("Error: www: socket(%d)\n", s);
    return;
  }


  if (bind(s, (struct sockaddr *) &serverAddr, sockAddrSize) == ERROR)
  {
    printf("Error: www: bind\n");
    return;
  }

  if (listen(s, SERVER_MAX_CONNECTIONS) == ERROR)
  {
    perror("www listen");
    close(s);
    return;
  }

  printf("www server running\n");

  while(1)
  {
    /* accept waits for somebody to connect and the returns a new file descriptor */
    if ((newFd = accept(s, (struct sockaddr *) &clientAddr, &sockAddrSize)) == ERROR)
    {
      perror("www accept");
      close(s);
      return;
    }

    /* The client connected from IP address inet_ntoa(clientAddr.sin_addr)
       and port ntohs(clientAddr.sin_port).

       Start a new task for each request. The task will parse the request
       and sends back the response.

       Don't forget to close newFd at the end */
  }
}
