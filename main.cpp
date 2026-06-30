#include <iostream>
#include <cstdint>
#include <fstream>
#include <sstream>


using namespace std;


class VirtualMachine;

struct Instruction {
    int count;
    string operat;
    string opr1;
    string opr2;
    Instruction() : count(0), operat(""), opr1(""), opr2("") {} // needed so InstructionArray can create blank Instruction objects internally

};

//InstructionArray class
// custom dynamic array that starts with capacity 16, doubles in size whenever it is full



class InstructionArray {
    Instruction* data;
    int capacity;
    int size;

    void resize(){
        capacity *= 2;
        Instruction* newData = new Instruction[capacity];
        for (int i = 0; i < size; i++)
            newData[i] = data[i];
        delete[] data;
        data = newData;
    }

    public:
        InstructionArray() : capacity(16), size(0){
            data = new Instruction[capacity];
        }

        ~InstructionArray(){
            delete[] data;
        }

        void push_back(const Instruction& inst){
            if (size >= capacity) resize();
            data[size++] = inst;
        }

        Instruction& get(int index){ return data[index]; }

        int getSize() const { return size; }
};

class Commands {
    protected:
        Instruction inst;

    public:
        virtual void execute(VirtualMachine& vm) = 0;
        virtual ~Commands() {}
};

// a single node in the linked list used by CommandQueue below

struct QueueNode {
    Commands* data;
    QueueNode* next;
    QueueNode(Commands* d) : data(d), next(nullptr) {}
};

// CommandQueue class
// custom linked-list queue that supports push, pop, getFront, empty, and auto-cleanup


class CommandQueue {
    private:
        QueueNode* front;
        QueueNode* rear;
        int count;

    public:
        CommandQueue() : front(nullptr), rear(nullptr), count(0) {}

        ~CommandQueue(){
            while (!empty()){
                QueueNode* temp = front;
                front = front->next;
                delete temp;
            }
        }

        bool empty() const {
            return front == nullptr;
        }

        void push(Commands* cmd){
            QueueNode* node = new QueueNode(cmd);
            if (rear == nullptr){
                front = rear = node;
            } else {
                rear->next = node;
                rear = node;
            }
            count++;
        }

        Commands* getFront(){ return front->data; }

        void pop(){
            if (empty()) return;
            QueueNode* temp = front;
            front = front->next;
            if (front == nullptr) rear = nullptr;
            delete temp;
            count--;
        }
};

class SingleOperand : public Commands {
    public:
        SingleOperand(Instruction t){ inst = t; }
        void execute (VirtualMachine& vm) override;
};

class DoubleOperand : public Commands {
    public:
        DoubleOperand(Instruction t){ inst = t; }
        void execute (VirtualMachine& vm) override;

};

string removeComma(string str){
    string str2 = "";
    for (size_t i = 0; i < str.length(); i++){
        if (str[i] != ',')
            str2.push_back(str[i]);
    }
    return str2;
}

// strips [ and ] from a token, e.g. "[R1]" becomes "R1", "[20]" becomes "20"
// used when parsing memory operands in MOV, LOAD, STORE

string removeBrackets(string str){
    string str2 = "";
    for (size_t i = 0; i < str.length(); i++){
        if (str[i] != '[' && str[i] != ']')
            str2.push_back(str[i]);
    }
    return str2;
}

// returns true if a token contains '[', indicating it is a memory reference
bool hasBrackets(const string& str){
    return str.find('[') != string::npos;
}



 // 8 data register //
 class DataRegisters {
    int8_t R[8];

    public:
        DataRegisters() { initReg(); }

        void initReg() {
            for (int i = 0; i < 8; i++){
                R[i] = 0;
            }
        }

        int getIndex(const string& regStr) {
            if (regStr.length() == 2 && regStr[0] == 'R' && regStr[1] >= '0' && regStr[1] <= '7') {
                return regStr[1] - '0';
            }
            return -1;
        }

        int8_t getReg(int num) {
            if (num >= 0 && num < 8)
                return R[num];
            else {
                cerr << "Unknown Register" << endl;
                return -1;
            }
        }

        void setReg(int num, int8_t value){
            if (num >= 0 && num < 8)
                R[num] = value;
            else {
                cerr << "Unknown Register" << endl;
            }
        }
 };

