/*
 *  Copyright (c) 2014, Ivor Wanders
 *  MIT License, see the LICENSE.md file in the root folder.
*/


#include <SPI.h>
#include <Arduino.h>

#ifndef BARE_RFM69_H
#define BARE_RFM69_H

#include <bareRFM69_const.h>

/*
    The bareRFM69 object only provides convenient methods to interact with the
    hardware. It does not do anything by itself.

    The order of methods is identical to the order in which the registers are
    described in page 61 to page 74.

    Tested on the RFM69CW from HopeRF:
        http://www.hoperf.com/rf/fsk_module/RFM69CW.htm
*/



#if !defined(SPI_HAS_TRANSACTION)
#error "You need to have support for SPI transactions in the SPI library."
#error "https://github.com/PaulStoffregen/SPI/blob/master/SPI.h"
#error "https://github.com/PaulStoffregen/SPI/blob/master/SPI.cpp"
#endif

class bareRFM69 {
    private:
        uint8_t cs_pin; // chip select pin.


        // SPI relevant stuff
        void writeRegister(uint8_t reg, uint8_t data);
        void writeMultiple(uint8_t reg, void* data, uint8_t len);

        uint8_t readRegister(uint8_t reg);
        uint16_t readRegister16(uint8_t reg);
        uint32_t readRegister24(uint8_t reg);
        uint32_t readRegister32(uint8_t reg);
        void readMultiple(uint8_t reg, void* data, uint8_t len);

        void inline chipSelect(bool enable);

    public:
        bareRFM69(uint8_t cs_pin){
            this->cs_pin = cs_pin;
            pinMode(this->cs_pin, OUTPUT);
            pinMode(this->cs_pin, HIGH);

            // Max 10 MHz clock, MSB first, CPOL= 0 and CPHA = 0
            // according to the datasheet.
            // SPISettings(10000000, MSBFIRST, SPI_MODE0)
            // is to be used.
        };

        uint8_t readRawRegister(uint8_t reg){return this->readRegister(reg);}
        // used for debugging.


        //#####################################################################
        // Generic stuff
        //#####################################################################
        
        /*
            Constants in the documentation:

            FXO_SC = 32e6 # 32 MHz for all frequencies.
            FSTEP = FXO_SC/(2**19) # = 61.03515625 Hz; Frequency synthesizer
                    step FSTEP

        */

        static void reset(uint8_t reset_pin);
        // sends a hard reset using the reset pin on the RFM69.
        // does not require an instance, use bareRFM69::reset(pin);


        //#####################################################################
        // FiFo
        //#####################################################################
        void writeFIFO(void* buffer, uint8_t len);
        //Write from buffer to FIFO for 'len' bytes.
        void readFIFO(void* buffer, uint8_t len);
        //Read from buffer to FIFO for 'len' bytes.

        uint8_t readVariableFIFO(void* buffer, uint8_t max_length);
        // Reads up to 'len' bytes, len is determined by first byte in the FIFO
        // this byte is also placed in the buffer. The max_length argument can
        // be used to limit the number of bytes.

        
        //#####################################################################
        // Operating stuff
        //#####################################################################

        /*
            On the Automatic sequencer, p35, section 4.2:

                TX start procedure:
                In Packet mode, the RFM69CW will automatically modulate the RF
                signal with preamble bytes as soon as TxReady or ModeReady
                happen. The actual packet transmission (starting with the number
                of preambles specified in PreambleSize) will start when the
                TxStartCondition is fulfilled

                RX start procedure:
                In Packet mode, the receiver will start locking its Bit
                Synchronizer on a minimum or 12 bits of received preamble (see
                section 3.4.13 for details), before the reception of correct
                Data, or Sync Word (if enabled) can occur.


        */

        void setMode(uint8_t mode){this->writeRegister(RFM69_OPMODE, mode);};
        /*
            Sets the mode of the radio module.
            Mode is a sum of the following categories:
                Sequencer:
                        RFM69_MODE_SEQUENCER_OFF
                        RFM69_MODE_SEQUENCER_ON
                    Allows the automatic mode sequencer to be disabled.
                    With OFF, a mode can be forced.

                Listen:
                        RFM69_MODE_LISTEN_ON
                        RFM69_MODE_LISTEN_OFF
                    Allows enabling or disable the listen mode.
                Listen Abort:
                        RFM69_MODE_LISTEN_ABORT
                    Aborts listen mode when set with RFM69_MODE_LISTEN_OFF.
                Mode:                   
                        RFM69_MODE_SLEEP
                        RFM69_MODE_STANDBY
                        RFM69_MODE_FREQ_SYNTH
                        RFM69_MODE_TRANSMITTER
                        RFM69_MODE_RECEIVER
        */
        uint8_t getMode(){return this->readRegister(RFM69_OPMODE);};


