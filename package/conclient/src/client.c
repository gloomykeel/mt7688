/*********************************************
alex.kqiang init
*********************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <unistd.h>
#include <curses.h>
#include <uci.h>
#include "config.h"
#include "util.h"

int firmware_upgrade()
{
  int ret = 0;
  char cmd[128] = "";
  
  if(access("/tmp/update.bin", F_OK) == F_OK){
    sprintf(cmd, "/sbin/sysupgrade %s %s&\n", "-n", "/tmp/update.bin");
    ret = system(cmd);
  }
  return ret;
}

int app_install(char *path)
{
  int ret = 0;
  char cmd[128] = "";
  
  if(access(path, F_OK) == F_OK){
    sprintf(cmd, "opkg --force-overwrite install %s\n", path);
    ret = system(cmd);
  }
  return ret;
}

int local_download(struct server_paraments *sparaments, char *localfile)
{
  int ret = 0;
  char cmd[128] = "";
  char *ftm = "tftp -r %s -l /tmp/%s -g %s %d";
  int sport = 69;

  snprintf(cmd, sizeof(cmd), ftm, sparaments->filename, localfile , sparaments->ip, sport);
  ret = system(cmd);

  return ret;
}

int get_input()
{
  char end[64] = "";
  int num = -1;

  if((fgets(end, sizeof(end), stdin) != NULL)
      && (strlen(end) == strspn(end, "0123456789\n"))
      && (strlen(end) >= 2)){
    end[strlen(end)-1]='\0';
    num=atoi(end);
  }

  return num;
}

int get_input_string(char *buf)
{
  char end[128] = "";
  char *token = NULL;
  int pos = 0;
  int num = -1;

  if(fgets(end, sizeof(end), stdin) != NULL){
    if(strlen(end) <= 2 &&(strlen(end) == strspn(end, "0123456789\n"))){
      end[strlen(end)-1]='\0';
      num=atoi(end);
    }else {
      token = strtok(end, " ");
      if(token){
        if(strlen(token) == strspn(token, "0123456789\n"))
          num = atoi(token);
        while(NULL != (token = strtok(NULL, " "))){
          if(*(token+strlen(token)-1) == '\n')
            *(token+strlen(token)-1) = '\0';
          pos += snprintf(buf+pos, 128-pos, "%s ", token);
        }
      }
    }
  }

  return num;
}

void config_show(const char *file)
{
  struct uci_context *ctx;
  struct uci_package *pkg = NULL;

  ctx = uci_alloc_context();
  if(ctx == NULL){
    printf("Out of memory\n");
    return;
  }

  if(UCI_OK != uci_load(ctx, file, &pkg))
    goto cleanup;
  uci_show_package(pkg);

cleanup:
  uci_free_context(ctx);
}

void config_set(const char *file, char *argv)
{
  int ret = 0;
  struct uci_context *ctx;
  struct uci_package *pkg = NULL;
  struct uci_ptr ptr;

  ctx = uci_alloc_context();
  if(ctx == NULL){
    printf("Out of memory\n");
    return;
  }
  if(UCI_OK != uci_load(ctx, file, &pkg))
    goto cleanup;

  if(uci_lookup_ptr(ctx, &ptr, argv, true)!= UCI_OK){
    printf("paraments error!\n");
    return;
  }

  if(ptr.value){
    ret = uci_set(ctx, &ptr);
    if (ret == UCI_OK)
      ret = uci_save(ctx, ptr.p);
  }

cleanup:
  uci_free_context(ctx);
}

void device_config()
{
  int cmd;
  char buf[128] = "";
  int flag = 1;
  int ret = 0;
  char *test_menu =
    "\n==============device configuration menu============"
    "\n 0: exit"
    "\n\tGo back to the prev menu"
    "\n 1: configuration show"
    "\n\t<config>.<section>[.<option>]"
    "\n 2: configuration set"
    "\n\t<config>.<section>[.<option>]=<value>"
    "\n==================================================="
    "\nEnter cmd: \n";

  while(flag){
    printf("%s", test_menu);
    cmd = get_input_string((char *)&buf);
    switch(cmd){
      case 0:
          flag = 0;
          break;
      case 1:
          printf("===================show start =====================\n");
          config_show(UCI_CONFIG_FILE);
          printf("===================show end =======================\n");
          break;
      case 2:
          if(strlen(buf))
            config_set(UCI_CONFIG_FILE, (char *)&buf);
          else printf("paraments error!\n");
          break;
      default:
          printf("Invalid input!\n");
          break;
    }
  }
}

void device_update()
{
  int cmd;
  char buf[128] = "";
  int flag = 1;
  int ret = 0;
  struct server_paraments sparaments;
  char *test_menu =
    "\n==============local upgrade menu============"
    "\n 0: exit"
    "\n 1 firmware update"
    "\n 2 get update state"
    "\n 3 local firmware download"
    "\n\t3 serverip fimrware_name"
    "\n\t:tftp server"
    "\n 4 app install"
    "\n 5 get app install status"
    "\n 6 local app.ipk download"
    "\n\t6 serverip app_name"
    "\n\t:tftp server"
    "\n==================================="
    "\nEnter cmd: \n";

  while(flag){
    printf("%s", test_menu);
    cmd = get_input_string((char *)&buf);
    switch(cmd){
      case 0:
          flag = 0;
          break;
      case 1:
          ret = firmware_upgrade();
          if(!ret){
            printf("The system is flashing now.\n\t%s\n",
                "\033[31mDO NOT POWER OFF THE DEVICE!\033[0m Wait a few minutes before you try to reconnect.");
            (void)pause();
          }else printf("firmware fail!\n");
          break;
      case 2:
          printf("get state\n");
          break;
      case 3:
          if(strlen(buf)){
            if(!(ret = check_local_download((char *)&buf, &sparaments))){
              printf("The system is downloading now.\n\t%s\n",
                  "\033[31mDO NOT POWER OFF THE DEVICE!\033[0m Wait a few seconds before you try to input.");
              ret = local_download(&sparaments, "update.bin");
            }
            printf("local_download %s\n", ret?"fail!":"successful!");
          }else printf("paraments error!\n");
          break;
      case 4:
          ret = app_install("/tmp/app.ipk");
          if(ret){
            printf("The system is installing app now.\n");
          }else printf("app.ipk install fail\n");
          break;
          break;
      case 5:
          printf("get state\n");
          break;
      case 6:
          if(strlen(buf)){
            if(!(ret = check_local_download((char *)&buf, &sparaments))){
              printf("The system is downloading now.\n\t%s\n",
                  "\033[31mDO NOT POWER OFF THE DEVICE!\033[0m Wait a few seconds before you try to input.");
              ret = local_download(&sparaments, "app.ipk");
            }
            printf("local_download %s\n", ret?"fail!":"successful!");
          }else printf("paraments error!\n");
          break;
      default:
          printf("Invalid input!\n");
          break;
    }
  }

}


void main_menu(void)
{
  int cmd;
  char buf[64];
  int flag = 1;
  char *test_menu = "\n==============main menu============"
                    "\n 0: exit"
                    "\n 1: manage"
                    "\n 2: update"
                    "\n==================================="
                    "\nEnter cmd: \n";

  while(flag){
    fflush(stdout);
    printf("%s", test_menu);
    cmd = get_input();
    switch(cmd){
      case 0:
          flag = 0;
          break;
      case 1:
          device_config();
          break;
      case 2:
          device_update();
          break;
      default:
        printf("Invalid input!\n");
        break;
    }
  }

}

void *menu_thread(void *argv)
{
  sigset_t mask;

  sigemptyset(&mask);
  sigaddset(&mask, SIGALRM);
  pthread_sigmask(SIG_SETMASK, &mask, NULL);
  pthread_detach(pthread_self());

  main_menu();
  exit(0);
  return NULL;
}

int main(int argc, char **argv)
{
  int ret;
  int f = 0;
  static pthread_t tid = 0;
  if(argc == 2)
    if(**(argv+1) == '1')
      f = 1;
  printf("%s start!", __func__);
  if((ret = pthread_create(&tid, NULL, menu_thread, NULL)) != 0){
    printf("pthread_create operation error %d", ret);
    ret = -1;
  }

  signal(SIGINT, SIG_IGN);
  signal(SIGTSTP, SIG_IGN);
  while(1)pause();
  if(f)
    system("exec /bin/login");
  return ret;
}
