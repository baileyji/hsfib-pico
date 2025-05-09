//
// Created by Jeb Bailey on 4/24/25.
//

#include "pico_zyre.h"
#include "socket.h"
#include "dhcp.h"
#include "FreeRTOS.h"
#include "task.h"
#include "wizchip_conf.h"
#include "pico/unique_id.h"

#include <vector>
#include <cstring>
#include <cstdio>

#include <nlohmann/json.hpp>
#include <cstring>

#include "log_util.h"

using nlohmann::json;

namespace pico_zyre {

    static bool wait_for_ip() {
        uint8_t ip[4] = {0};
        for (int i = 0; i < 100; ++i) {
            getIPfromDHCP(ip);
            if (ip[0] != 0 || ip[1] != 0 || ip[2] != 0 || ip[3] != 0) {
                SAFE_PRINTF("[coms] IP acquired: %d.%d.%d.%d\\n", ip[0], ip[1], ip[2], ip[3]);
                return true;
            }
            vTaskDelay(pdMS_TO_TICKS(100));
        }
        SAFE_PRINTF("[coms] DHCP timeout. No IP acquired.\\n");
        return false;
    }

    static const uint32_t ENTER_INTERVAL_S = 10;
    static const uint8_t SOCK_UDP_ZRE = 0;
    static const uint8_t WHISPER_SOCKETS[] = {1, 2, 3, 4}; // 0 reserved for UDP, 5 for PUB
    static const uint8_t WHISPER_OVERFLOW = 5;
    static const uint8_t SOCK_TCP_PUB = 6;
    static const uint8_t DHCP_SOCKET = 7;
    static const uint16_t PORT_ZRE_UDP = 5670;
    static const uint16_t PORT_ZRE_TCP = 5000;

    static bool socket_initialized[SOCKET_COUNT] = {false};

    static uint8_t uuid[16];
    static bool enter_sent = false;

    void generate_uuid() {
        pico_unique_board_id_t id;
        pico_get_unique_board_id(&id);
        const char* salt = __TIME__ __DATE__;
        uint32_t hash[4] = {0x811c9dc5, 0x811c9dc5, 0x811c9dc5, 0x811c9dc5};
        for (size_t i = 0; i < sizeof(id.id); ++i)
            hash[i % 4] = (hash[i % 4] ^ id.id[i]) * 0x01000193;
        for (size_t i = 0; i < strlen(salt); ++i)
            hash[i % 4] = (hash[i % 4] ^ salt[i]) * 0x01000193;
        for (int i = 0; i < 16; ++i)
            uuid[i] = static_cast<uint8_t>((hash[i % 4] >> ((i / 4) * 8)) & 0xFF);
    }

    static std::string_view msgtype_to_string(MsgType type) {
        switch (type) {
            case MsgType::ACK: return "ack";
            case MsgType::RESPONSE: return "response";
            case MsgType::ERROR: return "error";
            default: return "invalid";
        }
    }

    static MsgType string_to_msgtype(std::string_view s) {
        if (s == "ack") return MsgType::ACK;
        if (s == "response") return MsgType::RESPONSE;
        if (s == "error") return MsgType::ERROR;
        return MsgType::INVALID;
    }

    bool ZyreFramer::decode(const uint8_t* data, size_t len, Command& out, size_t& consumed) {
        std::vector<std::string> parts;
        size_t i = 0;
        consumed = 0;

        while (i + 2 <= len) {
            bool more = data[i++] & 0x01;
            uint8_t size = data[i++];
            if (i + size > len) return false;
            parts.emplace_back(reinterpret_cast<const char*>(&data[i]), size);
            i += size;
            if (!more) break;
        }

        if (parts.size() < 4) return false;

        out.type = string_to_msgtype(parts[0]);

        if (parts[1].size() != 16) return false;
        std::memcpy(out.req_id.data(), parts[1].data(), 16);

        out.key = parts[2];

        out.args = json::parse(parts[3], /* callback */ nullptr, /* allow exceptions */ false);
        if (out.args.is_discarded())
            return false;

        consumed = i;
        return true;
    }


    size_t ZyreFramer::encode(const Response& msg, uint8_t* out_buf, size_t buf_size) {
        std::vector<std::string> frames;
        frames.push_back(std::string(msgtype_to_string(msg.type)));

        std::string reqid(reinterpret_cast<const char*>(msg.req_id.data()), 16);
        frames.push_back(reqid);

        frames.push_back(msg.key);
        frames.push_back(msg.payload);

        size_t offset = 0;
        for (size_t i = 0; i < frames.size(); ++i) {
            bool more = (i < frames.size() - 1);
            const std::string& s = frames[i];
            size_t required = 2 + s.size();
            if (offset + required > buf_size) return 0;
            out_buf[offset++] = more ? 0x01 : 0x00;
            out_buf[offset++] = static_cast<uint8_t>(s.size());
            std::memcpy(&out_buf[offset], s.data(), s.size());
            offset += s.size();
        }

        return offset;
    }


