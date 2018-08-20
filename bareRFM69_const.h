/**
 *
 *  bareRFM69_const.h - An RFM69-series radio library created by Ivor Wanders (constants).
 *
 *  Copyright (c) 2014, Ivor Wanders
 *  Copyright (c) 2018, Charles Taylor
 *
 *  MIT License - See the LICENSE.md file for details.
 *
 **/

 
#define RFM69_FIFO                                         0x00
#define RFM69_OPMODE                                       0x01
#define RFM69_DATA_MODUL                                   0x02

#define RFM69_BITRATE_MSB                                  0x03
#define RFM69_BITRATE_LSB                                  0x04

#define RFM69_FDEV_MSB                                     0x05
#define RFM69_FDEV_LSB                                     0x06

#define RFM69_FRF_MSB                                      0x07
#define RFM69_FRF_MID                                      0x08
#define RFM69_FRF_LSB                                      0x09

#define RFM69_OSC1                                         0x0A
#define RFM69_AFC_CTRL                                     0x0B

#define RFM69_LISTEN1                                      0x0D
#define RFM69_LISTEN2                                      0x0E
#define RFM69_LISTEN3                                      0x0F

#define RFM69_VERSION                                      0x10

#define RFM69_PA_LEVEL                                     0x11
#define RFM69_PA_RAMP                                      0x12

#define RFM69_OCP                                          0x13

#define RFM69_LNA                                          0x18
#define RFM69_RX_BW                                        0x19
#define RFM69_AFC_BW                                       0x1A

#define RFM69_OOK_PEAK                                     0x1B
#define RFM69_OOK_AVG                                      0x1C
#define RFM69_OOK_FIX                                      0x1D

#define RFM69_AFC_FEI                                      0x1E
#define RFM69_AFC_MSB                                      0x1F
#define RFM69_AFC_LSB                                      0x20

#define RFM69_FEI_MSB                                      0x21
#define RFM69_FEI_LSB                                      0x22

#define RFM69_RSSI_CONFIG                                  0x23
#define RFM69_RSSI_VALUE                                   0x24

#define RFM69_DIO_MAPPING1                                 0x25
#define RFM69_DIO_MAPPING2                                 0x26

#define RFM69_IRQ_FLAGS1                                   0x27
#define RFM69_IRQ_FLAGS2                                   0x28

#define RFM69_RSSI_THRESH                                  0x29


#define RFM69_RX_TIMEOUT1                                  0x2A
#define RFM69_RX_TIMEOUT2                                  0x2B

#define RFM69_PREAMBLE_MSB                                 0x2C
#define RFM69_PREAMBLE_LSB                                 0x2D

#define RFM69_SYNC_CONFIG                                  0x2E

#define RFM69_SYNC_VALUE1                                  0x2F
#define RFM69_SYNC_VALUE2                                  0x30
#define RFM69_SYNC_VALUE3                                  0x31
#define RFM69_SYNC_VALUE4                                  0x32
#define RFM69_SYNC_VALUE5                                  0x33
#define RFM69_SYNC_VALUE6                                  0x34
#define RFM69_SYNC_VALUE7                                  0x35
#define RFM69_SYNC_VALUE8                                  0x36

#define RFM69_PACKET_CONFIG1                               0x37
#define RFM69_PAYLOAD_LENGTH                               0x38

#define RFM69_NODE_ADRESS                                  0x39
#define RFM69_BROADCAST_ADRESS                             0x3A

#define RFM69_AUTO_MODES                                   0x3B

#define RFM69_FIFO_THRESH                                  0x3C

#define RFM69_PACKET_CONFIG2                               0x3D

#define RFM69_AES_KEY1                                     0x3E
#define RFM69_AES_KEY2                                     0x3F
#define RFM69_AES_KEY3                                     0x40
#define RFM69_AES_KEY4                                     0x41
#define RFM69_AES_KEY5                                     0x42
#define RFM69_AES_KEY6                                     0x43
#define RFM69_AES_KEY7                                     0x44
#define RFM69_AES_KEY8                                     0x45
#define RFM69_AES_KEY9                                     0x46
#define RFM69_AES_KEY10                                    0x47
#define RFM69_AES_KEY11                                    0x48
#define RFM69_AES_KEY12                                    0x49
#define RFM69_AES_KEY13                                    0x4A
#define RFM69_AES_KEY14                                    0x4B
#define RFM69_AES_KEY15                                    0x4C
#define RFM69_AES_KEY16                                    0x4D

