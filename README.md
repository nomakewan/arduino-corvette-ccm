# arduino-corvette-ccm
This sketch pretends to be the ECM/PCM for a 1990 to 1996 Chevrolet Corvette.
It listens for the CCM poll request bytes then sends the appropriate
diagnostic string upon receipt. The diagnostic string is idle values except for
coolant temp, which is either static (236F) or dynamic (based on ADC0 input).

To switch to static mode, change the value of `ECM_DYNAMIC_CTS` to `0`.
To switch to dynamic mode, change the value of `ECM_DYNAMIC_CTS` to `1`.

1990-1991 Corvettes use shorter polls and have shorter responses.
To use for a 1990-1991 Corvette, change the value of `CCM_YEAR` to `1990` or `1991`.

The 1992-1993 Corvettes use the same poll as the 1994-1996, but their response is shorter.
To use for a 1992-1993 Corvette, change the value of `CCM_YEAR` to `1992` or `1993`.

By default this sketch is set up to work with the 1994-1996 Corvette.
To use for a 1994-1996 Corvette, change the value of `CCM_YEAR` to `1994`, `1995`, or `1996`.

This sketch only works on the Arduino Mega 2560 family as-is.
This is because it uses `Serial1`, `UCSR1B`, `RXEN1`, `RXCIE1`, etc.
If you want to try it on a different board family, change these accordingly.

To use, connect RX and TX of Serial1 together then connect directly to the ALDL pin
on your ECM/PCM harness, or plug into the ALDL pin on your ALDL port (make sure
your stock ECM is unplugged if you do this).
