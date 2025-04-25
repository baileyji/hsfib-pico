//
// Created by Jeb Bailey on 4/25/25.
//

#include "executor_task.h"
#include "photodiode.h"
#include "attenuator.h"
#include "photodiode.h"
#include "pico/stdlib.h"
#include <cstdio>
#include <stdexcept>

#include "pico_zyre.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "mems_switching.h"


#include "dacx578.h"
#include "attenuator.h"
#include <cstring>
#include <cstdio>
#include <array>
#include "hardware_context.h"
// #include <string>
// #include <cstring>



void send_ack(QueueHandle_t& respq, const pico_zyre::Command& cmd) {
    pico_zyre::Response m{cmd.identity, pico_zyre::MsgType::ACK, cmd.req_id, cmd.key, "null"};
    xQueueSend(respq, &m, 0);
}

void send_response(QueueHandle_t& respq, const pico_zyre::Command& cmd, const std::string& payload) {
    pico_zyre::Response m{cmd.identity, pico_zyre::MsgType::RESPONSE, cmd.req_id, cmd.key, payload};
    xQueueSend(respq, &m, 0);
}

void send_error(QueueHandle_t& respq, const pico_zyre::Command& cmd, const std::string& err) {
    pico_zyre::Response m{cmd.identity, pico_zyre::MsgType::ERROR, cmd.req_id, cmd.key, "{\"error\":\"" + err + "\"}"};
    xQueueSend(respq, &m, 0);
}

void handle_set(QueueHandle_t& respq, const pico_zyre::Command& cmd) {
    if (cmd.key == "attenuator.4.db") {
        bool success = attenuators[4].set(cmd.args["db"]);
    }
    else if (cmd.key == "photodiode.1.gain") {
        std::lock_guard lock(pd_mutex);
        if (photodiodes[1].set_gain(cmd.args["gain"])) {
            send_response(respq, cmd, "OK");
        } else {
            send_error(respq, cmd, "Failed");
        }
        return;
    }
    else
        send_error(respq, cmd, "Unknown key");
}

std::string handle_get(QueueHandle_t& respq, const pico_zyre::Command& cmd) {
    if (cmd.key == "attenuator.4.db") {
        send_response(respq, cmd, std::to_string(attenuators[4].get()));
    }
    else if (cmd.key == "photodiode.1.gain") {
        send_response(respq, cmd, photodiodes[1].gain());
    }

    send_error(respq, cmd, "Unknown key");
}


void executor_task(void *param) {

    auto* ctx = static_cast<HardwareContext*>(param);

    vTaskDelay(pdMS_TO_TICKS(100)); // give time for other tasks to init

    pico_zyre::Command cmd;

    while (true) {
        if (xQueueReceive(ctx->command_in, &cmd, portMAX_DELAY) != pdTRUE)
            continue;

        if (cmd.type == pico_zyre::MsgType::SET) {
            send_ack(ctx->response_out, cmd);
            handle_set(ctx->response_out, cmd);
        } else {
            handle_get(ctx->response_out, cmd);
        }

    }

}







