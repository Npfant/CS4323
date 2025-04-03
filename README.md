    #include <stdio.h>
    #include <stdlib.h>
    #include <unistd.h>
    #include <sys/types.h>
    #include <sys/ipc.h>
    #include <sys/msg.h>

    #define ACQUIRE 1
    #define GRANT 2
    #define RELEASE 3

    // Message structure
    typedef struct msg_buffer {
        long msg_type; // Message type (ACQUIRE, GRANT, RELEASE)
        int train_id;  // Train ID making the request
        int intersection_id; // Requested intersection
    } Message;

    // Function to simulate train behavior
    void train_process(int train_id, int msgid) {
        Message msg;

        // Train sends an ACQUIRE request
        msg.msg_type = ACQUIRE;
        msg.train_id = train_id;
        msg.intersection_id = train_id % 5; // Assign an intersection, 5 intersections
        msgsnd(msgid, &msg, sizeof(Message) - sizeof(long), 0);
    
        printf("Train %d requests Intersection %d\n", train_id, msg.intersection_id);
        //Here we need to lock semaphore/mutex
        // Wait for the server's GRANT message
        msgrcv(msgid, &msg, sizeof(Message) - sizeof(long), GRANT, 0);
        printf("Train %d granted Intersection %d\n", train_id, msg.intersection_id);

        sleep(5); // Simulate the train crossing the intersection

        // Send a RELEASE message when done, Here we need to unlock semaphore/mutex 
        msg.msg_type = RELEASE;
        msgsnd(msgid, &msg, sizeof(Message) - sizeof(long), 0);
        printf("Train %d released Intersection %d\n", train_id, msg.intersection_id);
    }

    // Function to simulate the server's behavior
    void server_process(int msgid) {
        Message msg;

        while (1) {
            // Receive ACQUIRE requests
            msgrcv(msgid, &msg, sizeof(Message) - sizeof(long), ACQUIRE, 0);
            printf("Server received ACQUIRE from Train %d for Intersection %d\n", msg.train_id, msg.intersection_id);

            // Grants access immediately, DOes not have deadlock.
            msg.msg_type = GRANT;
            msgsnd(msgid, &msg, sizeof(Message) - sizeof(long), 0);
            printf("Server granted Train %d access to Intersection %d\n", msg.train_id, msg.intersection_id);

            // Receive RELEASE message // Not sure to add Semaphores here or during the release process. 
            msgrcv(msgid, &msg, sizeof(Message) - sizeof(long), RELEASE, 0);
            printf("Server received RELEASE from Train %d for Intersection %d\n", msg.train_id, msg.intersection_id);
        }
    }

    int main() {
        // Create a message queue with a unique key
        key_t key = ftok("train_sim", 69);
        int msgid = msgget(key, 0777 | IPC_CREAT); // Create the message Q

        if (fork() == 0) { // Child process to run the server
            server_process(msgid);
            exit(0);
        }

        sleep(3); // Give the server some time to start

        // Create 3 train processes
        for (int i = 1; i <= 3; i++) {
            if (fork() == 0) { // Child train process
                train_process(i, msgid);
                exit(0);
            }
        }

        sleep(10); // Allow time for trains to finish

        msgctl(msgid, IPC_RMID, NULL); // Cleanup the message Q
        return 0;
    }
