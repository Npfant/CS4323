#ifndef MESSAGES_H
#define MESSAGES_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include "logger.h"
#include "logging.h"

#define MAX_NAME_LEN 32

// Message structure for request queue (train to server)
struct request_msg {
    long msg_type;
    char train_name[MAX_NAME_LEN];
    char intersection[MAX_NAME_LEN];
};

// Message structure for response queue (server to train)
struct response_msg {
    long msg_type;
    char response[MAX_NAME_LEN];
};

// Send a request from train to server
void send_request(int req_id, const char* train_name, const char* inter_name) {
    struct request_msg msg;
    msg.msg_type = getpid();  // Unique identifier for each train

    strncpy(msg.train_name, train_name, MAX_NAME_LEN - 1);
    msg.train_name[MAX_NAME_LEN - 1] = '\0';

    strncpy(msg.intersection, inter_name, MAX_NAME_LEN - 1);
    msg.intersection[MAX_NAME_LEN - 1] = '\0';

    if (msgsnd(req_id, &msg, sizeof(msg) - sizeof(long), 0) == -1) {
        perror("msgsnd failed");
        return;
    }

    // Log the REQ event
    Event e;
    strcpy(e.type, "REQ");
    e.trainNum1 = atoi(train_name + 5);  // Assumes name like "Train1"
    strcpy(e.intersecLetter, inter_name);
    e.intersecNum = 1;
    log_event(e);
}

// Receive a request (server side)
void receive_request(int req_id, struct request_msg* msg) {
    if (msgrcv(req_id, msg, sizeof(*msg) - sizeof(long), 0, 0) == -1) {
        perror("msgrcv failed");
    }
}

// Send a response from server to train
void send_response(int res_id, long msg_type, const char* response) {
    struct response_msg msg;
    msg.msg_type = msg_type;

    strncpy(msg.response, response, MAX_NAME_LEN - 1);
    msg.response[MAX_NAME_LEN - 1] = '\0';

    if (msgsnd(res_id, &msg, sizeof(msg) - sizeof(long), 0) == -1) {
        perror("msgsnd failed");
    }
}

// Receive a response (train side)
void receive_response(int res_id, long msg_type, struct response_msg* msg) {
    if (msgrcv(res_id, msg, sizeof(*msg) - sizeof(long), msg_type, 0) == -1) {
        perror("msgrcv failed");
    }
}

#endif // MESSAGES_H
