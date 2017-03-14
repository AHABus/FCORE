///
/// @file        RTXEncoder.c
/// @brief       AHABus Packet Radio - frame & packet encoding routines
/// @author      Cesar Parent
/// @copyright   2017 Cesar Parent
///
#include <stdio.h>
#include "RTXEncoder.h"
#include "rs8.h"

#define HIGH16(u16) (((u16) >> 8) & 0x00ff)
#define LOW16(u16)  ((u16) & 0x00ff) 

// We can't use memset() in case we're running on a platform without any
// libc implementation.
static inline void _clearFrame(uint8_t frame[FRAME_SIZE]) {
    for(uint16_t i = 0; i < FRAME_SIZE; ++i) {
        frame[i] = 0;
    }
}

// Write a frame using the encoder's write callback.
static inline bool _writeFrame(RTXCoder* encoder, uint8_t frame[FRAME_SIZE]) {
    // First write one sync byte
    if(!encoder->writeCallback(0xAA, encoder->writeData)) { return false; }
    
    // Then write the frame itself
    for(uint16_t i = 0; i < FRAME_SIZE; ++i) {
        if(!encoder->writeCallback(frame[i], encoder->writeData)) { return false; }
    }
    return true;
}

static inline uint8_t _write16(uint16_t data, uint8_t* frame) {
    *(frame++) = HIGH16(data);
    *(frame++) = LOW16(data);
    return 2;
}

static inline uint8_t _write32(int32_t data, uint8_t* frame) {
    for(int8_t i = 3; i >= 0; --i) {
        *(frame++) = (data >> (8*i)) & 0x000000ff;
    }
    return 4;
}

static uint8_t _writeFrameHeader(RTXCoder* encoder, uint8_t frame[FRAME_SIZE]) {
    uint8_t idx = 0;
    frame[idx++] = 0x5A;
    frame[idx++] = PROTOCOL_VERSION;
    idx += _write16(encoder->sequenceNumber, &frame[idx]);
    encoder->sequenceNumber += 1;
    
    return idx;
}

static uint8_t _writePacketHeader(RTXPacketHeader* header,
                                  uint8_t offset,
                                  uint8_t frame[FRAME_SIZE]) {
    uint8_t idx = 0;
    uint8_t* lengthField;
    
    frame[offset + idx++] = PROTOCOL_VERSION;
    frame[offset + idx++] = header->payloadID;
    
    // reserve space for the length.
    lengthField = &frame[offset + idx];
    idx += 2;
    
    idx += _write32(header->latitude, &frame[offset+idx]);
    idx += _write32(header->longitude, &frame[offset+idx]);
    idx += _write16(header->altitude, &frame[offset+idx]);
    
    // now we now the exact length of the header, we can write the total
    // packet length
    
    _write16(header->length + PACKET_HEADERSIZE, lengthField);
    
    return idx;
}

static int16_t _writePacketData(RTXCoder* encoder, uint8_t offset, uint16_t toWrite, uint8_t frame[FRAME_SIZE]) {
    for(uint16_t i = offset; i < FRAME_DATASIZE && toWrite > 0; ++i) {
        if(!encoder->readCallback(&frame[i], encoder->readData)) { return toWrite - 1; }
        toWrite -= 1;
    }
    return toWrite;
}

static void _writeFEC(uint8_t frame[FRAME_SIZE]) {
    encode_rs_8(&frame[1], &frame[FRAME_DATASIZE], 0);
}

int16_t fcore_rtxEncodePacket(RTXCoder* encoder, RTXPacketHeader* header) {
    
    if(header->length == 0) { return 0; }
    
    uint8_t     frame[FRAME_SIZE];
    uint16_t    frameOffset = 0;
    uint16_t    toWrite = header->length;
    uint16_t    frameCount = 0;
    
    _clearFrame(frame);
    frameOffset += _writeFrameHeader(encoder, frame);
    frameOffset += _writePacketHeader(header, frameOffset, frame);
    toWrite = _writePacketData(encoder, frameOffset, toWrite, frame);
    _writeFEC(frame);
    if(!_writeFrame(encoder, frame)) { return - 1; }
    
    frameCount += 1;
    
    while(toWrite > 0) {
        frameOffset = 0;
        _clearFrame(frame);
        frameOffset += _writeFrameHeader(encoder, frame);
        toWrite = _writePacketData(encoder, frameOffset, toWrite, frame);
        _writeFEC(frame);
        if(!_writeFrame(encoder, frame)) { return - 1; }
        frameCount += 1;
    }
    
    return frameCount;
}
