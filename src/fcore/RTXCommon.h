///
/// @file        RTXCommon.h
/// @brief       AHABus Packet Radio - common includes and functions
/// @author      Cesar Parent
/// @copyright   2017 Cesar Parent
///
#pragma once
#include <stdbool.h>
#include <stdint.h>

#define PROTOCOL_VERSION    0x01

#define FRAME_SIZE          256
#define FRAME_HEADERSIZE    4
#define FRAME_DATASIZE      224

#define PACKET_HEADERSIZE   12
#ifndef PACKET_MAXSIZE
#define PACKET_MAXSIZE      512
#endif

/// Callback that can be called when a byte of a frame has been encoded.
/// Returns whether the write was successful or not.
typedef bool (*RTXWrite)(uint8_t, void*);

/// Callback that can be called when a byte is required to continue decoding.
/// Returns true if a byte was read, false otherwise.
/// Should block if no data is available, but is expected.
typedef bool (*RTXRead)(uint8_t*, void*);

/// Fixed-point 9.23 bit decimal number.
typedef int32_t fp823_t;

/// Defines the data required to write a packet.
typedef struct {
    /// Primary Packet Header.
    uint8_t     payloadID;
    uint16_t    length;
    
    /// Secondary Packet Header.
    fp823_t     latitude;
    fp823_t     longitude;
    uint16_t    altitude;
} RTXPacketHeader;

typedef struct {
    uint16_t    sequenceNumber;
    void*       readData;
    RTXRead     readCallback;
    void*       writeData;
    RTXWrite    writeCallback;
} RTXCoder;
