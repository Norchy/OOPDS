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
    Instruction() : count(0), operat(""), opr1(""), opr2("") {}
};

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

struct QueueNode {
    Commands* data;
    QueueNode* next;
    QueueNode(Commands* d) : data(d), next(nullptr) {}
};

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

string removeBrackets(string str){
    string str2 = "";
    for (size_t i = 0; i < str.length(); i++){
        if (str[i] != '[' && str[i] != ']')
            str2.push_back(str[i]);
    }
    return str2;
}

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
        for (int i = 0; i < MAX_STACK; i++){
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
        return static_cast<uint8_t> (top + 1);
    }
};

 class Memory {
    int8_t MEM[64];
    public:
        Memory(){
            for ( int i = 0; i < 64; i++){
                MEM[i] = (int8_t)0;
            }
        }
 };

 class Flags {
    public:
        bool Z, C, O, U;

        Flags () {
            initFlags();
        }
        void initFlags() {
            Z = U = O = C = false;
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
        void barrier() { cout << "---------------------------------" << endl; }
        void intro () { cout << "-------- Virtual Machine --------" << endl; }

        VirtualMachine() : PC(0){
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

        void dump () {
            intro();

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
        }


        void Runner(queue<Commands*>& programQueue) {
            try {
                while (!programQueue.empty()) {
                    Commands* currentCmd = programQueue.front();
                    programQueue.pop();


                    currentCmd->execute(*this);

                    this->incPC();


                    dump();
                    delete currentCmd;
                 }
            }
            catch (const STACKERROR& e) {
                cerr << "Stack Error:  " << e.what() << endl;
                exit(1);
            }
        }
 };

void SingleOperand::execute(VirtualMachine& vm) {
    int regIdx = vm.getRegisters().getIndex(inst.opr1);
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

int main() {

    vector<Instruction> PROGRAM;
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

        PROGRAM.push_back(inst);
    }
    input.close();

    queue<Commands *> prg;
    for ( auto &x : PROGRAM ){
        if (x.count == 3)
            prg.push(new DoubleOperand(x));
        else
            prg.push(new SingleOperand(x));
    }


    VirtualMachine vm;
    vm.Runner(prg);

    return 0;
}
