///
/// @file        nmea.h
/// @brief       FCORE - GNSS// parser for NMEA sentences sent by GNSS modules
/// @author      Amy Parent
/// @copyright   2017 Amy Parent
///
#include <fcore/fcore.h>
#include <fcore/buses/uart.h>
#include <fcore/gnss/gnss_parser.h>
#include <fcore/gnss/minmea.h>
#include <stdlib.h>
#include <stdint.h>

#define GNSS_BUFFERSIZE     1024

typedef enum {
    STATE_WASTING,
    STATE_INSENTENCE,
    STATE_ENDING,
    STATE_DONE,
    STATE_INVALID,
} _State;

static char         _gnssBuffer[GNSS_BUFFERSIZE];
static uint16_t     _gnssIdx    = 0;
static _State       _gnssState  = STATE_WASTING;

void fcore_gnssReset() {
    _gnssIdx = 0;
    _gnssState = STATE_WASTING;
}

static inline uint32_t _mneaToFP32(float nmea) {
    return (uint32_t)(nmea * 10000);
}

static GNSSStatus _gnssParse() {
    _gnssBuffer[_gnssIdx] = '\0';
    
    if(!minmea_check(_gnssBuffer, false)) { return GNSS_INVALID; }
    if(minmea_sentence_id(_gnssBuffer, false) != MINMEA_SENTENCE_GGA) { return GNSS_NOTGGA; }
    struct minmea_sentence_gga frame;
    minmea_parse_gga(&frame, _gnssBuffer);
    
    FCORE.latitude = _mneaToFP32(minmea_tocoord(&frame.latitude));
    FCORE.longitude = _mneaToFP32(minmea_tocoord(&frame.longitude));
    FCORE.altitude = minmea_rescale(&frame.altitude, 1);
    
    FCORE.satellites = frame.satellites_tracked;
    FCORE.fixQuality = frame.fix_quality;
    return GNSS_VALID;
}

GNSSStatus fcore_gnssFeed(char c) {
    
    switch(_gnssState) {
        case STATE_WASTING:
            //fcore_rtxWrite((uint8_t*)"**W\r\n", 5);
            if(c == '$') {
                _gnssBuffer[_gnssIdx++] = '$';
                _gnssState = STATE_INSENTENCE;
            } else {
                _gnssState = STATE_WASTING;
            }
            break;
            
        case STATE_INSENTENCE:
            //fcore_rtxWrite((uint8_t*)"**I\r\n", 5);
            if(c == '\r') {
                _gnssState = STATE_ENDING;
            }
            else {
                if(_gnssIdx >= GNSS_BUFFERSIZE) {
                    _gnssState = STATE_INVALID;
                } else {
                    _gnssBuffer[_gnssIdx++] = c;
                    _gnssState = STATE_INSENTENCE;
                }
            }
            break;
            
        case STATE_ENDING:
            //fcore_rtxWrite((uint8_t*)"**E\r\n", 5);
            _gnssState = (c == '\n') ? STATE_DONE : STATE_INVALID;
            break;
            
        case STATE_DONE:
            //fcore_rtxWrite((uint8_t*)"**D\r\n", 5);
            _gnssBuffer[_gnssIdx++] = '\r';
            _gnssBuffer[_gnssIdx++] = '\n';
            return _gnssParse();
            break;
            
        case STATE_INVALID:
            fcore_rtxWrite((uint8_t*)"NOPE\r\n", 6);
            return GNSS_INVALID;
            break;
    }
    
    return GNSS_PARSING;
}
