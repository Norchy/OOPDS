#include <iostream>
#include <cstdint>
#include <fstream>
#include <sstream>
using namespace std;

string removeComma(string str){
    string str2 = "";
    for (int i = 0; i < str.length(); i++){
        if (str[i] != ',')
            str2.push_back(str[i]);
    }
    return str2;
};


int main() {
    ifstream input ("assembly.asm");
    if (input.fail()){
        cerr << "File not found.";
        exit(1);
    }

    string line;
    while (getline(input, line)) {
        stringstream str(line);

        string word;
        while (str >> word){
            word = removeComma(word);
            cout << word << ":";
        }
        cout << endl;
    }
    input.close();
    return 0;

};