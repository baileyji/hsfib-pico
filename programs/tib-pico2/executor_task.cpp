// executor_task.cpp

#include "executor_task.h"
#include "hardware_context.h"
#include "photodiode.h"
#include "attenuator.h"
#include "pico/stdlib.h"
#include "mktl_keys.h"
#include "pico_zyre.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "mems_switching.h"
#include "dacx578.h"

#include <array>
#include <string_view>
#include <unordered_map>
#include <nlohmann/json.hpp>

#include "log_util.h"

using pico_zyre::Command;
using pico_zyre::Response;
using pico_zyre::MsgType;

inline bool starts_with(std::string_view str, std::string_view prefix) {
    return str.substr(0, prefix.size()) == prefix;
}

void send_ack(QueueHandle_t& respq, const Command& cmd) {
    Response m{cmd.identity, MsgType::ACK, cmd.req_id, cmd.key, "null"};
    xQueueSend(respq, &m, 0);
}

void send_response(QueueHandle_t& respq, const Command& cmd, const std::string& payload) {
    Response m{cmd.identity, MsgType::RESPONSE, cmd.req_id, cmd.key, payload};
    xQueueSend(respq, &m, 0);
}

void send_error(QueueHandle_t& respq, const Command& cmd, const std::string& err) {
    Response m{cmd.identity, MsgType::ERROR, cmd.req_id, cmd.key, "{\"error\":\"" + err + "\"}"};
    xQueueSend(respq, &m, 0);
}

MaimanDriver* laser_for_key(HardwareContext& hw, std::string_view key) {
    if (starts_with(key, "1028_laser.")) return &hw.lasers[0];
    if (starts_with(key, "1270_laser.")) return &hw.lasers[1];
    if (starts_with(key, "yj1420_laser.")) return &hw.lasers[2];
    if (starts_with(key, "hk1420_laser.")) return &hw.lasers[3];
    if (starts_with(key, "1510_laser.")) return &hw.lasers[4];
    if (starts_with(key, "2330_laser.")) return &hw.lasers[5];
    return nullptr;
}

Attenuator* attenuator_for_key(HardwareContext& hw, std::string_view key) {
    if (starts_with(key, "1028_atten.")) return &hw.attenuators[0];
    if (starts_with(key, "1270_atten.")) return &hw.attenuators[1];
    if (starts_with(key, "yj1410_atten.")) return &hw.attenuators[2];
    if (starts_with(key, "hk1410_atten.")) return &hw.attenuators[3];
    if (starts_with(key, "1510_atten.")) return &hw.attenuators[4];
    if (starts_with(key, "2330_atten.")) return &hw.attenuators[5];
    return nullptr;
}

std::string dispatch_get_laser_current(HardwareContext& hw, const Command& cmd) {
    float value = 0.0f;
    MaimanDriver* laser = laser_for_key(hw, cmd.key);
    if (!laser || !laser->getCurrent(value)) {
        return nlohmann::json({{"error", "Failed to get laser current"}}).dump();
    }
    return nlohmann::json({{"value", value}}).dump();
}

std::string dispatch_set_laser_current(HardwareContext& hw, const Command& cmd) {
    if (!cmd.args.contains("value") || !cmd.args["value"].is_number()) {
        return nlohmann::json({{"error", "Invalid set current argument"}}).dump();
    }
    MaimanDriver* laser = laser_for_key(hw, cmd.key);
    if (!laser || !laser->setCurrent(cmd.args["value"].get<float>())) {
        return nlohmann::json({{"error", "Failed to set laser current"}}).dump();
    }
    return nlohmann::json({{"status", "ok"}}).dump();
}

std::string dispatch_get_attenuator(HardwareContext& hw, const Command& cmd) {
    float value = 0.0f;
    Attenuator* att = attenuator_for_key(hw, cmd.key);
    if (!att || !att->get(value)) {
        return nlohmann::json({{"error", "Failed to get attenuator"}}).dump();
    }
    return nlohmann::json({{"value", value}}).dump();
}

std::string dispatch_set_attenuator(HardwareContext& hw, const Command& cmd) {
    if (!cmd.args.contains("value") || !cmd.args["value"].is_number()) {
        return nlohmann::json({{"error", "Invalid set attenuator argument"}}).dump();
    }
    Attenuator* att = attenuator_for_key(hw, cmd.key);
    if (!att || !att->set(cmd.args["value"].get<float>())) {
        return nlohmann::json({{"error", "Failed to set attenuator"}}).dump();
    }
    return nlohmann::json({{"status", "ok"}}).dump();
}

