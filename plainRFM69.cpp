/*
 *  Copyright (c) 2014, Ivor Wanders
 *  MIT License, see the LICENSE.md file in the root folder.
*/


#include "plainRFM69.h"



/*
        Public Methods
*/

void plainRFM69::setRecommended(){

    // p67, 200 ohm, internal AGC loop.
    this->setLNA(RFM69_LNA_IMP_200OHM, RFM69_LNA_GAIN_AGC_LOOP);

    // p71, 3 preamble bytes.
    this->setPreambleSize(3); 

    // p71, 4 bytes sync of 0x01, only start listening when sync is matched.
    uint8_t syncthing[] = {1, 1, 1, 1};
    this->setSyncConfig(true, false, sizeof(syncthing),0);
    this->setSyncValue(&syncthing, sizeof(syncthing));

    // p70,  -114 dBm.
    this->setRSSIThreshold(0xe4);

    // p74, Improved margin, use if AfcLowBetaOn=0
    this->setContinuousDagc(RFM69_CONTINUOUS_DAGC_IMPROVED_AFCLOWBETAOFF); 


    // p63, and p21 for more information about the shaping.
    this->setDataModul(RFM69_DATAMODUL_PROCESSING_PACKET, RFM69_DATAMODUL_FSK, RFM69_DATAMODUL_SHAPING_GFSK_NONE);
    

    // actually recommended, but might result in packets not being transmitted with variable length.
    this->setFifoThreshold(RFM69_THRESHOLD_CONDITION_FIFOLEVEL,15);


    // p67, recommended Rx Bandwith, is changed by baudrate methods.
    this->setRxBw(0b010, 0b10, 0b101);
    // p67, no further information.
    this->setAfcBw(0b100, 0b01, 0b011); 
    
}


void plainRFM69::setPacketType(bool variable_length, bool use_addressing){

    this->use_variable_length = variable_length;
    this->use_addressing = use_addressing;

    uint8_t flags = RFM69_PACKET_CONFIG_DC_FREE_WHITENING | RFM69_PACKET_CONFIG_CRC_ON;
    // uint8_t flags = RFM69_PACKET_CONFIG_DC_FREE_MANCHESTER | RFM69_PACKET_CONFIG_CRC_ON;
    // uint8_t flags = RFM69_PACKET_CONFIG_DC_FREE_NONE | RFM69_PACKET_CONFIG_CRC_ON; // This is actually recommended, surprisingly.

    flags |= variable_length ? RFM69_PACKET_CONFIG_LENGTH_VARIABLE : RFM69_PACKET_CONFIG_LENGTH_FIXED;
    // enable variable length.

    flags |= use_addressing ? RFM69_PACKET_CONFIG_ADDRESS_FILTER_NODE_BROADCAST : RFM69_PACKET_CONFIG_ADDRESS_FILTER_NONE;
    // enable address filtering.

    this->setPacketConfig1(flags);

    // setPacketConfig2(uint8_t InterPacketRxDelay, bool RestartRx, bool AutoRxRestartOn, bool AesOn)
    this->setPacketConfig2(0, false, false, this->use_AES);
    // when we have a packet, wait until it's retreived, do not restart Rx.

    // Set the fifo thresshold to just start sending....
    // The SPI clock _should_ be faster than the bitrate in any case.
    this->setFifoThreshold(RFM69_THRESHOLD_CONDITION_NOT_EMPTY, 0);
    

}

void plainRFM69::setBufferSize(uint8_t size){
    this->buffer_size = size;
}

void plainRFM69::setPacketLength(uint8_t length){
    this->packet_length = length + this->use_addressing;

    // allocate the packet buffer, first allocate a list of pointers
    this->packet_buffer = (uint8_t**) malloc(this->buffer_size * sizeof(uint8_t*));
    for (uint8_t i = 0; i < this->buffer_size; i++){
        // then allocate the packet length per buffer slot.
        this->packet_buffer[i] = (uint8_t*) malloc(this->packet_length + this->use_variable_length);
    }

    // this is mostly a separate function such that it can be overloaded.
    this->setRawPacketLength();

}

void plainRFM69::setFrequency(uint64_t freq){
    // 61 should be 61.03515625 for precision.
    const uint64_t divider = 61;
    this->setFrf(freq / divider);
}


void plainRFM69::setAES(bool use_AES){
    this->use_AES = use_AES;
}


