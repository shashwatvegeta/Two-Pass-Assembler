/*
Name- Shashwat Kumar Singh
Roll No.- 2301CS50
*/
#include <bits/stdc++.h>
using namespace std;

// Custom types for better readability
using LineNumber = int;
using Address = int;
using Message = string;
using Label = string;

// Instruction structure
struct Instruction {
    string label;
    string mnemonic;
    string operand;
    
    Instruction(string l = "", string m = "", string o = ""): 
        label(l), mnemonic(m), operand(o) {}
};

// Output file entry structure
struct OutputEntry {
    string address;
    string machineCode;
    string text;
};

// Core assembler class
class Assembler {
private:
    // State management
    map<string, pair<string, int>> opcodeTable;
    map<LineNumber, Message> errorLog;
    map<LineNumber, Message> warningLog;
    map<Label, pair<Address, LineNumber>> symbolTable; //stores the address of the label
    map<Label, vector<LineNumber>> labelReferences;//stores the line number where the label is used
    map<Label, string> setInstructions; //stores the value of the variable in SET instruction
    vector<string> machineCode; //stores the machine code
    vector<OutputEntry> listingOutput;//stores the output in the format required for lst file
    vector<string> sourceLines;//stores the source code
    map<Address, Instruction> programMemory;
    
    // Utility functions
    class TokenValidator {
    public:
        bool isValidLabel(const string& str) {
            if(str.empty() || !isalpha(str[0]) && str[0] != '_') return false;
            return all_of(str.begin(), str.end(), 
                         [](char c) { return isalnum(c) || c == '_'; });
        }
        
        bool isDecimal(const string& str) {
            if(str.empty()) return false;
            if(str[0] == '0' && str.length() > 1) return false;
            return all_of(str.begin(), str.end(), ::isdigit);
        }
        
        bool isOctal(const string& str) {
            if(str.empty() || str[0] != '0') 
                return false;
            return all_of(str.begin() + 1, str.end(), [](char c) { return c >= '0' && c <= '7'; });
        }

        
        bool isHexadecimal(const string& str) {
            if(str.length() < 3 || str[0] != '0' || (str[1] != 'x' && str[1] != 'X')) 
                return false;
            return all_of(str.begin() + 2, str.end(), 
                         [](char c) { return isxdigit(c); });
        }
    };
    
    class NumberConverter {
    public:
        string toDecimal(const string& num) {
            if(num.empty()) return "0";
            
            // Check if it's an octal number
            if(num.length() >= 2 && num[0] == '0' ) {
                return to_string(stoi(num.substr(1), nullptr, 8));
            }
            // Check if it's a hexadecimal number
            else if(num.length() >= 3 && num[0] == '0' && (num[1] == 'x' || num[1] == 'X')) {
                return to_string(stoi(num.substr(2), nullptr, 16));
            }
            // Decimal number
            else {
                return to_string(stoi(num));
            }
        }
        
        string toHex(const string& value) {
            int decimalValue = stoi(toDecimal(value));
            stringstream ss;
            ss << setfill('0') << setw(8) << hex << static_cast<unsigned int>(decimalValue);
            return ss.str();
        }

        string toHex(int value) {
            stringstream ss;
            ss << setfill('0') << setw(8) << hex << static_cast<unsigned int>(value);
            return ss.str();
        }
    };

    
    TokenValidator validator;
    NumberConverter converter;
    
    vector<string> tokenizeLine(const string& line) {
        vector<string> tokens;
        stringstream ss(line);
        string token;
        
        while(ss >> token) {
            if(token[0] == ';') break;
            
            auto colonPos = token.find(':');
            if(colonPos != string::npos && token.back() != ':') {
                tokens.push_back(token.substr(0, colonPos + 1));
                token = token.substr(colonPos + 1);
            }
            
            if(token.back() == ';') {
                token.pop_back();
                tokens.push_back(token);
                break;
            }
            
            tokens.push_back(token);
        }
        return tokens;
    }
    
