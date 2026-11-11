#include "Instructions.h"
#include <utility>

void BinaryOpInstr::execute(IExecutionContext& ctx) {
    ctx.writeReg(R, compute(ctx.readReg(S), ctx.readReg(T)));
}
std::string BinaryOpInstr::disassemble() const {
    return mnemonic() + " " + HexFormatter::reg(R) + ", " +
           HexFormatter::reg(S) + ", " + HexFormatter::reg(T);
}

void LoadInstr::execute(IExecutionContext& ctx) { ctx.writeReg(R, ctx.readMem(XY)); }
std::string LoadInstr::disassemble() const {
    return "LOAD " + HexFormatter::reg(R) + ", [" + HexFormatter::prefixed(XY, 2) + "]";
}

void LoadImmInstr::execute(IExecutionContext& ctx) { ctx.writeReg(R, XY); }
std::string LoadImmInstr::disassemble() const {
    return "LOADI " + HexFormatter::reg(R) + ", " + HexFormatter::prefixed(XY, 2);
}

void StoreInstr::execute(IExecutionContext& ctx) { ctx.writeMem(XY, ctx.readReg(R)); }
std::string StoreInstr::disassemble() const {
    return "STORE " + HexFormatter::reg(R) + ", [" + HexFormatter::prefixed(XY, 2) + "]";
}

void MoveInstr::execute(IExecutionContext& ctx) { ctx.writeReg(S, ctx.readReg(R)); }
std::string MoveInstr::disassemble() const {
    return "MOVE " + HexFormatter::reg(R) + ", " + HexFormatter::reg(S);
}

void RotateInstr::execute(IExecutionContext& ctx) {
    ctx.writeReg(R, ALU::rotateRight(ctx.readReg(R), X));
}
std::string RotateInstr::disassemble() const {
    return "ROTR " + HexFormatter::reg(R) + ", " + std::to_string(X);
}

void JumpInstr::execute(IExecutionContext& ctx) {
    if (ctx.readReg(R) == ctx.readReg(0)) ctx.setPC(XY);
}
std::string JumpInstr::disassemble() const {
    return "JUMP " + HexFormatter::reg(R) + ", " + HexFormatter::prefixed(XY, 2);
}

void HaltInstr::execute(IExecutionContext& ctx) { ctx.halt(); }
std::string HaltInstr::disassemble() const { return "HALT"; }

void InstructionFactory::registerOp(uint8_t opcode, Creator creator) {
    creators_[opcode] = std::move(creator);
}
std::unique_ptr<Instruction> InstructionFactory::create(InstructionWord word) const {
    auto it = creators_.find(word.opcode());
    if (it == creators_.end()) throw InvalidOpcodeException(word.opcode());
    return it->second(word);
}
bool InstructionFactory::isRegistered(uint8_t opcode) const { return creators_.count(opcode) > 0; }

InstructionFactory InstructionFactory::createDefault() {
    InstructionFactory f;
    f.registerType<LoadInstr>(0x1);
    f.registerType<LoadImmInstr>(0x2);
    f.registerType<StoreInstr>(0x3);
    f.registerType<MoveInstr>(0x4);
    f.registerType<AddIntInstr>(0x5);
    f.registerType<AddFloatInstr>(0x6);
    f.registerType<OrInstr>(0x7);
    f.registerType<AndInstr>(0x8);
    f.registerType<XorInstr>(0x9);
    f.registerType<RotateInstr>(0xA);
    f.registerType<JumpInstr>(0xB);
    f.registerType<HaltInstr>(0xC);
    return f;
}//
// Created by amrya on 11/11/2026.
//