void plainRFM69::setTxPower(int8_t power_level_dBm, bool enable_boost)
{
    /*
       The code below is based on the knowledge gleaned from this article:

         https://andrehessling.de/2015/02/07/figuring-out-the-power-level-settings-of-hoperfs-rfm69-hwhcw-modules/

       Additionally, I studied the following drivers for inspiration and understanding.

         https://github.com/ahessling/RFM69-STM32
         https://github.com/LowPowerLab/RFM69
         http://www.airspayce.com/mikem/arduino/RadioHead/classRH__RF69.html
    */

    // Determine the hardware type.
    if (use_HP_module)
    {
        // ***** High power module is installed. Things are complicated. *****

        /*
           Possible output power levels for the high-power hardware are -2 to +20 dBm.

             -2 to +13  Using just PA1
             +2 to +17  Using PA1 and PA2

           +3 dBm on top of the above levels is available by also enabling the
           RFM69_TEST_PA1 and RFM69_TEST_PA2 registers. This boost mode comes with
           some caveats.

           First, the +20dBm level is limited to a 1% Duty Cycle. Given the design of
           the module and the amount of power it takes to reach these levels, this
           makes me think the radio might have a thermal problem when these
           capabilities are used.

           Second, the datasheet explicitly states on page 22 that the high power
           settings must not be used in Receive mode. This requires adding the
           tx_power_boosted option to enable these when transmitting and disable them
           when receiving.

           Lastly, the on-module over-current protection must be disabled. This will
           cause strain on the underlying power system, probably explaining the need
           for extra capacitors in certain configurations.

           Because of the above, this function is set up to require that the boost
           mode be explicitly enabled to reach those power levels.
        */

        // Boost mode control.
        if (enable_boost)
        {
            // Enable boost register control in the transmit/receive functions.
            this->tx_power_boosted = true;

            // Explicitly configure Over Current Protection as specified on page 22.
            this->setOCP(0);

            // Set the upper limit on the requested power.
            if (power_level_dBm > 20) power_level_dBm = 20;
        } else {
            // Set the upper limit on the requested power, without boost mode.
            if (power_level_dBm > 17) power_level_dBm = 17;
        }

        // The low-end power range is more restricted with high power modules.
        if (power_level_dBm < -2) power_level_dBm = -2;

        // Up to +13dBm, only the PA1 amplifier is needed.
        if (power_level_dBm <= 13)
        {
            this->setPALevel(RFM69_PA_LEVEL_PA1_ON, power_level_dBm + 18);
        } else {
            this->setPALevel(RFM69_PA_LEVEL_PA1_ON + RFM69_PA_LEVEL_PA2_ON, power_level_dBm + 11);
        }

    } else {
        // ***** Low power module is installed. Things are simple. *****
        // Enable PA0 and set the power directly using an offset from the passed value.
        if (power_level_dBm < -18) power_level_dBm = -18;
        if (power_level_dBm > 13) power_level_dBm = 13;
        this->setPALevel(RFM69_PA_LEVEL_PA0_ON, power_level_dBm + 18);
    }
}


bool plainRFM69::canSend(){
    return (this->state == RFM69_PLAIN_STATE_RECEIVING);
    // if we're receiving, we can send.
    // This is perhaps slightly naive, as a packet might be being received.
    
    /*
    perhaps also check RSSI:

    uint8_t flags1;
    flags1 = this->getIRQ1Flags();
    debug_rfm("canSend, flags1: "); debug_rfmln(flags1);
    return ((this->state == RFM69_PLAIN_STATE_RECEIVING) and ((flags1 & RFM69_IRQ1_RSSI)==0));

    It is more correct, but is it necessary?
    The IRQ1_RSSI flag is almost always set, dependent on RSSIthresshold...
    TODO: Investigate this if this causes problems with multiple transmitters.
    */
    // perhaps also place a timeout on the sending state??
    // Just in case an interrupt is missed.
}

void plainRFM69::sendAddressedVariable(uint8_t address, void* buffer, uint8_t len){
    this->tx_buffer[1] = address; // set address byte.
    this->tx_buffer[0] = len+1; // set length, add one for address byte.
    memcpy(&(this->tx_buffer[2]), buffer, len); // write the payload.
    this->sendPacket(this->tx_buffer, len+2); // send the payload.
}
void plainRFM69::sendVariable(void* buffer, uint8_t len){
    this->tx_buffer[0] = len;
    memcpy(&(this->tx_buffer[1]), buffer, len);
    this->sendPacket(this->tx_buffer, len+1);
}

void plainRFM69::sendAddressed(uint8_t address, void* buffer){
    this->tx_buffer[0] = address;
    memcpy(&(this->tx_buffer[1]), buffer, this->packet_length);
    this->sendPacket(this->tx_buffer, this->packet_length+1);
}

void plainRFM69::send(void* buffer){
    this->sendPacket(buffer, this->packet_length);
}



