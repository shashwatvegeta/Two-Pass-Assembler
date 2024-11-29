/*
    TITLE: Claims																																
    AUTHOR: Shashwat Kumar Singh
    Declaration of Authorship
*/




#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip>
#include <string>
#include <algorithm>
#include <memory>
#include <sstream>

class CPUEmulator {
private:
    struct MemoryAccess {
        int address;
        int value;
        int newValue;  // Used for writes
    };

    class CPU {
    private:
        int pc = 0;    // Program Counter
        int sp = 0;    // Stack Pointer      
        int a = 0;     // Accumulator A
        int b = 0;     // Accumulator B

    public:
        void reset() { pc = sp = a = b = 0; }
        
        // Getters
        int getPC() const { return pc; }
        int getSP() const { return sp; }
        int getA() const { return a; }
        int getB() const { return b; }
        
        // Setters with state management
        void setPC(int value) { pc = value; }
        void setSP(int value) { sp = value; }
        void setA(int value) { a = value; }
        void setB(int value) { b = value; }
        
        void incrementPC() { pc++; }
    };

    std::vector<int> memory;
    std::vector<MemoryAccess> reads;
    std::vector<MemoryAccess> writes;
    std::vector<int> originalMemory;
    CPU cpu;

    // Helper function to convert decimal to hexadecimal string
    std::string toHex(int value) const {
        std::stringstream ss;
        ss << std::setfill('0') << std::setw(8) << std::hex << (unsigned int)value;
        return ss.str();
    }

    struct Instruction {
        int opcode;
        int operand;

        Instruction(int instruction) {
            opcode = instruction & 0xFF;//8 LSB
            operand = (instruction >> 8) & 0x7FFFFF;//extracts thee next 23 bits
            if (instruction & (1 << 31)) {
                operand -= (1 << 23);
            }//checks for the sign bit and if it is 1 then it subtracts 2^23 from the operand
        }
    };

    int readMemory(int address) {
        int value = memory[address];
        reads.push_back({address, value, 0});
        return value;
    }

    void writeMemory(int address, int value) {
        int oldValue = memory[address];
        memory[address] = value;
        writes.push_back({address, oldValue, value});
    }

public:
    CPUEmulator() = default;

    void loadProgram(const std::string& filename) {
        std::ifstream file(filename, std::ios::binary);
        if (!file) {
            throw std::runtime_error("Unable to open file: " + filename);
        }

        // Clear existing memory
        memory.clear();
        originalMemory.clear();

        // Read program into memory
        int instruction;
        while (file.read(reinterpret_cast<char*>(&instruction), sizeof(int))) {
            memory.push_back(instruction);
        }
        
        // Make a copy for the original memory dump
        originalMemory = memory;
    }

    void executeInstruction(const Instruction& instr) {
        switch (instr.opcode) {
            case 0:  // ldc
                cpu.setB(cpu.getA());
                cpu.setA(instr.operand);
                break;
            case 1:  // adc
                cpu.setA(cpu.getA() + instr.operand);
                break;
            case 2:  // ldl
                cpu.setB(cpu.getA());
                cpu.setA(readMemory(cpu.getSP() + instr.operand));
                break;
            case 3:  // stl
                writeMemory(cpu.getSP() + instr.operand, cpu.getA());
                cpu.setA(cpu.getB());
                break;
            case 4:  // ldnl
                {
                    int addr = cpu.getA() + instr.operand;
                    cpu.setA(readMemory(addr));
                }
                break;
            case 5:  // stnl
                writeMemory(cpu.getA() + instr.operand, cpu.getB());
                break;
            case 6:  // add
                cpu.setA(cpu.getB() + cpu.getA());
                break;
            case 7:  // sub
                cpu.setA(cpu.getB() - cpu.getA());
                break;
            case 8:  // shl
                cpu.setA(cpu.getB() << cpu.getA());
                break;
            case 9:  // shr
                cpu.setA(cpu.getB() >> cpu.getA());
                break;
            case 10: // adj
                cpu.setSP(cpu.getSP() + instr.operand);
                break;
            case 11: // a2sp
                cpu.setSP(cpu.getA());
                cpu.setA(cpu.getB());
                break;
            case 12: // sp2a
                cpu.setB(cpu.getA());
                cpu.setA(cpu.getSP());
                break;
            case 13: // call
                cpu.setB(cpu.getA());
                cpu.setA(cpu.getPC());
                cpu.setPC(cpu.getPC() + instr.operand);
                break;
            case 14: // return
                {
                    int temp = cpu.getA();
                    cpu.setA(cpu.getB());
                    cpu.setPC(temp);
                }
                break;
            case 15: // brz
                if (cpu.getA() == 0) {
                    cpu.setPC(cpu.getPC() + instr.operand);
                }
                break;
            case 16: // brlz
                if (cpu.getA() < 0) {
                    cpu.setPC(cpu.getPC() + instr.operand);
                }
                break;
            case 17: // br
                cpu.setPC(cpu.getPC() + instr.operand);
                break;
            case 18: // halt
                throw std::runtime_error("HALT");
                break;
        }
    }