#define RFM69_TEMP1                                        0x4E
#define RFM69_TEMP2                                        0x4F

#define RFM69_TEST_LNA                                     0x58
#define RFM69_TEST_PA1                                     0x5A
#define RFM69_TEST_PA2                                     0x5C
#define RFM69_TEST_DAGC                                    0x6F
#define RFM69_TEST_AFC                                     0x71

#define RFM69_WRITE_REG_MASK                               0x80
#define RFM69_READ_REG_MASK                                0x7F


/**
 *
 * Defines for various bitshifts and constants, per register.
 *
 **/


#define RFM69_MODE_SLEEP                                   0b00000
#define RFM69_MODE_STANDBY                                 0b00100
#define RFM69_MODE_FREQ_SYNTH                              0b01000
#define RFM69_MODE_TRANSMITTER                             0b01100
#define RFM69_MODE_RECEIVER                                0b10000


#define RFM69_MODE_SEQUENCER_OFF                           (1<<7)
#define RFM69_MODE_SEQUENCER_ON                            0
#define RFM69_MODE_LISTEN_ON                               (1<<6)
#define RFM69_MODE_LISTEN_OFF                              0
#define RFM69_MODE_LISTEN_ABORT                            (1<<5)


#define RFM69_DATAMODUL_PROCESSING_PACKET                  0b00
#define RFM69_DATAMODUL_PROCESSING_CONT_SYNCRHONISER       0b10
#define RFM69_DATAMODUL_PROCESSING_CONT                    0b11
#define RFM69_DATAMODUL_OOK                                true
#define RFM69_DATAMODUL_FSK                                false

#define RFM69_DATAMODUL_SHAPING_GFSK_NONE                  0
#define RFM69_DATAMODUL_SHAPING_GFSK_BT_1_0                0b01
#define RFM69_DATAMODUL_SHAPING_GFSK_BT_0_5                0b10
#define RFM69_DATAMODUL_SHAPING_GFSK_BT_0_3                0b11

#define RFM69_DATAMODUL_SHAPING_OOK_NONE                   0
#define RFM69_DATAMODUL_SHAPING_OOK_FCUTOFF_BR             0b01
#define RFM69_DATAMODUL_SHAPING_OOK_FCUTOFF_2BR            0b10

#define RFM69_PA_LEVEL_PA0_ON                              0b10000000
#define RFM69_PA_LEVEL_PA1_ON                              0b01000000
#define RFM69_PA_LEVEL_PA2_ON                              0b00100000

#define RFM69_PA_RAMP_3400US                               0b0000
#define RFM69_PA_RAMP_2000US                               0b0001
#define RFM69_PA_RAMP_1000US                               0b0010
#define RFM69_PA_RAMP_500US                                0b0011
#define RFM69_PA_RAMP_250US                                0b0100
#define RFM69_PA_RAMP_125US                                0b0101
#define RFM69_PA_RAMP_100US                                0b0110
#define RFM69_PA_RAMP_62US                                 0b0111
#define RFM69_PA_RAMP_50US                                 0b1000
#define RFM69_PA_RAMP_40US                                 0b1001
#define RFM69_PA_RAMP_31US                                 0b1010
#define RFM69_PA_RAMP_25US                                 0b1011
#define RFM69_PA_RAMP_20US                                 0b1100
#define RFM69_PA_RAMP_15US                                 0b1101
#define RFM69_PA_RAMP_12US                                 0b1110
#define RFM69_PA_RAMP_10US                                 0b1111

#define RFM69_PACKET_CONFIG_LENGTH_FIXED                   0b0
#define RFM69_PACKET_CONFIG_LENGTH_VARIABLE                (1<<7)

