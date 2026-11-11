#ifndef HARDWARE_H
#define HARDWARE_H

#include "VoleExceptions.h"
#include <array>
#include <cstdint>
#include <string>
#include <vector>

class Memory {
public:
    static constexpr std::size_t SIZE = 256;
    uint8_t read(uint8_t addr) const;
    void write(uint8_t addr, uint8_t value);
    std::array<uint8_t, SIZE> snapshot() const;
    void reset();
private:
    std::array<uint8_t, SIZE> cells_{};
};

class RegisterFile {
public:
    static constexpr std::size_t COUNT = 16;
    uint8_t read(uint8_t index) const;
    void write(uint8_t index, uint8_t value);
    std::array<uint8_t, COUNT> snapshot() const;
    void reset();
private:
    static void checkIndex(uint8_t index);
    std::array<uint8_t, COUNT> regs_{};
};

class Screen {
public:
    void write(uint8_t value);
    uint8_t lastValue() const;
    const std::vector<uint8_t>& getHistory() const;
    std::string asText() const;
    void clear();
private:
    std::vector<uint8_t> history_;
};

class MemoryBus {
public:
    static constexpr uint8_t SCREEN_ADDR = 0xFF;
    static constexpr uint8_t ALT_SCREEN_ADDR = 0x00;

    MemoryBus(Memory& memory, Screen& screen);
    uint8_t read(uint8_t addr) const;
    void write(uint8_t addr, uint8_t value);
    bool isScreenAddress(uint8_t addr) const;
    void loadBlock(const std::vector<uint8_t>& bytes, uint8_t start);
private:
    Memory& memory_;
    Screen& screen_;
};

class InstructionWord {
public:
    InstructionWord() = default;
    explicit InstructionWord(uint16_t raw) : raw_(raw) {}
    uint16_t value() const { return raw_; }
    uint8_t opcode() const { return (raw_ >> 12) & 0xF; }
    uint8_t nibble(int index) const {
        if (index < 0 || index > 3) throw VoleException("nibble index out of range");
        return (raw_ >> (12 - 4 * index)) & 0xF;
    }
    uint8_t lowByte() const { return raw_ & 0xFF; }
private:
    uint16_t raw_ = 0;
};

class ALU {
public:
    static uint8_t addInt(uint8_t a, uint8_t b);
    static uint8_t addFloat(uint8_t a, uint8_t b);
    static uint8_t bitOr(uint8_t a, uint8_t b);
    static uint8_t bitAnd(uint8_t a, uint8_t b);
    static uint8_t bitXor(uint8_t a, uint8_t b);
    static uint8_t rotateRight(uint8_t v, uint8_t steps);
    static double decodeFloat(uint8_t bits);
    static uint8_t encodeFloat(double value);
};

#endif // HARDWARE_H