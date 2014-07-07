#include <sys/socket.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>

static
unsigned long long _nano_time() {
  struct timespec tspec = {0};
  if (clock_gettime(CLOCK_REALTIME,&tspec) == 0) {
    return 1000000000L*tspec.tv_sec + tspec.tv_nsec;
  } else {
    return 0;
  }
}


static void * client(void * a) {
  int socket_fd = 0;
  struct sockaddr_in addr = {0};
  int i = 0;

  socket_fd = socket(AF_INET, SOCK_STREAM, 0);

  if (socket_fd < 0) {
    printf("Client failed\n");
    return 0;
  }

  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = inet_addr("127.0.0.1");
  addr.sin_port = htons(2020);

  if (connect(socket_fd, (struct sockaddr*)&addr, sizeof(addr))< 0) {
    printf("Failed to connect\n");
    close(socket_fd);
    return 0;
  }
  

  char data[1024*1024] = {0};
  memset(data, 0x42, sizeof(data));
  printf("Sending\n");
  for(i = 0; i < 128;++i) {
    write(socket_fd, data, 1024*1024);
    read(socket_fd, data, 1024*1024);
    write(socket_fd, data, 1024*1024);
    fsync(socket_fd);
  }
  printf("Data sent\n");
  close(socket_fd);
}


int main(int c, char **a) {
  pthread_t t = {0};
  int server_fd = socket(AF_INET, SOCK_STREAM, 0);
  int port = 2020;
  int reuse_addr_option = 1;
  struct sockaddr_in serv_addr = {0};

    
  if (setsockopt(server_fd, SOL_SOCKET,
                 SO_REUSEADDR,
                 (char *)&reuse_addr_option, sizeof(reuse_addr_option)) < 0 ) {
    close(server_fd);
    return -1;
  }
  
  
  
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(port); 
  {
    socklen_t lserv_addr = sizeof(serv_addr);
    if ( bind ( server_fd, (struct sockaddr *)&serv_addr,	
                lserv_addr) < 0) {
      close(server_fd);
      return -2;
    }}
  
  {
    uint flags = fcntl(server_fd, F_GETFL, 0);
    fcntl(server_fd, F_SETFL, flags | O_NONBLOCK );
  }
  
  if (listen(server_fd, 20) != 0) {
    return 0;
  }

  pthread_create(&t,0,client,0);

  {
    int i = 0;
    int client_fd = 0;
    char buf[1024*1024] = {0};
    while ((client_fd = accept(server_fd,0,0)) < 0) {
      if (errno != EAGAIN) {
        printf("Error unable to accept\n");
        return 0;
      }
      usleep(2);
    }
    unsigned long long start = _nano_time();
    printf("Got client ... %d\n",client_fd);
    for(i = 0; i < 128;++i) {
      read(client_fd, buf, 1024*1024);
      write(client_fd, buf, 1024*1024);
      read(client_fd, buf, 1024*1024);
    }
    close(client_fd);
    printf("Time %llu\n",_nano_time() - start);
  }
}