        //#####################################################################
        // RF frequency
        //#####################################################################
        

        // RF carrier frequency
        void setFrf(uint32_t Frf){
            this->writeMultiple(RFM69_FRF_MSB, &Frf, 3);};
        /*
            Frf = FSTEP * Frf(23,0)
            Defaults to 0xc0e452; 915 MHz

                Ranges In MHz   Min     Typ     Max
                315MHz Module   290             340
                433MHz Module   424             510
                868MHz Module   862             890
                915MHz Module   890            1020

            Example values:
                434 MHz; 434e6/FSTEP = 7110656 = 0x6c8000 (measured; 433.98 MHz)
                500 MHz; 500e6/FSTEP = 8192000 = 0x7d0000 (outside of ISM)
                450 MHz; 450e6/FSTEP = 7372800 = 0x708000 (outside of ISM)
        */


        // Frequency Deviation
        void setFdev(uint16_t Fdev){
            this->writeMultiple(RFM69_FDEV_MSB, &Fdev, 2);};
        /*
            Fdev = FSTEP * Fdev(15,0) ; in Hz.
            Defaults to 0x52; 5000 Hz

            In FSK Requirement:
                FDEV + BR/2 <= 500 kHz
                ->
                BR <= 2*(500 kHz - Fdev)

            For demodulator, most efficient when modulation index greater than
            0.5 and below 10:
                0.5 <= \beta = (2*Fdev)/(BitRate) <= 10
        */


        void startRCCalibration(){this->writeRegister(RFM69_OSC1, 1<<7);}
        /*  Triggers the calibration of the RC oscillator when set.
            Always reads 0. RC calibration must be triggered in Standby mode.

            See section 4.3.5 of the RFM69CW datasheet. The timing of the Listen
            Mode depends on the internal low-power RC oscillator.

            This oscillator is automatically calibrated but needs recalibration
            when used over large temperature variations.            
        */

        bool completedRCCalibration(){return this->readRegister(RFM69_OSC1) & (1<<6);}
        /* Reads RC calibration finished flag */
        // takes about 125 usec between start and completion.


        //#####################################################################
        // data modulation parameters
        //#####################################################################
        

        // Bit rate
        void setBitRate(uint16_t BitRate){
            this->writeMultiple(RFM69_BITRATE_MSB, &BitRate, 2);};
        /*
            BitRate = FXO_SC/BitRate(15,0) ; in kbit/s.
            Defaults to 0x1a0b; 4.8 kb/s

            In FSK Requirement:
                FDEV + BR/2 <= 500 kHz
                ->
                BR <= 2*(500 kHz - Fdev)

            For demodulator, most efficient when modulation index greater than
            0.5 and below 10:
                0.5 <= \beta = (2*Fdev)/(BitRate) <= 10

            Note: to respect oversampling rules in the decimation chain of the
                  receiver, the Bit Rate cannot be set at a higher value 
                  than 2 times the single-side receiver bandwidth (BitRate < 2
                  x RxBw) page 26

        */


        // data modulation
        void setDataModul(uint8_t processing, bool use_ook, uint8_t modulationshaping){
            this->writeRegister(RFM69_DATA_MODUL, (processing << 6) + (use_ook << 3));};
        /*
            processing is either:
                RFM69_DATAMODUL_PROCESSING_PACKET
                    Packet mode (recommended): user only provides/retrieves payload bytes to/from the FIFO.
                    The packet is automatically built with preamble, Sync word, and optional AES, CRC, and DC-free encoding schemes 

                RFM69_DATAMODUL_PROCESSING_CONT_SYNCRHONISER
                RFM69_DATAMODUL_PROCESSING_CONT
                    Continuous mode: each bit transmitted or received is accessed in real time at the DIO2/DATA pin

            use_ook is either:
                RFM69_DATAMODUL_FSK
                    Use frequency shift keying implicitly recommended via default type.
                RFM69_DATAMODUL_OOK
                    Use On-Off Keying

            modulationshaping is either:
                RFM69_DATAMODUL_SHAPING_GFSK_NONE
                    No shaping
                RFM69_DATAMODUL_SHAPING_GFSK_BT_1_0
                    Gaussian filter, BT = 1.0
                RFM69_DATAMODUL_SHAPING_GFSK_BT_0_5
                    Gaussian filter, BT = 0.5
                RFM69_DATAMODUL_SHAPING_GFSK_BT_0_3
                    Gaussian filter, BT = 0.3

                RFM69_DATAMODUL_SHAPING_OOK_NONE
                    No Shaping
                RFM69_DATAMODUL_SHAPING_OOK_FCUTOFF_BR
                    Filtering with fcutoff=BR
                RFM69_DATAMODUL_SHAPING_OOK_FCUTOFF_2BR
                    Filtering with fcutoff=2*BR

            At higher bitrates it is advisable to use modulation shaping.
             
        */

