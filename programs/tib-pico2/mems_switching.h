//
// Created by Jeb Bailey on 4/23/25.
//

#ifndef MEMS_SWITCHING_H
#define MEMS_SWITCHING_H


#include <vector>
#include <string>
#include <unordered_map>
#include "pcal6416a.h"
// #include <string_view>
// #include <utility>


class MEMSSwitch {
public:
    MEMSSwitch(PCAL6416A& gpio, uint8_t pinA, uint8_t pinB);
    void setStateA();
    void setStateB();

private:
    void pulse(uint8_t pin);
    PCAL6416A& _gpio;
    uint8_t _pinA;
    uint8_t _pinB;
};


struct StringPairHash {
    std::size_t operator()(const std::pair<std::string_view, std::string_view>& p) const {
        return std::hash<std::string_view>{}(p.first) ^ (std::hash<std::string_view>{}(p.second) << 1);
    }
};

class MEMSRouter {
public:
    MEMSRouter(const std::pair<std::string_view, MEMSSwitch*>* switches, size_t count);

    void defineRoute(std::string_view input, std::string_view output,
                     const std::vector<std::pair<std::string_view, char>>& path);
    bool setSwitch(std::string_view name, char state);
    bool route(std::string_view input, std::string_view output);
    std::vector<std::pair<std::string, std::string>> activeRoutes() const;

private:
    std::unordered_map<std::string_view, MEMSSwitch*> _switches;
    std::unordered_map<std::pair<std::string_view, std::string_view>,
                       std::vector<std::pair<std::string_view, char>>,
                       StringPairHash> _routes;
};


#endif // MEMS_SWITCHING_H