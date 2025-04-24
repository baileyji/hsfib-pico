//
// Created by Jeb Bailey on 4/23/25.
//

#ifndef MEMS_SWITCHING_H
#define MEMS_SWITCHING_H


#include <vector>
#include <string>
#include <unordered_map>
#include "pcal6416a.h"

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
    std::size_t operator()(const std::pair<std::string, std::string>& p) const {
        return std::hash<std::string>{}(p.first) ^ (std::hash<std::string>{}(p.second) << 1);
    }
};

class MEMSRouter {
public:
    MEMSRouter();

    void addSwitch(const std::string& name, MEMSSwitch* sw);
    void setState(const std::string& name, char state); // 'A' or 'B'

    void defineRoute(const std::string& input, const std::string& output,
                     const std::vector<std::pair<std::string, char>>& switches);
    bool route(const std::string& input, const std::string& output);
    std::vector<std::pair<std::string, std::string>> activeRoutes() const;

private:
    std::unordered_map<std::string, MEMSSwitch*> _switches;
    std::unordered_map<std::pair<std::string, std::string>,
                       std::vector<std::pair<std::string, char>>,
                       StringPairHash> _routes;
};

#endif // MEMS_SWITCHING_H