    void initializeOpcodes() {
        vector<tuple<string, string, int>> opcodeInit = {
            make_tuple("data", "", 2), make_tuple("ldc", "00", 2), 
            make_tuple("adc", "01", 2), make_tuple("ldl", "02", 3), 
            make_tuple("stl", "03", 3), make_tuple("ldnl", "04", 3),
            make_tuple("stnl", "05", 3), make_tuple("add", "06", 1), 
            make_tuple("sub", "07", 1), make_tuple("shl", "08", 1), 
            make_tuple("shr", "09", 1), make_tuple("adj", "0a", 2),
            make_tuple("a2sp", "0b", 1), make_tuple("sp2a", "0c", 1), 
            make_tuple("call", "0d", 3), make_tuple("return", "0e", 1), 
            make_tuple("brz", "0f", 3), make_tuple("brlz", "10", 3),
            make_tuple("br", "11", 3), make_tuple("HALT", "12", 1), 
            make_tuple("SET", "", 2)
        };
        
        for(const auto& opcode : opcodeInit) {
            string mnem, code;
            int type;
            tie(mnem, code, type) = opcode;
            opcodeTable[mnem] = make_pair(code, type);
        }
    }
    
public:
    Assembler() {
        initializeOpcodes();
    }
    
    void loadSource(const string& filename) {
        ifstream file(filename);
        string line;
        while(getline(file, line)) {
            sourceLines.push_back(line);
        }
    }
    
    void firstPass() {
        Address currentAddress = 0;
        
        for(LineNumber lineNum = 0; lineNum < sourceLines.size(); lineNum++) {
            auto tokens = tokenizeLine(sourceLines[lineNum]);
            if(tokens.empty()) continue;
            
            size_t tokenIndex = 0;
            Instruction inst;
            
            // Parse label
            if(tokens[tokenIndex].back() == ':') {
                inst.label = tokens[tokenIndex];
                inst.label.pop_back();
                tokenIndex++;
                
                if(!validator.isValidLabel(inst.label)) {
                    errorLog[lineNum + 1] = "Invalid label name";
                    continue;
                }
                
                if(symbolTable.count(inst.label) && symbolTable[inst.label].first != -1) {
                    errorLog[lineNum + 1] = "Duplicate label definition";
                    continue;
                }
                
                symbolTable[inst.label] = make_pair(currentAddress, lineNum + 1);
            }
            
            // Parse mnemonic and operand
            if(tokenIndex < tokens.size()) {
                inst.mnemonic = tokens[tokenIndex++];
                
                if(tokenIndex < tokens.size()) {
                    inst.operand = tokens[tokenIndex++];
                }
                
                if(tokenIndex < tokens.size()) {
                    errorLog[lineNum + 1] = "Extra operands on line";
                }
                
                if(!opcodeTable.count(inst.mnemonic)) {
                    errorLog[lineNum + 1] = "Invalid mnemonic";
                    continue;
                }
                
                if(opcodeTable[inst.mnemonic].second > 1 && inst.operand.empty()) {
                    errorLog[lineNum + 1] = "Missing operand";
                    continue;
                }
                
                
                // Handle SET instruction
                if(inst.mnemonic == "SET") {
                    if(inst.label.empty()) {
                        errorLog[lineNum + 1] = "SET instruction requires label";
                        continue;
                    }
                    setInstructions[inst.label] = inst.operand;
                }
                
                if (!inst.operand.empty()) {
                    if (!validator.isValidLabel(inst.operand)) {
                        if (!validator.isDecimal(inst.operand) &&
                            !validator.isOctal(inst.operand) &&
                            !validator.isHexadecimal(inst.operand)) {
                            errorLog[lineNum + 1] = "Invalid operand: " + inst.operand;
                            continue;
                        }
                    }
                }

                
                programMemory[currentAddress] = inst;
                currentAddress++;
            }
        }
        
        // Verify label usage
        for(const auto& symbol : symbolTable) {
            const string& label = symbol.first;
            const auto& info = symbol.second;
            if(info.first == -1) {
                for(auto lineNum : labelReferences[label]) {
                    errorLog[lineNum] = "Undefined label: " + label;
                }
            }
            else if(!labelReferences.count(label)) {
                warningLog[info.second] = "Unused label: " + label;
            }
        }
    }
    