#define RFM69_PACKET_CONFIG_DC_FREE_NONE                   (0b00<<5)
#define RFM69_PACKET_CONFIG_DC_FREE_MANCHESTER             (0b01<<5)
#define RFM69_PACKET_CONFIG_DC_FREE_WHITENING              (0b10<<5)
#define RFM69_PACKET_CONFIG_CRC_ON                         (1<<4)
#define RFM69_PACKET_CONFIG_CRC_OFF                        0
#define RFM69_PACKET_CONFIG_CRC_FAIL_KEEP                  (1<<3)
#define RFM69_PACKET_CONFIG_CRC_FAIL_DISCARD               0
#define RFM69_PACKET_CONFIG_ADDRESS_FILTER_NONE            0
#define RFM69_PACKET_CONFIG_ADDRESS_FILTER_NODE            (0b01<<1)
#define RFM69_PACKET_CONFIG_ADDRESS_FILTER_NODE_BROADCAST  (0b10<<1)

#define RFM69_THRESHOLD_CONDITION_NOT_EMPTY                (1<<7)
#define RFM69_THRESHOLD_CONDITION_FIFOLEVEL                0

#define RFM69_AFC_CTRL_STANDARD                            0
#define RFM69_AFC_CTRL_IMPROVED                            1


#define RFM69_LISTEN_RESOL_IDLE_64US                       (0b01<<6)
#define RFM69_LISTEN_RESOL_IDLE_4_1MS                      (0b10<<6)
#define RFM69_LISTEN_RESOL_IDLE_262MS                      (0b11<<6)

#define RFM69_LISTEN_RESOL_RX_64US                         (0b01<<4)
#define RFM69_LISTEN_RESOL_RX_4_1MS                        (0b10<<4)
#define RFM69_LISTEN_RESOL_RX_262MS                        (0b11<<4)
#define RFM69_LISTEN_CRITERIA_RSSI                         (0 << 3)
#define RFM69_LISTEN_CRITERIA_RSSI_SYNC                    (1 << 3)
#define RFM69_LISTEN_END_STAY_RX_LISTEN_STOP               (0b00 << 1)
#define RFM69_LISTEN_END_RX_UNTIL_LISTEN_STOP              (0b01 << 1)
#define RFM69_LISTEN_END_RX_UNTIL_LISTEN_RESUME            (0b10 << 1)


#define RFM69_AUTOMODE_ENTER_NONE_AUTOMODES_OFF            (0b000<<5)
#define RFM69_AUTOMODE_ENTER_RISING_FIFONOTEMPTY           (0b001<<5)
#define RFM69_AUTOMODE_ENTER_RISING_FIFOLEVEL              (0b010<<5)
#define RFM69_AUTOMODE_ENTER_RISING_CRCOK                  (0b011<<5)
#define RFM69_AUTOMODE_ENTER_RISING_PAYLOADREADY           (0b100<<5)
#define RFM69_AUTOMODE_ENTER_RISING_SYNCADDRESS            (0b101<<5)
#define RFM69_AUTOMODE_ENTER_RISING_PACKETSENT             (0b110<<5)
#define RFM69_AUTOMODE_ENTER_FALLING_FIFONOTEMPTY          (0b111<<5)
//(I.E.FIFOEMPTY) 


#define RFM69_AUTOMODE_EXIT_NONE_AUTOMODES_OFF             (0b000<<2)
#define RFM69_AUTOMODE_EXIT_FALLING_FIFONOTEMPTY           (0b001<<2)
//(I.E._FIFOEMPTY)
#define RFM69_AUTOMODE_EXIT_RISING_FIFOLEVEL_OR_TIMEOUT    (0b010<<2)
#define RFM69_AUTOMODE_EXIT_RISING_CRCOK_OR_TIMEOUT        (0b011<<2)
#define RFM69_AUTOMODE_EXIT_RISING_PAYLOADREADY_OR_TIMEOUT (0b100<<2)
#define RFM69_AUTOMODE_EXIT_RISING_SYNCADDRESS_OR_TIMEOUT  (0b101<<2)
#define RFM69_AUTOMODE_EXIT_RISING_PACKETSENT              (0b110<<2)
#define RFM69_AUTOMODE_EXIT_RISING_TIMEOUT                 (0b111<<2)

#define RFM69_AUTOMODE_INTERMEDIATEMODE_SLEEP              (0b00)
#define RFM69_AUTOMODE_INTERMEDIATEMODE_STANDBY            (0b01)
#define RFM69_AUTOMODE_INTERMEDIATEMODE_RECEIVER           (0b10)
#define RFM69_AUTOMODE_INTERMEDIATEMODE_TRANSMITTER        (0b11)

