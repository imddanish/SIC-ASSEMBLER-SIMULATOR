#include <bits/stdc++.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
using namespace std;

const string datastart(".DATA");
const string codestart(".CODE");
int length;
int dataLength;
int programCounter = 0;
int memoryPointer = 0;
bool ERROR_FLAG = false;
bool first = true;
//bool STRING_COMPARE_FLAG = false;
int COMPARE_FLAG = 0; // 0 : equal ; 1 : greater ; -1 : less

struct Program {
	string label;
	string opcode;
	string operand;
} program[200];

struct Data {
	string varname;
	bool reserve;
	bool word;
	bool hex;
	char value[100];
} data[100];

struct Register {
	char value[100];
} reg[4];	// 0 : A , 1 : X , 2 : S, 3 : B

struct Memory {
	int number;
}mem[1000];


map<string, int> SYMTAB;

void saveLine(string &line, int pos) {
	int len = line.size();
	program[pos].label = "";
	program[pos].opcode = "";
	program[pos].operand = "";
	int i = 0;
	while(line[i] == ' ')
		i++;
	for(;i < len; i++) {
		if(line[i] != ' ')
			program[pos].label += line[i];
		else
			break;
	}
	while(line[i] == ' ')
		i++;
	for(;i < len; i++) {
		if(line[i] != ' ')
			program[pos].opcode += line[i];
		else
			break;
	}
	while(line[i] == ' ')
		i++;
	for(;i < len; i++) {
		if(line[i] != ' ')
			program[pos].operand += line[i];
		else
			break;
	}
	if(program[pos].label != "NULL" && SYMTAB.find(program[pos].label) == SYMTAB.end())
		SYMTAB[program[pos].label] = pos;
	else if(program[pos].label == "NULL") ;
	else
		ERROR_FLAG = true;
}

void printLines() {
	for(int i = 0; i < length; i++) {
		cout<<program[i].label<<"\t"<<program[i].opcode<<"\t"<<program[i].operand<<endl;
	}
	cout<<endl<<endl;
	for(map<string,int>:: iterator it = SYMTAB.begin(); it != SYMTAB.end(); it++)
		cout<<it->first<<"\t"<<it->second<<endl;
}

void printErrorMessage(int line) {
	cout<<"Error in line number "<<line<<endl;
}

void saveDataLine(string &line) {
	int len = line.size();
	int count = 0;
	string op("");
	char oper[100];
	data[dataLength].varname = "";
	data[dataLength].reserve = false;
	data[dataLength].hex = false;
	string val;
	int i = 0;
	while(line[i] == ' ')
		i++;
	for(;i < len; i++) {
		if(line[i] != ' ')
			data[dataLength].varname += line[i];
		else
			break;
	}
	while(line[i] == ' ')
		i++;
	for(;i < len; i++) {
		if(line[i] != ' ') {
			op += line[i];
		}
		else
			break;
	}
	while(line[i] == ' ')
		i++;
	if(op != "WORD") {
		for(;i < len; i++) {
			if(line[i] != ' ') {
				oper[count++] = line[i];
			}
			else {
				oper[count] = '\0';
				break;
			}
		}
		data[dataLength].word = false;
	}
	else {
		i++;
		for(;i < len; i++) {
			if(line[i] != '"') {
				oper[count++] = line[i];
			}
			else {
				oper[count] = '\0';
				break;
			}
		}
		strcpy(data[dataLength].value,oper);
		data[dataLength].word = true;
	}
	if(op == "RESB" || op == "RESW") {
		data[dataLength].reserve = true;
	}
	if(op != "WORD" && oper[count - 1] == 'H' || oper[count - 1] == 'h') {
		data[dataLength].hex = true;
	}
	if(op != "WORD" && data[dataLength].reserve == false && data[dataLength].hex == true) {
		oper[count - 1] = '\0';
		count--;
		strcpy(data[dataLength].value,oper);
	}
	if(op != "WORD" && data[dataLength].reserve == false && data[dataLength].hex == false) {
		strcpy(data[dataLength].value,oper);
	}
}

void getDataLines(ifstream &opcode) {
	string line;
	dataLength = 0;
	while(getline(opcode, line)) {
		while(line == "\n" || line == ".DATA") {
			getline(opcode,line);
		}
		saveDataLine(line);
		dataLength++;
	}
}

void printDataLines() {
	/*if(first == true) {
		ofstream outfile("Substringpre.txt");
		for(int i = 0; i < dataLength; i++) {
			string out(data[i].value);
			outfile<<data[i].varname<<";"<<out<<endl;
		}
	}
	else {
		ofstream outfile("Substringpost.txt");
		for(int i = 0; i < dataLength; i++) {
			string out(data[i].value);
			outfile<<data[i].varname<<";"<<out<<endl;
		}
	}*/
	for(int i = 0; i < dataLength; i++) {
		first = false;
		string out(data[i].value);
		cout<<data[i].varname<<"\t"<<data[i].reserve<<"\t"<<data[i].hex<<"\t"<<out<<endl;
	}
	/*
	for(int i = 0; i < dataLength; i++) {
		int number = atoi(data[i].value);
		char buffer[100];
		itoa(number,buffer,16);
		string out(buffer);
		cout<<data[i].varname<<"\t"<<data[i].reserve<<"\t"<<data[i].hex<<"\t"<<out<<endl;
	}*/
}

