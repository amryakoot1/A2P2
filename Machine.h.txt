#ifndef MACHINE_H
#define MACHINE_H

#include "CPU.h"
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

class ProgramLoader {
public:
    static std::vector<uint8_t> readFile(const std::string& path);
    static std::vector<uint8_t> parseHex(const std::string& text);
    static void validate(const std::vector<uint8_t>& image, uint8_t start);
private:
    static uint8_t parseByte(const std::string& digits, const std::string& token);
};

class Machine {
public:
    static constexpr std::size_t MAX_STEPS = 10000;

    Machine();
    Machine(const Machine&) = delete;
    Machine& operator=(const Machine&) = delete;

    std::size_t loadProgram(const std::string& path, uint8_t start);
    StepResult step();
    std::size_t run(std::size_t maxSteps = MAX_STEPS);
    void reset();

    void setMemory(uint8_t addr, uint8_t value);
    void setRegister(uint8_t index, uint8_t value);

    const Memory& memory() const { return memory_; }
    const Screen& screen() const { return screen_; }
    const CPU& cpu() const { return cpu_; }

private:
    Memory memory_;
    Screen screen_;
    MemoryBus bus_;
    InstructionFactory factory_;
    CPU cpu_;
};

class ConsoleUI {
public:
    explicit ConsoleUI(Machine& machine);
    void run();
private:
    struct InputClosed {};
    Machine& machine_;

    static std::string trim(const std::string& s);
    std::string readLine(const std::string& prompt);
    unsigned readHex(const std::string& prompt, unsigned maxValue);
    void showMenu() const;
    bool handleChoice(unsigned choice);
    void loadProgram();
    void stepOnce();
    void runToHalt();
    void showState() const;
    void showMemory() const;
    void showScreen() const;
    void editRegister();
    void editMemory();
};

#endif // MACHINE_H