    static bool last_link_status = true;
    static absolute_time_t last_enter_time = {0};
    static bool dhcp_active = false;
    static uint8_t dhcp_buf[548];

    bool link_up() {
        return wizphy_getphylink() == PHY_LINK_ON;
    }

    void handle_link_down() {
        SAFE_PRINTF("[zyre] handling link down.\n");
        if (last_link_status) {
            enter_sent = false;
            dhcp_active = false;
            for (int sn = 0; sn < SOCKET_COUNT; ++sn) {
                // if (socket_initialized[sn]) {
                disconnect(sn);
                close(sn);
                socket_initialized[sn] = false;
                // }
            }
            DHCP_stop();
        }
        last_link_status = false;
    }

    bool link_restored() {
        if (!last_link_status && link_up()) {
            last_link_status = true;
            return true;
        }
        return false;
    }


    void start_dhcp() {
        SAFE_PRINTF("[zyre] Starting DHCP\n");
        DHCP_init(DHCP_SOCKET, dhcp_buf);
        for (int i = 0; i < 100; ++i) {
            if (DHCP_run() == DHCP_IP_LEASED) {
                dhcp_active = true;
                SAFE_PRINTF("[zyre] DHCP complete\n");
                return;
            }
            vTaskDelay(pdMS_TO_TICKS(100));
        }
        SAFE_PRINTF("[zyre] No lease, stopping DHCP\n");
        DHCP_stop();
    }

    void maybe_send_enter(const std::string& name) {
        if (!enter_sent || absolute_time_diff_us(last_enter_time, get_absolute_time()) > ENTER_INTERVAL_S * 1000 * 1000) {

            uint8_t bcast_ip[4] = {255, 255, 255, 255};

            std::vector<uint8_t> frame;
            frame.push_back(0x02);  // ZRE_MSG_ENTER
            frame.insert(frame.end(), uuid, uuid + 16);

            uint8_t ip[4] = {0};
            getIPfromDHCP(ip);

            char endpoint_buf[32];
            snprintf(endpoint_buf, sizeof(endpoint_buf), "tcp://%d.%d.%d.%d:%d", ip[0], ip[1], ip[2], ip[3], PORT_ZRE_TCP);
            std::string endpoint(endpoint_buf);

            for (const auto& field : {name, endpoint, std::string("")}) {
                uint16_t len = field.size();
                frame.push_back((len >> 8) & 0xFF);
                frame.push_back(len & 0xFF);
                frame.insert(frame.end(), field.begin(), field.end());
            }


            SAFE_PRINTF("[zyre] Announcing self on %s\n", endpoint.c_str());

            if (!socket_initialized[SOCK_UDP_ZRE]) {
                socket(SOCK_UDP_ZRE, Sn_MR_UDP, 0, 0);
                socket_initialized[SOCK_UDP_ZRE] = true;
            }

            sendto(SOCK_UDP_ZRE, frame.data(), frame.size(), bcast_ip, PORT_ZRE_UDP);
            last_enter_time = get_absolute_time();
            enter_sent = true;
        }
    }

    ZyreBeacon::ZyreBeacon(const std::string& name) : _name(name) {}

    void ZyreBeacon::start() {
        generate_uuid();
        enter_sent = false;
    }

    void ZyreBeacon::tick() {
        if (!link_up()) {
            handle_link_down();
            return;
        }

        if (link_restored()) {
            SAFE_PRINTF("[zyre] link restored.\n");
            dhcp_active = false;
        }

        if (!dhcp_active) {
            start_dhcp();
            return;
        }

        maybe_send_enter(_name);
    }

    bool ZyreBeacon::try_receive_on_socket(uint8_t sn, Command& out, uint8_t rx_bufs[SOCKET_COUNT][ZYRE_MAX_RECV_BYTES], size_t rx_len[]) {
        int bytes = recv(sn, &rx_bufs[sn][rx_len[sn]], ZYRE_MAX_RECV_BYTES - rx_len[sn]);
        if (bytes > 0) {
            rx_len[sn] += bytes;

            size_t consumed = 0;
            if (ZyreFramer::decode(rx_bufs[sn], rx_len[sn], out, consumed)) {
                out.identity = sn;

                if (consumed > 0 && consumed < rx_len[sn]) {
                    memmove(rx_bufs[sn], &rx_bufs[sn][consumed], rx_len[sn] - consumed);
                    rx_len[sn] -= consumed;
                } else {
                    rx_len[sn] = 0;
                }

                return true;
            }

            if (rx_len[sn] == ZYRE_MAX_RECV_BYTES) {
                SAFE_PRINTF("[zyre] Overflow on socket %d, dropping message.\\n", sn);
                rx_len[sn] = 0;
            } else {
                SAFE_PRINTF("[zyre] Incomplete or malformed message on socket %d.\\n", sn);
            }
        } else if (getSn_SR(sn) == SOCK_CLOSE_WAIT || getSn_SR(sn) == SOCK_CLOSED) {
            close(sn);
            socket_initialized[sn] = false;
            rx_len[sn] = 0;
        }

        return false;
    }