void displayMemory() {
	cout<<endl<<endl;
	cout<<"MEMORY STATUS"<<endl;
	cout<<"-------------"<<endl;
	for (int i = 0; i < memoryPointer; ++i) {
		cout<<"Location : "<<(i + 1)<<"\t:\t"<<mem[i].number<<endl;
	}
}

void PASS1(char *filename) {
	ifstream opcode(filename);
	string line;
	int cur_code = 0;
	if(opcode.is_open()) {
		getline(opcode, line);
		if(line == datastart || line == "END") {
			return;
		}
		if(line == codestart) {
			while(getline(opcode, line)) {
				if(line == datastart || line == "END")
					break;
				saveLine(line,cur_code);
				if(ERROR_FLAG) {
					printErrorMessage(cur_code);
					return;
				}
				cur_code ++;

			}
		}
	}
	length = cur_code;
	//printLines();
	getDataLines(opcode);
	printDataLines();
}

bool numberSort(const struct Memory &lhs, const struct Memory &rhs) {
	if(lhs.number > rhs.number)
		return false;
	else
		return true;
}

void checkError() {
	if(ERROR_FLAG == true) {
		cout<<"error in code"<<endl;
		//exit(0);
	}
}

void ftoa(float number) {
	sprintf(reg[0].value,"%f",number);
}

int getDataPosition(string &operand) {
	//const char *point;
	//point = operand.c_str();
	for(int i = 0; i < dataLength; i++) {
		if(operand == data[i].varname)
			return i;
	}
	//cout<<operand<<endl;
	ERROR_FLAG = true;
	return -1;
}

void LDA() {
	int pos = getDataPosition(program[programCounter].operand);
	checkError();
	strcpy(reg[0].value,data[pos].value);
}

void LDX() {
	int pos = getDataPosition(program[programCounter].operand);
	checkError();
	strcpy(reg[1].value,data[pos].value);
}

void LDS() {
	int pos = getDataPosition(program[programCounter].operand);
	checkError();
	int accumulator = atoi(reg[0].value);
	int index = atoi(reg[1].value);
	char newstring[100];
	strcpy(newstring,data[pos].value);
	strncpy(reg[2].value,newstring + index, accumulator);
}

void MOVAS() {
	strcpy(reg[0].value,reg[2].value);
}

void MOVSA() {
	strcpy(reg[2].value,reg[0].value);
}

void MOVAX() {
	strcpy(reg[0].value,reg[1].value);
}

void MOVXA() {
	strcpy(reg[1].value,reg[0].value);
}

void MOVAB() {
	strcpy(reg[0].value,reg[3].value);
}

void MOVBA() {
	strcpy(reg[3].value,reg[0].value);
}

void TIX() {
	int number = atoi(reg[1].value);
	number++;
	itoa(number,reg[1].value,10);
}

void JEQ(string &label) {
	if(COMPARE_FLAG == 0) {
		programCounter = SYMTAB[label];
	}
	else
		programCounter++;
}

void JLT(string &label) {
	if(COMPARE_FLAG == -1) {
		programCounter = SYMTAB[label];
	}
	else
		programCounter++;
}

void JGT(string &label) {
	if(COMPARE_FLAG == 1) {
		programCounter = SYMTAB[label];
	}
	else
		programCounter++;
}

void J(string &label) {
	programCounter = SYMTAB[label];
}

void STA() {
	int pos = getDataPosition(program[programCounter].operand);
	checkError();
	strcpy(data[pos].value,reg[0].value);
}

void STX() {
	int pos = getDataPosition(program[programCounter].operand);
	checkError();
	strcpy(data[pos].value,reg[1].value);
}

void ADD() {
	int pos = getDataPosition(program[programCounter].operand);
	checkError();
	int addend = atoi(data[pos].value);
	int accumulator = atoi(reg[0].value);
	int sum = addend + accumulator;
	itoa(sum,reg[0].value,10);
}

void SUB() {
	int pos = getDataPosition(program[programCounter].operand);
	checkError();
	int subtrahend = atoi(data[pos].value);
	int accumulator = atoi(reg[0].value);
	int difference = accumulator - subtrahend;
	itoa(difference,reg[0].value,10);
}

void MUL() {
	int pos = getDataPosition(program[programCounter].operand);
	checkError();
	int multiplicand = atoi(data[pos].value);
	int accumulator = atoi(reg[0].value);
	int product = accumulator * multiplicand;
	itoa(product,reg[0].value,10);
}

