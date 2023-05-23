#include <iostream>
#include <string>
#include <fstream>
#include "stdlib.h"

#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>

#include "socket_path.h"

using namespace std;

#define MAX 100

int fd;
int sock = 0;
struct sockaddr_un addr;
struct sockaddr_un from;

int recv_msg_from_server(int socketfd, char *dataReceive)
{

  struct msghdr msg;
  struct iovec iov;
  int s;

  memset(&msg, 0, sizeof(msg));
  memset(&iov, 0, sizeof(iov));

  msg.msg_name = NULL;
  msg.msg_namelen = 0;
  iov.iov_base = dataReceive;
  iov.iov_len = MAX;
  msg.msg_iov = &iov;
  msg.msg_iovlen = 1;
  msg.msg_control = NULL;
  msg.msg_controllen = 0;
  msg.msg_flags = 0;


  printf("Receive msg .............\n");
  s = recvmsg(socketfd, &msg, 0);
  if (s < 0)
  {
    printf("Receive msg error: %d\n", s);
    return s;
  }
  printf("Receive msg: %s, length: %d\n", (char *)msg.msg_iov->iov_base, msg.msg_iov->iov_len);
  return s;
}

int send_msg_to_server(int socketfd, char *dataSend)
{
  struct msghdr msg;
  struct iovec iov;
  int s;
  int timeout;

  memset(&addr, 0, sizeof(addr));
  addr.sun_family = AF_UNIX;
  strcpy(addr.sun_path, SERVER_SOCK_FILE);
  timeout = 0;
  while (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
  {
    usleep(100000);
    timeout++;
    if (timeout > 50)
    {
      printf("Connect server error: %d\n", s);
      return -1;
    }
  }

  memset(&msg, 0, sizeof(msg));
  memset(&iov, 0, sizeof(iov));

  msg.msg_name = NULL;
  msg.msg_namelen = 0;
  iov.iov_base = dataSend;
  iov.iov_len = strlen(dataSend) + 1;
  msg.msg_iov = &iov;
  msg.msg_iovlen = 1;
  msg.msg_control = NULL;
  msg.msg_controllen = 0;
  msg.msg_flags = 0;

  printf("Send msg: %s, length: %d\n", (char *)msg.msg_iov->iov_base, msg.msg_iov->iov_len);

  s = sendmsg(socketfd, &msg, 0);

  if (s < 0)
  {
    printf("Send msg error: %d\n", s);
    return s;
  }

  return s;
}

int create_socket_client(void)
{
  int timeout = 0;
  if ((fd = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0)
  {
    printf("Create socket error\n");
    return -1;
  }

  memset(&addr, 0, sizeof(addr));
  addr.sun_family = AF_UNIX;
  strcpy(addr.sun_path, CLIENT_SOCK_FILE);
  unlink(CLIENT_SOCK_FILE);
  if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
  {
    printf("Bind socket error\n");
    return -1;
  }

  return 1;
}

int main(int argc, char *argv[])
{
  char dataReceive[MAX];
  create_socket_client();

  for (int i = 0; i < argc; i++)
  {
    if (strcmp(argv[i], "-r") == 0)
    {
      recv_msg_from_server(fd, dataReceive);
    }

    if (strcmp(argv[i], "-w") == 0)
    {
      send_msg_to_server(fd, argv[i + 1]);
    }
  }
  

  return 0;
}
