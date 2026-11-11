#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

#include "Hardware.h"
#include <functional>
#include <map>
#include <memory>
#include <string>

class IExecutionContext {
public:
    virtual ~IExecutionContext() = default;
    virtual uint8_t readReg(uint8_t i) const = 0;
    virtual void writeReg(uint8_t i, uint8_t v) = 0;
    virtual uint8_t readMem(uint8_t addr) const = 0;
    virtual void writeMem(uint8_t addr, uint8_t v) = 0;
    virtual uint16_t getPC() const = 0;
    virtual void setPC(uint8_t addr) = 0;
    virtual void halt() = 0;
};

class Instruction {
public:
    explicit Instruction(InstructionWord w) : word(w) {}
    virtual ~Instruction() = default;
    uint8_t opcode() const { return word.opcode(); }
    virtual void execute(IExecutionContext& ctx) = 0;
    virtual std::string disassemble() const = 0;
protected:
    InstructionWord word;
};

class RXYInstr : public Instruction {
public:
    explicit RXYInstr(InstructionWord w) : Instruction(w), R(w.nibble(1)), XY(w.lowByte()) {}
protected:
    uint8_t R;
    uint8_t XY;
};

class RSTInstr : public Instruction {
public:
    explicit RSTInstr(InstructionWord w)
        : Instruction(w), R(w.nibble(1)), S(w.nibble(2)), T(w.nibble(3)) {}
protected:
    uint8_t R, S, T;
};

class BinaryOpInstr : public RSTInstr {
public:
    explicit BinaryOpInstr(InstructionWord w) : RSTInstr(w) {}
    void execute(IExecutionContext& ctx) override;
    std::string disassemble() const override;
protected:
    virtual uint8_t compute(uint8_t a, uint8_t b) const = 0;
    virtual std::string mnemonic() const = 0;
};

class LoadInstr : public RXYInstr {
public:
    explicit LoadInstr(InstructionWord w) : RXYInstr(w) {}
    void execute(IExecutionContext& ctx) override;
    std::string disassemble() const override;
};

class LoadImmInstr : public RXYInstr {
public:
    explicit LoadImmInstr(InstructionWord w) : RXYInstr(w) {}
    void execute(IExecutionContext& ctx) override;
    std::string disassemble() const override;
};

class StoreInstr : public RXYInstr {
public:
    explicit StoreInstr(InstructionWord w) : RXYInstr(w) {}
    void execute(IExecutionContext& ctx) override;
    std::string disassemble() const override;
};

class MoveInstr : public Instruction {
public:
    explicit MoveInstr(InstructionWord w) : Instruction(w), R(w.nibble(2)), S(w.nibble(3)) {}
    void execute(IExecutionContext& ctx) override;
    std::string disassemble() const override;
private:
    uint8_t R, S;
};

class AddIntInstr : public BinaryOpInstr {
public:
    explicit AddIntInstr(InstructionWord w) : BinaryOpInstr(w) {}
protected:
    uint8_t compute(uint8_t a, uint8_t b) const override { return ALU::addInt(a, b); }
    std::string mnemonic() const override { return "ADDI"; }
};

class AddFloatInstr : public BinaryOpInstr {
public:
    explicit AddFloatInstr(InstructionWord w) : BinaryOpInstr(w) {}
protected:
    uint8_t compute(uint8_t a, uint8_t b) const override { return ALU::addFloat(a, b); }
    std::string mnemonic() const override { return "ADDF"; }
};

class OrInstr : public BinaryOpInstr {
public:
    explicit OrInstr(InstructionWord w) : BinaryOpInstr(w) {}
protected:
    uint8_t compute(uint8_t a, uint8_t b) const override { return ALU::bitOr(a, b); }
    std::string mnemonic() const override { return "OR"; }
};

class AndInstr : public BinaryOpInstr {
public:
    explicit AndInstr(InstructionWord w) : BinaryOpInstr(w) {}
protected:
    uint8_t compute(uint8_t a, uint8_t b) const override { return ALU::bitAnd(a, b); }
    std::string mnemonic() const override { return "AND"; }
};

class XorInstr : public BinaryOpInstr {
public:
    explicit XorInstr(InstructionWord w) : BinaryOpInstr(w) {}
protected:
    uint8_t compute(uint8_t a, uint8_t b) const override { return ALU::bitXor(a, b); }
    std::string mnemonic() const override { return "XOR"; }
};

class RotateInstr : public Instruction {
public:
    explicit RotateInstr(InstructionWord w) : Instruction(w), R(w.nibble(1)), X(w.nibble(3)) {}
    void execute(IExecutionContext& ctx) override;
    std::string disassemble() const override;
private:
    uint8_t R, X;
};

class JumpInstr : public RXYInstr {
public:
    explicit JumpInstr(InstructionWord w) : RXYInstr(w) {}
    void execute(IExecutionContext& ctx) override;
    std::string disassemble() const override;
};

class HaltInstr : public Instruction {
public:
    explicit HaltInstr(InstructionWord w) : Instruction(w) {}
    void execute(IExecutionContext& ctx) override;
    std::string disassemble() const override;
};

class InstructionFactory {
public:
    using Creator = std::function<std::unique_ptr<Instruction>(InstructionWord)>;
    void registerOp(uint8_t opcode, Creator creator);
    std::unique_ptr<Instruction> create(InstructionWord word) const;
    bool isRegistered(uint8_t opcode) const;
    static InstructionFactory createDefault();
private:
    template <typename T>
    void registerType(uint8_t opcode) {
        registerOp(opcode, [](InstructionWord w) -> std::unique_ptr<Instruction> {
            return std::make_unique<T>(w);
        });
    }
    std::map<uint8_t, Creator> creators_;
};

#endif // INSTRUCTIONS_H