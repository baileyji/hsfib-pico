//
// Created by Jeb Bailey on 4/23/25.
//

#include "pub_task.h"
#include "zmq_frame.h"
#include <cstring>
#include <cstdio>
#include "pico/stdlib.h"
#include "wizchip_conf.h"
#include "socket.h"

#define PUB_PORT 5001
#define PUB_BUF_SIZE 256

void pub_task(void* param) {
    QueueHandle_t in_queue = static_cast<QueueHandle_t>(param);
    uint8_t sock = 1;
    uint8_t buf[PUB_BUF_SIZE];

    socket(sock, Sn_MR_TCP, PUB_PORT, 0);
    listen(sock);

    while (true) {
        if (getSn_SR(sock) == SOCK_ESTABLISHED) {
            char msg[128];
            if (xQueueReceive(in_queue, &msg, portMAX_DELAY) == pdTRUE) {
                zmq_msg_t zmsg;
                zmsg.count = 2;
                memcpy(zmsg.frames[0], "adc.telemetry", 13); zmsg.sizes[0] = 13;
                size_t len = strlen(msg);
                if (len >= ZMQ_MAX_FRAME_SIZE) len = ZMQ_MAX_FRAME_SIZE - 1;
                memcpy(zmsg.frames[1], msg, len); zmsg.sizes[1] = len;

                size_t out_len = zmq_encode(&zmsg, buf, sizeof(buf));
                send(sock, buf, out_len);
            }
        } else if (getSn_SR(sock) == SOCK_CLOSED) {
            close(sock);
            socket(sock, Sn_MR_TCP, PUB_PORT, 0);
            listen(sock);
        }

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}