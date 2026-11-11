#include "CPU.h"

CPU::CPU(MemoryBus& bus, const InstructionFactory& factory)
    : bus_(bus), factory_(factory) {}

void CPU::reset(uint16_t startPC) {
    PC_ = startPC;
    IR_ = InstructionWord();
    halted_ = false;
    registers_.reset();
}

StepResult CPU::step() {
    if (halted_) return StepResult{false, PC_, IR_, "CPU is halted"};
    const uint16_t address = PC_;
    fetch();
    std::unique_ptr<Instruction> instr = decode();
    StepResult result{true, address, IR_, instr->disassemble()};
    instr->execute(*this);
    return result;
}

std::string CPU::peekNext() const {
    if (halted_) return "(halted)";
    if (PC_ + 1 >= END_OF_MEMORY) return "(end of memory)";
    const uint16_t raw = static_cast<uint16_t>(
        (bus_.read(static_cast<uint8_t>(PC_)) << 8) | bus_.read(static_cast<uint8_t>(PC_ + 1)));
    try {
        return factory_.create(InstructionWord(raw))->disassemble();
    } catch (const VoleException& e) {
        return std::string("(") + e.what() + ")";
    }
}

uint8_t CPU::readReg(uint8_t i) const { return registers_.read(i); }
void CPU::writeReg(uint8_t i, uint8_t v) { registers_.write(i, v); }
uint8_t CPU::readMem(uint8_t addr) const { return bus_.read(addr); }
void CPU::writeMem(uint8_t addr, uint8_t v) { bus_.write(addr, v); }
uint16_t CPU::getPC() const { return PC_; }
void CPU::setPC(uint8_t addr) { PC_ = addr; }
void CPU::halt() { halted_ = true; }

void CPU::fetch() {
    if (PC_ + 1 >= END_OF_MEMORY) throw MemoryAccessException(PC_);
    const uint8_t hi = bus_.read(static_cast<uint8_t>(PC_));
    const uint8_t lo = bus_.read(static_cast<uint8_t>(PC_ + 1));
    IR_ = InstructionWord(static_cast<uint16_t>((hi << 8) | lo));
    PC_ += 2;
}

std::unique_ptr<Instruction> CPU::decode() const { return factory_.create(IR_); }