        //#####################################################################
        // Listen Mode
        //##################################################################### 
        /*
            Listen mode:
                The circuit can be set to Listen mode, by setting ListenOn in
                RegOpMode to 1 while in Standby mode. In this mode, RFM69CW
                spends most of the time in Idle mode, during which only the RC
                oscillator runs. Periodically the receiver is woken up and
                listens for an RF signal. If a wanted signal is detected, the
                receiver is kept on and the data is demodulated. Otherwise, if
                a wanted signal hasn't been detected after a pre-defined period
                of time, the receiver is disabled until the next time period.

            Time interval given by:
                t ListenX = ListenCoefX ∗ Listen Re solX

                Where X is either 'Idle' or 'Rx'

        */
        

        void setListenConfig(uint8_t ListenResolIdle, uint8_t ListenResolRx, uint8_t ListenCriteria, uint8_t ListenEnd){
            this->writeRegister(RFM69_LISTEN1, ListenResolIdle+ListenResolRx+ListenCriteria+ListenEnd);};
        /*
            ListenResolIdle one of:
                    RFM69_LISTEN_RESOL_IDLE_64US
                    RFM69_LISTEN_RESOL_IDLE_4_1MS
                    RFM69_LISTEN_RESOL_IDLE_262MS
                Sets the listen mode idle time to 64 usec, 4.1ms or 262 ms
            ListenResolRx one of:
                    RFM69_LISTEN_RESOL_RX_64US
                    RFM69_LISTEN_RESOL_RX_4_1MS
                    RFM69_LISTEN_RESOL_RX_262MS
                Sets the listen mode Rx time to 64 usec, 4.1ms or 262 ms
            ListenCriteria
                Criteria for packet acceptance in listen mode, either:
                    RFM69_LISTEN_CRITERIA_RSSI
                        signal strength above RssiThreshold
                    RFM69_LISTEN_CRITERIA_RSSI_SYNC
                        Signal strength above RssiThresshold and SyncAddress matched.

            ListenEnd:
                Action taken after acceptance of a packet in Listen mode:
                    RFM69_LISTEN_END_STAY_RX_LISTEN_STOP
                        Chip stays in Rx mode, listen mode stops and must be disabled.
                    RFM69_LISTEN_END_RX_UNTIL_LISTEN_STOP
                        Chip stays in Rx mode until PayloadReady or Timeout interrupt
                        occurs. Listen mode stops and must be disabled.
                    RFM69_LISTEN_END_RX_UNTIL_LISTEN_RESUME
                        Chip stays in Rx mode until PayloadReady or Timeout interrupt
                        occurs. Listen mode then resumes in Idle State.
                        FIFO lost at next RX wakeup.

            Default: (RFM69_LISTEN_RESOL_IDLE_4_1MS, RFM69_LISTEN_RESOL_RX_64US,
                RFM69_LISTEN_CRITERIA_RSSI, RFM69_LISTEN_END_RX_UNTIL_LISTEN_STOP)
        */

        void setListenCoefIdle(uint8_t coeff){this->writeRegister(RFM69_LISTEN2, coeff);};
        /*
            Duration of the Idle phase in Listen mode.
                t ListenIdle = ListenCoefIdle ∗ ListenResolIdle

            Default 0xf5; 245
        */
        void setListenCoefRx(uint8_t coeff){this->writeRegister(RFM69_LISTEN3, coeff);};
        /*
            Duration of the Rx phase in Listen mode.
                t ListenRx = ListenCoefRx ∗ ListenResolRx

            Default 0x20; 32
        */

        //#####################################################################
        // Version
        //#####################################################################
        
        // version of chip
        uint8_t getVersion(){return readRegister(RFM69_VERSION);}



        //#####################################################################
        // Transmitter registers
        //#####################################################################
        