std::string dispatch_get_mems_routes(HardwareContext& hw, const Command& cmd) {
    auto routes = hw.router->activeRoutes();
    nlohmann::json j = nlohmann::json::array();
    for (const auto& [input, output] : routes) {
        j.push_back({{"input", input}, {"output", output}});
    }
    return j.dump();
}

std::string dispatch_set_mems_route(HardwareContext& hw, const Command& cmd) {
    if (!cmd.args.contains("input") || !cmd.args.contains("output")) {
        return nlohmann::json({{"error", "Missing input/output"}}).dump();
    }
    if (!hw.router->route(cmd.args["input"].get<std::string_view>(),
                          cmd.args["output"].get<std::string_view>())) {
        return nlohmann::json({{"error", "Failed to set route"}}).dump();
    }
    return nlohmann::json({{"status", "ok"}}).dump();
}

std::string dispatch_get_mems_switch(HardwareContext& hw, const Command& cmd) {
    if (!cmd.args.contains("name") || !cmd.args["name"].is_string()) {
        return nlohmann::json({{"error", "Missing switch name"}}).dump();
    }
    char state = '0';
    if (!hw.router->getSwitch(cmd.args["name"].get<std::string_view>(), state)) {
        return nlohmann::json({{"error", "Failed to get switch"}}).dump();
    }
    return nlohmann::json({{"state", std::string(1, state)}}).dump();
}

std::string dispatch_set_mems_switch(HardwareContext& hw, const Command& cmd) {
    if (!cmd.args.contains("name") || !cmd.args.contains("state")) {
        return nlohmann::json({{"error", "Missing name/state"}}).dump();
    }
    char state = cmd.args["state"].get<std::string>()[0];
    if (!hw.router->setSwitch(cmd.args["name"].get<std::string_view>(), state)) {
        return nlohmann::json({{"error", "Failed to set switch"}}).dump();
    }
    return nlohmann::json({{"status", "ok"}}).dump();
}

using DispatcherFunc = std::string(*)(HardwareContext&, const Command&);
struct CommandEntry {
    DispatcherFunc get_handler = nullptr;
    DispatcherFunc set_handler = nullptr;
};

static const std::unordered_map<std::string_view, CommandEntry> dispatch_table = {
    { "1028_laser.level",    {dispatch_get_laser_current, dispatch_set_laser_current} },
    { "1270_laser.level",    {dispatch_get_laser_current, dispatch_set_laser_current} },
    { "yj1420_laser.level",  {dispatch_get_laser_current, dispatch_set_laser_current} },
    { "hk1420_laser.level",  {dispatch_get_laser_current, dispatch_set_laser_current} },
    { "1510_laser.level",    {dispatch_get_laser_current, dispatch_set_laser_current} },
    { "2330_laser.level",    {dispatch_get_laser_current, dispatch_set_laser_current} },

    { "1028_atten.db",       {dispatch_get_attenuator, dispatch_set_attenuator} },
    { "1270_atten.db",       {dispatch_get_attenuator, dispatch_set_attenuator} },
    { "yj1410_atten.db",     {dispatch_get_attenuator, dispatch_set_attenuator} },
    { "hk1410_atten.db",     {dispatch_get_attenuator, dispatch_set_attenuator} },
    { "1510_atten.db",       {dispatch_get_attenuator, dispatch_set_attenuator} },
    { "2330_atten.db",       {dispatch_get_attenuator, dispatch_set_attenuator} },

    { "mems.switch",         {dispatch_get_mems_switch, dispatch_set_mems_switch} },
    { "mems.route",          {dispatch_get_mems_routes, dispatch_set_mems_route} },
};

void executor_task(void* param) {
    auto* ctx = static_cast<HardwareContext*>(param);
    vTaskDelay(pdMS_TO_TICKS(100));

    pico_zyre::Command cmd;

    SAFE_PRINTF("Started executor\n");

    while (true) {
        if (xQueueReceive(ctx->command_in, &cmd, portMAX_DELAY) != pdTRUE)
            continue;

        send_ack(ctx->response_out, cmd);

        auto it = dispatch_table.find(cmd.key);
        if (it == dispatch_table.end()) {
            send_error(ctx->response_out, cmd, "Unknown key");
            continue;
        }

        DispatcherFunc func = (cmd.type == MsgType::SET) ?
                              it->second.set_handler :
                              it->second.get_handler;

        if (!func) {
            send_error(ctx->response_out, cmd, "Unsupported operation");
            continue;
        }

        std::string result = func(*ctx, cmd);
        send_response(ctx->response_out, cmd, result);
    }
}
