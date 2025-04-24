//
// Created by Jeb Bailey on 4/23/25.
//

#ifndef ZMQ_FRAME_H
#define ZMQ_FRAME_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ZMQ_MAX_FRAMES 8
#define ZMQ_MAX_FRAME_SIZE 128

    // A parsed multipart message
    typedef struct {
        int count;
        uint8_t frames[ZMQ_MAX_FRAMES][ZMQ_MAX_FRAME_SIZE];
        size_t sizes[ZMQ_MAX_FRAMES];
    } zmq_msg_t;

    bool zmq_decode(const uint8_t* in, size_t in_len, zmq_msg_t* out);
    size_t zmq_encode(const zmq_msg_t* in, uint8_t* out, size_t max_len);

#ifdef __cplusplus
}
#endif


#endif //ZMQ_FRAME_H