    void trace() {
        cpu.reset();
        int instructionCount = 0;
        const int MAX_INSTRUCTIONS = 1 << 24;

        try {
            while (cpu.getPC() < memory.size()) {
                if (instructionCount > MAX_INSTRUCTIONS) {
                    throw std::runtime_error("Exceeded maximum instruction count");
                }

                Instruction instr(memory[cpu.getPC()]);
                
                // Execute instruction
                executeInstruction(instr);
                
                // Increment PC and instruction count
                cpu.incrementPC();
                instructionCount++;

                // Print CPU state
                std::cout << "PC= " << toHex(cpu.getPC()) 
                         << " SP= " << toHex(cpu.getSP())
                         << " A= " << toHex(cpu.getA())
                         << " B= " << toHex(cpu.getB()) << "\n";
            }
            
            std::cout << "Total instructions ran are\n" << instructionCount << "\n";
        }
        catch (const std::runtime_error& e) {
            if (std::string(e.what()) != "HALT") {
                std::cout << "Segmentation fault or some other error\n";
                exit(0);
            }
            std::cout << "Total instructions ran are\n" << instructionCount << "\n";
        }
    }

    void showISA() const {
        std::cout << "Opcode Mnemonic Operand\n"
                  << "0      ldc      value\n"
                  << "1      adc      value\n"
                  << "2      ldl      value\n"
                  << "3      stl      value\n"
                  << "4      ldnl     value\n"
                  << "5      stnl     value\n"
                  << "6      add\n"
                  << "7      sub\n"
                  << "8      shl\n"
                  << "9      shr\n"
                  << "10     adj      value\n"
                  << "11     a2sp\n"
                  << "12     sp2a\n"
                  << "13     call     offset\n"
                  << "14     return\n"
                  << "15     brz      offset\n"
                  << "16     brlz     offset\n"
                  << "17     br       offset\n"
                  << "18     HALT\n"
                  << "       SET      value\n";
    }

    void showMemoryDump(bool afterExecution = false) const {
        const auto& memoryToShow = afterExecution ? memory : originalMemory;
        std::cout << "Memory dump " << (afterExecution ? "after" : "before") << " execution:\n";
        
        // Print header
        std::cout << "Address  | Value \n";
        std::cout << "---------+--------\n";
        
        for (size_t i = 0; i < memoryToShow.size(); ++i) {
            // Print address
            std::cout << std::setfill('0') << std::setw(8) << std::hex << (i * 4) << " | ";
            
            // Print value in hex
            std::cout << std::setfill('0') << std::setw(8) << std::hex << memoryToShow[i] << " | ";
            
            // // Print instruction interpretation
            // Instruction instr(memoryToShow[i]);
            // std::cout << "op:" << std::setfill('0') << std::setw(2) << std::hex << instr.opcode;
            // if (instr.opcode <= 5 || instr.opcode == 10 || (instr.opcode >= 13 && instr.opcode <= 17)) {
            //     std::cout << " operand:" << std::setfill('0') << std::setw(6) << std::hex << instr.operand;
            // }
            std::cout << "\n";
        }
        std::cout << std::dec; // Reset to decimal format
    }

    void showMemoryReads() const {
        for (const auto& read : reads) {
            std::cout << "Read memory[" << toHex(read.address) 
                     << "] found " << toHex(read.value) << "\n";
        }
    }

    void showMemoryWrites() const {
        for (const auto& write : writes) {
            std::cout << "Wrote memory[" << toHex(write.address) 
                     << "] was " << toHex(write.value)
                     << " now " << toHex(write.newValue) << "\n";
        }
    }

    void reset() {
        cpu.reset();
        reads.clear();
        writes.clear();
    }
};

int main() {
    CPUEmulator emulator;
    
    std::cout << "Enter the File name for which you want to run emulator for"
              << "(write name exactly with extension)\n";
    std::string filename;
    std::cin >> filename;
    
    try {
        emulator.loadProgram(filename);
        
        while (true) {
            std::cout << "MENU\n"
                      << "1: To get trace\n"
                      << "2: To display ISA\n"
                      << "3: To wipe out all the flags\n"
                      << "4: To show memory dump before execution\n"
                      << "5: To show memory dump after execution\n"
                      << "6: Show memory reads\n"
                      << "7: Show memory writes\n"
                      << "Any number >=8 to exit\n\n"
                      << "Enter the type of instruction which you want to do\n";
            
            int choice;
            std::cin >> choice;
            
            switch (choice) {
                case 1: emulator.trace(); break;
                case 2: emulator.showISA(); break;
                case 3: emulator.reset(); break;
                case 4: emulator.showMemoryDump(false); break;
                case 5: emulator.showMemoryDump(true); break;
                case 6: emulator.showMemoryReads(); break;
                case 7: emulator.showMemoryWrites(); break;
                default: return 0;
            }
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    
    return 0;
}