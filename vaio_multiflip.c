#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <android/log.h>

#define APPNAME                         "vaio_multiflip"

#define SYSFS_FLIP_STATUS               "/sys/devices/platform/sony-laptop/tablet"

#define DISABLE_AUTOROTATE              "content insert --uri content://settings/system --bind name:s:accelerometer_rotation --bind value:i:0"
#define ENABLE_AUTOROTATE               "content insert --uri content://settings/system --bind name:s:accelerometer_rotation --bind value:i:1"

#define ORIENTATION_LANDSCAPE           0
#define ORIENTATION_PORTRAIT            1
#define ORIENTATION_REVERSE_LANDSCAPE   2
#define ORIENTATION_REVERSE_PORTRAIT    3


char running = 1;

void term(int signal){
  __android_log_print(ANDROID_LOG_INFO, APPNAME, "Received termination signal %d", signal);
  running = 0;
}

void set_orientation(int orientation){
  char* cmd = NULL;
  asprintf(&cmd, "content insert --uri content://settings/system --bind name:s:user_rotation --bind value:i:%d", orientation);
  system(cmd);
  free(cmd);
}

void update_state(int status){
  __android_log_print(ANDROID_LOG_INFO, APPNAME, "status is %c", status);
  if(status == '0'){ //laptop
    set_orientation(ORIENTATION_LANDSCAPE);
    system(DISABLE_AUTOROTATE);
  }
  else if(status == '1'){ //presentation
    set_orientation(ORIENTATION_REVERSE_LANDSCAPE);
    system(DISABLE_AUTOROTATE);
  }
  else if(status == '2'){ //tablet
    system(ENABLE_AUTOROTATE);
  }
}

void poll_status(){

  int cnt, rv, select_fd;
  fd_set exceptfds;
  char curr_status;
  char prev_status = -1;

  if ((select_fd = open(SYSFS_FLIP_STATUS, O_RDONLY)) < 0){
    perror("Unable to open flip status file");
    exit(1);
  }

  __android_log_print(ANDROID_LOG_INFO, APPNAME, "Reading current state");
  read(select_fd, &curr_status, 1);
  lseek(select_fd, SEEK_SET, 0);
  update_state(curr_status);

  while(running){

    FD_ZERO(&exceptfds);
    FD_SET(select_fd, &exceptfds);

    __android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "polling");
    if((rv = select(select_fd+1, NULL, NULL, &exceptfds, NULL)) < 0 && running){
      __android_log_print(ANDROID_LOG_ERROR, APPNAME, "poll error");
    }
    else if (rv == 0){
      __android_log_print(ANDROID_LOG_INFO, APPNAME, "Timeout occurred!");
    }
    else if (FD_ISSET(select_fd, &exceptfds)){
      read(select_fd, &curr_status, 1);
      lseek(select_fd, SEEK_SET, 0);

      if(curr_status != prev_status){
        __android_log_print(ANDROID_LOG_INFO, APPNAME, "Status changed: %c", curr_status);
        update_state(curr_status);
        prev_status = curr_status;
      }
    }
  }
  close(select_fd);
}

int main(int argc, char **argv){
  (void)argc;
  (void)argv;

  signal(SIGINT, &term);
  signal(SIGTERM, &term);

  poll_status();

  __android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "Finished.");
  return 0;
}
