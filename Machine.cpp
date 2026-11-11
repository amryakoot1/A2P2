#include "Machine.h"
#include <cctype>
#include <fstream>
#include <iostream>
#include <sstream>

std::vector<uint8_t> ProgramLoader::readFile(const std::string& path) {
    std::ifstream in(path);
    if (!in) throw ProgramLoadException("cannot open file '" + path + "'");
    std::stringstream buffer;
    buffer << in.rdbuf();
    return parseHex(buffer.str());
}

std::vector<uint8_t> ProgramLoader::parseHex(const std::string& text) {
    std::vector<uint8_t> bytes;
    std::istringstream lines(text);
    std::string line;
    while (std::getline(lines, line)) {
        const std::size_t comment = line.find_first_of("#;");
        if (comment != std::string::npos) line.erase(comment);
        std::istringstream tokens(line);
        std::string token;
        while (tokens >> token) {
            if (token.size() != 2 && token.size() != 4)
                throw ProgramLoadException("token '" + token + "' must be 2 or 4 hex digits");
            for (std::size_t i = 0; i < token.size(); i += 2)
                bytes.push_back(parseByte(token.substr(i, 2), token));
        }
    }
    return bytes;
}

void ProgramLoader::validate(const std::vector<uint8_t>& image, uint8_t start) {
    if (image.empty())
        throw ProgramLoadException("program file contains no bytes");
    if (image.size() % 2 != 0)
        throw ProgramLoadException("program must consist of whole 2-byte instructions");
    if (start % 2 != 0)
        throw ProgramLoadException("start address must be even");
    if (static_cast<std::size_t>(start) + image.size() > Memory::SIZE)
        throw ProgramLoadException("program does not fit in memory from the start address");
}

uint8_t ProgramLoader::parseByte(const std::string& digits, const std::string& token) {
    for (char c : digits)
        if (!std::isxdigit(static_cast<unsigned char>(c)))
            throw ProgramLoadException("invalid hex token '" + token + "'");
    return static_cast<uint8_t>(std::stoul(digits, nullptr, 16));
}

Machine::Machine()
    : bus_(memory_, screen_),
      factory_(InstructionFactory::createDefault()),
      cpu_(bus_, factory_) {}

std::size_t Machine::loadProgram(const std::string& path, uint8_t start) {
    std::vector<uint8_t> image = ProgramLoader::readFile(path);
    ProgramLoader::validate(image, start);
    reset();
    bus_.loadBlock(image, start);
    cpu_.reset(start);
    return image.size();
}

StepResult Machine::step() { return cpu_.step(); }

std::size_t Machine::run(std::size_t maxSteps) {
    std::size_t executed = 0;
    while (!cpu_.isHalted()) {
        if (executed >= maxSteps) throw ExecutionLimitException(maxSteps);
        cpu_.step();
        ++executed;
    }
    return executed;
}

void Machine::reset() {
    memory_.reset();
    screen_.clear();
    cpu_.reset(0);
}

void Machine::setMemory(uint8_t addr, uint8_t value) { memory_.write(addr, value); }
void Machine::setRegister(uint8_t index, uint8_t value) { cpu_.registers().write(index, value); }

ConsoleUI::ConsoleUI(Machine& machine) : machine_(machine) {}

void ConsoleUI::run() {
    std::cout << "==============================================\n"
              << "  Vole Machine Simulator (CS213 - Assignment 2)\n"
              << "==============================================\n";
    try {
        bool running = true;
        while (running) {
            showMenu();
            const unsigned choice = readHex("Choice: ", 9);
            try {
                running = handleChoice(choice);
            } catch (const VoleException& e) {
                std::cout << "[Error] " << e.what() << "\n";
            }
        }
    } catch (const InputClosed&) {
        std::cout << "\nInput closed.\n";
    }
    std::cout << "Goodbye.\n";
}

std::string ConsoleUI::trim(const std::string& s) {
    const std::size_t first = s.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    const std::size_t last = s.find_last_not_of(" \t\r\n");
    return s.substr(first, last - first + 1);
}

std::string ConsoleUI::readLine(const std::string& prompt) {
    std::cout << prompt << std::flush;
    std::string line;
    if (!std::getline(std::cin, line)) throw InputClosed{};
    return trim(line);
}

unsigned ConsoleUI::readHex(const std::string& prompt, unsigned maxValue) {
    while (true) {
        const std::string text = readLine(prompt);
        try {
            std::size_t used = 0;
            const unsigned long value = std::stoul(text, &used, 16);
            if (used == text.size() && value <= maxValue)
                return static_cast<unsigned>(value);
        } catch (const std::logic_error&) {}
        std::cout << "  Invalid input. Enter a hex value from 0 to "
                  << HexFormatter::digits(maxValue, 1) << ".\n";
    }
}

