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


MEMSRouter::MEMSRouter() {}

void MEMSRouter::addSwitch(const std::string& name, MEMSSwitch* sw) {
    _switches[name] = sw;
}

void MEMSRouter::setState(const std::string& name, char state) {
    auto it = _switches.find(name);
    if (it == _switches.end()) {
        printf("MEMS Router: switch '%s' not found\n", name.c_str());
        return;
    }

    if (state == 'A') {
        it->second->setStateA();
    } else if (state == 'B') {
        it->second->setStateB();
    } else {
        printf("MEMS Router: invalid state '%c' for '%s'\n", state, name.c_str());
    }
}

bool MEMSRouter::route(const std::string& input, const std::string& output) {
    auto path = _routes.find({input, output});
    if (path == _routes.end()) {
        printf("MEMS Router: no route from '%s' to '%s'\n", input.c_str(), output.c_str());
        return false;
    }
    for (const auto& [name, state] : path->second) {
        setState(name, state);
    }
    return true;
}

void MEMSRouter::defineRoute(const std::string& input, const std::string& output,
                             const std::vector<std::pair<std::string, char>>& switches) {
    _routes[{input, output}] = switches;
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
        if (match) connections.push_back(route);
    }
    return connections;
}