        void setPALevel(uint8_t amplifiers, uint8_t OutputPower){
            this->writeRegister(RFM69_PA_LEVEL, amplifiers + (OutputPower % (1<<5)));};
        /*
            Amplifiers is one of or sum of:
                RFM69_PA_LEVEL_PA0_ON
                RFM69_PA_LEVEL_PA1_ON
                RFM69_PA_LEVEL_PA2_ON
            Output power setting, with 1 dB steps, so in [0,31]
            Pout = -18 + OutputPower [dBm] , with PA0
            Only the16 upper values of OutputPower are accessible according to
            datasheet, measurements indicate 0-31 is usable.


            p21: A low power mode, where -2dBm < Pout < 13dBm, with PA1 enabled
            Default is 0b10011111; RFM69_PA_LEVEL_PA0_ON + 13dBm gain

            Truth table on page 21 only lists PA0 ON with effect...
            Possibly only RFM69_PA_LEVEL_PA0_ON has effect.
        */

        void setPARamp(uint8_t ramp){this->writeRegister(RFM69_PA_RAMP, ramp);};
        /*
            Set Rise/Fall time of ramp up/down in FSK
            Valid fields are:
                RFM69_PA_RAMP_#US
            Where # is one of {3400, 2000, 1000, 500, 250, 125, 100, 62, 50,
                                                40, 31, 25, 20, 15, 12, 10}
            So from 3.4 ms, down to 10 us.
            Defaults to 0b1001; 40us
        */

        void setOCP(uint8_t ocplimit){
            this->writeRegister(RFM69_OCP, ((ocplimit != 0)<<7) + (((ocplimit-45)/5)&0b1111));};
        /*
            It helps preventing surge currents required when the transmitter is
            used at its highest power levels, thus protecting the battery that
            may power the application.
            
            Ocplimit is either 0, current limit is disabled then.
                Imax = 45 + 5 ⋅*OcpTrim
                Ocptrim = (Imax - 45)/5
            Or it is one of [45, 50, ..., 115, 120]
            Defaults to 0b1010; 95 mA, enabled.
        */
        
        //#####################################################################
        // Receiver Registers
        //#####################################################################

        void setLNA(bool LnaZin, uint8_t LnaGainSelect){
            this->writeRegister(RFM69_LNA, LnaZin+LnaGainSelect);};
        /*  
            LnaZin:
                Impedance of the LNA:
                    RFM69_LNA_IMP_50OHM:
                        50 ohm (default)
                    RFM69_LNA_IMP_200OHM:
                        200 ohm (recommended)
                LNA Gain:
                    RFM69_LNA_GAIN_AGC_LOOP:
                        gain set by the internal AGC loop (default)
                    RFM69_LNA_GAIN_#DB:
                        Highest gain - # dB, with # from {0, 6, 12, 24, 36, 48}
        */

        uint8_t getLNA(){return ((this->readRegister(RFM69_LNA)>>3)&0b111);}
        /*
            Returns set LNA, as RFM69_LNA_GAIN_#DB 
        */

        void setRxBw(uint8_t DccFreq, uint8_t RxBwMant, uint8_t RxBwExp){
            this->writeRegister(RFM69_RX_BW, (DccFreq<<5) + (RxBwMant << 3) + (RxBwExp ));};
        /*
            DccFreq:
                Cut-off frequency of the DC offset canceller (DCC):
                Fc = (4*RxBw)/(2pi * 2**(DccFreq+2))
                Default: 0b100, Recommended: 0b010, ~4% of RxBw.
            RxBwMant:
                Channel filter bandwith control
                0b00: RxBwMant = 16 (default)
                0b01: RxBwMant = 20
                0b10: RxBwMant = 24 (recommended)
            RxBwExp (3 bits):
                Channel filter bandwith control:
                FSK Mode:
                    RxBw = FXO_SC/((RxBwMant*4+16)*2**(RxBwExp+2))
                OOK Mode:
                    RxBw = FXO_SC/((RxBwMant*4+16)*2**(RxBwExp+3))
                See Table 14 for tabulated values.
                Default:0b110, recommended:0b101

            Recommended: RxBwMant=24, RxBwExp=5; 10.4 Khz in FSK, /2 for OOK.

            Note: to respect oversampling rules in the decimation chain of the
                  receiver, the Bit Rate cannot be set at a higher value 
                  than 2 times the single-side receiver bandwidth (BitRate < 2
                  x RxBw) page 26

        */

        void setAfcBw(uint8_t DccFreqAfc, uint8_t RxBwMantAfc, uint8_t RxBwExpAfc){
                this->writeRegister(RFM69_AFC_BW, (DccFreqAfc <<5)+(RxBwMantAfc << 3) + (RxBwExpAfc ));}
        /*
            DccFreqAfc:
                DccFreq parameter used during the AFC
                Default: 0b100
            RxBwMantAfc:
                RxBwMant parameter used during the AFC
                Default: 0b01
            RxBwExpAfc:
                RxBwExp parameter used during the AFC
                Default 0b010, Recommended 0b011
        */