void ConsoleUI::showMenu() const {
    std::cout << "\n--- MENU ---\n"
              << " 1. Load program file\n"
              << " 2. Step one instruction\n"
              << " 3. Run until HALT\n"
              << " 4. Show CPU state (PC, IR, registers)\n"
              << " 5. Show memory (16x16)\n"
              << " 6. Show screen output\n"
              << " 7. Edit register\n"
              << " 8. Edit memory cell\n"
              << " 9. Reset machine\n"
              << " 0. Exit\n";
}

bool ConsoleUI::handleChoice(unsigned choice) {
    switch (choice) {
    case 1: loadProgram(); break;
    case 2: stepOnce(); break;
    case 3: runToHalt(); break;
    case 4: showState(); break;
    case 5: showMemory(); break;
    case 6: showScreen(); break;
    case 7: editRegister(); break;
    case 8: editMemory(); break;
    case 9:
        machine_.reset();
        std::cout << "Machine reset.\n";
        break;
    case 0: return false;
    default: break;
    }
    return true;
}

void ConsoleUI::loadProgram() {
    const std::string path = readLine("File path: ");
    const unsigned start = readHex("Start address (00-FF, must be even): ", 0xFF);
    const std::size_t bytes = machine_.loadProgram(path, static_cast<uint8_t>(start));
    std::cout << "Loaded " << bytes << " byte(s) at "
              << HexFormatter::prefixed(start, 2) << ". PC set to "
              << HexFormatter::prefixed(start, 2) << ".\n";
}

void ConsoleUI::stepOnce() {
    const StepResult r = machine_.step();
    if (!r.executed) {
        std::cout << "CPU is halted. Load a program or reset the machine.\n";
        return;
    }
    std::cout << HexFormatter::prefixed(r.address, 2) << ": "
              << HexFormatter::digits(r.word.value(), 4) << "  " << r.text << "\n";
}

void ConsoleUI::runToHalt() {
    const std::size_t steps = machine_.run();
    std::cout << "HALT reached after " << steps << " step(s).\n";
    showState();
    showScreen();
}

void ConsoleUI::showState() const {
    const CPU& cpu = machine_.cpu();
    std::cout << "PC   = " << HexFormatter::prefixed(cpu.programCounter(), 2) << "\n"
              << "IR   = " << HexFormatter::digits(cpu.instructionRegister().value(), 4) << "\n"
              << "Halt = " << (cpu.isHalted() ? "yes" : "no") << "\n"
              << "Next = " << cpu.peekNext() << "\n"
              << "Registers:\n";
    const auto regs = cpu.registers().snapshot();
    for (std::size_t i = 0; i < regs.size(); ++i) {
        std::cout << "  " << HexFormatter::reg(static_cast<unsigned>(i))
                  << " = " << HexFormatter::digits(regs[i], 2);
        std::cout << ((i % 8 == 7) ? "\n" : "   ");
    }
}

void ConsoleUI::showMemory() const {
    const auto cells = machine_.memory().snapshot();
    std::cout << "     ";
    for (int col = 0; col < 16; ++col)
        std::cout << "  " << HexFormatter::digits(col, 1) << " ";
    std::cout << "\n";
    for (int row = 0; row < 16; ++row) {
        std::cout << "0x" << HexFormatter::digits(row, 1) << "0 ";
        for (int col = 0; col < 16; ++col)
            std::cout << " " << HexFormatter::digits(cells[row * 16 + col], 2) << " ";
        std::cout << "\n";
    }
}

void ConsoleUI::showScreen() const {
    const Screen& screen = machine_.screen();
    const auto& history = screen.getHistory();
    if (history.empty()) {
        std::cout << "Screen is empty.\n";
        return;
    }
    std::cout << "Screen (hex) : ";
    for (uint8_t v : history) std::cout << HexFormatter::digits(v, 2) << " ";
    std::cout << "\nScreen (text): " << screen.asText() << "\n";
}

void ConsoleUI::editRegister() {
    const unsigned idx = readHex("Register index (0-F): ", 0xF);
    const unsigned val = readHex("New value (00-FF): ", 0xFF);
    machine_.setRegister(static_cast<uint8_t>(idx), static_cast<uint8_t>(val));
    std::cout << HexFormatter::reg(idx) << " <- " << HexFormatter::digits(val, 2) << "\n";
}

void ConsoleUI::editMemory() {
    const unsigned addr = readHex("Address (00-FF): ", 0xFF);
    const unsigned val = readHex("New value (00-FF): ", 0xFF);
    machine_.setMemory(static_cast<uint8_t>(addr), static_cast<uint8_t>(val));
    std::cout << "Memory[" << HexFormatter::prefixed(addr, 2) << "] <- "
              << HexFormatter::digits(val, 2) << "\n";
}