class STACKERROR {
    private:
        const char* errormsg;

    public:
        explicit STACKERROR(const char* msg) : errormsg(msg) {}

        const char* what() const {
            return errormsg;
        }
};

class STACK {
    private:
        static const int MAX_STACK = 8;
        int8_t data[MAX_STACK];
        int top;

    public:
    STACK(){
        top = -1;
        for (int i = 0; i < 8; i++){
            data[i] = 0;
        }
    }

    bool isFull() const {
        return top >= (MAX_STACK - 1);
    }

    bool empty() const {
        return top == -1;
    }

    void push(int8_t value){
        if (isFull()){
            throw STACKERROR("ERROR: Stack is full");
        }
        top++;
        data[top] = value;
    }

    int8_t pop(){
        if (empty()) {
            cerr << "ERROR: Stack is empty";
            exit(1);
        }

        int8_t temp = data[top];
        top--;
        return temp;
    }

    uint8_t getSIValue() const {
        return static_cast<uint8_t>(top + 1);
    }
};

// ADDED by [Azim]   : getMemory(), setMemory(), dumpMemory() methods
 class Memory {
    int8_t MEM[64];
    public:
        Memory(){
            for ( int i = 0; i < 64; i++){
                MEM[i] = (int8_t)0;
            }
        }

    // reads one signed byte from memory at the given address
    // returns 0 and prints error if address is out of range 0-63
    int8_t getMemory(int addr){
        if (addr < 0 || addr > 63){
            cerr << "Memory address out of range: " << addr << endl;
            return 0;
        }
        return MEM[addr];
    }

    // writes one signed byte to memory at the given address
    // prints error and does nothing if address is out of range 0-63
    void setMemory(int addr, int8_t value){
        if (addr < 0 || addr > 63){
            cerr << "Memory address out of range: " << addr << endl;
            return;
        }
        MEM[addr] = value;
    }

    // prints all 64 memory bytes in the required output format
    // 8 values per row, each formatted as 4 digits e.g. #0044# or #-005#
    void dumpMemory(){
        cout << "#Memory#" << endl;
        for (int row = 0; row < 8; row++){
            for (int col = 0; col < 8; col++){
                int val = (int)MEM[row * 8 + col];
                if (val >= 0){
                    if (val < 10)        cout << "#000" << val;
                    else if (val < 100)  cout << "#00"  << val;
                    else                 cout << "#0"   << val;
                }
                else {
                    int absVal = -val;
                    if (absVal < 10)       cout << "#-00" << absVal;
                    else if (absVal < 100) cout << "#-0"  << absVal;
                    else                   cout << "#-"   << absVal;
                }
            }
            cout << "#" << endl;
        }
    }
 };

 // Class Flags (Implemented by Megat)
 class Flags {
       public:
        bool Z, C, O, U;

        Flags () {
            initFlags();
        }
        void initFlags() {
            Z = U = O = C = false;
        }

        // called after every arithmetic operation with the raw integer result
        // OF set if result > 127, UF set if result < -128
        // CF set if result is out of 8-bit range (same condition as OF or UF)
        // ZF set if the truncated 8-bit result equals zero
        void updateFlags(int result){
            initFlags();
            if (result > 127)                  O = true;
            if (result < -128)                 U = true;
            if (result > 127 || result < -128) C = true;
            if ((int8_t)result == 0)           Z = true;
        }

        // resets a single named flag to false, used by the RESET instruction
        // e.g. RESET CF sets C = false without touching the other flags
        void resetFlag(const string& flagName){
            if      (flagName == "CF") C = false;
            else if (flagName == "OF") O = false;
            else if (flagName == "UF") U = false;
            else if (flagName == "ZF") Z = false;
            else cerr << "Unknown flag: " << flagName << endl;
        }
 };

 class VirtualMachine {
    private:
        DataRegisters Dreg; // Class composition
        Memory Mem; // Class composition
        Flags flags; // Class composition
        STACK Stack;  // Class composition

        uint8_t PC;

    public:
        void barrier() { cout << "----------------------------" << endl; }

        void intro () { cout << "-------- Virtual Machine --------" << endl; }


        VirtualMachine() : PC(0) {
            Dreg.initReg();
            flags.initFlags();
        }

        DataRegisters& getRegisters() { return Dreg; }
        Memory& getMemory() { return Mem; }
        Flags& getFlags() { return flags; }
        STACK& getStack() { return Stack; }

        uint8_t getSI() const {
            return Stack.getSIValue();
        }

        uint8_t getPC() const { return PC; }
        void incPC(){ PC++; }

        // Function dump()      (Implemented by Megat)
        void dump () {
            cout << "#Begin#" << endl;

            cout << "#Registers#";
            for (int i = 0; i < 8; i++) {
                int val = (int)Dreg.getReg(i);

                if (val >= 0) {
                    if (val < 10) cout << "000" << val << "#";
                    else if (val < 100) cout << "00" << val << "#";
                    else cout << "0" << val << "#";
                }
                else {
                    int absVal = -val;
                    if (absVal < 10) cout << "-00" << absVal << "#";
                    else if (absVal < 100) cout << "-0" << absVal << "#";
                    else cout << "-" << absVal << "#";
                }
            }
            cout << endl;

            cout << "#Flags#OF#" << flags.O << "#UF#" << flags.U << "#CF#" << flags.C << "#ZF#" << flags.Z << "#" << endl;

            cout << "#PC#";
            int pcVal = (int)PC;
            if (pcVal < 10) cout << "000" << pcVal;
            else if (pcVal < 100) cout << "00" << pcVal;
            else cout << "0" << pcVal;
            cout << "#" << endl;

            Mem.dumpMemory();

            cout << "#End#" << endl;
        }
        
        // CHANGED by [Azim] : Runner parameter changed from queue<Commands*> to CommandQueue
        // CHANGED by [Azim] : programQueue.front() changed to programQueue.getFront()
        // because custom CommandQueue uses getFront() not front()
        // everything else inside Runner is identical to [Luqman]'s original
        // Runner: the main execution loop of the virtual machine
        // Takes the queue of parsed Commands (built in main()) and executes them
        // one at a time, in the same order they appeared in the .asm file (FIFO)
        void Runner(CommandQueue& programQueue) {
            try {
                while (!programQueue.empty()) {
                    // Get the next instruction waiting at the front of the queue
                    // without removing it yet
                    Commands* currentCmd = programQueue.getFront();
                    // Remove that instruction from the queue now that we have a copy of the pointer
                    programQueue.pop();

                    // Run the instruction — this calls either SingleOperand::execute()
                    // or DoubleOperand::execute() depending on the actual object type
                    // (polymorphism: the correct override is chosen automatically at runtime)
                    currentCmd->execute(*this);

                    // Move the program counter forward by 1 to reflect that
                    // one more instruction has just finished executing
                    this->incPC();

         
                    dump();
                    // Clean up the current command
                    delete currentCmd;
                 }
            }  
            // If a PUSH is attempted while the stack is already full,
            // STACK::push() throws a STACKERROR — this catch block stops
            // the program safely and reports the error instead of crashing silently
            catch (const STACKERROR& e) {
                cerr << "Stack Error:  " << e.what() << endl;
                exit(1);
            }
        }
 };