        void setAfcCtrl(bool improved_AFC){this->writeRegister(RFM69_AFC_CTRL,  improved_AFC<<5);}
        /*
            Improved AFC routine for signals with modulation index
            lower than 2. Refer to section 3.4.16 for details
                RFM69_AFC_CTRL_STANDARD
                    Standard AFC routine (default)
                RFM69_AFC_CTRL_IMPROVED
                    Improved AFC routine
        */


        void startRssi(){
            this->writeRegister(RFM69_RSSI_CONFIG, 1);};
        bool completedRssi(){
            return this->readRegister(RFM69_RSSI_CONFIG) & 0b10;};
        uint8_t getRssiValue(){
            return this->readRegister(RFM69_RSSI_VALUE);};
        /*
            Initiated RSSI measurement, check wheter it is completed and get it's value.
            Absolute value of the RSSI in dBm, 0.5dB steps.
                RSSI = -RssiValue/2 [dBm]
        */

        //#####################################################################
        // Pin IO and IRQ
        //#####################################################################

        void setDioMapping1(uint8_t mappings){
            this->writeRegister(RFM69_DIO_MAPPING1, mappings);}
        /*
            Sets Digital input/output mappings.

            OR-ed statements of the following:

            For Dio0Mapping:
                in Rx mode:
                    RFM69_PACKET_DIO_0_RX_CRC_OK (default)
                    RFM69_PACKET_DIO_0_RX_PAYLOAD_READY
                    RFM69_PACKET_DIO_0_RX_SYNC_ADDRESS
                    RFM69_PACKET_DIO_0_RX_RSII

                in Tx Mode:
                    RFM69_PACKET_DIO_0_TX_PACKET_SENT (default)
                    RFM69_PACKET_DIO_0_TX_TX_READY
                    RFM69_PACKET_DIO_0_TX_PLL

            For Dio1Mapping:
                RFM69_PACKET_DIO_1_FIFO_LEVEL (default)
                RFM69_PACKET_DIO_1_FIFO_FULL
                RFM69_PACKET_DIO_1_FIFO_NOT_EMPTY

            For Dio2Mapping:
                RFM69_PACKET_DIO_2_FIFO_NOT_EMPTY (default)
                RFM69_PACKET_DIO_2_AUTOMODE

            For Dio3Mapping:
                RFM69_PACKET_DIO_3_FIFO_FULL (default)

        */



        uint8_t getIRQ1Flags(){
            return this->readRegister(RFM69_IRQ_FLAGS1);};
        /*
            Returns IRQ1 flags, bitmask, the following bits can be set:

            RFM69_IRQ1_MODEREADY
                Set when the operation mode requested in Mode, is ready 
                    - Sleep: Entering Sleep mode
                    - Standby: XO is running
                    - FS: PLL is locked
                    - Rx: RSSI sampling starts
                    - Tx: PA ramp-up completed
                Cleared when changing operating mode.
            RFM69_IRQ1_RXREADY
                Set in Rx mode, after RSSI, AGC and AFC. Cleared when leaving
                Rx.
            RFM69_IRQ1_TXREADY
                Set in Tx mode, after PA ramp-up. Cleared when leaving Tx.
            RFM69_IRQ1_PLLLOCK
                Set (in FS, Rx or Tx) when the PLL is locked.
                Cleared when it is not.
            RFM69_IRQ1_RSSI
                Set in Rx when the RssiValue exceeds RssiThreshold.
                Cleared when leaving Rx.
            RFM69_IRQ1_TIMEOUT
                Set when a timeout occurs (see TimeoutRxStart and
                TimeoutRssiThresh) Cleared when leaving Rx or FIFO is emptied.
            RFM69_IRQ1_AUTOMODE
                Set when entering Intermediate mode. Cleared when exiting
                Intermediate mode. Please note that in Sleep mode a small delay
                can be observed between AutoMode interrupt and the corresponding
                enter/exit condition.
            RFM69_IRQ1_SYNCADDRESSMATCH
                Set when Sync and Address (if enabled) are detected. Cleared
                when leaving Rx or FIFO is emptied.This bit is read only in
                Packet mode, rwc in Continuous mode.

        */
        uint8_t getIRQ2Flags(){return this->readRegister(RFM69_IRQ_FLAGS2);};
        /*
            Returns IRQ2 flags, bitmask, the following bits can be set:

            RFM69_IRQ2_FIFOFULL
                Set when FIFO is full (i.e. contains 66 bytes), else cleared.
            RFM69_IRQ2_FIFONOTEMPTY
                Set when FIFO contains at least one byte, else cleared
            RFM69_IRQ2_FIFOLEVEL
                Set when the number of bytes in the FIFO strictly exceeds
                FifoThreshold, else cleared. Flag(s) and FIFO are cleared when
                this bit is set. The
            RFM69_IRQ2_FIFOOVERRUN
                Set when FIFO overrun occurs. (except in Sleep mode) FIFO then
                becomes immediately available for the next transmission / 
                reception.
            RFM69_IRQ2_PACKETSENT
                Set in Tx when the complete packet has been sent.
                Cleared when exiting Tx.
            RFM69_IRQ2_PAYLOADREADY
                Set in Rx when the payload is ready (i.e. last byte received and
                CRC, if enabled and CrcAutoClearOff is cleared, is Ok).
                Cleared when FIFO is empty.
            RFM69_IRQ2_CRCOK
                Set in Rx when the CRC of the payload is Ok.
                Cleared when FIFO is empty.

        */