void plainRFM69::receive(){
    /*
        Setup the automode such that we go into standby mode when a packet is
        available in the FIFO. Automatically go back into receiving mode when it
        is read.

        See the datasheet, p42 for information.
    */

    this->setAutoMode(RFM69_AUTOMODE_ENTER_RISING_PAYLOADREADY, RFM69_AUTOMODE_EXIT_FALLING_FIFONOTEMPTY, RFM69_AUTOMODE_INTERMEDIATEMODE_STANDBY);
    // one disadvantage of this is that the PayloadReady Interrupt is not asserted.
    // however, the intermediate mode can be detected easily.

    // p22 - Turn off the high power boost registers in receiving mode.
    if (this->tx_power_boosted)
    {
        this->setPa13dBm1(false);
        this->setPa13dBm2(false);
    }

    // set the mode to receiver.
    this->setMode(RFM69_MODE_SEQUENCER_ON+RFM69_MODE_RECEIVER);
    this->state = RFM69_PLAIN_STATE_RECEIVING;
}




void plainRFM69::poll(){
    uint8_t flags1;
    // uint8_t flags2;

    flags1 = this->getIRQ1Flags();
    // flags2 = this->getIRQ2Flags();
    

    // debug_rfm("Flags1: "); debug_rfmln(flags1);
    switch (this->state){
        case (RFM69_PLAIN_STATE_RECEIVING):
            if (flags1 & RFM69_IRQ1_AUTOMODE){
                debug_rfmln("Automode in receiving!");
                debug_rfm("Flags1: "); debug_rfmln(flags1);
                debug_rfm("Flags2: "); debug_rfmln(this->getIRQ2Flags());

                this->readPacket();
            }
            break;

        case (RFM69_PLAIN_STATE_SENDING):
            if ((flags1 & RFM69_IRQ1_AUTOMODE)==0){ // no longer in automode
                debug_rfm("Flags1: "); debug_rfmln(flags1);
                debug_rfm("Flags2: "); debug_rfmln(this->getIRQ2Flags());

                this->receive(); // we're done sending, set the receiving mode.
            }
            break;
        default:
            // this should not happen... 
            debug_rfm("In undefined state!");
    };
}


bool plainRFM69::available(){
    // return whether the indices do not align. If they do not align, read 
    // index has to catch up.
    // Overflows are not handled.
    // TODO: handle buffer overflows.
    return (this->buffer_read_index != this->buffer_write_index);
}

uint8_t plainRFM69::read(void* buffer){
    debug_rfm("Read");

    // no data to return.
    if (buffer_read_index == buffer_write_index){
        return 0;
    }

    // read packet length.
    uint8_t length = this->packet_length;
    uint8_t payload_start = 0;
    if (this->use_variable_length){

        // if variable length is used, read length from the first byte.
        length = this->packet_buffer[buffer_read_index][0];
        // debug_rfm("Length from packet: ");debug_rfm(length);
        // prevent buffer overflow, take shortest length of Rx length and packet length.
        length = (length > this->packet_length) ? this->packet_length : length;
        // debug_rfm("maxed length: ");debug_rfm(length);

        // payload starts one byte later.
        payload_start++;
    }

    // copy the message into the buffer.
    memcpy(buffer, &(this->packet_buffer[buffer_read_index][payload_start]), length);
    
    // increase the read index.
    this->buffer_read_index = (this->buffer_read_index+1) % this->buffer_size;

    // return the length of the packet written.
    return length;
}



// Baud rate configurations below.



void plainRFM69::baud4800(){
    // FXO_SC / 0x1a0b = 4799.76 ~= 4800 bps
    this->setBitRate(0x1a0b); 

    // 0x52 * FSTEP = 5004.88 Hz
    this->setFdev(0x52); 

    // RxBwMant=16, RxBwExp=5; 15.62 Khz in FSK
    // RxBwMant=0b00; RxBwExp=0b101; FXO_SC/((RxBwMant*4+16)*2**(RxBwExp+2))= 15625 Hz
    this->setRxBw(0b010, 0b00, 0b101); 
}

void plainRFM69::baud9600(){
    this->setBitRate(0x1a0b/2);  // FXO_SC / 0x1a0b = 9599.52 ~= 9600 bps
    this->setFdev(0x52*2); // 0x52*2 * FSTEP = 10009.7 Hz


    // RxBwMant=16, RxBwExp=5; 15.62 Khz in FSK
    // RxBwMant=0b10; RxBwExp=0b101; FXO_SC/((RxBwMant*4+16)*2**(RxBwExp+2))= 15625 Hz
    this->setRxBw(0b010, 0b00, 0b101); // RxBwMant=24, RxBwExp=4;
}

