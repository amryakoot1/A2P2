#ifndef CPU_H
#define CPU_H

#include "Instructions.h"
#include <cstdint>
#include <memory>
#include <string>

struct StepResult {
    bool executed = false;
    uint16_t address = 0;
    InstructionWord word;
    std::string text;
};

class CPU : public IExecutionContext {
public:
    static constexpr uint16_t END_OF_MEMORY = 0x100;

    CPU(MemoryBus& bus, const InstructionFactory& factory);
    void reset(uint16_t startPC = 0);
    StepResult step();
    std::string peekNext() const;

    uint16_t programCounter() const { return PC_; }
    InstructionWord instructionRegister() const { return IR_; }
    bool isHalted() const { return halted_; }
    RegisterFile& registers() { return registers_; }
    const RegisterFile& registers() const { return registers_; }

    uint8_t readReg(uint8_t i) const override;
    void writeReg(uint8_t i, uint8_t v) override;
    uint8_t readMem(uint8_t addr) const override;
    void writeMem(uint8_t addr, uint8_t v) override;
    uint16_t getPC() const override;
    void setPC(uint8_t addr) override;
    void halt() override;

private:
    void fetch();
    std::unique_ptr<Instruction> decode() const;

    MemoryBus& bus_;
    const InstructionFactory& factory_;
    uint16_t PC_ = 0;
    InstructionWord IR_;
    bool halted_ = false;
    RegisterFile registers_;
};

#endif // CPU_H//
// Created by amrya on 11/11/2026.
//

#ifndef CPU_H
#define CPU_H

#endif //CPU_H
