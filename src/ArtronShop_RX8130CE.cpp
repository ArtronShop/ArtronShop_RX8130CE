#include "ArtronShop_RX8130CE.h"


ArtronShop_RX8130CE::ArtronShop_RX8130CE(TwoWire *wire) : _wire(wire) {
    // ----
}

bool ArtronShop_RX8130CE::write_reg(uint8_t reg, uint8_t *value, size_t len) {
    this->_wire->beginTransmission(this->_addr);
    this->_wire->write(reg);
    this->_wire->write(value, len);
    return this->_wire->endTransmission() == 0;
}

bool ArtronShop_RX8130CE::write_reg(uint8_t reg, uint8_t value) {
    return this->write_reg(reg, &value, 1);
}

bool ArtronShop_RX8130CE::read_reg(uint8_t reg, uint8_t *value, size_t len) {
    this->_wire->beginTransmission(this->_addr);
    this->_wire->write(reg);
    if (this->_wire->endTransmission(false) != 0) {
        return false;
    }

    int n = this->_wire->requestFrom(this->_addr, len);
    if (n != len) {
        return false;
    }

    this->_wire->readBytes(value, len);

    return true;
}

uint8_t ArtronShop_RX8130CE::read_reg(uint8_t reg) {
    uint8_t value = 0;
    this->read_reg(reg, &value, 1);
    return value;
}

bool ArtronShop_RX8130CE::stop(bool stop) {
    return this->write_reg(0x1E, stop ? 0x040 : 0x00);
}

uint8_t ArtronShop_RX8130CE::bcd2dec(uint8_t bcd) {
    return ((bcd >> 4) * 10) + (bcd & 0x0F);
}

uint8_t ArtronShop_RX8130CE::dec2bcd(uint8_t bin) {
    return (((bin / 10) << 4) & 0xF0) | ((bin % 10) & 0x0F);
}

#define CHECK_OK(OP) { \
    if (!(OP)) { \
        return false; \
    } \
}

bool ArtronShop_RX8130CE::begin() {
    /* 
     * Digital offset register:
     *   [7]   DET: 0 ->  disabled
     *   [6:0] L7-L1: 0 -> no offset
     */
    CHECK_OK(this->write_reg(0x30, 0x00));

    /* 
     * Extension Register register:
     *   [7:6]   FSEL: 0 ->  0
     *   [5]     USEL: 0 -> 0
     *   [4]     TE: 0 ->
     *   [3]     WADA: 0 -> 0
     *   [2-0]   TSEL: 0 -> 0
     */
    CHECK_OK(this->write_reg(0x1C, 0x00));

    /* 
     * Flag Register register:
     *   [7]     VBLF: 0 ->  0
     *   [6]     0: 0 -> 
     *   [5]     UF: 0 -> 
     *   [4]     TF: 0 ->
     *   [3]     AF: 0 -> 0
     *   [2]     RSF: 0 -> 0
     *   [1]     VLF: 0 -> 0
     *   [0]     VBFF: 0 -> 0
     */
    CHECK_OK(this->write_reg(0x1D, 0x00));


    /* 
     * Control Register0 register:
     *   [7]     TEST: 0 ->  0
     *   [6]     STOP: 0 -> 
     *   [5]     UIE: 0 -> 
     *   [4]     TIE: 0 ->
     *   [3]     AIE: 0 -> 0
     *   [2]     TSTP: 0 -> 0
     *   [1]     TBKON: 0 -> 0
     *   [0]     TBKE: 0 -> 0
     */
    CHECK_OK(this->write_reg(0x1E, 0x00));

    /* 
     * Control Register1 register:
     *   [7-6]   SMPTSEL: 0 ->  0
     *   [5]     CHGEN: 0 -> 
     *   [4]     INIEN: 0 -> 
     *   [3]     0: 0 ->
     *   [2]     RSVSEL: 0 -> 0
     *   [1-0]   BFVSEL: 0 -> 0
     */
    CHECK_OK(this->write_reg(0x1F, 0x00));

    CHECK_OK(this->stop(false)); // clear STOP bit

    /* 
     * Function register:
     *   [7]   100TH: 0 ->  disabled
     *   [6:5] Periodic interrupt: 0 -> no periodic interrupt
     *   [4]   RTCM: 0 -> real-time clock mode
     *   [3]   STOPM: 0 -> RTC stop is controlled by STOP bit only
     *   [2:0] Clock output frequency: 000 (Default value)
     */
    CHECK_OK(this->write_reg(0x28, 0x00)); 
    
    // Battery switch register
    CHECK_OK(this->write_reg(0x26, 0x00)); // enable battery switch feature

    return true;
}

bool ArtronShop_RX8130CE::setTime(struct tm t) {
    uint8_t buff[7];
    buff[0] = dec2bcd(t.tm_sec) & 0x7F;
    buff[1] = dec2bcd(t.tm_min) & 0x7F;
    buff[2] = dec2bcd(t.tm_hour) & 0x3F;
    buff[3] = dec2bcd(t.tm_wday) & 0x07;
    buff[4] = dec2bcd(t.tm_mday) & 0x3F;
    buff[5] = dec2bcd(t.tm_mon + 1) & 0x1F;
    buff[6] = dec2bcd((t.tm_year + 1900) % 100);

    CHECK_OK(this->stop(true));
    CHECK_OK(this->write_reg(0x10, buff, sizeof(buff)));
    CHECK_OK(this->stop(false));

    return true;
}

bool ArtronShop_RX8130CE::getTime(struct tm *t) {
    uint8_t buff[7];
    CHECK_OK(this->read_reg(0x10, buff, sizeof(buff)));

    t->tm_sec = bcd2dec(buff[0] & 0x7F);
    t->tm_min = bcd2dec(buff[1] & 0x7F);
    t->tm_hour = bcd2dec(buff[2] & 0x3F);
    t->tm_wday = bcd2dec(buff[3] & 0x07);
    t->tm_mday = bcd2dec(buff[4] & 0x3F);
    t->tm_mon = bcd2dec(buff[5] & 0x1F) - 1;
    t->tm_year = bcd2dec(buff[6]) + 2000 - 1900;

    return true;
}

bool ArtronShop_RX8130CE::writeRAM(uint8_t address, uint8_t value) {
    return this->writeRAM(address, &value, 1);
}

size_t ArtronShop_RX8130CE::writeRAM(uint8_t address, uint8_t *value, size_t len) {
    if (address > 3) { // Oversize of 64-bytes RAM
        return 0;
    }

    if ((address + len) > 3) { // Data size over RAM size
        len = 3 - address;
    }

    if (!this->write_reg(0x20 + address, value, len)) {
        return 0;
    }

    return len;
}

bool ArtronShop_RX8130CE::readRAM(uint8_t address, uint8_t *value, size_t len) {
    if (address > 3) { // Oversize of 64-bytes RAM
        return false;
    }

    if ((address + len) > 3) { // Data size over RAM size
        len = 3 - address;
    }

    return this->read_reg(0x20 + address, value, len);
}

uint8_t ArtronShop_RX8130CE::readRAM(uint8_t address) {
    uint8_t value = 0xFF;
    this->readRAM(address, &value, 1);
    return value;
}
