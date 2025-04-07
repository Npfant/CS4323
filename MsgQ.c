#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>

#define MSG_SIZE 16

int main() {
  int msqid;
  key_t key;
  struct message msg;

  if((key = ftok("pathname", 'A')) == -1) {
    perror("ftok");
    exit(1);
  }

  if((msqid = msgget(key, 0666 | IPC_CREAT)) == -1) {
    perror("msgget");
    exit(1);
  }

  int pid = fork();
  if(pid == 0){
    msg.mtype = 1;
    strcpt(msg.mtext, "take1");
    if(msgsnd(msqid, &msg, strlen(msg.mtext)+1, 0) == -1) {
      perror("msgsnd");
      exit(1);
    }
    printf("message send | child process");
  }
  if(pid != 0){
    wait(1);
    if(msgrcv(msqid, &msg, MSG_SIZE, 1, 0) == -1) {
      perror("msgrcv");
      exit(1);
    }
    printf("message received | parent process");
    printf(mtext);
  }
}

struct message {
  long mtype;
  char mtext[MSG_SIZE];
};
