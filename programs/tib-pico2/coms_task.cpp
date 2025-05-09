//
// Created by Jeb Bailey on 4/24/25.
//

#include "coms_task.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "wizchip_conf.h"
#include "pico_zyre.h"

#include <cstring>
#include <cstdio>
#include "dhcp.h"
#include <string>
#include <vector>
#include "hardware_context.h"
#include "log_util.h"

#define ZRE_NAME "hsfib-tib"
// -------------------------


void coms_task(void* param) {
    auto* ctx = static_cast<HardwareContext*>(param);

    SAFE_PRINTF("[coms] Starting coms.\n");

    pico_zyre::ZyreBeacon beacon=pico_zyre::ZyreBeacon(ZRE_NAME);
    beacon.start();
    beacon.tick();

    pico_zyre::Command msg;
    pico_zyre::Response reply;
    pico_zyre::PubMessage pub;

    while (true) {

        beacon.tick();

        // Handle incoming WHISPERs
        if (beacon.receive(msg)) {
            if (ctx->command_in) {
                if (xQueueSend(ctx->command_in, &msg, 0) != pdTRUE) {
                    SAFE_PRINTF("[coms] Warning: command_in queue full.\n");
                    reply.identity=msg.identity;
                    reply.key=msg.key;
                    reply.req_id=msg.req_id;
                    reply.payload="{error: too busy, try again}";;
                    reply.type=pico_zyre::MsgType::ERROR;
                    beacon.send_reply(reply);
                } else {
                    reply.identity=msg.identity;
                    reply.key=msg.key;
                    reply.req_id=msg.req_id;
                    reply.type=pico_zyre::MsgType::ACK;
                    reply.payload="";
                    beacon.send_reply(reply);
                }
            }
        }

        if (ctx->response_out && xQueueReceive(ctx->response_out, &reply, 0) == pdTRUE) {
            beacon.send_reply(reply);
        }

        // Handle outgoing PUBs
        if (ctx->pub_out && xQueueReceive(ctx->pub_out, &pub, 0) == pdTRUE) {
            beacon.send_pub(pub);
        }

        vTaskDelay(pdMS_TO_TICKS(10));  //TODO do we really want to throttle this way?
    }
}

/* early pub task code
 *
 *


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

/*
 *
 *Early REP task code
 *
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
*/