// SingleOperand::execute
//[Luqman] wrote the INPUT, DISPLAY, PUSH, POP block and the regIdx lookup 
// ADDED by [Azim]   : string op variable at the top for cleaner comparisons
// ADDED by [Azim]   : RESET handler at the top (before regIdx check)
// ADDED by [Azim]   : INC handler
// ADDED by [Azim]   : DEC handler
void SingleOperand::execute(VirtualMachine& vm) {
    // local variable for cleaner if comparisons
    string op = inst.operat;

    //  RESET instruction
    // resets one specific flag to false without touching the others
    // e.g. RESET CF --> C = false
    if (op == "RESET"){
        vm.getFlags().resetFlag(inst.opr1);
        return;
    }

    int regIdx = vm.getRegisters().getIndex(inst.opr1);

    // INC instruction
    // adds 1 to the destination register and updates all flags
    if (op == "INC" && regIdx != -1){
        int result = (int)vm.getRegisters().getReg(regIdx) + 1;
        vm.getFlags().updateFlags(result);
        vm.getRegisters().setReg(regIdx, (int8_t)result);
        return;
    }

    // DEC instruction
    // subtracts 1 from the destination register and updates all flags
    if (op == "DEC" && regIdx != -1){
        int result = (int)vm.getRegisters().getReg(regIdx) - 1;
        vm.getFlags().updateFlags(result);
        vm.getRegisters().setReg(regIdx, (int8_t)result);
        return;
    }

    if (regIdx != -1) {
        if (inst.operat == "INPUT") {
            cout << "?" << endl;
            long long inputVal;
            if (cin >> inputVal) {
                vm.getFlags().initFlags();
                if (inputVal > 127) vm.getFlags().O = true;
                if (inputVal < -128) vm.getFlags().U = true;
                if (inputVal == 0) vm.getFlags().Z = true;
                vm.getRegisters().setReg(regIdx, static_cast<int8_t>(inputVal));
            }
        }
        else if (inst.operat == "DISPLAY") {
            cout << (int)vm.getRegisters().getReg(regIdx) << endl;
        }
        else if (inst.operat == "PUSH") {
            int8_t val = vm.getRegisters().getReg(regIdx);
            vm.getStack().push(val);
        }
        else if (inst.operat == "POP") {
            int8_t val = vm.getStack().pop();
            vm.getRegisters().setReg(regIdx, val);
        }
    } else {
        cout << "One operand command" << endl;
        cout << inst.operat << " : " << inst.opr1 << endl;
    }
}