void plainRFM69::baud153600(){
    // FXO_SC / 0x1a0b = 153592.32 ~= 153600 bps
    this->setBitRate(0x1a0b/32);
    // 0x52*32 * FSTEP = 160156.25 Hz
    this->setFdev(0x52*32);

    
    // RxBwMant=16, RxBwExp=0; 500 Khz in FSK
    // RxBwMant=0b00; RxBwExp=0b00; FXO_SC/((RxBwMant*4+16)*2**(RxBwExp+2))= 500000 Hz
    this->setRxBw(0b010, 0b00, 0); 

    // FDEV + BR/2 <= 500 kHz; 160156.25 + (153592.32/2) = 236952.41
    // \beta = (2*Fdev)/(BitRate)
    // \beta = (2*160156.25) / (153592.32) = 2.085472112147274

    // bitrate is high, use modulation shaping to prevent intersymbol interference.
    this->setDataModul(RFM69_DATAMODUL_PROCESSING_PACKET, RFM69_DATAMODUL_FSK, RFM69_DATAMODUL_SHAPING_GFSK_BT_0_5);
}

void plainRFM69::baud300000(){

    // FXO_SC / 299065.42 ~= 300000 bps
    this->setBitRate(0x006b);
    // 0x52*64 * FSTEP = 320312.5 Hz
    this->setFdev(0x52*64); 

    // RxBwMant=16, RxBwExp=0; 500 Khz in FSK
    // RxBwMant=0b00; RxBwExp=0b00; FXO_SC/((RxBwMant*4+16)*2**(RxBwExp+2))= 500000 Hz
    this->setRxBw(0b010, 0b00, 0); 

    // Cut off frequency DC offset canceller: Fc = (4*RxBw)/(2pi * 2**(DccFreq+2))
    // Default: 0b100, Recommended: 0b010, ~4% of RxBw.
    // In this case: (4*500000)/(2*pi * 2**(0b010+2)) = 19894.36 Hz


    // FDEV + BR/2 <= 500 kHz; 320312.5 + (300000/2) = 470312.5
    // \beta = (2*Fdev)/(BitRate)
    // \beta = (2*160156.25) / (299065.42) = 1.0710449238832094

    // bitrate is very high, use modulation shaping to prevent intersymbol interference.
    this->setDataModul(RFM69_DATAMODUL_PROCESSING_PACKET, RFM69_DATAMODUL_FSK, RFM69_DATAMODUL_SHAPING_GFSK_BT_1_0);

    // modulation index is lower than 2, 
    this->setAfcCtrl(RFM69_AFC_CTRL_IMPROVED);
    this->setContinuousDagc(RFM69_CONTINUOUS_DAGC_IMPROVED_AFCLOWBETAON);

    // The user should ensure that the programmed offset exceeds the
    // DC canceller’s cutoff frequency, set through DccFreqAfc in RegAfcBw.
    // Offset = LowBetaAfcOffset x 488 Hz
    // Dcc_Fc / 488 = 19894.36 / 488 = 40.76 ~= 45
    this->setLowBetaAfcOffset(45);
}


void plainRFM69::emitPreamble(){
    this->setMode(RFM69_MODE_SEQUENCER_OFF | RFM69_MODE_TRANSMITTER);
}



/*
        Protected Methods
*/

void plainRFM69::sendPacket(void* buffer, uint8_t len){
    /*
        Just like with Receive mode, the automode is used.

        First, Rx mode is disabled by going into standby.
        Then the automode is set to start transmitting when FIFO level is above
        the thresshold, it stops transmitting after PacketSent is asserted.

        This results in a minimal Tx time and packetSent can be detected when
        automode is left again.
        
    */
    this->setMode(RFM69_MODE_SEQUENCER_ON | RFM69_MODE_STANDBY);
    this->setAutoMode(RFM69_AUTOMODE_ENTER_RISING_FIFOLEVEL, RFM69_AUTOMODE_EXIT_RISING_PACKETSENT, RFM69_AUTOMODE_INTERMEDIATEMODE_TRANSMITTER);
    // perhaps RFM69_AUTOMODE_ENTER_RISING_FIFONOTEMPTY is faster?
    
    // set it into automode for transmitting

    // p22 - Turn on the high power boost registers in transmitting mode.
    if (this->tx_power_boosted)
    {
        this->setPa13dBm1(true);
        this->setPa13dBm2(true);
    }

    // write the fifo.
    this->state = RFM69_PLAIN_STATE_SENDING; // set the state to sending.
    this->writeFIFO(buffer, len);
}



void plainRFM69::setRawPacketLength(){
    // allocate the Tx Buffer
    this->tx_buffer = (uint8_t*) malloc(this->packet_length + this->use_variable_length);
    // set the length in the hardware.
    this->setPayloadLength(this->packet_length); // packet length byte is not included in length count. So NOT +1

    

}




void plainRFM69::readPacket(){
    // read it into the buffer.
    if (this->use_variable_length) {
        this->readVariableFIFO(this->packet_buffer[this->buffer_write_index], this->packet_length + this->use_variable_length);
    } else{
        this->readFIFO(this->packet_buffer[this->buffer_write_index], this->packet_length);
    }

    // increase the write index.
    this->buffer_write_index = (this->buffer_write_index+1) % this->buffer_size;
}
