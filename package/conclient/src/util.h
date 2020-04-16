#ifndef __UTIL__H
#define __UTIL__H
struct server_paraments{
  char ip[32];
  char filename[128];
  int port;
};

int check_local_download(char *paraments, struct server_paraments *s);

#endif