// ===========================================================
// Function: DoubleOperand::execute

// Implemented instruction groups:
//   - MOV (3.4)        : Megat
//   - ADD/SUB/MUL/DIV  : Megat
//   - ROL/ROR (3.7)    : Megat
//   - SHL/SHR (3.8)    : Megat
//   - LOAD/STORE (3.9) : Azim

// ===========================================================
void DoubleOperand::execute(VirtualMachine& vm) {
    string op   = inst.operat;
    string opr1 = inst.opr1;
    string opr2 = inst.opr2;

    DataRegisters& regs  = vm.getRegisters();
    Memory&        mem   = vm.getMemory();
    Flags&         flags = vm.getFlags();

    // ---- MOV ---- (3.4) Alan
    // mode 1: MOV R0, 10    -- load immediate value into register
    // mode 2: MOV R0, R1    -- copy register to register
    // mode 3: MOV R3, [R1]  -- read from memory address stored in R1
    if (op == "MOV"){
        int destIdx = regs.getIndex(opr1);
        if (destIdx == -1){ cerr << "MOV: bad destination " << opr1 << endl; return; }

        if (hasBrackets(opr2)){
            // MOV R3, [R1] -- read from memory at address stored in R1
            string inner = removeBrackets(opr2);
            int srcIdx = regs.getIndex(inner);
            if (srcIdx != -1){
                int addr = (int)(uint8_t)regs.getReg(srcIdx);
                regs.setReg(destIdx, mem.getMemory(addr));
            } else {
                cerr << "MOV: bad source register in brackets" << endl;
            }
        }
        else if (regs.getIndex(opr2) != -1){
            // MOV R0, R1 -- register to register
            regs.setReg(destIdx, regs.getReg(regs.getIndex(opr2)));
        }
        else {
            // MOV R0, 10 -- immediate value
            int val = stoi(opr2);
            regs.setReg(destIdx, (int8_t)val);
        }
        return;
    }

    // ---- ADD ---- (3.5)  Implemented by Megat
    // adds source to destination, stores in destination, updates flags
    if (op == "ADD"){
        int destIdx = regs.getIndex(opr1);
        int srcIdx  = regs.getIndex(opr2);
        if (destIdx == -1){ cerr << "ADD: bad register" << endl; return; }
        int srcVal = (srcIdx != -1) ? (int)regs.getReg(srcIdx) : stoi(opr2);
        int result = (int)regs.getReg(destIdx) + srcVal;
        flags.updateFlags(result);
        regs.setReg(destIdx, (int8_t)result);
        return;
    }

    // ---- SUB ---- (3.5) Implemented by Megat
    // subtracts source from destination, stores in destination, updates flags
    if (op == "SUB"){
        int destIdx = regs.getIndex(opr1);
        int srcIdx  = regs.getIndex(opr2);
        if (destIdx == -1){ cerr << "SUB: bad register" << endl; return; }
        int srcVal = (srcIdx != -1) ? (int)regs.getReg(srcIdx) : stoi(opr2);
        int result = (int)regs.getReg(destIdx) - srcVal;
        flags.updateFlags(result);
        regs.setReg(destIdx, (int8_t)result);
        return;
    }

    // ---- MUL ---- (3.5) Implemented by Megat
    // multiplies destination by source, stores in destination, updates flags
    if (op == "MUL"){
        int destIdx = regs.getIndex(opr1);
        int srcIdx  = regs.getIndex(opr2);
        if (destIdx == -1){ cerr << "MUL: bad register" << endl; return; }
        int srcVal = (srcIdx != -1) ? (int)regs.getReg(srcIdx) : stoi(opr2);
        int result = (int)regs.getReg(destIdx) * srcVal;
        flags.updateFlags(result);
        regs.setReg(destIdx, (int8_t)result);
        return;
    }

    // ---- DIV ---- (3.5) Implemented by Megat
    // divides destination by source, stores in destination, updates flags
    // exits with error if source is zero to prevent division by zero
    if (op == "DIV"){
        int destIdx = regs.getIndex(opr1);
        int srcIdx  = regs.getIndex(opr2);
        if (destIdx == -1){ cerr << "DIV: bad register" << endl; return; }
        int srcVal = (srcIdx != -1) ? (int)regs.getReg(srcIdx) : stoi(opr2);
        if (srcVal == 0){ cerr << "DIV: division by zero" << endl; exit(1); }
        int result = (int)regs.getReg(destIdx) / srcVal;
        flags.updateFlags(result);
        regs.setReg(destIdx, (int8_t)result);
        return;
    }

    // ---- ROL ---- (3.7) Implemented by Megat
    // rotates bits of destination left by count positions
    // bits shifted out from the left wrap around to the right
    if (op == "ROL"){
        int destIdx = regs.getIndex(opr1);
        if (destIdx == -1){ cerr << "ROL: bad register" << endl; return; }
        int count = stoi(opr2) % 8;
        uint8_t val = (uint8_t)regs.getReg(destIdx);
        uint8_t result = (val << count) | (val >> (8 - count));
        regs.setReg(destIdx, (int8_t)result);
        return;
    }

    // ---- ROR ---- (3.7) Implemented by Megat
    // rotates bits of destination right by count positions
    // bits shifted out from the right wrap around to the left
    if (op == "ROR"){
        int destIdx = regs.getIndex(opr1);
        if (destIdx == -1){ cerr << "ROR: bad register" << endl; return; }
        int count = stoi(opr2) % 8;
        uint8_t val = (uint8_t)regs.getReg(destIdx);
        uint8_t result = (val >> count) | (val << (8 - count));
        regs.setReg(destIdx, (int8_t)result);
        return;
    }

    // ---- SHL ---- (3.8) Implemented by Megat
    // shifts bits of destination left by count positions
    // vacated bit positions on the right are filled with 0
    // shifting 8 or more times results in 0
    if (op == "SHL"){
        int destIdx = regs.getIndex(opr1);
        if (destIdx == -1){ cerr << "SHL: bad register" << endl; return; }
        int count = stoi(opr2);
        uint8_t val = (uint8_t)regs.getReg(destIdx);
        uint8_t result = (count >= 8) ? 0 : (val << count);
        regs.setReg(destIdx, (int8_t)result);
        return;
    }

    // ---- SHR ---- (3.8) Implemented by Megat
    // shifts bits of destination right by count positions
    // vacated bit positions on the left are filled with 0
    // shifting 8 or more times results in 0
    if (op == "SHR"){
        int destIdx = regs.getIndex(opr1);
        if (destIdx == -1){ cerr << "SHR: bad register" << endl; return; }
        int count = stoi(opr2);
        uint8_t val = (uint8_t)regs.getReg(destIdx);
        uint8_t result = (count >= 8) ? 0 : (val >> count);
        regs.setReg(destIdx, (int8_t)result);
        return;
    }

    // ---- LOAD ---- (3.9) Implemented by Azim
    // loads a value from memory into a destination register
    // LOAD R1, [20]  -- reads from fixed memory address 20
    // LOAD R1, [R2]  -- reads from address stored in register R2
    if (op == "LOAD") {
        int destIdx = regs.getIndex(opr1);
        if (destIdx == -1) { cerr << "LOAD: bad destination register" << endl; return; }
        if (!hasBrackets(opr2)) { cerr << "LOAD: missing brackets in " << opr2 << endl; return; }

        string inner = removeBrackets(opr2);
        int srcRegIdx = regs.getIndex(inner);
        int addr;
        if (srcRegIdx != -1) {
            // LOAD R1, [R2]
            addr = (int)(uint8_t)regs.getReg(srcRegIdx);
        }
        else {
            // LOAD R1, [20]
            addr = stoi(inner);
        }
        regs.setReg(destIdx, mem.getMemory(addr));
        return;
    }

    // ---- STORE ---- (3.9) Implemented by Azim
    // stores a register value into memory
    // STORE R1, 43    -- stores R1 into fixed memory address 43
    // STORE R1, [R2]  -- stores R1 into the address found in R2
    if (op == "STORE") {
        int regIdxOpr1 = regs.getIndex(opr1);
        int regIdxOpr2 = regs.getIndex(opr2);

        if (regIdxOpr1 != -1) {
            // STORE R1, 43   or   STORE R1, [R2]
            int8_t val = regs.getReg(regIdxOpr1);
            if (hasBrackets(opr2)) {
                // STORE R1, [R2]
                string inner = removeBrackets(opr2);
                int addrRegIdx = regs.getIndex(inner);
                if (addrRegIdx == -1) { cerr << "STORE: bad address register" << endl; return; }
                int addr = (int)(uint8_t)regs.getReg(addrRegIdx);
                mem.setMemory(addr, val);
            }
            else {
                // STORE R1, 43
                int addr = stoi(opr2);
                mem.setMemory(addr, val);
            }
        }
        else {
            // STORE 20, R3   -- address first, register second
            if (regIdxOpr2 == -1) { cerr << "STORE: bad source register" << endl; return; }
            int addr = stoi(opr1);
            mem.setMemory(addr, regs.getReg(regIdxOpr2));
        }
        return;
    }

    cerr << "Unknown double-operand instruction: " << op << endl;

}

