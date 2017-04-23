AHABus - FCORE
==============

    Version 0.1-A3
    Author: Amy parent <amy@amyparent.com>
    Date:   2017-04-21
    

FCORE (Flight Computer Operations and Resources Environment) is the embedded
software component of the AHABus architecture. FCORE is based on
[esp-openrtos][1] for the [ESP8266 micro-controller][2].

Usage
-----

FCORE payloads are given priority levels between 1 (highest) and 3 (lowest).
The higher the priority, the most often the payload is polled by the flight
controller.

Payloads should implement the AHABus Payload Bus Protocol. Each payload has an
address, and the payload list is defined in [`src/fcore.json`][3]. The format
of the mission configuration file is the following:

    {
        "name": "Mission Name",
        "payloads": [
            {
                "address":  <bus address>,
                "name":     "<payload name>",
                "priority": <priority level>
            },
            ...
        ]
    }

Once the configuration file is written, `make config` will parse it and generate
the required firmware files, and `make flash` will flash the resulting firmware
on the board.

`SDK_DIR` in `Makefile` should be set to your installation of the esp-opertos
SDK, and `ESPPORT` to the port the ESP board is connected to.

[1]: https://github.com/SuperHouse/esp-open-rtos
[2]: https://espressif.com/en/products/hardware/esp8266ex/overview
[3]: src/fcore.json