        void secondPass() {
        if(!errorLog.empty()) return;
        
        for(const auto& memory : programMemory) {
            const auto& addr = memory.first;
            const auto& inst = memory.second;
            
            string mcode = "        ";
            int type = opcodeTable[inst.mnemonic].second;
            
            if(type == 1) {
                mcode = "000000" + opcodeTable[inst.mnemonic].first;
            }
            else if(type == 2) {
                if(inst.mnemonic == "data" || inst.mnemonic == "SET") {
                    mcode = converter.toHex(inst.operand);  // Updated to use new toHex method
                }
                else {
                    string value;
                    if(setInstructions.count(inst.operand)) {
                        value = converter.toHex(setInstructions[inst.operand]);  // Handle SET values
                    }
                    else if(symbolTable.count(inst.operand)) {
                        value = converter.toHex(symbolTable[inst.operand].first);  // Handle labels
                    }
                    else {
                        value = converter.toHex(inst.operand);  // Handle direct numbers
                    }
                    mcode = value.substr(2) + opcodeTable[inst.mnemonic].first;
                }
            }
            else if(type == 3) {
                int offset;
                if(symbolTable.count(inst.operand)) {
                    offset = symbolTable[inst.operand].first - (addr + 1);
                }
                else {
                    offset = stoi(converter.toDecimal(inst.operand));  // Handle branch offsets
                }
                mcode = converter.toHex(offset).substr(2) + opcodeTable[inst.mnemonic].first;
            }
            
            machineCode.push_back(mcode);
            
            string text = inst.label;
            if(!text.empty()) text += ": ";
            text += inst.mnemonic;
            if(!inst.operand.empty()) text += " " + inst.operand;
            
            listingOutput.push_back({converter.toHex(addr), mcode, text});
        }
    }
    
    void writeOutput(const string& baseName) {
        // Write log file
        ofstream logFile(baseName + ".log");
        if(errorLog.empty()) {
            logFile << "Error nhi hai.\n";
            for(const auto& warn : warningLog) {
                logFile << "Line Number " << warn.first << " WARNING: " << warn.second << "\n";
            }
        }
        else {
            for(const auto& err : errorLog) {
                logFile << "Line Number " << err.first << " ERROR: " << err.second << "\n";
            }
        }
        logFile.close();
        
        if(!errorLog.empty()) return;
        
        // Write listing file
        ofstream lstFile(baseName + ".lst");
        for(const auto& entry : listingOutput) {
            lstFile << entry.address << " " << entry.machineCode << " " << entry.text << "\n";
        }
        lstFile.close();
        
        // Write object file
        ofstream objFile(baseName + ".o", ios::binary);
        for(const auto& code : machineCode) {
            if(code.empty() || code == "        ") continue;
            unsigned int value = stoul(code, nullptr, 16);
            objFile.write(reinterpret_cast<const char*>(&value), sizeof(value));
        }
        objFile.close();
    }
};

int main(int argc, char* argv[]) {
    if(argc != 2) {
        cout << "Usage: " << argv[0] << " <input_file.asm>" << endl;
        return 1;
    }
    
    string inputFile = argv[1];
    string baseName = inputFile.substr(0, inputFile.find_last_of('.'));
    
    Assembler assembler;
    
    try {
        assembler.loadSource(inputFile);
        assembler.firstPass();
        assembler.secondPass();
        assembler.writeOutput(baseName);
    }
    catch(const exception& e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }
    
    return 0;
}