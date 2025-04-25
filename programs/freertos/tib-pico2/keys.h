//
// Created by Jeb Bailey on 4/25/25.
//

#ifndef KEYS_H
#define KEYS_H

#include <cstdint>

namespace dispatch {

    constexpr uint32_t hash(const char* str) {
        uint32_t h = 5381;
        while (*str) {
            h = ((h << 5) + h) ^ static_cast<uint8_t>(*str++);  // djb2 XOR variant
        }
        return h;
    }

    // Known mKTL keys
    constexpr const char KEY_PD0_GAIN[] = "photodiode.0.gain";
    constexpr const char KEY_PD1_GAIN[] = "photodiode.1.gain";
    constexpr const char KEY_ATT4_DB[] = "attenuator.4.db";
    constexpr const char KEY_ROUTER_ROUTE[] = "router.set_route";
    constexpr const char KEY_LASER0_ENABLE[] = "laser.0.enable";

    // Hashes (used in switch statements)
    constexpr uint32_t H_PD0_GAIN       = hash(KEY_PD0_GAIN);
    constexpr uint32_t H_PD1_GAIN       = hash(KEY_PD1_GAIN);
    constexpr uint32_t H_ATT4_DB        = hash(KEY_ATT4_DB);
    constexpr uint32_t H_ROUTER_ROUTE   = hash(KEY_ROUTER_ROUTE);
    constexpr uint32_t H_LASER0_ENABLE  = hash(KEY_LASER0_ENABLE);




constexpr uint32_t hashes[] = {
    H_PD0_GAIN,
    H_PD1_GAIN,
    H_ATT4_DB,
    H_ROUTER_ROUTE,
    H_LASER0_ENABLE
};

constexpr bool has_collisions() {
    for (size_t i = 0; i < sizeof(hashes)/sizeof(hashes[0]); ++i)
        for (size_t j = i + 1; j < sizeof(hashes)/sizeof(hashes[0]); ++j)
            if (hashes[i] == hashes[j]) return true;
    return false;
}

static_assert(!has_collisions(), "Hash collision detected in dispatch keys!");

} // namespace dispatch
#endif //KEYS_H
