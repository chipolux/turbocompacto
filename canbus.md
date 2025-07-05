# Motocompacto CANBUS Messages

## Capture With DSLogic At Idle
* Appears to operate at 500kbps, standard CAN, no extended IDs.
* ID 0
    * Transmits one message every second that seem to trigger the other IDs.
* ID 49
    * 2 byte, 02 01 (seen 2x)
    * 8 byte, 06 00 00 0a 00 00 00 00 (seen 2x)
    * 2 byte, 0a 52 (seen 2x)
    * 4 byte, 07 59 91 00 (seen 2x)
    * 4 byte, 08 00 a6 00
    * 2 byte, 0e 01
    * 2 byte, 13 02
    * 2 byte, 04 02
    * 2 byte, 0d 02
    * 2 byte, 03 02
    * 7 byte, 0c 01 00 20 00 00 00
    * 7 byte, 0c 02 00 1b 00 00 00
* ID 156
    * Unless otherwise noted these seem to be quick responses to messages from
      ID 201 with the same first byte. (or first 2 bytes for 0f ones)
    * 5 byte, 07 00 00 98 74 (seen 3x)
    * 5 byte, 1b 00 00 00 21 (seen 2x)
    * 4 byte, 0f 01 00 1b
    * 4 byte, 0f 02 00 1b
    * 2 byte, 15 45 (seen 3x)
    * 5 byte, 1a 00 00 00 01 (seen 3x)
    * 5 byte, 1c 00 00 00 00 (seen 2x)
* ID 201
    * 1 byte, 07 (seen 3x)
    * 8 byte, 1b 00 00 00 00 00 00 00 (seen 2x)
    * 2 byte, 0f 01
    * 2 byte, 0f 02
    * 8 byte, 15 00 00 00 00 00 00 00 (seen 2x)
    * 1 byte, 15
    * 8 byte, 1a 00 00 00 00 00 00 00 (seen 2x)
    * 1 byte, 1a
    * 8 byte, 1c 00 00 00 00 00 00 00 (seen 2x)