        void setRSSIThreshold(uint8_t level){
            this->writeRegister(RFM69_RSSI_THRESH, level);};
        /*
            RSSI trigger level for Rssi interrupt :
                triggers at: -RssiThreshold / 2 [dBm]
            Default 0xFF.
            Recommended 0xE4; -0xE4/2 =  -114 dBm
        */
        void setTimeoutRxStart(uint8_t duration){
            this->writeRegister(RFM69_RX_TIMEOUT1, duration);};
        /*
            Timeout interrupt is generated TimeoutRxStart*16*Tbit
            after switching to Rx mode if Rssi interrupt doesn’t occur
            (i.e. RssiValue > RssiThreshold)
            0x00: TimeoutRxStart is disabled
            Defaults: 0x00; disabled
        */

        void TimeoutRssiThresh(uint8_t duration){
            this->writeRegister(RFM69_RX_TIMEOUT2, duration);};
        /*
            Timeout interrupt is generated TimeoutRssiThresh*16*Tbit
            after Rssi interrupt if PayloadReady interrupt doesn’t
            occur.
            0x00: TimeoutRssiThresh is disabled
            Defaults: 0x00; disabled
        */

        //#####################################################################
        // Packet engine registers
        //#####################################################################
        void setPreambleSize(uint16_t size){
            this->writeMultiple(RFM69_PREAMBLE_MSB, &size, 2);};
        /*
            Size of the preamble to be sent (from TxStartCondition fulfilled). 
            Number of 0b10101010 (0xAA) bytes to be added in front of transmission
            Defaults to 0x03.
        */

        void setSyncConfig(bool syncOn, bool FiFoFillCondition, uint8_t SyncSize, uint8_t SyncTol){
            this->writeRegister(RFM69_SYNC_CONFIG, (syncOn<<7) + (FiFoFillCondition << 6) + (((SyncSize-1)&0b111)<<3) + (SyncTol&0b111));};
        /*
            Sets the sync config register.
                syncOn (default 1):
                    Enabled the sync word generation and detection.
                FiFoFillCondition (default 0):
                    FIFO filling condition
                    false: if syncaddress interrupt occurs
                    true:  as long as FiFoFillCondition is set
                SyncSize (default 0b011):
                    Size of the Sync Word bytes.
                SyncTol (default 0b000):
                    Number of tolerated bit errors in Sync word.
                
        */

        void setSyncValue(void* buffer, uint8_t len){
            this->writeMultiple(RFM69_SYNC_VALUE1, buffer, len);};
        /*
            1st byte of Sync word. (MSB byte)
                Used if SyncOn is set.
            Up to len=8 can be written.
        */

        void setPacketConfig1(uint8_t modifiers){
            this->writeRegister(RFM69_PACKET_CONFIG1, modifiers);};
        /*
            Set PACKET configuration parameters. 
            
            Should be a sum of the following:
                Packet length (implicit: RFM69_PACKET_CONFIG_LENGTH_FIXED):
                    Can be set to variable length with:RFM69_PACKET_CONFIG_LENGTH_VARIABLE
                    Called PacketFormat in datasheet.

                DC Free methods (implicit: RFM69_PACKET_CONFIG_DC_FREE_NONE):
                    Either
                        Manchester encoding: RFM69_PACKET_CONFIG_DC_FREE_MANCHESTER
                    or
                        Whitening filter: RFM69_PACKET_CONFIG_DC_FREE_WHITENING

                CRC checksum calculation (implicit: RFM69_PACKET_CONFIG_CRC_OFF)
                    Can be enabled with RFM69_PACKET_CONFIG_CRC_ON

                CRC failure options (implicit RFM69_PACKET_CONFIG_CRC_FAIL_DISCARD)
                    Normally discard packets with failed CRC
                    RFM69_PACKET_CONFIG_CRC_FAIL_KEEP can be used to keep failed CRC packets.

                Address based filtering (implicit: RFM69_PACKET_CONFIG_ADDRESS_FILTER_NONE)
                    Either:
                        Accept on node address with: RFM69_PACKET_CONFIG_ADDRESS_FILTER_NODE
                    or
                        Accept on node address or broadast address: RFM69_PACKET_CONFIG_ADDRESS_FILTER_NODE_BROADCAST

            Defaults to RFM69_PACKET_CONFIG_CRC_ON.
        */

