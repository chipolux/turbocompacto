# Motocompacto CANBUS Messages

## Capture With DSLogic At Idle
* Appears to operate at 500kbps, standard CAN, no extended IDs.
* ID 0
    * Transmits one message every second that seem to trigger the other IDs.
    * Transmits 1 byte 0x01 twice at startup. The first 0x00 0x00 message sneaks
      in between these two messages.
    * Otherwise transmits 2 byte 0x00 message every second.
        * Second byte just increments with every message, likely some kind of
          bus keeper/monitor.
    * Transmits 1 byte 0x01 when 0x00 counter byte rolls over.
        * But not super accurately, like, the counter rolls over and then 0x01
          emits just after the counter ticks to 0x01.
    * No special messages when turning off via idle or power button press.
* ID 19
    * Transmits infrequently.
    * 3 byte message, starting 0x01
        * Emits on single press of button on display.
        * Also emitted when changing light state via app.
        * Second byte is 0x02 when headlights are turning off, 0x01 when turning on
        * Third byte seems to always be 0x02
    * 2 byte message, starting 0x02
        * Seems to be mode, emits when you toggle mode via button on display (double click).
        * Also emitted when changing ride mode via app.
        * Second byte is 0x01 when switching to mode 1 and 0x02 for mode 2
    * 2 byte message, starting 0x03
        * Emitted when changing lock state via app.
        * Second byte seems to be 0x01 when locked, 0x02 when unlocked.
    * 2 byte message, starting 0x08
        * Emitted at startup and when safety sensor state changes.
        * Not emitted when rear wheel safety sensor changes just handlebar.
        * Second byte 0x02 when handlebars misaligned, 0x01 when aligned.
        * Second byte starts 0x00 at startup, but ends 0x01/0x02 depending on handlebar alignment.
    * 2 byte message, starting 0xFF
        * Emits on long press of power button to turn off motocompacto
        * Doesn't emit when devices does idle powerdown
        * Second byte seems to always be 0x00
* ID 49
    * Transmits constantly (every ~10ms or so).
    * 2 byte message, starting 0x02
        * Emits every ~250ms, but can be faster, maybe as a response?
        * At startup second byte is 0x02 for one message, then 0x01.
        * Second byte seems to represent mode the same as ID 19 message 0x02
        * Doesn't seem to do anything interesting on powerdown.
    * 2 byte message, starting 0x03, always seems to be 0x02
    * 2 byte message, starting 0x04, always seems to be 0x02
    * 2 byte message, starting 0x0A
        * Emits every ~250ms, doesn't seem to every go faster, maybe very rarely.
        * Second byte seems to be 0x64 when throttle is engaged. (Little bit of a deadzone at start)
        * At startup you get one message with 0x00
        * Very infrequently there I have seen 0x62, unclear what causes it.
        * Vast majority of the time second byte is 0x63.
    * 2 byte message, starting 0x0D
        * Emits every ~250ms, same as 0x0A
        * Second byte seems to reflect if user is braking, 0x01 when brake pulled, 0x02 released.
    * 2 byte message, starting 0x0E
        * Emits every ~250ms, same as 0x0A
        * Second byte seems to be handlebar alignment sensor, 0x01 when aligned, 0x02 when misaligned.
        * Like ID 19 message 0x08
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
