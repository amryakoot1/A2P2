#include "Hardware.h"
#include <cmath>

uint8_t Memory::read(uint8_t addr) const { return cells_[addr]; }
void Memory::write(uint8_t addr, uint8_t value) { cells_[addr] = value; }
std::array<uint8_t, Memory::SIZE> Memory::snapshot() const { return cells_; }
void Memory::reset() { cells_.fill(0); }

uint8_t RegisterFile::read(uint8_t index) const {
    checkIndex(index);
    return regs_[index];
}
void RegisterFile::write(uint8_t index, uint8_t value) {
    checkIndex(index);
    regs_[index] = value;
}
std::array<uint8_t, RegisterFile::COUNT> RegisterFile::snapshot() const { return regs_; }
void RegisterFile::reset() { regs_.fill(0); }
void RegisterFile::checkIndex(uint8_t index) {
    if (index >= COUNT)
        throw VoleException("Invalid register index " + std::to_string(index));
}

void Screen::write(uint8_t value) { history_.push_back(value); }
uint8_t Screen::lastValue() const { return history_.empty() ? 0 : history_.back(); }
const std::vector<uint8_t>& Screen::getHistory() const { return history_; }
std::string Screen::asText() const {
    std::string text;
    for (uint8_t b : history_)
        text += (b >= 0x20 && b < 0x7F) ? static_cast<char>(b) : '.';
    return text;
}
void Screen::clear() { history_.clear(); }

MemoryBus::MemoryBus(Memory& memory, Screen& screen) : memory_(memory), screen_(screen) {}
uint8_t MemoryBus::read(uint8_t addr) const { return memory_.read(addr); }
void MemoryBus::write(uint8_t addr, uint8_t value) {
    memory_.write(addr, value);
    if (isScreenAddress(addr)) screen_.write(value);
}
bool MemoryBus::isScreenAddress(uint8_t addr) const {
    return addr == SCREEN_ADDR || addr == ALT_SCREEN_ADDR;
}
void MemoryBus::loadBlock(const std::vector<uint8_t>& bytes, uint8_t start) {
    if (static_cast<std::size_t>(start) + bytes.size() > Memory::SIZE)
        throw ProgramLoadException("program does not fit in memory");
    for (std::size_t i = 0; i < bytes.size(); ++i)
        memory_.write(static_cast<uint8_t>(start + i), bytes[i]);
}

uint8_t ALU::addInt(uint8_t a, uint8_t b) { return static_cast<uint8_t>(a + b); }
uint8_t ALU::addFloat(uint8_t a, uint8_t b) { return encodeFloat(decodeFloat(a) + decodeFloat(b)); }
uint8_t ALU::bitOr(uint8_t a, uint8_t b) { return a | b; }
uint8_t ALU::bitAnd(uint8_t a, uint8_t b) { return a & b; }
uint8_t ALU::bitXor(uint8_t a, uint8_t b) { return a ^ b; }
uint8_t ALU::rotateRight(uint8_t v, uint8_t steps) {
    steps %= 8;
    if (steps == 0) return v;
    return static_cast<uint8_t>((v >> steps) | (v << (8 - steps)));
}
double ALU::decodeFloat(uint8_t bits) {
    const double sign = (bits & 0x80) ? -1.0 : 1.0;
    const int exponent = ((bits >> 4) & 0x7) - 4;
    const int mantissa = bits & 0xF;
    return sign * (mantissa / 16.0) * std::pow(2.0, exponent);
}
uint8_t ALU::encodeFloat(double value) {
    uint8_t best = 0;
    double bestErr = std::abs(decodeFloat(0) - value);
    for (int b = 1; b < 256; ++b) {
        const double err = std::abs(decodeFloat(static_cast<uint8_t>(b)) - value);
        if (err < bestErr) {
            bestErr = err;
            best = static_cast<uint8_t>(b);
        }
    }
    return best;
}