void ADDF() {
	int pos = getDataPosition(program[programCounter].operand);
	checkError();
	float addend = atof(data[pos].value);
	float accumulator = atof(reg[0].value);
	float sum = accumulator + addend;
	ftoa(sum);
}

void SUBF() {
	int pos = getDataPosition(program[programCounter].operand);
	checkError();
	float subtrahend = atof(data[pos].value);
	float accumulator = atof(reg[0].value);
	float difference = accumulator - subtrahend;
	ftoa(difference);
}

void MULF() {
	int pos = getDataPosition(program[programCounter].operand);
	checkError();
	float multiplicand = atof(data[pos].value);
	float accumulator = atof(reg[0].value);
	float product = accumulator * multiplicand;
	ftoa(product);
}

void CMP() {
	int pos = getDataPosition(program[programCounter].operand);//COMPARE_FLAG = strcmp(reg[1].value, data[pos].value);
	int reg_val = atoi(reg[1].value);
	int data_val = atoi(data[pos].value);
	//cout<<reg_val<<"\t"<<reg_val<<endl;
	COMPARE_FLAG = reg_val <= data_val ? (reg_val == data_val ? 0 : -1) : 1;
}

void CMPS() {
	COMPARE_FLAG = 1;
	int pos = getDataPosition(program[programCounter].operand);
	//printf("%s\t %s",reg[2].value,data[pos].value);
	COMPARE_FLAG = strcmp(reg[2].value, data[pos].value);
}

void READ() {
	int read;
	cin>>read;
	mem[memoryPointer++].number = read;
	itoa(read,reg[3].value,10);
}

void WRITE() {
	int pos = getDataPosition(program[programCounter].operand);
	printf("%s", data[pos].value);
}

void SORT() {
	int pos = getDataPosition(program[programCounter].operand);
	int startVal = atoi(data[pos].value);
	sort(mem + startVal, mem + memoryPointer, &numberSort);
}

void executeCode() {
	while(true) {
		bool normal_code = true;
		if(program[programCounter].opcode == "LDA") {
			LDA();
		}
		else if(program[programCounter].opcode == "LDX") {
			LDX();
		}
		else if(program[programCounter].opcode == "LDS") {
			LDS();
		}
		else if(program[programCounter].opcode == "MOV" && program[programCounter].operand == "S,A") {
			MOVSA();
		}
		else if(program[programCounter].opcode == "MOV" && program[programCounter].operand == "A,S") {
			MOVAS();
		}
		else if(program[programCounter].opcode == "MOV" && program[programCounter].operand == "A,X") {
			MOVAX();
		}
		else if(program[programCounter].opcode == "MOV" && program[programCounter].operand == "X,A") {
			MOVXA();
		}
		else if(program[programCounter].opcode == "MOV" && program[programCounter].operand == "A,B") {
			MOVAB();
		}
		else if(program[programCounter].opcode == "MOV" && program[programCounter].operand == "B,A") {
			MOVBA();
		}
		else if(program[programCounter].opcode == "TIX") {
			TIX();
		}
		else if(program[programCounter].opcode == "JEQ") {
			JEQ(program[programCounter].operand);
			normal_code = false;
		}
		else if(program[programCounter].opcode == "JLT") {
			JLT(program[programCounter].operand);
			normal_code = false;
		}
		else if(program[programCounter].opcode == "J") {
			J(program[programCounter].operand);
			normal_code = false;
		}
		else if(program[programCounter].opcode == "JGT") {
			JGT(program[programCounter].operand);
			normal_code = false;
		}
		else if(program[programCounter].opcode == "STA") {
			STA();
		}
		else if(program[programCounter].opcode == "STX") {
			STX();
		}
		else if(program[programCounter].opcode == "ADD") {
			ADD();
		}
		else if(program[programCounter].opcode == "ADDF") {
			ADDF();
		}
		else if(program[programCounter].opcode == "SUB") {
			SUB();
		}
		else if(program[programCounter].opcode == "SUBF") {
			SUBF();
		}
		else if(program[programCounter].opcode == "MUL") {
			MUL();
		}
		else if(program[programCounter].opcode == "MULF") {
			MULF();
		}
		else if(program[programCounter].opcode == "CMP") {
			CMP();
		}
		else if(program[programCounter].opcode == "CMPS") {
			CMPS();
		}
		else if(program[programCounter].opcode == "READ") {
			READ();
		}
		else if(program[programCounter].opcode == "WRITE") {
			WRITE();
		}
		else if(program[programCounter].opcode == "SORT") {
			SORT();
		}
		if(programCounter == length)
			break;
		if(normal_code)
			programCounter++;
	}
}


int main() {
	char filename[100];
	cout<<"Enter the Filename : ";
	scanf("%s",filename);
	PASS1(filename);
	executeCode();
	cout<<"Data after Loader : "<<endl<<endl;
	printDataLines();
	displayMemory();
}
