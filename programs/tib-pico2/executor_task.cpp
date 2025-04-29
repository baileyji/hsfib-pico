//
// Created by Jeb Bailey on 4/25/25.
//

#include "executor_task.h"
#include "photodiode.h"
#include "attenuator.h"
#include "photodiode.h"
#include "pico/stdlib.h"
#include "mktl_keys.h"
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




// static const CommandEntry dispatch_table[] = {
//     {"photodiode.0.gain", [](const Command& cmd) {
//         PhotodiodeCommand pcmd;
//         pcmd.index = 0;
//         pcmd.type = (cmd.type == MsgType::SET) ? PhotodiodeCommand::SetGain : PhotodiodeCommand::GetGain;
//         pcmd.value = cmd.args["gain"];
//         xQueueSend(photodiode_queue, &pcmd, 0);
//     }},
// };

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

std::string fetch_laser_state(MaimanDriver& laser) {
    float laser.getTecTemperatureMeasured();
    float laser.getPcbTemperatureMeasured();
    float laser.getTecTemperatureValue();
    float laser.getCurrentMeasured();
    float laser.getFrequency();
    float laser.getDuration();
    float laser.getVoltageMeasured();
    float laser.getCurrentMaxLimit();
    float laser.getCurrentProtectionThreshold();
    float laser.getCurrentSetCalibration();
    float laser.getNtcB25_100Coefficient();
    float laser.getTecCurrentMeasured();
    float laser.getTecVoltage();
    uint16_t laser.getSerialNumber();
    uint16_t laser.getRawStatus();
    bool laser.isOperationStarted();
    bool laser.isCurrentSetInternal();
    bool laser.isEnableInternal();
    bool laser.isExternalNtcDenied();
    bool laser.isInterlockDenied();
}

void handle_set(HardwareContext* ctx, const pico_zyre::Command& cmd) {
    bool success;

    if (cmd.key == "atten") {
        success = ctx->attenuators[4].set(cmd.args["db"]);
    }
    else if (cmd.key == "laser") {
        // TODO handle each of the lasers
        // TODO hanlde getting level
        // TODO handle getting config
        success = ctx->lasers[1].startDevice();
        success = ctx->lasers[1].setCurrent(cmd.args["current"]);
    }
    else if (cmd.key == "switch") {
        success = ctx->router->route(cmd.args["from"].get<std::string>(), cmd.args["to"].get<std::string>());
    }
    else if (cmd.key == "photodiodes") {
        gpio_write(POWER_PIN, POWER_ON);
        cxt->send_photodiode_data = true;
    }
    else {
        send_error(ctx->response_out, cmd, "Unknown key");
        success=false;
    }
    if (success) {
        send_response(ctx->response_out, cmd, "OK");
    } else {
        send_error(ctx->response_out, cmd, "Failed");
    }

}

std::string handle_get(HardwareContext* ctx, const pico_zyre::Command& cmd) {
    bool success;

    if (cmd.key == "atten") {
        float val=0;
        success = ctx->attenuators[4].get(val);
        json_data = "{\"db\":" + std::to_string(val) + "}";
    }
    else if (cmd.key == "laser") {
        // TODO handle each of the lasers
        // TODO hanlde getting level
        // TODO handle getting config
        success = ctx->lasers[1].getCurrent(val);
        json_data = "{\"current\":" + std::to_string(val) + "}";
    }
    else if (cmd.key == mktl_keys::MEMS_ROUTE) {
        auto routes = ctx->router->activeRoutes();
        json_data = "{\"routes\":[";
        for (auto& route : routes) {
            json_data += "{\"from\":\"" + route.first + "\",\"to\":\"" + route.second + "\"},";
        }
        json_data += "]}";
    }
    else if (cmd.key == mktl_keys::SYSTEM_STATUS) {
        //TODO build json with VERSION, power gpio state
        json_data = "{\"version\":[";
        for (auto& route : routes) {
            json_data += "{\"from\":\"" + route.first + "\",\"to\":\"" + route.second + "\"},";
        }
        json_data += "]}";
    }
    else {
        send_error(ctx->response_out, cmd, "Unknown key");
        success=false;
    }
    if (success) {
        send_response(ctx->response_out, cmd, json_data);
        send_response(ctx->response_out, cmd, "OK");
    } else {
        send_error(ctx->response_out, cmd, "Failed");
    }
}


void executor_task(void *param) {

    auto* ctx = static_cast<HardwareContext*>(param);

    vTaskDelay(pdMS_TO_TICKS(100)); // give time for other tasks to init

    pico_zyre::Command cmd;

    while (true) {
        if (xQueueReceive(ctx->command_in, &cmd, portMAX_DELAY) != pdTRUE)
            continue;

        send_ack(ctx->response_out, cmd);
        if (cmd.type == pico_zyre::MsgType::SET) {
            handle_set(ctx, cmd);
        } else {
            handle_get(ctx, cmd);
        }

    }

}







