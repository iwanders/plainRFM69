/*
 *  Copyright (c) 2014, Ivor Wanders
 *  MIT License, see the LICENSE.md file in the root folder.
*/

#include <SPI.h>
#include <plainRFM69.h>

// slave select pin.
#define SLAVE_SELECT_PIN 10     

// connected to the reset pin of the RFM69.
#define RESET_PIN 23

// tie this pin down on the receiver.
#define SENDER_DETECT_PIN 15

// Pin DIO 2 on the RFM69 is attached to this digital pin.
// Pin should have interrupt capability.
#define DIO2_PIN 0

/*
    Sends variable length packets.

    The packets increase in length. Shows that packages larger than the set
    packetlength are not received.

*/

plainRFM69 rfm = plainRFM69(SLAVE_SELECT_PIN);

void sender(){

    uint32_t start_time = millis();

    uint8_t length = 1;
    uint8_t length_overflow = 36;

    uint8_t tx_buffer[length_overflow];
    uint32_t* counter = (uint32_t*) &tx_buffer;

    while(true){
        if (!rfm.canSend()){
            continue; // sending is not possible, already sending.
        }

        if ((millis() - start_time) > 500){ // every 500 ms. 
            start_time = millis();

            // be a little bit verbose.
            Serial.print("Send Packet ("); Serial.print(length); Serial.print("): "); Serial.println(*counter);

            // send the number of bytes equal to that set with setPacketLength.
            // read those bytes from memory where counter starts.
            rfm.sendVariable(&tx_buffer, length);

            length = (length + 1) % length_overflow;
            if (length == 0){
                length++;
            }
            
            
            (*counter)++; // increase the counter.
        }
       
    }
}

void receiver(){
    uint8_t rx_buffer[66] = {0};
    uint32_t* rx_counter = (uint32_t*) &rx_buffer;

    while(true){ // do forever

        while(rfm.available()){ // for all available messages:

            uint32_t received_count = 0; // temporary for the new counter.
            uint8_t len = rfm.read(&rx_buffer); // read the packet into the new_counter.

            // print verbose output.
            Serial.print("Received Packet ("); Serial.print(len); Serial.print("): "); Serial.println(*rx_counter);

        }
    }
}

void interrupt_RFM(){
    rfm.poll(); // in the interrupt, call the poll function.
}

void setup(){
    Serial.begin(9600);
    SPI.begin();

    bareRFM69::reset(RESET_PIN); // sent the RFM69 a hard-reset.

    rfm.setRecommended(); // set recommended paramters in RFM69.
    rfm.setPacketType(true, false); // set the used packet type.

    rfm.setBufferSize(5);   // set the internal buffer size.
    rfm.setPacketLength(32); // set the packet length.
    rfm.setFrequency((uint32_t) 434*1000*1000); // set the frequency.

    rfm.baud9600();



    pinMode(SENDER_DETECT_PIN, INPUT_PULLUP);
    delay(5);
    /*
        setup up interrupts such that we don't have to call poll() in a loop.
    */

    // tell the RFM to represent whether we are in automode on DIO 2.
    rfm.setDioMapping1(RFM69_PACKET_DIO_2_AUTOMODE);

    // set pinmode to input.
    pinMode(DIO2_PIN, INPUT);

    // Tell the SPI library we're going to use the SPI bus from an interrupt.
    SPI.usingInterrupt(DIO2_PIN);

    // hook our interrupt function to any edge.
    attachInterrupt(DIO2_PIN, interrupt_RFM, CHANGE);

    // start receiving.
    rfm.receive();
    delay(2000);
}

void loop(){
    if (digitalRead(SENDER_DETECT_PIN) == LOW){
        Serial.println("Going Receiver!");
        receiver(); 
        // this function never returns and contains an infinite loop.
    } else {
        Serial.println("Going sender!");
        sender();
        // idem.
    }
}