    bool ZyreBeacon::receive(Command& out) {
        static uint8_t rx_bufs[SOCKET_COUNT][ZYRE_MAX_RECV_BYTES];  // Include overflow socket
        static size_t rx_len[SOCKET_COUNT] = {0};

        for (uint8_t sn : WHISPER_SOCKETS) {
            if (!socket_initialized[sn]) {
                socket(sn, Sn_MR_TCP, PORT_ZRE_TCP, 0);
                listen(sn);
                socket_initialized[sn] = true;
            }

            if (getSn_SR(sn) == SOCK_ESTABLISHED) {
                if (try_receive_on_socket(sn, out, rx_bufs, rx_len))
                    return true;
            }
        }

        // Overflow socket — handle latecomers with "too busy"
        uint8_t sn = WHISPER_OVERFLOW;
        if (!socket_initialized[sn]) {
            socket(sn, Sn_MR_TCP, PORT_ZRE_TCP, 0);
            listen(sn);
            socket_initialized[sn] = true;
        }

        if (getSn_SR(sn) == SOCK_ESTABLISHED) {
            if (try_receive_on_socket(sn, out, rx_bufs, rx_len)) {
                Response resp;
                resp.type = MsgType::ERROR;
                resp.identity=out.identity;
                resp.key=out.key;
                resp.req_id=out.req_id;
                resp.payload = R"({"error":"too busy"})";
                send_reply(resp);
                vTaskDelay(pdMS_TO_TICKS(100)); // allow buffer flush
            }

            disconnect(sn);
            close(sn);
            socket_initialized[sn] = false;
        }

        return false;
    }

    void ZyreBeacon::send_reply(const Response& in) {
        uint8_t sn = in.identity;

        if (getSn_SR(sn) != SOCK_ESTABLISHED) {
            SAFE_PRINTF("[zyre] Socket %d not established, dropping reply.\\n", sn);
            if (getSn_SR(sn) != SOCK_CLOSED) {
                disconnect(sn);
                close(sn);
            }
            socket(sn, Sn_MR_TCP, PORT_ZRE_TCP, 0);
            listen(sn);
            return;
        }

        // Add ZMTP null identity prefix
        uint8_t full_buf[ZYRE_MAX_RECV_BYTES];
        full_buf[0] = 0x01;
        full_buf[1] = 0x00;
        size_t len = ZyreFramer::encode(in, &full_buf[2], sizeof(full_buf)-2);
        if (len == 0) {
            SAFE_PRINTF("[zyre] Failed to encode frame for reply.\\n");
            return;
        }

        size_t total_len = len + 2;

        if (getSn_TX_FSR(sn) < total_len) {
            SAFE_PRINTF("[zyre] Socket %d TX full, dropping and resetting.\\n", sn);
            disconnect(sn);
            close(sn);
            socket(sn, Sn_MR_TCP, PORT_ZRE_TCP, 0);
            listen(sn);
            return;
        }

        send(sn, full_buf, total_len);
    }

    void ZyreBeacon::send_pub(const PubMessage& pub) {
        std::vector<uint8_t> frame;

        auto add_part = [&frame](const std::string& s, bool more) {
            frame.push_back(more ? 0x01 : 0x00);
            frame.push_back(static_cast<uint8_t>(s.size()));
            frame.insert(frame.end(), s.begin(), s.end());
        };

        add_part(pub.topic, true);
        add_part(pub.payload, false);

        uint8_t sn = SOCK_TCP_PUB;

        // Initialize or re-listen if needed
        if (getSn_SR(sn) != SOCK_ESTABLISHED) {
            if (getSn_SR(sn) != SOCK_CLOSED)
                close(sn);
            socket(sn, Sn_MR_TCP, PORT_ZRE_TCP + 1, 0); // use adjacent port (e.g., 5001)
            listen(sn);
            return;
        }

        // Check TX free size; flush socket if not enough room
        uint16_t tx_free = getSn_TX_FSR(sn);
        if (tx_free < frame.size()) {
            disconnect(sn);
            close(sn);
            socket(sn, Sn_MR_TCP, PORT_ZRE_TCP + 1, 0);
            listen(sn);
            return;
        }

        send(sn, frame.data(), frame.size());
    }
}
