Teensy_RFM69CW breakout
====================

This are the breakout boards I use to connect the radio module to the Teensy chips. I used two PCB's instead of one to save cost and to allow access to the pins below the radio module. Although only DIO2 is required for the library, all DIO pins are made accessible. The silkscreen on the boards is not all that clear, so I suggest opening the schematic and board files when soldering to check where the parts should go. I have switched to KiCad since making this, so I'm not going to invest any time in updating the boards.

The lower part has a jumper to connect the clock of the SPI port to either pin 13 or 14, it is connected to pin 14 by default so that the led is still available for use. This lower part also contains an 8 pin header for SPI, in the same pin-layout as the NRF24L0+ radio modules.

The upper part has a loading circuit for a LiPo battery, based on the [MAX1555][max1555] battery charger IC. It connects the Vusb from the Teensy to the USB input of the Max1555 chip, a header is provided for the DC pin of the Max1555 chip. The battery's positive lead is connected to the battery pin of the Max1555 IC and the Vin of the Teensy. Cutting the Vusb-Vin pad on the Teensy is required when using this board.

One wire is required from the upper to the lower board, this is the MISO line of the SPI port. I used a thick piece of wire for this in order to relieve the stress on the radio module when detaching the assembly from a Teensy. In case the board should also work without a battery attached, I recommend adding a larger electrolytic capacitor over the battery pins (something like 220uF should do).

Pinout for lower board
-------------
![Schematic for lower board](media/teensy_rfm69cw_nrf_lower.png?raw=true "Lower board schematic")
Related to the RFM69CW radio module:

| RFM           | Teensy |
| ------------- | ----------- |
| MOSI          |   11 DOUT |
| SCK           |   14 SCK2 (jumper to 13) |
| NSS           |   10 CS |
| DIO3          |   16 |

For the NRF header:

| NRF   |   Teensy |
| ------|--------- |
| CE    |   9 |
| SCK   |  14 SCK2 (jumper to 13) |
| MISO  |  12 DIN |
| CSN   |  15 CS |
| MOSI  |  11 DOUT |
| IRQ   |  Solderpad |


Pinout for the upper board
-------------------
![Schematic for upper board](media/teensy_rfm69_cw_nrf_upper.png?raw=true "Upper board schematic")

Related to the RFM69CW radio module:

| RFM           | Teensy |
| ------------- | ----------- |
| MISO          |  Solderpad |
| RESET         |  23 |
| DIO0          |  2 |
| DIO1          |  1 |
| DIO2          |  0 |
| DIO5          | Solderpad |

Related to the charging Circuit:

| MAX1555       | Connected to          |
| ------------- | ----------- |
| Bat           | Battery header positive, Teensy Vin |
| DC            | DC input header positive |
| USB           | Teensy Vusb |
| CHG           | To an led |

Credits
---------
I used the Eagle part libraries from [Mikes-Eagle-Libraries](https://github.com/mgrusin/Mikes-Eagle-Libraries) and [SparkFun-PowerIC](https://github.com/sparkfun/SparkFun-Eagle-Libraries/blob/master/SparkFun-PowerIC.lbr). Inspiration to include the charging circuit came from [Onehorse's appallingly small LiPo Charger addon](https://forum.pjrc.com/threads/26462-Appallingly-small-LiPo-charger-add-on-for-Teensy-3-1).

Assembled
--------
![Top view for boards and radio module](media/top.jpg?raw=true "Top view")

![Bottom view for boards and radio module](media/bottom.jpg?raw=true "Bottom view")

[max1555]: https://www.maximintegrated.com/en/products/power/battery-management/MAX1555.html
