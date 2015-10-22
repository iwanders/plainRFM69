/*
 *  Copyright (c) 2014, Ivor Wanders
 *  MIT License, see the LICENSE.md file in the root folder.
*/


#include <Arduino.h>
#include <bareRFM69.h>
#include <bareRFM69_const.h>

#ifndef PLAIN_RFM69_H
#define PLAIN_RFM69_H

/*
    The plainRFM69 object only provides the essential functions to receive and
    sent packets using the RFM69 radio module. It builds on top of bareRFM69
    to address the hardware.

    The normal state of the radio is to listen to packets, the poll() method
    should be called to obtain the packet from the radio hardare and place it
    into the plainRFM69's buffer. From this buffer it can then be read.

    The AutoModes of the radio module are used. This allows the most efficient
    transmissions of data, only enabling the transmitter when the data is ready
    to be sent. The AutoMode is also used for the receiving state, once a packet
    is received the radio is put in standby mode until the packet is retreived
    by the poll() method.

    The poll() method can also be attached to an interrupt, allowing efficient
    retrieval of packets from the FIFO and placing them in the buffer such that
    they are available outside of the interrupt.

*/


#ifdef RFM69_PLAIN_DEBUG
// #if RFM69_PLAIN_DEBUG==1
    #define debug_rfm(a) Serial.print(a);
    #define debug_rfmln(a) Serial.println(a);
#else
    #define debug_rfm(a)
    #define debug_rfmln(a)
#endif




/*
    Assumes we are little endian.
        0A 0B 0C 0D; a=0D, a+1=0C, a+2=0B, a+3=0A
*/


#define RFM69_PLAIN_STATE_RECEIVING 0
#define RFM69_PLAIN_STATE_SENDING 1

class plainRFM69 : public bareRFM69{
    protected:

        bool use_variable_length;
        bool use_addressing;
        bool use_AES;

        // state of the radio module.
        volatile uint8_t state;

        // Rx packet buffer.
        uint8_t packet_length;
        uint8_t** packet_buffer;
        uint8_t buffer_size;

        // Rx packet buffer write and read index.
        volatile uint8_t buffer_read_index;
        volatile uint8_t buffer_write_index;

        // Temporary buffer to compose the message in before writing to FIFO
        uint8_t* tx_buffer;

        void sendPacket(void* buffer, uint8_t len);
        /*
            Set the radio to Tx automode and write buffer up to len to the fifo.
            Sets the state to sending.
        */

        virtual void readPacket();
        /*
            Read a packet from the hardware to the internal buffer.
        */

        virtual void setRawPacketLength();
        /*
            Sets the real packetlength in the hardware.
        */

    public:

        plainRFM69(uint8_t cs_pin) : bareRFM69(cs_pin){
            this->packet_buffer = 0;
            this->buffer_size = 0;
            this->buffer_read_index = 0;
            this->buffer_write_index = 0;
            this->state = RFM69_PLAIN_STATE_RECEIVING;
            this->use_AES = false;
        };
        /*

            Order of calling the methods is important.
            rfm.setRecommended(); 
            // rfm.setAES(false); // should come before setPacketType
            rfm.setPacketType(false, false);

            rfm.setBufferSize(10); // should come after setPacketType
            rfm.setPacketLength(32); 
            // should come after setBufferSize, allocates the buffer

            // Might have to change the fifo thresshold
            // rfm.setFifoThreshold(RFM69_THRESHOLD_CONDITION_FIFOLEVEL,3); // bareRFM
            // if we use filtering, set the node and or broadcast address here.
            // rfm.setNodeAddress(0x01);

            rfm.setFrequency(434*1000*1000); // set the frequency.
            rfm.baud300000(); // set the modulation parameters.
            rfm.receive(); // set the radio to receive.
        */

        void setRecommended();
        /*
            Sets various parameters in the radio module to the recommended
            values as in the datasheet.
        */

        void setPacketType(bool use_variable_length, bool use_addressing);
        /*
            Sets message properties:

            use_variable_length = true:
                Uses variable length packets.

            use_variable_length = false:
                Fixed packet length.

            use_addressing = false:
                Disables filtering on address.

            use_addressing = true:
                Enables filtering on address.
                Set addressses with:
                    void setNodeAddress(uint8_t address)
                    void setBroadcastAddress(uint8_t address)

            This function sets:
                setFifoThreshold(RFM69_THRESHOLD_CONDITION_NOT_EMPTY,0);
            Such that transmissions start when the packetlength is below 15
            packets, which is the recommended value. If a slow SPI bus is used,
            it might be necessary to manually use setFifoThreshold(). 
        */
        void setBufferSize(uint8_t length);
        /*
            Sets the number of buffers slots to buffer messages into.

            Should at least two, otherwise there is no way to know whether the
            message has been read from the buffer.
            (That is the read and write index are always the same...)
        */

