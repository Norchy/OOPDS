#include <iostream>
#include <cstdint>
using namespace std;

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

class STACK { 
    // must complete can use try and catch
    int8_t data[8];
    int top;
    public:
    STACK(){
        top = -1;
        for (int i = 0; i < 8; i++){
            data[i] = 0;
        }
    }

    void push(int8_t value){
        top++;
        data[top] = value;
    }

    int8_t pop(){
        int8_t temp = data[top];
        top--;
        return temp;
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
    bool Z, C, O, U;

    public:
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
        Flags Flags; // Class composition
        STACK Stack;  // Class composition


        uint8_t PC; // need to do intialize
        uint8_t SI; // need to do intialize
    
    public:
        void barrier() { cout << "----------------------------" << endl; }

        void intro () { cout << "-------- Virtual Machine --------" << endl; }


        VirtualMachine() {
            Dreg.initReg();
            Flags.initFlags();
        }


        void incPC(){ PC++; } // Incrementing program counter

        // Dumping output for Program Counter and Data Registers
        void dump () {
            intro();

            cout << "R : ";
            for (int i = 0; i < 8; i++)
                cout << " R[" << i << "] = " << (int)Dreg.getReg(i) << ",";
            cout << endl;
            
            barrier();
            cout << "PC =" << (int)PC << endl;
        }

        void Runner(){
            srand(time(0));
            incPC();
            incPC();
            incPC();
            for (int i = 0; i < 8; i++)
                Dreg.setReg(i, rand() % 256 - 128);
            
            dump();
        }
    };

int main() {
    VirtualMachine vm;
    vm.Runner();

    return 0;
};