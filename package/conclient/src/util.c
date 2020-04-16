#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "util.h"

int check_local_download(char *paraments, struct server_paraments *s)
{
  char *token = NULL;
  if(!(paraments&&s))
    return -1;
  if(NULL == (token = strtok(paraments, " ")))
    return -1;
  if((INADDR_NONE == inet_addr(token))||!strchr(token, '.'))
    return -1;
  strcpy(s->ip, token);
  if(NULL == (token = strtok(NULL, " ")))
    return -1;
  strcpy(s->filename, token);

  return 0;
}

