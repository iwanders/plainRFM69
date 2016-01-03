plainRFM69
=========

This is a library for HopeRF's RFM69 radio series. The main goal of this library
is to use the hardware as efficiently as possible. With this goal in mind, the
used approach is different from that of the already existing libraries for this
radio module.

Approach
-------
The library is composed of two parts, the first; bareRFM69 provides a bare bones
interface to the hardware. It does nothing by itself and only provides methods
to interact with the hardware. Most of the configuration parameters are listed
in the header file. Constants are used for easy configuration and to prevent
having to look up parameters in the datasheet.
It uses the the new SPI transaction system.

The second part; plainRFM69, provides the user with more relevant functions for
sending messages. It allows for easy sending and receiving of packets. It
contains an internal buffer for received messages. Both receiving and
transmitting packets is done using the RFM69's AutoMode system. Which means the
radio spends as little time as possible in the transmitter phase and that
received packets can be retrieved from the module efficiently. This also allows
the send methods to be non-blocking, as it is unnecessary to wait until the
radio completes transmission.


Internal Workings
---------------
The plainRFM69 class provides several methods to configure the radio module with
its recommended parameters. The module is used in packet mode. Variable length
and address filtering is supported. 

The AutoMode system (described on page 42 of the RFM69CW datasheet) allows the
radio to change its mode based on a start condition, it then enters the
Intermediate Mode until an exit condition is met. 

### Receiving
By default, the radio is listening for packets. The AutoMode is configured such
that the radio goes to standby when a packet is received. This means that the
radio waits until that packet is retrieved by the microcontroller before it
starts listening for packets again. Retrieval of this packet is done with the
poll() method, which writes the packet to the internal buffer of plainRFM69.
Once the packet had been read, the radio automatically returns to listening to
packets.

### Sending
When a packet is to be transmitted, the radio is placed into standby mode. The
AutoMode is configured such that the radio automatically switches to transmitter
mode when there are bytes in the FIFO. It leaves transmission mode after the
packet is completely sent. This ensures that the transmitter is enabled 
exactly for the time necessary to sent the packet. The next time poll() is
called, the radio is configured for receiving packets with the AutoMode, as 
described in the previous paragraph.

### Interrupt
If the radio has received a packet, it waits in the Intermediate Mode until the
packet is read from the FIFO. When a packet is transmitted, the radio is in the
Intermediate Mode until the packet is completely sent. The radio module can
represent whether it is in the Intermediate State on a digital IO pin (DIO2).

The microcontroller can attach an interrupt to this pin and call the poll()
method on every change. This means that if a packet is received, the
intermediate state is entered, DIO2 will switch from LOW to HIGH, resulting in
an interrupt trigger on the microcontroller, this calls poll() which reads the
packet from the FIFO, after this the radio automatically returns to receiving
state. The same method is possible with transmissions, where the poll() switches
the radio back to receiver mode after the transmission is complete. This means
that the microcontroller does not have to wait until the transmission is
complete, as this is also managed by the same interrupt and subsequent poll()
call. So the method to send a packet does not block until the transmission is
complete.


Testing & Performance
-------------------
The library was developed and tested on two [Teensy 3.1][teensy31]'s with
RFM69CW's attached. The pins used are described in the examples. The library is
tested with Arduino 1.0.6 and Teensyduino 1.20.

All tests were performed with the radio modules located next to each other. The
measured results are of course dependent on the parameters chosen, the
values from the examples show below use a GFSK modulation at 300000 bps.

####Maximum_Speed
Sends 64 byte packets from the sender to the receiver. For every message sent
a counter is incremented. This counter is repeated in the message to create the
payload. At the receiver the payload is checked and the counter is used to check
whether there were missed packets.
```
Total packets: 101600
Packetloss count: 1
Per second: 416
```
This example can also be used to verify that the internal buffer is working. For
example when a delay is introduced in the loop of the receiver.


####Maximum_PingPong
Sends 64 byte packets from the sender to the receiver, which echoes the packet,
the sender waits for this echo and checks the counter and message payload.
```
Successful pingpongs: 123468
Incorrect pingpongs: 0
Timeout pingpongs: 0
Successful / second: 188
Average ping (uSec): 5313
```

####Buffering
The internal buffer of bareRFM69 is demonstrated in the BusyMan example. In this
example the interrupt mechanism is used and the receiving end is very busy in
the loop, this results in several messages queueing up in the internal buffer.
```
We are busy, doing important things!
Oh look, there are packets!
Packet (4): 2035
Packet (4): 2036
Packet (4): 2037
Packet (4): 2038
Packet (4): 2039
Packet (4): 2040
We had 6 packets in the buffer.
```


Usage
-----
As said, this library was created to be efficient. The goal was not really to be
user friendly. The examples were tested and are working without any known bugs.
Feel free to use it, but if it breaks you get to keep both parts, if you have a
fix for the problem feel free to share it.

The examples should provide pointers on how to use this, I suggest looking at
Minimal and MinimalInterrupt, where the latter should be the most minimal
implementation to build on. The internal buffering is shown in the BusyMan
example.

The header files for both bareRFM69 and plainRFM69 provide explanation of the
methods.

I've only tested the examples with [Teensy 3][teensy31], so I cannot comment on
this working with other hardware. Other libraries that might interest you are
[Radiohead][radiohead] or LowPowerLabs' [RFM69][rfm69] which have seen extensive
testing, also on hardware other than the Teensy. That being said, an example has
been added which is known to work on the Arduino UNO and Moteino, see
issue [#1](https://github.com/iwanders/plainRFM69/issues/1) for further
information.

The PCB's I designed to connect the radio modules to the Teensy can be found in
[extras/hardware/](extras/hardware/).

License
------
MIT License, see LICENSE.md.

Copyright (c) 2014 Ivor Wanders


[teensy31]: http://www.pjrc.com/teensy/
[radiohead]: http://www.airspayce.com/mikem/arduino/RadioHead/
[rfm69]: https://github.com/LowPowerLab/RFM69
