///
/// @file        nmea.h
/// @brief       FCORE - GNSS// parser for NMEA sentences sent by GNSS modules
/// @author      Amy Parent
/// @copyright   2017 Amy Parent
///
#include <fcore/gnss/nmea.h>
#include <stdlib.h>
#include <stdint.h>

#define NMEA_BUFFERSIZE     1024

typedef enum {
    STATE_WASTING,
    STATE_INSENTENCE,
    STATE_ENDING,
    STATE_DONE,
    STATE_INVALID,
} _State;

static char _nmeaBuffer[NMEA_BUFFERSIZE];
static uint16_t     _nmeaIdx    = 0;
static _State       _nmeaState  = STATE_WASTING;
static const char*  _nmeaStart  = NULL;
static const char*  _nmeaEnd    = NULL;



void fcore_nmeaInit() {
    _nmeaIdx = 0;
    _nmeaState = STATE_WASTING;
    _nmeaStart = _nmeaEnd = NULL;
}

static NMEAStatus _nmeaParse() {
    return NMEA_VALID;
}

NMEAStatus fcore_nmeaFeed(char c) {
    
    switch(_nmeaState) {
        case STATE_WASTING:
            _nmeaState = (c == '$') ? STATE_INSENTENCE : STATE_WASTING;
            break;
            
        case STATE_INSENTENCE:
            if(c == '\r') {
                _nmeaState = STATE_ENDING;
            } else {
                if(_nmeaIdx >= NMEA_BUFFERSIZE) {
                    _nmeaState = STATE_INVALID;
                    return NMEA_INVALID;
                }
                _nmeaBuffer[_nmeaIdx++] = c;
                _nmeaState = STATE_INSENTENCE;
            }
            break;
            
        case STATE_ENDING:
            _nmeaState = (c == '\n') ? STATE_DONE : STATE_INVALID;
            break;
            
        case STATE_DONE:
            return _nmeaParse();
            break;
            
        case STATE_INVALID:
            return NMEA_INVALID;
            break;
    }
    
    return NMEA_PARSING;
}