        void setPacketLength(uint8_t length);
        /*
            With variable length, this sets the Rx maximum length.
            With fixed length this sets the packet length for both Rx and Tx.

            Should only be called once. And after SetBufferSize and
            setPacketType.

            The length should be between 0-64, 64 bytes length is the maximum.

            If AES is enabled, any length below 16 results in zero padding by
            the radio module. So shorter lengths than 16 bytes do not result in
            more efficient transmissions.

            The setPacketType() method sets the FIFO thresshold, to start trans-
            mission, however, if a slow SPI is used, it might be necessary to
            set an adequate transmission start thresshold using:
                setFifoThreshold(RFM69_THRESHOLD_CONDITION_FIFOLEVEL, X);

            Where X determines how many bytes must be available before the
            transmission starts.

        */

        void setFrequency(uint32_t freq);
        /*
            Sets the frequency to approximately Freq.
            Uses 61 as Fstep instead of 61.03515625 which it actually is.
            For more precise control, use void setFrf from bareRFM69.

            Example:
                setFrequency((uint32_t) 450*1000*1000); sets to ~450 MHz (actually 450.259)
                setFrequency((uint32_t) 434*1000*1000); sets to ~434 MHz (actually 434.250)
        */

        void setAES(bool use_AES);
        /*
            enable or disable AES.
            use bareRFM69::setAesKey(void* buffer, uint8_t len); to set the key.

            Cipher mode is ECB, so every 16 byte block is encrypted with this
            key, identical plaintext results in identical ciphertexts.

            Remember it does not provide security against replay attacks.

            It does provide some sort of whitening filter.
        */
        

        bool canSend();
        /*
            Returns whether the module can send, or if it is busy sending.

            The current implementation is naive. This method can definitely be
            improved by taking RSSI into acocunt.

            If the interupt method is used, a missed interrupt currently causes
            the object to become stuck in the sending state.
        */

        virtual void sendAddressedVariable(uint8_t address, void* buffer, uint8_t len);
        // send to specific address with variable length. Use with setPacketType(true, true).
        // It's not possible to send anything with length zero.
        
        virtual void sendVariable(void* buffer, uint8_t len);
        // send without addressing with variable length; Use with setPacketType(true, false).
        // It's not possible to send anything with length zero.

        virtual void sendAddressed(uint8_t address, void* buffer);
        // send with addressing and fixed length. Use with setPacketType(false, true).
        
        virtual void send(void* buffer);
        // send without addressing and fixed length. Use with setPacketType(false, false).

        void receive();
        // sets the radio into receiver mode.
        // should be called after setup.



        void poll();
        /*
            Polls the radio to check for packets in the fifo. If a packet is
            received, it is written to the buffer, from which it can be
            retreived with read(void* buffer).

            When the radio has received a packet, it stops receiving and waits
            for the packet to be removed from the FIFO.

            This method also puts the radio back into receiving mode after a
            packet has been completely sent.

            It is recommended to to call this method from an interrupt attached
            to the AutoMode indicator.
        */


        bool available();
        /*
            Returns true when the read pointer is not aligned with the write
            pointer, this means packets are available, returns false in case
            they align and no new packets are available.

            There is no method to detect overflows of the buffer.
        */

        uint8_t read(void* buffer);
        /*
            Reads the next packet from the buffer to the pointer.

            If the packetlength is set to 'size' with: setPacketLength(size):

            Returns the number of bytes in the packet. For variable length
            payloads, it writes up to size bytes.

            For variable and addessing, it writes up to size+1 bytes.
            The first byte being the address to which it was sent.

            For addressing only, it writes up to size+1 bytes.

            It returns the packetlength of the packet read from the buffer.

            Returns zero in case no packet is available.
        */


        void baud4800();
        void baud9600();
        void baud153600();
        void baud300000();

        void emitPreamble(); // continuously emit a preamble
};

//PLAIN_RFM69_H
#endif