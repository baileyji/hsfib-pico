//
// Created by Jeb Bailey on 4/23/25.
//

// rep_task.cpp

#include "rep_task.h"
#include "zmq_frame.h"
#include <cstring>
#include <cstdio>
#include "pico/stdlib.h"
#include "wizchip_conf.h"
#include "socket.h"

#define REP_PORT 5000
#define REP_BUF_SIZE 256

void rep_task(void* param) {
    QueueHandle_t out_queue = static_cast<QueueHandle_t>(param);
    uint8_t sock = 0;
    uint8_t buf[REP_BUF_SIZE];

    socket(sock, Sn_MR_TCP, REP_PORT, 0);
    listen(sock);

    while (true) {
        if (getSn_SR(sock) == SOCK_ESTABLISHED) {
            int len = recv(sock, buf, sizeof(buf));
            if (len > 0) {
                zmq_msg_t zmq_msg;
                if (zmq_decode(buf, len, &zmq_msg)) {
                    if (zmq_msg.count >= 3) {
                        printf("REP: got ZMQ request frame %.*s\n", (int)zmq_msg.sizes[2], zmq_msg.frames[2]);

                        // Forward 3rd frame (JSON command) to DAC queue
                        char json_msg[128];
                        size_t copy_len = zmq_msg.sizes[zmq_msg.count - 1];
                        if (copy_len < sizeof(json_msg)) {
                            memcpy(json_msg, zmq_msg.frames[zmq_msg.count - 1], copy_len);
                            json_msg[copy_len] = 0;
                            xQueueSend(out_queue, &json_msg, portMAX_DELAY);
                        }
                    }

                    // Send back ack
                    zmq_msg_t reply;
                    reply.count = 2;
                    memcpy(reply.frames[0], "ack", 3); reply.sizes[0] = 3;
                    memcpy(reply.frames[1], "req-001", 7); reply.sizes[1] = 7;
                    size_t reply_len = zmq_encode(&reply, buf, sizeof(buf));
                    send(sock, buf, reply_len);
                }
            }
        } else if (getSn_SR(sock) == SOCK_CLOSED) {
            close(sock);
            socket(sock, Sn_MR_TCP, REP_PORT, 0);
            listen(sock);
        }

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}