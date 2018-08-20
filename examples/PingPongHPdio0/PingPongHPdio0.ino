/*
 *  Copyright (c) 2014, Ivor Wanders
 *  Copyright (c) 2018, Charles Taylor
 *  
 *  MIT License, see the LICENSE.md file in the root folder.
*/

// Set up and tested with the AdaFruit Feather M0 RFM69HCW modules.

#include <SPI.h>
#include <plainRFM69.h>

// slave select pin.
#define SLAVE_SELECT_PIN 8

// connected to the reset pin of the RFM69.
#define RESET_PIN 4

// Pin should have interrupt capability.
#define DIO2_PIN 3

// Tie this pin down on the receiver.
#define SENDER_DETECT_PIN 12

/*
    In this example, one node sends 'pings' the other replies to this by
    returning the message payload.

    Addressing is used in this example.
*/


plainRFM69 rfm = plainRFM69(SLAVE_SELECT_PIN);

void sender(){
    rfm.setNodeAddress(0x02);

    rfm.setTxPower(17);
  
    uint8_t rx_buffer[5];
    uint32_t* rx_counter = (uint32_t*) &(rx_buffer[1]);

    uint32_t last_transmit_time = micros();

    uint32_t start_time = millis();

    uint32_t counter = 0; // the counter which we are going to send.

    bool are_waiting_for_receipt = false;

    uint32_t successful_pingpongs = 0;
    uint32_t timeout_pongs = 0;

    uint32_t display_time = millis(); // keep track of time to print statistics.

    uint32_t total_pingpong_time = 0;

    while(true){

        if ((millis() - display_time) > 2000){
            Serial.print("Sender: successful pingpongs: "); Serial.println(successful_pingpongs);
            Serial.print("Timeout pingpongs: "); Serial.println(timeout_pongs);
            Serial.print("Sum of RTT: "); Serial.println(total_pingpong_time);
            Serial.print("Successful / second: "); Serial.println(successful_pingpongs / ((millis() - start_time)/1000));
            Serial.print("Average ping (uSec): "); Serial.println((total_pingpong_time / successful_pingpongs));
            display_time = millis();
        }

        while (rfm.available()){ // check available messages.

            // if we are waiting for messages.
            if (are_waiting_for_receipt){

                // read the buffer.
                rfm.read(&rx_buffer);

                Serial.print("Sender: received: ");
                Serial.println(*rx_counter);

                // check if the counter matches what we expect to get back.
                if (*rx_counter == counter){
                    are_waiting_for_receipt = false;
                    
                    total_pingpong_time += (micros() - last_transmit_time);
                    successful_pingpongs++;
                } else {
                    Serial.println("Received something while waiting that didn't match.");
                }

            } else {
                rfm.read(&rx_buffer); // read it, don't do anything with it.
                Serial.print("Received something while not waiting: ");
                Serial.println(*rx_counter);
            }
        }
        

        // if we have been waiting for receipt for over 100 ms
        if (((micros() - last_transmit_time) > 100*1000) and (are_waiting_for_receipt)){ // if not received after 100 ms
            are_waiting_for_receipt = false;
            timeout_pongs++;
            Serial.print("Timeout on: "); Serial.println(counter);

        }

        if (are_waiting_for_receipt == false){
            if (!rfm.canSend()){
                continue; // sending is not possible, already sending.
            }
            // we're not waiting, sent a message.
            last_transmit_time = micros();
            counter++; // increase the counter.

            rfm.sendAddressed(0x01, &counter);

            are_waiting_for_receipt = true;
            // Serial.print("Send:");Serial.println(counter);
            
            
        }
    }
}

void receiver(){
    rfm.setNodeAddress(0x01);

    rfm.setTxPower(17);

    uint8_t buffer[5];
    uint32_t* received_counter = (uint32_t*) &(buffer[1]);

    uint8_t rssi;

    while(true){ // do forever

        while(rfm.available()){ // for all available messages:

            rssi = rfm.getRssiValue();

            // receive ping.
            uint8_t len = rfm.read(&buffer); // read the packet into the new_counter.

            Serial.print("Receiver: addressed to: ");
            Serial.print(buffer[0]);
            Serial.print(" payload: ");
            Serial.print(*received_counter);
            Serial.print(" RSSI: ");
            Serial.print(-(rssi / 2));
            Serial.print(" (");
            Serial.print(rssi);
            Serial.println(")");
            
            // return pong.
            rfm.sendAddressed(0x02, &(buffer[1]));

        }
    }
}



void interrupt_RFM(){
    rfm.poll(); // in the interrupt, call the poll function.
}


void setup(){
    delay(2000);  // Wait for USB interface to come up.
    Serial.begin(115200);
    SPI.begin();

    bareRFM69::reset(RESET_PIN); // sent the RFM69 a hard-reset.

    // Break out the setRecommended function to control.

    rfm.setRecommended(); // set recommended paramters in RFM69.
    
    //rfm.setLNA(RFM69_LNA_IMP_200OHM, RFM69_LNA_GAIN_AGC_LOOP); // Cut gain and disable AGC for testing.
    
    rfm.setPacketType(false, true); // set the used packet type.

    rfm.setBufferSize(2);   // set the internal buffer size.
    rfm.setPacketLength(6); // set the packet length.
    rfm.setFrequency((uint32_t) 923*1000*1000); // set the frequency.

    //rfm.baud9600();

    rfm.baud153600(); // Set the baudRate to
    // rfm.setPreambleSize(15); 

    // Configure as expected on the Feather M0
    rfm.setDioMapping1(RFM69_PACKET_DIO_0_RX_CRC_OK | RFM69_PACKET_DIO_0_TX_PACKET_SENT);

    // tell the RFM to represent whether we are in automode on DIO 2.
    //rfm.setDioMapping1(RFM69_PACKET_DIO_2_AUTOMODE);

    // set pinmode to input.
    pinMode(DIO2_PIN, INPUT);

    // Tell the SPI library we're going to use the SPI bus from an interrupt.
    SPI.usingInterrupt(DIO2_PIN);

    // hook our interrupt function to any edge.
    attachInterrupt(DIO2_PIN, interrupt_RFM, CHANGE);

    rfm.setHighPowerModule();

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