        void setPacketConfig2(uint8_t InterPacketRxDelay, bool RestartRx, bool AutoRxRestartOn, bool AesOn){
            this->writeRegister(RFM69_PACKET_CONFIG2, ((InterPacketRxDelay&0b1111)<<4) + (RestartRx<<2) + (AutoRxRestartOn<<1)+AesOn);};
       
        /*
            Defaults to: AutoRxRestartOn = true, rest 0b0.
            InterPacketRxDelay:
                After PayloadReady occured, defines the delay between FIFO empty and the start of a new RSSI phase for next packet. Must match the transmitter's PA ramp-down Time.
                Tdelay = 0 if InterPacketRxDelay >= 12
                Tdelay = (2^(interPackerRxDelay)) / BitRate otherwise
            RestartRx:
                Forces the Receiver in WAIT mode, in Continuous Rx mode.
            AutoRestartRxOn:
                Enables automatic Rx restart (RSSI phase) after PayloadReady occurred
                    and packet has been completely read from FIFO:
                False:RestartRx can be used.
                True: Rx automatically restarted after InterPacketRxDelay.
            AesOn:
                Enables AES encryption/decryption, true to enable, limits payload to 66 bytes.

                
        */

        void setPayloadLength(uint8_t length){this->writeRegister(RFM69_PAYLOAD_LENGTH, length);};
        /*
            Sets the length of the payload, slightly dependent on PacketFormat
                If PacketFormat = 0 (fixed), sets payload length.
                If PacketFormat = 1 (variable), max length in Rx, not used in Tx.
            Defaults to 0x40=64
        */

        void setNodeAddress(uint8_t address){this->writeRegister(RFM69_NODE_ADRESS, address);};
        /*
            Node address used in address filtering.
            Defaults to 0x00.
        */

        void setBroadcastAddress(uint8_t address){this->writeRegister(RFM69_BROADCAST_ADRESS, address);};
        /*
            Broadcast address used in address filtering.
            Defaults to 0x00.
        */


        void setFifoThreshold(bool TxStartCondition, uint8_t FifoThreshold){
            this->writeRegister(RFM69_FIFO_THRESH, TxStartCondition + (FifoThreshold % 128));};
        /*
            Start transmission on a certain FIFO situation:
            TxStartCondition (implicit: RFM69_THRESHOLD_CONDITION_FIFOLEVEL)
                Can be set to start when atleast 1 byte is available by RFM69_THRESHOLD_CONDITION_NOT_EMPTY
            FifoThreshold (defaults 0b1111), number of bytes in fifo on which to start transmission if RFM69_THRESHOLD_CONDITION_FIFOLEVEL
        */

