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
    This is a minimal example with the interrupt to call poll().

    It transmits a 4 byte integer every 500 ms.
    
*/

plainRFM69 rfm = plainRFM69(SLAVE_SELECT_PIN);


void sender(){

    uint32_t start_time = millis();

    uint32_t counter = 0; // the counter which we are going to send.

    while(true){
        if (!rfm.canSend()){
            continue; // sending is not possible, already sending.
        }

        if ((millis() - start_time) > 500){ // every 500 ms. 
            start_time = millis();

            // be a little bit verbose.
            Serial.print("Send:");Serial.println(counter);

            // send the number of bytes equal to that set with setPacketLength.
            // read those bytes from memory where counter starts.
            rfm.send(&counter);
            
            counter++; // increase the counter.
        }
       
    }
}

void receiver(){
    uint32_t counter = 0; // to count the messages.

    while(true){
        while(rfm.available()){ // for all available messages:

            uint32_t received_count = 0; // temporary for the new counter.
            uint8_t len = rfm.read(&received_count); // read the packet into the new_counter.

            // print verbose output.
            Serial.print("Packet ("); Serial.print(len); Serial.print("): "); Serial.println(received_count);

            if (counter+1 != received_count){
                // if the increment is larger than one, we lost one or more packets.
                Serial.println("Packetloss detected!");
            }

            // assign the received counter to our counter.
            counter = received_count;
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
    rfm.setPacketType(false, false); // set the used packet type.

    rfm.setBufferSize(2);   // set the internal buffer size.
    rfm.setPacketLength(4); // set the packet length.
    rfm.setFrequency((uint32_t) 434*1000*1000); // set the frequency.

    // baudrate is default, 4800 bps now.

    rfm.receive();
    // set it to receiving mode.

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


    pinMode(SENDER_DETECT_PIN, INPUT_PULLUP);
    delay(5);
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


