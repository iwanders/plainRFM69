/*
 *  This file is part of plainRFM69.
 *  Copyright (c) 2014, Ivor Wanders
 *  MIT License, see the LICENSE.md file in the root folder.
*/

#include "bareRFM69.h"

// Most functions are implemented in the header file.

void inline bareRFM69::chipSelect(bool enable){
    digitalWrite(this->cs_pin, (enable) ? LOW : HIGH );
}

void bareRFM69::writeRegister(uint8_t reg, uint8_t data){
    SPI.beginTransaction(SPISettings(10000000, MSBFIRST, SPI_MODE0));  // gain control of SPI bus
    this->chipSelect(true); // assert chip select
    SPI.transfer(RFM69_WRITE_REG_MASK | (reg & RFM69_READ_REG_MASK)); 
    SPI.transfer(data);
    this->chipSelect(false);// deassert chip select
    SPI.endTransaction();    // release the SPI bus
}

uint8_t bareRFM69::readRegister(uint8_t reg){
    uint8_t foo;
    SPI.beginTransaction(SPISettings(10000000, MSBFIRST, SPI_MODE0));  // gain control of SPI bus
    this->chipSelect(true); // assert chip select
    SPI.transfer((reg % RFM69_READ_REG_MASK));
    foo = SPI.transfer(0);
    this->chipSelect(false);// deassert chip select
    SPI.endTransaction();    // release the SPI bus
    return foo;
}

void bareRFM69::writeMultiple(uint8_t reg, void* data, uint8_t len){
    SPI.beginTransaction(SPISettings(10000000, MSBFIRST, SPI_MODE0));  // gain control of SPI bus
    this->chipSelect(true); // assert chip select
    SPI.transfer(RFM69_WRITE_REG_MASK | (reg & RFM69_READ_REG_MASK)); 
    uint8_t* r = reinterpret_cast<uint8_t*>(data);
    for (uint8_t i=0; i < len ; i++){
        SPI.transfer(r[len - i - 1]);
    }
    this->chipSelect(false);// deassert chip select
    SPI.endTransaction();    // release the SPI bus
}

void bareRFM69::readMultiple(uint8_t reg, void* data, uint8_t len){
    SPI.beginTransaction(SPISettings(10000000, MSBFIRST, SPI_MODE0));  // gain control of SPI bus
    this->chipSelect(true); // assert chip select
    
    SPI.transfer((reg % RFM69_READ_REG_MASK));
    uint8_t* r = reinterpret_cast<uint8_t*>(data);
    for (uint8_t i=0; i < len ; i++){
        r[len - i - 1] = SPI.transfer(0);
    }
    this->chipSelect(false);// deassert chip select
    SPI.endTransaction();    // release the SPI bus
}

uint32_t bareRFM69::readRegister32(uint8_t reg){
    uint32_t f = 0;
    this->readMultiple(reg, &f, 4);
    return f;
}
uint32_t bareRFM69::readRegister24(uint8_t reg){
    uint32_t f = 0;
    this->readMultiple(reg, &f, 3);
    return f;
}
uint16_t bareRFM69::readRegister16(uint8_t reg){
    uint16_t f = 0;
    this->readMultiple(reg, &f, 2);
    return f;
}

void bareRFM69::writeFIFO(void* buffer, uint8_t len){
    uint8_t* r = reinterpret_cast<uint8_t*>(buffer);
    SPI.beginTransaction(SPISettings(10000000, MSBFIRST, SPI_MODE0));  // gain control of SPI bus
    this->chipSelect(true); // assert chip select
    SPI.transfer(RFM69_WRITE_REG_MASK | (RFM69_FIFO & RFM69_READ_REG_MASK)); 
    for (uint8_t i=0; i < len ; i++){
        // Serial.print("Writing to FIFO: "); Serial.println(r[i]);
        SPI.transfer(r[i]);
    }
    this->chipSelect(false);// deassert chip select
    SPI.endTransaction();    // release the SPI bus
}

void bareRFM69::readFIFO(void* buffer, uint8_t len){
    uint8_t* r = reinterpret_cast<uint8_t*>(buffer);
    SPI.beginTransaction(SPISettings(10000000, MSBFIRST, SPI_MODE0));  // gain control of SPI bus
    this->chipSelect(true); // assert chip select
    
    SPI.transfer((RFM69_FIFO % RFM69_READ_REG_MASK));
    for (uint8_t i=0; i < len ; i++){
        r[i] = SPI.transfer(0);
    }
    this->chipSelect(false);// deassert chip select
    SPI.endTransaction();    // release the SPI bus
}

uint8_t bareRFM69::readVariableFIFO(void* buffer, uint8_t max_length){
    uint8_t* r = reinterpret_cast<uint8_t*>(buffer);

    SPI.beginTransaction(SPISettings(10000000, MSBFIRST, SPI_MODE0));  // gain control of SPI bus
    this->chipSelect(true); // assert chip select
    
    SPI.transfer((RFM69_FIFO % RFM69_READ_REG_MASK));
    uint8_t len = SPI.transfer(0);
    r[0] = len;
    // Serial.print("readVariableFIFO, len:"); Serial.println(len);
    len = len > (max_length-1) ? (max_length-1) : len;
    // Serial.print("readVariableFIFO, len:"); Serial.println(len);
    for (uint8_t i=0; i < len; i++){
        r[i+1] = SPI.transfer(0);
        // Serial.print("readVariableFIFO, r[i+1]"); Serial.println(r[i+1]);
    }
    this->chipSelect(false);// deassert chip select
    SPI.endTransaction();    // release the SPI bus
    return len;
}

 void bareRFM69::reset(uint8_t pin){ // function to send the RFM69 a hardware reset.
    // p 75 of datasheet;
    pinMode(pin, OUTPUT);
    digitalWrite(pin, HIGH);
    delayMicroseconds(150); // pull high for >100 uSec
    pinMode(pin, INPUT); // release
    delay(10); //  wait 10 milliseconds before SPI is possible.
}

