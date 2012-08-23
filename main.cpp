#include "spdy_common.h"
#include <list>
#include <string.h>
#include <errno.h>
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
#include <semaphore.h>

typedef struct
{
  int id;
  int state;
  int read;
  ev_io worker_read;
  ev_io worker_write;
  char  rbuffer[8192];
  char  wbuffer[8192];
} CONTEXT;

static int listener_socket;
static pid_t parent_pid;
static void handle_read(struct ev_loop *loop, ev_io *w, int revents);
static void handle_accept(struct ev_loop *loop, ev_io *w, int revents);

typedef struct {
  CONTEXT **handlers;
  std::list<CONTEXT *> contexts;
} HANDLER;

HANDLER handler;

void *worker_thread(void *data)
{
  long arg;
  ev_io pipe_read;
  int pipefd[2];
  int index = (int) data;
  struct ev_loop *loop;
  static ev_io worker_read;
  int count;

  handler.handlers = (CONTEXT **) malloc(sizeof(CONTEXT *) * 8192);
  for (count = 0; count < 8192; count++) {
    handler.handlers[count] = (CONTEXT *) malloc(sizeof(CONTEXT));
    handler.handlers[count]->id = count;
    handler.contexts.push_back(handler.handlers[count]);
  }

  loop = ev_loop_new(EVFLAG_AUTO);

  ev_io_init(&worker_read, handle_accept, listener_socket, EV_READ);
  ev_io_start(loop, &worker_read);

  ev_loop(loop, 0);

  exit(0);
}

int create_server_socket()
{
  long arg;
  int listenfd = 0;
  struct sockaddr_in serv_addr;

  listenfd = socket(AF_INET, SOCK_STREAM, 0);
  serv_addr.sin_family = AF_INET; 
  serv_addr.sin_port   = htons(8080); 
  serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); 

  arg = fcntl(listenfd, F_GETFL, NULL);
  arg |= O_NONBLOCK;
  fcntl(listenfd, F_SETFL, arg);

  bind(listenfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));

  listen(listenfd, 10000);

  return listenfd;
}

void dump_buffer(const char *buf, int size)
{
  for (int count = 0; count < size; count++) {
    printf("0x%02X, ", buf[count] & 0x00FF);
    if ((count % 32) == 31) printf("\n");
  }
  printf("\n");
}

static void handle_read(struct ev_loop *loop, ev_io *w, int revents)
{
  CONTEXT *c = (CONTEXT *) w->data;

  if (c != NULL) {
    c->read = recv(w->fd, c->rbuffer, sizeof(c->rbuffer), 0);
    if (c->read == 0) {
      ev_io_stop(loop, w);
      handler.contexts.push_back(c);
    } else {
      dump_buffer(c->rbuffer, c->read);
    }
  }
}

static void handle_write(struct ev_loop *loop, ev_io *w, int revents)
{
  char buffer[8192];
  int read;
}

static void handle_accept(struct ev_loop *loop, ev_io *w, int revents)
{
  struct sockaddr_in addr;
  int clientfd;
  socklen_t sl = sizeof(addr);

  clientfd = accept(w->fd, (struct sockaddr *) &addr, &sl);
  if (clientfd == -1) return;

  CONTEXT *context = NULL;

  if (handler.contexts.size() > 0) {
    context = handler.contexts.front();
    handler.contexts.pop_front();
  }

  ev_io_init(&context->worker_read, handle_read, clientfd, EV_READ);
  ev_io_init(&context->worker_write, handle_write, clientfd, EV_WRITE);
  context->worker_read.data = (void *) context;
  context->worker_write.data = (void *) context;
  ev_io_start(loop, &context->worker_read);
}

void start_server()
{
  listener_socket = create_server_socket();
}

static void signal_terminate(int signo)
{
  if (getpid() == parent_pid) {
    close(listener_socket);
    printf("Parent Quit\n");
  }
 
  exit(0);
}

void init_signals()
{
  struct sigaction sig;

  sig.sa_flags = 0;
  sig.sa_handler = signal_terminate;

  if (sigaction(SIGINT, &sig, NULL) < 0) {
    printf("SIGINT failed : %s\n", strerror(errno));
    exit(0);
  }
  if (sigaction(SIGTERM, &sig, NULL) < 0) {
    printf("SIGTERM failed : %s\n", strerror(errno));
    exit(0);
  }

}

int main(int argc, char **argv)
{
  pid_t pid;
  int count;

  start_server();

  init_signals();

  parent_pid = getpid();

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

