//
// Created by Jeb Bailey on 4/23/25.
//

#include "zmq_frame.h"
#include <string.h>

bool zmq_decode(const uint8_t* in, size_t in_len, zmq_msg_t* out) {
    size_t pos = 0;
    out->count = 0;

    while (pos < in_len && out->count < ZMQ_MAX_FRAMES) {
        uint8_t flags = in[pos++];
        size_t len = in[pos++];

        if (len > ZMQ_MAX_FRAME_SIZE || (pos + len) > in_len)
            return false;

        memcpy(out->frames[out->count], &in[pos], len);
        out->sizes[out->count] = len;
        pos += len;

        out->count++;
        if (!(flags & 0x01)) break; // no more frames
    }

    return true;
}

size_t zmq_encode(const zmq_msg_t* in, uint8_t* out, size_t max_len) {
    size_t pos = 0;

    for (int i = 0; i < in->count; ++i) {
        if ((pos + 2 + in->sizes[i]) > max_len)
            return 0;

        uint8_t flags = (i < in->count - 1) ? 0x01 : 0x00;
        out[pos++] = flags;
        out[pos++] = (uint8_t)in->sizes[i];
        memcpy(&out[pos], in->frames[i], in->sizes[i]);
        pos += in->sizes[i];
    }

    return pos;
}
