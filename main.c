#include <ev.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <fcntl.h>

typedef struct
{
  int id;
  ev_io worker_read;
  ev_io worker_write;
  char  buffer[8192];
} CONTEXT;

static int listener_socket;

static void handle_read(struct ev_loop *loop, ev_io *w, int revents);
static void handle_accept(struct ev_loop *loop, ev_io *w, int revents);

static CONTEXT **handlers = NULL;

void *worker_thread(void *data)
{
  long arg;
  ev_io pipe_read;
  int pipefd[2];
  int index = (int) data;
  struct ev_loop *loop;
  static ev_io worker_read;

  printf("I0 %d %d\n", listener_socket, index);

  loop = ev_loop_new(EVFLAG_AUTO);

  ev_io_init(&worker_read, handle_accept, listener_socket, EV_READ);
  ev_io_start(loop, &worker_read);

  printf("Index : %p %d %d\n", loop, getpid(), index);
  ev_loop(loop, 0);
  printf("Index End\n");

  exit(0);
}

int create_server_socket()
{
  long arg;
  int listenfd = 0;
  struct sockaddr_in serv_addr;

  listenfd = socket(AF_INET, SOCK_STREAM, 0);
  serv_addr.sin_family = AF_INET; 
  serv_addr.sin_port   = htons(8082); 
  serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); 

  arg = fcntl(listenfd, F_GETFL, NULL);
  arg |= O_NONBLOCK;
  fcntl(listenfd, F_SETFL, arg);

  bind(listenfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));

  listen(listenfd, 10000);

  return listenfd;
}

static void handle_read(struct ev_loop *loop, ev_io *w, int revents)
{
  char buffer[8192];
  int read;

  printf("Handle Read : %p %d %d \n", loop, getpid(), read);
  read = recv(w->fd, buffer, 8192, 0);
  if (read == 0) ev_io_stop(loop, w);
}

static void handle_write(struct ev_loop *loop, ev_io *w, int revents)
{
  char buffer[8192];
  int read;

  printf("Handle Write : %p %d %d \n", loop, getpid(), read);
}

static void handle_accept(struct ev_loop *loop, ev_io *w, int revents)
{
  struct sockaddr_in addr;
  int clientfd;
  socklen_t sl = sizeof(addr);

  printf("EV Socket Accept \n");
  clientfd = accept(w->fd, (struct sockaddr *) &addr, &sl);
  if (clientfd == -1) return;

  printf("clientfd : %d %p\n", clientfd, loop);

  printf("CONTEXT : %d\n", handlers[0]->id);
  printf("CONTEXT : %d\n", handlers[1]->id);
  printf("CONTEXT : %d\n", handlers[2]->id);
  printf("CONTEXT : %d\n", handlers[3]->id);


  CONTEXT *context = (CONTEXT *) malloc(sizeof(CONTEXT));
  ev_io_init(&context->worker_read, handle_read, clientfd, EV_READ);
  ev_io_init(&context->worker_write, handle_write, clientfd, EV_WRITE);
  context->worker_read.data = (void *) context;
  ev_io_start(loop, &context->worker_read);
}

void start_server()
{
  listener_socket = create_server_socket();

#if 0
  loop = ev_default_loop(EVFLAG_AUTO);

  ev_io_init(&listener, handle_accept, listener_socket, EV_READ);
  ev_io_start(loop, &listener);

  ev_loop(loop, 0);

  exit(1);
#endif
}

int main(int argc, char **argv)
{
  pid_t pid;
  int count;

  start_server();

  handlers = (CONTEXT **) malloc(sizeof(CONTEXT *) * 8192);
  for (count = 0; count < 8192; count++) {
    handlers[count] = malloc(sizeof(CONTEXT));
    handlers[count]->id = count;
  }

  for (count = 0; count < 2; count++) {
    pid = fork();
    if (pid == 0) {
      // Child
      printf("Worker Thread : %d\n", count);
      worker_thread((void *) count);
    }
  }

  while (1) { sleep(1); }

  return 0;
}

