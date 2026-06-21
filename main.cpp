#include <iostream>
#include <cstdint>
using namespace std;

 // 8 data register //

class Register {
private:
    int8_t value; 

public:

    Register() : value(0) {}

    int8_t getValue() const {
        return value;
    }

    void setValue(int8_t newValue) {
        value = newValue;
    }
};

class GeneralRegister : public Register {

};

class RegisterBank {
private:

    GeneralRegister registers[8];

    bool isValidIndex(int index) const {
        return (index >= 0 && index <= 7);
    }

public:

    int8_t readRegister(int index) const {
        if (!isValidIndex(index)) {
            return 0; 
        }
        return registers[index].getValue();
    }

    void writeRegister(int index, int8_t value) {
        if (isValidIndex(index)) {
            registers[index].setValue(value);
        }
    }

    void printBank() const {
        for (int i = 0; i < 8; ++i) {
           
            cout << "R" << i << ": " << static_cast<int>(registers[i].getValue()) << "\n";
        }
    }
};

int main() {
    RegisterBank regBank;

    regBank.writeRegister(0, 127);  
    regBank.writeRegister(1, -128); 
    regBank.writeRegister(5, 44);   

    cout << "--- Current State of General-Purpose Registers ---\n";
    regBank.printBank();

    return 0;
}