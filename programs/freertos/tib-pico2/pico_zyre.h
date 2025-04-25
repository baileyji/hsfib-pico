//
// Created by Jeb Bailey on 4/24/25.
//

#ifndef PICO_ZYRE_H
#define PICO_ZYRE_H

#include "pico/stdlib.h"
#include <string>
#include <vector>
#include "pico/unique_id.h"
#include "hardware/flash.h"
#include <cstring>
#include <vector>
#include "socket.h"         // WIZnet socket functions
#include "wizchip_conf.h"   // For getIPAddress()
#include <cstdint>
#include <string>
#include <array>


#define ZMQ_MAX_FRAMES 8
#define ZMQ_MAX_FRAME_SIZE 128
#define ZYRE_MAX_RECV_BYTES 1024
#define SOCKET_COUNT 7



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
        static bool decode(const uint8_t* data, size_t len, Message& out, size_t& consumed);
        static size_t encode(const Message& msg, uint8_t* out_buf, size_t buf_size);
    };

    struct PubMessage {
        std::string topic;
        std::string payload;
    };

    class ZyreBeacon {
    public:
        void start();                          // Initial boot/start logic (e.g. send ENTER)
        void tick();                           // Call periodically from main loop for housekeeping
        bool receive(Message& out);               // Receive WHISPER from active peer
        void send_reply(const Message& in);    // Respond to last WHISPER from peer
        void send_pub(const PubMessage& pub);  // Broadcast telemetry
    private:
        bool try_receive_on_socket(uint8_t sn, Message& out, uint8_t rx_bufs[SOCKET_COUNT][ZYRE_MAX_RECV_BYTES], size_t rx_len[]);
    };

} // namespace pico_zyre

#endif // PICO_ZYRE_H