#define RFM69_DIO_0_MAP_SHIFT 6
#define RFM69_DIO_1_MAP_SHIFT 4
#define RFM69_DIO_2_MAP_SHIFT 2
#define RFM69_DIO_3_MAP_SHIFT 0

#define RFM69_PACKET_DIO_0_RX_CRC_OK                       (0b00 << RFM69_DIO_0_MAP_SHIFT)
#define RFM69_PACKET_DIO_0_RX_PAYLOAD_READY                (0b01 << RFM69_DIO_0_MAP_SHIFT)
#define RFM69_PACKET_DIO_0_RX_SYNC_ADDRESS                 (0b10 << RFM69_DIO_0_MAP_SHIFT)
#define RFM69_PACKET_DIO_0_RX_RSII                         (0b11 << RFM69_DIO_0_MAP_SHIFT)

#define RFM69_PACKET_DIO_0_TX_PACKET_SENT                  (0b00 << RFM69_DIO_0_MAP_SHIFT)
#define RFM69_PACKET_DIO_0_TX_TX_READY                     (0b01 << RFM69_DIO_0_MAP_SHIFT)
#define RFM69_PACKET_DIO_0_TX_PLL                          (0b11 << RFM69_DIO_0_MAP_SHIFT)

#define RFM69_PACKET_DIO_1_FIFO_LEVEL                      (0b00 << RFM69_DIO_1_MAP_SHIFT)
#define RFM69_PACKET_DIO_1_FIFO_FULL                       (0b01 << RFM69_DIO_1_MAP_SHIFT)
#define RFM69_PACKET_DIO_1_FIFO_NOT_EMPTY                  (0b10 << RFM69_DIO_1_MAP_SHIFT)

#define RFM69_PACKET_DIO_2_FIFO_NOT_EMPTY                  (0b00 << RFM69_DIO_2_MAP_SHIFT)
#define RFM69_PACKET_DIO_2_AUTOMODE                        (0b11 << RFM69_DIO_2_MAP_SHIFT)

#define RFM69_PACKET_DIO_3_FIFO_FULL                       (0b00 << RFM69_DIO_3_MAP_SHIFT)


/**
 *
 * The above DIO constants are not extensive.
 * Only the ones which were deemed useful in packet mode are listed.
 *
 * See page 48 of the datasheet.
 *
 **/


#define RFM69_IRQ1_MODEREADY                               (1<<7)
#define RFM69_IRQ1_RXREADY                                 (1<<6)
#define RFM69_IRQ1_TXREADY                                 (1<<5)
#define RFM69_IRQ1_PLLLOCK                                 (1<<4)
#define RFM69_IRQ1_RSSI                                    (1<<3)
#define RFM69_IRQ1_TIMEOUT                                 (1<<2)
#define RFM69_IRQ1_AUTOMODE                                (1<<1)
#define RFM69_IRQ1_SYNCADDRESSMATCH                        (1<<0)


#define RFM69_IRQ2_FIFOFULL                                (1<<7)
#define RFM69_IRQ2_FIFONOTEMPTY                            (1<<6)
#define RFM69_IRQ2_FIFOLEVEL                               (1<<5)
#define RFM69_IRQ2_FIFOOVERRUN                             (1<<4)
#define RFM69_IRQ2_PACKETSENT                              (1<<3)
#define RFM69_IRQ2_PAYLOADREADY                            (1<<2)
#define RFM69_IRQ2_CRCOK                                   (1<<1)

#define RFM69_LNA_GAIN_AGC_LOOP                            0b000
#define RFM69_LNA_GAIN_0DB                                 0b001
#define RFM69_LNA_GAIN_6DB                                 0b010
#define RFM69_LNA_GAIN_12DB                                0b011
#define RFM69_LNA_GAIN_24DB                                0b100
#define RFM69_LNA_GAIN_36DB                                0b101
#define RFM69_LNA_GAIN_48DB                                0b110
#define RFM69_LNA_IMP_50OHM                                (0<<7)
#define RFM69_LNA_IMP_200OHM                               (1<<7)

#define RFM69_CONTINUOUS_DAGC_NORMAL                       0x00
#define RFM69_CONTINUOUS_DAGC_IMPROVED_AFCLOWBETAON        0x20
#define RFM69_CONTINUOUS_DAGC_IMPROVED_AFCLOWBETAOFF       0x30
