//
// Created by Jeb Bailey on 4/23/25.
//

#include "mems_switching.h"
#include <cstdio>
#include "FreeRTOS.h"
#include "task.h"


MEMSSwitch::MEMSSwitch(PCAL6416A& gpio, uint8_t pinA, uint8_t pinB)
    : _gpio(gpio), _pinA(pinA), _pinB(pinB) {
    _gpio.setPinMode(pinA, true);
    _gpio.setPinMode(pinB, true);
    _gpio.writePin(pinA, false);
    _gpio.writePin(pinB, false);
}

void MEMSSwitch::setStateA() {
    pulse(_pinA);
}

void MEMSSwitch::setStateB() {
    pulse(_pinB);
}

void MEMSSwitch::pulse(uint8_t pin) {
    _gpio.writePin(pin, false);
    vTaskDelay(pdMS_TO_TICKS(2));
    _gpio.writePin(pin, true);
    vTaskDelay(pdMS_TO_TICKS(2));
    _gpio.writePin(pin, false);
    vTaskDelay(pdMS_TO_TICKS(2));
}


MEMSRouter::MEMSRouter(const std::pair<std::string_view, MEMSSwitch*>* switches, size_t count) {
    for (size_t i = 0; i < count; ++i) {
        _switches[switches[i].first] = switches[i].second;
    }
}


bool MEMSRouter::setSwitch(std::string_view name, char state) {
    auto it = _switches.find(name);
    if (it == _switches.end()) {
        printf("MEMS Router: switch '%.*s' not found\n", (int)name.length(), name.data());
        return false;
    }

    if (state == 'A') {
        it->second->setStateA();
    } else if (state == 'B') {
        it->second->setStateB();
    } else {
        printf("MEMS Router: invalid state '%c' for '%.*s'\n", state, (int)name.length(), name.data());
        return false;
    }
    return true;
}

bool MEMSRouter::route(std::string_view input, std::string_view output) {
    auto path = _routes.find({input, output});
    if (path == _routes.end()) {
        printf("MEMS Router: no route from '%.*s' to '%.*s'\n",
            (int)input.length(), input.data(), (int)output.length(), output.data());
        return false;
    }

    for (const auto& [name, state] : path->second) {
        if (!setSwitch(name, state)) {
            return false;
        }
    }
    return true;
}

void MEMSRouter::defineRoute(std::string_view input, std::string_view output,
                              const std::vector<std::pair<std::string_view, char>>& path) {
    _routes[{input, output}] = path;
}


std::vector<std::pair<std::string, std::string>> MEMSRouter::activeRoutes() const {
    std::vector<std::pair<std::string, std::string>> connections;
    for (const auto& [route, path] : _routes) {
        bool match = true;
        for (const auto& [name, expected] : path) {
            auto it = _switches.find(name);
            if (it == _switches.end()) {
                match = false;
                break;
            }
            // If future state tracking is added, validate it here
        }
        if (match) connections.emplace_back(route.first, route.second);
    }
    return connections;
}