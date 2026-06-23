#include <iostream>
#include <cstdint>
#include <fstream>
#include <sstream>
#include <vector>
#include <queue>

using namespace std;

struct Instruction {
    int count;
    string operat;
    string opr1;
    string opr2;
};

class Commands {
    protected:
        Instruction inst;

    public:
        virtual void execute() = 0;

};

class SingleOperand : public Commands {
    public: 
        SingleOperand(Instruction t){ inst = t; }
        void execute () {
            cout << "One operand command" << endl;
            cout << inst.operat << " : " << inst.opr1 << endl;
        }
};

class DoubleOperand : public Commands {
    public: 
        DoubleOperand(Instruction t){ inst = t; }
        void execute () {
            cout << "Two operand command" << endl;
            cout << inst.operat << " : " << inst.opr1 << " : "  << inst.opr2 << endl;
        }
};


string removeComma(string str){
    string str2 = "";
    for (int i = 0; i < str.length(); i++){
        if (str[i] != ',')
            str2.push_back(str[i]);
    }
    return str2;
};


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

        cout << endl;
    }
    input.close();

    queue<Commands *> prg;  
    for ( auto &x : PROGRAM ){
        if (x.count == 3)
            prg.push(new DoubleOperand(x));
        else
            prg.push(new SingleOperand(x));

    }

    while (!prg.empty()){
        prg.front()->execute();
        prg.pop();
    }

    return 0;

};