int main() {

    InstructionArray PROGRAM; // CHANGED by Azim: was "vector<Instruction> PROGRAM"
                               // replaced std::vector with a custom dynamic array
                               // since vector is not allowed in this assignment
    ifstream input ("assembly.asm");
    if (input.fail()){
        cerr << "File not found.";
        exit(1);
    }

    string line;
    while (getline(input, line)) {
        stringstream str(line);

        string word;
        int count = 0;
        Instruction inst;
        while (str >> word){
            word = removeComma(word);
            if (count == 0)
                inst.operat = word;
            else if (count == 1)
                inst.opr1 = word;
            else if (count == 2)
                inst.opr2 = word;
            count ++;
        }
        inst.count = count;

        // ADDED by Azim: if the line was empty (count == 0), skip it
        // so blank lines in the .asm file don't get pushed as invalid instructions
        if (count == 0) continue;  

        PROGRAM.push_back(inst);
    }
    input.close();

    // CHANGED by Azim: was "for (auto &x : PROGRAM)" — range-based for loop
    // doesn't work with the custom InstructionArray, so switched to an
    // index-based loop using PROGRAM.getSize() and PROGRAM.get(i) instead

    CommandQueue prg;
    for (int i = 0; i < PROGRAM.getSize(); i++){
        Instruction& x = PROGRAM.get(i);
        if (x.count == 3)
            prg.push(new DoubleOperand(x));
        else
            prg.push(new SingleOperand(x));
}


    VirtualMachine vm;
    vm.Runner(prg);

    return 0;
}
