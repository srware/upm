/*
 * Author: Henry Bruce <henry.bruce@intel.com>
 * Copyright (c) 2014 Intel Corporation.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <iostream>
#include <unistd.h>
#include <string.h>
#include "lp8860.h"
#include "mraa-utils.h"

// Used to prevent silent failure in mraa_i2c_write()
// Value come from I2C_SMBUS_I2C_BLOCK_MAX in i2c driver
#define MAX_I2C_WRITE_SIZE 32

#define LP8860_I2C_ADDR 0x2D

#define LP8860_DISP_CL1_BRT_MSB		0x00
#define LP8860_DISP_CL1_BRT_LSB		0x01
#define LP8860_DISP_CL1_CURR_MSB	0x02
#define LP8860_DISP_CL1_CURR_LSB	0x03
#define LP8860_CL2_BRT_MSB		0x04
#define LP8860_CL2_BRT_LSB		0x05
#define LP8860_CL2_CURRENT		0x06
#define LP8860_CL3_BRT_MSB		0x07
#define LP8860_CL3_BRT_LSB		0x08
#define LP8860_CL3_CURRENT		0x09
#define LP8860_CL4_BRT_MSB		0x0a
#define LP8860_CL4_BRT_LSB		0x0b
#define LP8860_CL4_CURRENT		0x0c
#define LP8860_CONFIG			0x0d
#define LP8860_STATUS			0x0e
#define LP8860_FAULT			0x0f
#define LP8860_LED_FAULT		0x10
#define LP8860_FAULT_CLEAR		0x11
#define LP8860_ID			0x12
#define LP8860_TEMP_MSB			0x13
#define LP8860_TEMP_LSB			0x14
#define LP8860_DISP_LED_CURR_MSB	0x15
#define LP8860_DISP_LED_CURR_LSB	0x16
#define LP8860_DISP_LED_PWM_MSB		0x17
#define LP8860_DISP_LED_PWM_LSB		0x18
#define LP8860_EEPROM_CNTRL		0x19
#define LP8860_EEPROM_UNLOCK		0x1a

#define LP8860_EEPROM_REG_0		0x60
#define LP8860_EEPROM_REG_1		0x61
#define LP8860_EEPROM_REG_2		0x62
#define LP8860_EEPROM_REG_3		0x63
#define LP8860_EEPROM_REG_4		0x64
#define LP8860_EEPROM_REG_5		0x65
#define LP8860_EEPROM_REG_6		0x66
#define LP8860_EEPROM_REG_7		0x67
#define LP8860_EEPROM_REG_8		0x68
#define LP8860_EEPROM_REG_9		0x69
#define LP8860_EEPROM_REG_10		0x6a
#define LP8860_EEPROM_REG_11		0x6b
#define LP8860_EEPROM_REG_12		0x6c
#define LP8860_EEPROM_REG_13		0x6d
#define LP8860_EEPROM_REG_14		0x6e
#define LP8860_EEPROM_REG_15		0x6f
#define LP8860_EEPROM_REG_16		0x70
#define LP8860_EEPROM_REG_17		0x71
#define LP8860_EEPROM_REG_18		0x72
#define LP8860_EEPROM_REG_19		0x73
#define LP8860_EEPROM_REG_20		0x74
#define LP8860_EEPROM_REG_21		0x75
#define LP8860_EEPROM_REG_22		0x76
#define LP8860_EEPROM_REG_23		0x77
#define LP8860_EEPROM_REG_24		0x78

#define LP8860_LOCK_EEPROM		0x00
#define LP8860_UNLOCK_EEPROM		0x01
#define LP8860_LOAD_EEPROM		0x01
#define LP8860_PROGRAM_EEPROM		0x02
#define LP8860_EEPROM_CODE_1		0x08
#define LP8860_EEPROM_CODE_2		0xba
#define LP8860_EEPROM_CODE_3		0xef

#define LP8860_CLEAR_FAULTS		0x01
#define LP8860_INVALID_ID       0x00

using namespace upm;

LP8860::LP8860(int gpioPower, int i2cBus)
{
    status = MRAA_ERROR_INVALID_RESOURCE;
    mraa_set_log_level(7);
    pinPower = gpioPower;
    i2c = mraa_i2c_init(i2cBus);
    status = mraa_i2c_address(i2c, LP8860_I2C_ADDR);

    if (status != MRAA_SUCCESS) { printf("LP8860: I2C initialisation failed.\n"); return; }

    if(!isAvailable()) { status = MRAA_ERROR_INVALID_RESOURCE; return; }

    status = MRAA_SUCCESS;
}

LP8860::~LP8860()
{
}


bool LP8860::isOK()
{
    return status == MRAA_SUCCESS;
}

bool LP8860::isAvailable()
{
    uint8_t id;

    // Read ID register
    mraa_i2c_address(i2c, LP8860_I2C_ADDR);
    id = mraa_i2c_read_byte_data(i2c, LP8860_ID);

    if(id == -1 || id == LP8860_INVALID_ID ) { return false; }

    return true;
}

bool LP8860::getBrightnessRange(int* percentMin, int* percentMax)
{
    *percentMin = 0;
    *percentMax = 100;
    return true;
}

bool LP8860::isPowered()
{
    int level;
    if (MraaUtils::getGpio(pinPower, &level) == MRAA_SUCCESS)
        return level == 1;
    else
        return false;
}

bool LP8860::setPowerOn()
{
    if (!isPowered())
    {
        if (MraaUtils::setGpio(pinPower, 1) != MRAA_SUCCESS)
        {
            printf("setPowerOn failed\n");
            status = MRAA_ERROR_INVALID_RESOURCE;
            return false;
        }
        if (!setBrightness(0))
            printf("setBrightness failed\n");
        if (!loadEEPROM())
            printf("loadEEPROM failed\n");
        allowMaxCurrent();
    }
    return isOK();
}

bool LP8860::setPowerOff()
{
    return MraaUtils::setGpio(pinPower, 0) == MRAA_SUCCESS;
}


bool LP8860::getBrightness(int* percent)
{
    uint8_t msb;
    uint8_t lsb;
    if (i2cReadByte(LP8860_DISP_CL1_BRT_MSB, &msb) && i2cReadByte(LP8860_DISP_CL1_BRT_LSB, &lsb))
    {
        *percent = (100 * 0xFFFF) / ((int)msb << 8 | lsb);
        return true;
    }
    else
        return false;
}


bool LP8860::setBrightness(int dutyPercent)
{
    int value = (0xFFFF * dutyPercent) / 100;
    int msb = value >> 8;
    int lsb = value & 0xFF;
    i2cWriteByte(LP8860_DISP_CL1_BRT_MSB, msb);
    i2cWriteByte(LP8860_DISP_CL1_BRT_LSB, lsb);
    value = (0x1FFF * dutyPercent) / 100;
    msb = value >> 8;
    lsb = value & 0xFF;
    i2cWriteByte(LP8860_CL2_BRT_MSB, msb);
    i2cWriteByte(LP8860_CL2_BRT_LSB, lsb);
    i2cWriteByte(LP8860_CL3_BRT_MSB, msb);
    i2cWriteByte(LP8860_CL3_BRT_LSB, lsb);
    i2cWriteByte(LP8860_CL4_BRT_MSB, msb);
    i2cWriteByte(LP8860_CL4_BRT_LSB, lsb);
    return isOK();
}


bool LP8860::loadEEPROM()
{
    const int eepromTableSize = 0x19;
    uint8_t eepromInitTable[] = {
        0xEF, 0xFF, 0xDC, 0xAE, 0x5F, 0xE5, 0xF2, 0x77,
        0x77, 0x71, 0x3F, 0xB7, 0x17, 0xEF, 0xB0, 0x87,
        0xCF, 0x72, 0xC5, 0xDE, 0x35, 0x06, 0xDE, 0xFF,
        0x3E
    };

    uint8_t* buf = new unsigned char[eepromTableSize + 1];

    // Load EEPROM
    // printf("Loading LP8860 EEPROM\n");
    i2cWriteByte(LP8860_EEPROM_CNTRL, LP8860_LOAD_EEPROM);
    usleep(100000);


    // Check contents and program if not already done
    i2cReadBuffer(LP8860_EEPROM_REG_0, buf, eepromTableSize);
    if (!isOK()) printf("Read eeprom error\n");
    if (memcmp(eepromInitTable, buf, eepromTableSize) != 0)
    {
        printf("LP8860 EEPROM not initialized - programming...\n");
        // Unlock EEPROM
        i2cWriteByte(LP8860_EEPROM_UNLOCK, LP8860_EEPROM_CODE_1);
        i2cWriteByte(LP8860_EEPROM_UNLOCK, LP8860_EEPROM_CODE_2);
        i2cWriteByte(LP8860_EEPROM_UNLOCK, LP8860_EEPROM_CODE_3);
        i2cWriteBuffer(LP8860_EEPROM_REG_0, eepromInitTable, eepromTableSize);
        i2cWriteByte(LP8860_EEPROM_CNTRL, LP8860_PROGRAM_EEPROM);
        usleep(200000);
        i2cWriteByte(LP8860_EEPROM_CNTRL, 0);
        i2cWriteByte(LP8860_EEPROM_UNLOCK, LP8860_LOCK_EEPROM);
    }

    return isOK();
}


bool LP8860::allowMaxCurrent()
{
    i2cWriteByte(LP8860_DISP_CL1_CURR_MSB, 0x0F);
    i2cWriteByte(LP8860_DISP_CL1_CURR_LSB, 0xFF);
    i2cWriteByte(LP8860_CL2_CURRENT, 0xFF);
    i2cWriteByte(LP8860_CL3_CURRENT, 0xFF);
    i2cWriteByte(LP8860_CL4_CURRENT, 0xFF);
    return isOK();
}


bool LP8860::i2cWriteByte(int reg, int value)
{
    if (isOK())
        status = mraa_i2c_write_byte_data(i2c, static_cast<uint8_t>(value), static_cast<uint8_t>(reg));
    return isOK();
}


bool LP8860::i2cReadByte(uint8_t reg, uint8_t* value)
{
    mraa_i2c_write_byte(i2c, LP8860_I2C_ADDR);
    mraa_i2c_write_byte(i2c, reg);
    return mraa_i2c_read(i2c, value, 1) == 1;
}


bool LP8860::i2cWriteBuffer(int reg, uint8_t* buf, int length)
{
    if (length <= MAX_I2C_WRITE_SIZE)
    {
        uint8_t* writeBuf = new unsigned char[length + 1];
        writeBuf[0] = reg;
        memcpy(&writeBuf[1], buf, length);
        status = mraa_i2c_write(i2c, writeBuf, length + 1);
    }
    else
        status = MRAA_ERROR_INVALID_PARAMETER;
    return isOK();
}


bool LP8860::i2cReadBuffer(int reg, uint8_t* buf, int length)
{
    status = mraa_i2c_write_byte(i2c, reg);
    if (isOK())
    {
        if (mraa_i2c_read(i2c, buf, length) != length)
            status = MRAA_ERROR_NO_DATA_AVAILABLE;
    }
    return isOK();
}


