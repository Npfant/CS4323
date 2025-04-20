#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <stdbool.h>
#include <ctype.h>

#define MAX_NAME_LEN 32
#define MAX_HOLDING 10
#define MAX_LINE_LEN 100

// Message structure for request queue (train to server)
struct request_msg {
    long msg_type;        // Message type (should be > 0)
    char train_name[MAX_NAME_LEN];
    char intersection[MAX_NAME_LEN];
};

// Message structure for response queue (server to train)
struct response_msg {
    long msg_type;        // Message type (should be > 0)
    char response[MAX_NAME_LEN];
};

void send_request(int req_id, const char* train_name, const char* inter_name) {
    struct request_msg msg;
    msg.msg_type = 1;
    strncpy(msg.train_name, train_name, MAX_NAME_LEN - 1);
    strncpy(msg.intersection, inter_name, MAX_NAME_LEN - 1);
    if (msgsnd(req_id, &msg, sizeof(msg) - sizeof(long), 0) == -1) {
        perror("msgsnd failed");
    }
}

void send_response(int res_id, const char* response) {
    struct response_msg msg;
    msg.msg_type = 1;
    strncpy(msg.response, response, MAX_NAME_LEN - 1);
    if (msgsnd(res_id, &msg, sizeof(msg) - sizeof(long), 0) == -1) {
        perror("msgsnd failed");
    }
}

// Receive request from the request queue
void receive_request(int req_id, struct request_msg* msg) {
    if (msgrcv(req_id, msg, sizeof(*msg) - sizeof(long), 0, 0) == -1) {
        perror("msgrcv failed");
    }
}

// Receive response from the response queue
void receive_response(int res_id, struct response_msg* msg) {
    if (msgrcv(res_id, msg, sizeof(*msg) - sizeof(long), 0, 0) == -1) {
        perror("msgrcv failed");
    }
}
