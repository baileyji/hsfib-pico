//
// Created by Jeb Bailey on 4/23/25.
//


#ifndef PICO_ZYRE_FRAMER_H
#define PICO_ZYRE_FRAMER_H

#include <string>
#include <array>
#include <vector>
#include <cstdint>

namespace pico_zyre {

    enum class MsgType {
        ACK,
        RESPONSE,
        ERROR,
        INVALID
    };

    struct Message {
        MsgType type;
        std::array<uint8_t, 16> req_id;
        std::string key;
        std::string payload;
        uint8_t identity;  // Socket number
    };

    class ZyreFramer {
    public:
        static bool decode(const uint8_t* data, size_t len, Message& out);
        static size_t encode(const Message& msg, uint8_t* out_buf, size_t buf_size);
    };

} // namespace pico_zyre

#endif // PICO_ZYRE_FRAMER_H