        void setAutoMode(uint8_t enter, uint8_t exit, uint8_t intermediate_mode){
            this->writeRegister(RFM69_AUTO_MODES, enter+exit+intermediate_mode);};
        /*
            The enter and exit conditions cannot be used independently of each
            other i.e. both should be enabled at the same time.

            The initial and the final state is the one configured in Mode in RegOpMode.
            The initial & final states can be different by configuring the modes
            register while the module is in intermediate mode.
            The pictorial description of the auto modes is shown below.

            Initial state defined       Intermediate State  Final state defined
            By Mode in RegOpMode       -------------------- By Mode in RegOpMode
                                       |                  |
                                       |                  |
            ----------------------------                  ----------
                        EnterCondition ^    ExitCondition ^

            Where Enter condition is one of:
                RFM69_AUTOMODE_ENTER_NONE_AUTOMODES_OFF
                RFM69_AUTOMODE_ENTER_RISING_FIFONOTEMPTY
                RFM69_AUTOMODE_ENTER_RISING_FIFOLEVEL
                RFM69_AUTOMODE_ENTER_RISING_CRCOK
                RFM69_AUTOMODE_ENTER_RISING_PAYLOADREADY
                RFM69_AUTOMODE_ENTER_RISING_SYNCADDRESS
                RFM69_AUTOMODE_ENTER_RISING_PACKETSENT
                RFM69_AUTOMODE_ENTER_FALLING_FIFONOTEMPTY (I.E. FIFOEMPTY)

            Where Exit condition is one of:
                RFM69_AUTOMODE_EXIT_NONE_AUTOMODES_OFF
                RFM69_AUTOMODE_EXIT_FALLING_FIFONOTEMPTY (I.E._FIFOEMPTY)
                RFM69_AUTOMODE_EXIT_RISING_FIFOLEVEL_OR_TIMEOUT
                RFM69_AUTOMODE_EXIT_RISING_CRCOK_OR_TIMEOUT
                RFM69_AUTOMODE_EXIT_RISING_PAYLOADREADY_OR_TIMEOUT
                RFM69_AUTOMODE_EXIT_RISING_SYNCADDRESS_OR_TIMEOUT
                RFM69_AUTOMODE_EXIT_RISING_PACKETSENT
                RFM69_AUTOMODE_EXIT_RISING_TIMEOUT

            And Intermediate state:
                RFM69_AUTOMODE_INTERMEDIATEMODE_SLEEP
                RFM69_AUTOMODE_INTERMEDIATEMODE_STANDBY
                RFM69_AUTOMODE_INTERMEDIATEMODE_RECEIVER
                RFM69_AUTOMODE_INTERMEDIATEMODE_TRANSMITTER

        */


        void setAesKey(void* buffer, uint8_t len){this->writeMultiple(RFM69_AES_KEY1, buffer, len);};
        /*
            Takes 16 bytes as AES-128 key.
            Cipher mode is ECB, that means that the 16 byte blocks are treated
            independently of each other.

            Defaults to [0, ..., 0], left zeros if not overwritten.

        */

        //#####################################################################
        // Temperature register.
        //#####################################################################

        void startTempMeasure(){this->writeRegister(RFM69_TEMP1, 1<<3);};
        /*
            Triggers temperature measurement.
        */
        bool completedTempMeasure(){return !(this->readRegister(RFM69_TEMP1) & (1<<2));};
        /*
            The receiver can not be used while measuring temperature.

        */
        uint8_t getTempValue(){return this->readRegister(RFM69_TEMP2);};
        /*
            Returns Temperature value as -1 deg C, needs calibration.
        */


        //#####################################################################
        // Test registers.
        //#####################################################################

        void setSensitivityBoost(bool enable){this->writeRegister(RFM69_TEST_LNA, (enable) ? 0x2D : 0x1B);};
        /*
            High sensitivity or normal sensitivity mode:
                0x1B: Normal mode (default)
                0x2D: High sensitivity mode

        */

        void setPa13dBm1(bool enable){this->writeRegister(RFM69_TEST_PA1, (enable) ? 0x5D : 0x55);};
        /*
            Set to 0x5D for +13 dBm operation
                0x55: Normal mode and Rx mode (default)
                0x5D: +13 dBm mode
                Revert to 0x55 when receiving or using PA0
        */

        void setPa13dBm2(bool enable){this->writeRegister(RFM69_TEST_PA2, (enable) ? 0x7C : 0x70);};
        /*
            Set to 0x7C for +13 dBm operation
                0x70: Normal mode and Rx mode (default)
                0x7C: +13 dBm mode
                Revert to 0x70 when receiving or using PA0
        */

        void setContinuousDagc(uint8_t fading_margin){this->writeRegister(RFM69_TEST_DAGC, fading_margin);};
        /*
            Fading Margin Improvement, refer to 3.4.4
                RFM69_CONTINUOUS_DAGC_NORMAL (default):
                    Normal mode
                RFM69_CONTINUOUS_DAGC_IMPROVED_AFCLOWBETAON:
                    Improved margin, use if AfcLowBetaOn=1
                RFM69_CONTINUOUS_DAGC_IMPROVED_AFCLOWBETAOFF (recommended)
                    Improved margin, use if AfcLowBetaOn=0
        */

        void setLowBetaAfcOffset(uint8_t LowBetaAfcOffset){this->writeRegister(RFM69_TEST_AFC, LowBetaAfcOffset);};
        /*
            AFC offset set for low modulation index systems, used if 
            AfcLowBetaOn=1.
                Offset = LowBetaAfcOffset x 488 Hz
            Default = 0.
            The user should ensure that the programmed offset exceeds the
            DC canceller’s cutoff frequency, set through DccFreqAfc in RegAfcBw.

        */


};







//BARE_RFM69_H
#endif