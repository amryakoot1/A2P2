#ifndef VOLE_EXCEPTIONS_H
#define VOLE_EXCEPTIONS_H

#include <cstddef>
#include <cstdint>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <string>

class HexFormatter {
public:
    static std::string digits(unsigned value, int width) {
        std::ostringstream os;
        os << std::uppercase << std::hex << std::setw(width) << std::setfill('0') << value;
        return os.str();
    }
    static std::string prefixed(unsigned value, int width) {
        return "0x" + digits(value, width);
    }
    static std::string reg(unsigned index) {
        return "R" + digits(index, 1);
    }
};

class VoleException : public std::runtime_error {
public:
    explicit VoleException(const std::string& msg) : std::runtime_error(msg) {}
};

class InvalidOpcodeException : public VoleException {
public:
    explicit InvalidOpcodeException(uint8_t opcode)
        : VoleException("Invalid opcode " + HexFormatter::prefixed(opcode, 1)),
          opcode_(opcode) {}
    uint8_t getOpcode() const { return opcode_; }
private:
    uint8_t opcode_;
};

class MemoryAccessException : public VoleException {
public:
    explicit MemoryAccessException(uint16_t address)
        : VoleException("Memory access out of range at " + HexFormatter::prefixed(address, 2)),
          address_(address) {}
    uint16_t getAddress() const { return address_; }
private:
    uint16_t address_;
};

class ProgramLoadException : public VoleException {
public:
    explicit ProgramLoadException(const std::string& msg)
        : VoleException("Program load error: " + msg) {}
};

class ExecutionLimitException : public VoleException {
public:
    explicit ExecutionLimitException(std::size_t limit)
        : VoleException("Execution limit of " + std::to_string(limit) +
                        " steps reached (possible infinite loop)"),
          limit_(limit) {}
    std::size_t getLimit() const { return limit_; }
private:
    std::size_t limit_;
};

#endif // VOLE_EXCEPTIONS_H