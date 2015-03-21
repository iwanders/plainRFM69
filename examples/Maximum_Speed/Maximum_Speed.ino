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
    In this example messages are sent one directional. The receiver verifies
    these packets and checks whether the counter nicely increments.

    Be sure to start the receiver after the transmitter has been started. This
    results in correct measurements of number of packets per second. But always
    causes the packetloss count to be at least 1, as the first packet received
    from the sender does not contain counter=0.
*/



plainRFM69 rfm = plainRFM69(SLAVE_SELECT_PIN);

void sender(){
    uint8_t buffer[64] = {0};

    uint32_t counter = 0; // set the counter to zero.

    while(true){
        if (!rfm.canSend()){
            continue;
        }
        
        for (int i=0; i < (64/4); i++){
            uint32_t* this_counter = (uint32_t*) &(buffer[i*4]);
            *this_counter = counter;
            // fill the entire message with this counter, repeating every 4 bytes.
        }

        if ((counter % 100) == 0){
            Serial.print("Send:");Serial.println(counter);
        }

        rfm.send(&buffer);
        counter++; // increase the counter.
    }
}

void receiver(){

    uint8_t buffer[64] = {0}; // receive buffer.

    // use first four bytes of that buffer as uint32.
    uint32_t counter = 0;

    uint32_t oldcounter = 0; // keep track of the counter.
    uint32_t packetloss = 0; // counter for missed packets.

    // counter to indicate total number of received packets.
    uint32_t received_packets = 0;

    // time on which we start receiving.
    uint32_t start_time = millis();
    
    while(true){
        uint8_t packets=0;
        while(rfm.available()){
            packets++;
            received_packets++;

            // on the receipt of 100 packets, print some information.
            if ((received_packets % 100 == 0)){
                Serial.print("Total packets: "); Serial.println(received_packets);
                Serial.print("Packetloss count: "); Serial.println(packetloss);
                Serial.print("Per second: "); Serial.println(received_packets / ((millis() - start_time)/1000));
            }

            uint8_t len = rfm.read(&buffer); // read the packet into the buffer.

            // check the entire message if it consists of the same 4 byte blocks.
            uint32_t* this_counter = (uint32_t*) &(buffer[0]);
            for (int i=1; i < (64/4); i++){
                if (*(this_counter) != *((uint32_t*) &(buffer[i*4]))){
                    Serial.println("Message not correct!");
                    packetloss++;
                    continue;
                }
            }

            if ((counter+1) != *this_counter){
                Serial.println("Packetloss detected!");
                packetloss++;
            }

            counter = *this_counter;
        }

        // we can add some delay here, to show that the internal buffering works.
        // delayMicroseconds(4000);
        // try uncommenting this delay, the if statement below will be true from time to time.
        if (packets > 1){
            Serial.print("Packets this loop: "); Serial.println(packets);
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

    rfm.setRecommended();
    rfm.setPacketType(false, false);

    // allocate buffer in the library for received packets
    rfm.setBufferSize(10);      // allow buffering of up to ten packets.
    rfm.setPacketLength(64);    // length of packets.

    rfm.setFrequency((uint32_t) 434*1000*1000); // set frequency to 434 MHz.
    rfm.baud300000(); // Set the baudRate to 300000 bps

    // At higher packetrates it is necessary to increase this in order to ensure
    // packet detection at the receiving side.
    rfm.setPreambleSize(15); 

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

}

void loop(){
    // go receiver or sender depending on the SENDER_DETECT_PIN.
    pinMode(SENDER_DETECT_PIN, INPUT_PULLUP);
    delay(5);

    if (digitalRead(SENDER_DETECT_PIN) == LOW){
        Serial.println("Starting receiver!");
        receiver(); // we never return from this.
    } else {
        Serial.println("Starting sender!");
        sender(); // we never return from this.
    }
}

