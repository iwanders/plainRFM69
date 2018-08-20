/**
 *
 *  bareRFM69.cpp - A component of plainRFM69, created by Ivor Wanders.
 *
 *  Copyright (c) 2014, Ivor Wanders
 *  Copyright (c) 2018, Charles Taylor
 *
 *  MIT License - See the LICENSE.md file for details.
 *
 **/


#include "bareRFM69.h"


// ****************************************************
// *** NOTE: Some functions are implemented and all ***
// ***       are documented in the header file.     ***
// ****************************************************


// ********************************
// * Register/FIFO read and write *
// ********************************


/**
 *
 *  These functions all perform the following progression:
 *
 *    1. Gain control of the SPI bus using acquireSPI().
 *    2. Assert the chip select pin using acquireSPI().
 *    3. Perform the SPI transfer.
 *    4. Deassert the chip select pin using releaseSPI().
 *    5. Release the SPI bus using releaseSPI().
 *
 **/


uint8_t bareRFM69::readRegister(uint8_t reg)
{
    uint8_t data;

    bareRFM69::acquireSPI();
    SPI.transfer((reg % RFM69_READ_REG_MASK));
    data = SPI.transfer(0);
    bareRFM69::releaseSPI();

    return data;
}


void bareRFM69::writeRegister(uint8_t reg, uint8_t data)
{
    bareRFM69::acquireSPI();

    SPI.transfer(RFM69_WRITE_REG_MASK | (reg & RFM69_READ_REG_MASK));
    SPI.transfer(data);

    bareRFM69::releaseSPI();
}


void bareRFM69::readMultiple(uint8_t reg, void* data, uint8_t len)
{
    bareRFM69::acquireSPI();
    SPI.transfer((reg % RFM69_READ_REG_MASK));
    uint8_t* r = reinterpret_cast<uint8_t*>(data);
    for (uint8_t i=0; i < len ; i++){
        r[len - i - 1] = SPI.transfer(0);
    }
    bareRFM69::releaseSPI();
}


void bareRFM69::writeMultiple(uint8_t reg, void* data, uint8_t len)
{
    bareRFM69::acquireSPI();
    SPI.transfer(RFM69_WRITE_REG_MASK | (reg & RFM69_READ_REG_MASK)); 
    uint8_t* r = reinterpret_cast<uint8_t*>(data);
    for (uint8_t i=0; i < len ; i++){
        SPI.transfer(r[len - i - 1]);
    }
    bareRFM69::releaseSPI();
}


void bareRFM69::readFIFO(void* buffer, uint8_t len)
{
    uint8_t* r = reinterpret_cast<uint8_t*>(buffer);

    acquireSPI();
    
    SPI.transfer((RFM69_FIFO % RFM69_READ_REG_MASK));
    for (uint8_t i=0; i < len ; i++){
        r[i] = SPI.transfer(0);
    }

    releaseSPI();
}


uint8_t bareRFM69::readVariableFIFO(void* buffer, uint8_t max_length)
{
    uint8_t* r = reinterpret_cast<uint8_t*>(buffer);

    acquireSPI();
    
    SPI.transfer((RFM69_FIFO % RFM69_READ_REG_MASK));
    uint8_t len = SPI.transfer(0);
    r[0] = len;
    debug_rfm("readVariableFIFO, len: ");
    debug_rfmln(len);
    len = len > (max_length-1) ? (max_length-1) : len;
    debug_rfm("readVariableFIFO, len: ");
    debug_rfmln(len);
    for (uint8_t i=0; i < len; i++){
        r[i+1] = SPI.transfer(0);
        debug_rfm("readVariableFIFO, r[i+1]: ");
        debug_rfmln(r[i+1]);
    }

    releaseSPI();

    return len;
}


void bareRFM69::writeFIFO(void* buffer, uint8_t len)
{
    uint8_t* r = reinterpret_cast<uint8_t*>(buffer);

    acquireSPI();

    SPI.transfer(RFM69_WRITE_REG_MASK | (RFM69_FIFO & RFM69_READ_REG_MASK));
    for (uint8_t i=0; i < len ; i++){
        debug_rfm("Writing to FIFO: ");
        debug_rfmln(r[i]);
        SPI.transfer(r[i]);
    }

    releaseSPI();
}


// *******************************************************************
// * Register read and write wrappers to simplify small transactions *
// *******************************************************************


uint32_t bareRFM69::readRegister32(uint8_t reg)
{
    uint32_t f = 0;
    this->readMultiple(reg, &f, 4);
    return f;
}


uint32_t bareRFM69::readRegister24(uint8_t reg)
{
    uint32_t f = 0;
    this->readMultiple(reg, &f, 3);
    return f;
}


uint16_t bareRFM69::readRegister16(uint8_t reg)
{
    uint16_t f = 0;
    this->readMultiple(reg, &f, 2);
    return f;
}
