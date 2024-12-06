#ifndef __ARTRONSHOP_RX8130CE_H__
#define __ARTRONSHOP_RX8130CE_H__

#include <Arduino.h>
#include <Wire.h>
#include <time.h>

class ArtronShop_RX8130CE {
    private:
        const int _addr = 0x32;
        TwoWire *_wire = NULL;

        bool write_reg(uint8_t reg, uint8_t *value, size_t len) ;
        bool write_reg(uint8_t reg, uint8_t value) ;
        bool read_reg(uint8_t reg, uint8_t *value, size_t len = 1) ;
        uint8_t read_reg(uint8_t reg) ;

        bool stop(bool stop) ;

        uint8_t bcd2dec(uint8_t bcd) ;
        uint8_t dec2bcd(uint8_t bin) ;

    public:
        ArtronShop_RX8130CE(TwoWire *wire = &Wire) ;

        bool begin() ;
        bool setTime(struct tm t) ;
        bool getTime(struct tm *t) ;

        bool writeRAM(uint8_t address, uint8_t value) ;
        size_t writeRAM(uint8_t address, uint8_t *value, size_t len) ;
        bool readRAM(uint8_t address, uint8_t *value, size_t len) ;
        uint8_t readRAM(uint8_t address) ;

};

#endif
