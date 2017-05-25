#include "birdc.h"
#define SERVER_READ_BUF_LEN 4096


static int server_fd;
static byte server_read_buf[SERVER_READ_BUF_LEN];
static byte *server_read_pos = server_read_buf;

static void
bird_connect(char *server_path){
  struct sockaddr_un sa;

  server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
  if (server_fd < 0)
    printf("Cannot create socket");

  bzero(&sa, sizeof(sa));
  sa.sun_family = AF_UNIX;
  strcpy(sa.sun_path, server_path);
  if (connect(server_fd, (struct sockaddr *) &sa, SUN_LEN(&sa)) < 0)
    printf("Unable to connect to server control socket (%s)", server_path);
  if (fcntl(server_fd, F_SETFL, O_NONBLOCK) < 0)
    printf("fcntl");
}

static void
server_send(char *cmd){
  int l = strlen(cmd);
  byte *z = alloca(l + 1);

  memcpy(z, cmd, l);
  z[l++] = '\n';
  while (l){
    int cnt = write(server_fd, z, l);
    if (cnt < 0){
  	  if (errno == EAGAIN){}
  	    //WAIT?
  	  else if (errno == EINTR)
  	    continue;
  	  else
  	    printf("Server write error");
  	}
    else{
  	  l -= cnt;
  	  z += cnt;
  	}
  }
}


static void
server_got_reply(char *x){
  int code;
  int len = 0;

  if (*x == '+'){}                        /* Async reply */
    //PRINTF(len, ">>> %s\n", x+1);
  else if (x[0] == ' '){}                 /* Continuation */
    //PRINTF(len, "%s%s\n", 0 ? "     " : "", x+1);
  else if (strlen(x) > 4 &&
           sscanf(x, "%d", &code) == 1 && code >= 0 && code < 10000 &&
           (x[4] == ' ' || x[4] == '-')){
      if (code)
        //PRINTF(len, "%s\n", verbose ? x : x+5);

      if (x[4] == ' '){
        //busy = 0;
        //skip_input = 0;
        return;
      }
    }
  else
    PRINTF(len, "??? <%s>\n", x);

}


static void
server_read(char * buffer)
{
  int c;
  byte *start, *p;

 redo:
  c = read(server_fd, server_read_pos, server_read_buf + sizeof(server_read_buf) - server_read_pos);
  if (!c)
    printf("Connection closed by server");
  if (c < 0){
    if (errno == EINTR)
      goto redo;
    else
      printf("Server read error");
  }

  start = server_read_buf;
  p = server_read_pos;
  server_read_pos += c;
  while (p < server_read_pos)
    if (*p++ == '\n'){
    	p[-1] = 0;
    	server_got_reply(start);
    	start = p;
    }
  if (start != server_read_buf){
    int l = server_read_pos - start;
    memmove(server_read_buf, start, l);
    server_read_pos = server_read_buf + l;
  }
  else if (server_read_pos == server_read_buf + sizeof(server_read_buf)){
    strcpy(server_read_buf, "?<too-long>");
    server_read_pos = server_read_buf + 11;
  }
}
