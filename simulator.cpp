#include "simulator.h"
#include "ui_simulator.h"

#include<QFileDialog>
#include<QFile>
#include <QMessageBox>
#include <QTextStream>
#include <bits/stdc++.h>
#include<QDoubleSpinBox>
#include<QLineEdit>
#include<QApplication>

using namespace std;

const string datastart(".DATA");
const string codestart(".CODE");
int length;
int dataLength;
int programCounter = 0;
int memoryPointer = 0;
char message[20];
bool ERROR_FLAG = false;
int ERROR_LINE = -1;
bool OPCODE_ERROR = false;
bool first = true;
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
} dat[100];

struct Register {
    char value[100];
} reg[4];	// 0 : A , 1 : X , 2 : S, 3 : B

struct Memory {
    int number;
}mem[1000];


map<string, int> SYMTAB;

void saveLine(string &line, int pos) {  // This function is for saving code in the structure called "program"
    int len = line.size();
    program[pos].label = "";
    program[pos].opcode = "";
    program[pos].operand = "";  // Initialization of "program" structure
    int i = 0;
    while(line[i] == ' ')   // Breaking one line of code into labels, opcode and operand
        i++;
    for(;i < len; i++) {
        if(line[i] != ' ')
            program[pos].label += line[i];  // Breaking Labels
        else
            break;
    }
    while(line[i] == ' ')
        i++;
    for(;i < len; i++) {
        if(line[i] != ' ')
            program[pos].opcode += line[i]; // Breaking opcode
        else
            break;
    }
    while(line[i] == ' ')
        i++;
    for(;i < len; i++) {
        if(line[i] != ' ')
            program[pos].operand += line[i];    // Breaking operand
        else
            break;
    }
    if(program[pos].label != "NULL" && SYMTAB.find(program[pos].label) == SYMTAB.end()) // Checking new label is NULL or available in Symtab
        SYMTAB[program[pos].label] = pos;   // Entry in Symtab of new label found
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
    ERROR_LINE = line;
    sprintf(message,"%d",line);
    cout<<"Error in line number "<<line<<endl;
}

void saveDataLine(string &line) {   // This function is for saving data part of the program in the structure called "dat"
    int len = line.size();
    int count = 0;
    string op("");
    char oper[100];
    dat[dataLength].varname = "";
    dat[dataLength].reserve = false;
    dat[dataLength].hex = false;    // Initialization of variables of "dat" structure
    string val;
    int i = 0;
    while(line[i] == ' ')
        i++;
    for(;i < len; i++) {
        if(line[i] != ' ')
            dat[dataLength].varname += line[i]; // Breaking the variable name portion and saving it in the dat[ ].varname
        else
            break;
    }
    while(line[i] == ' ')
        i++;
    for(;i < len; i++) {
        if(line[i] != ' ') {
            op += line[i];  // Breaking the opcode of the data portion and saving it in the "op"  string
        }
        else
            break;
    }
    while(line[i] == ' ')
        i++;
    if(op != "WORD") {  // Checking if the variable is not "Word"
        for(;i < len; i++) {
            if(line[i] != ' ') {
                oper[count++] = line[i];
            }
            else {
                oper[count] = '\0';
                break;
            }
        }
        dat[dataLength].word = false;   // if not word then dat[ ].word = false
    }
    else {  // if it is Word
        i++;
        for(;i < len; i++) {
            if(line[i] != '"') {
                oper[count++] = line[i];    // taking the value of the operand
            }
            else {
                oper[count] = '\0';
                break;
            }
        }
        strcpy(dat[dataLength].value,oper); // copying the value of data in dat[ ].value
        dat[dataLength].word = true;
    }
    if(op == "RESB" || op == "RESW") {  // if reserve then dat[ ].reserve is true
        dat[dataLength].reserve = true;
    }
    if((op != "WORD" && oper[count - 1] == 'H') || oper[count - 1] == 'h') {
        dat[dataLength].hex = true; //if data is in hex we set dat[ ].hex to true
    }
    if(op != "WORD" && dat[dataLength].reserve == false && dat[dataLength].hex == true) {
        oper[count - 1] = '\0';
        count--;    // not taking 'h'
        strcpy(dat[dataLength].value,oper);
    }
    if(op != "WORD" && dat[dataLength].reserve == false && dat[dataLength].hex == false) {
        strcpy(dat[dataLength].value,oper);
    }
}

void getDataLines(ifstream &opcode) {   // This function takes lines of the data part and call savedataline() for breaking data
    string line;
    dataLength = 0;
    while(getline(opcode, line)) {
        while(line == "\n" || line == ".DATA") {    // Skipping the empty linesor .data linesafter code to get the data portion
            getline(opcode,line);
        }
        saveDataLine(line); // calling this function tobreak data into parts
        dataLength++;
    }
}

void printDataLines() {
    for(int i = 0; i < dataLength; i++) {
        first = false;
        string out(dat[i].value);
        cout<<dat[i].varname<<"\t"<<dat[i].reserve<<"\t"<<dat[i].hex<<"\t"<<out<<endl;
    }
}

void displayMemory() {
    cout<<endl<<endl;
    cout<<"MEMORY STATUS"<<endl;
    cout<<"-------------"<<endl;
    for (int i = 0; i < memoryPointer; ++i) {
        cout<<"Location : "<<(i + 1)<<"\t:\t"<<mem[i].number<<endl;
    }
}

void PASS1(char *filename) {    //Pass 1
    ifstream opcode(filename);
    string line;
    int cur_code = 0;
    if(opcode.is_open()) {
        getline(opcode, line);  //get each line of the code
        if(line == datastart || line == "END") {
            return;
        }
        if(line == codestart) {
            while(getline(opcode, line)) {
                if(line == datastart || line == "END")  // if a line of a code is not End or Not the datastart then "Saveline"
                    break;
                saveLine(line,cur_code);
                cur_code ++;
            }   // End of while , all instructions are broken into parts
        }
    }
    length = cur_code;
    getDataLines(opcode);   // End of Pass 1
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
        printErrorMessage(programCounter);
    }
}

void ftoa(float number) {
    sprintf(reg[0].value,"%f",number);
}

int getDataPosition(string &operand) {
    for(int i = 0; i < dataLength; i++) {
        if(operand == dat[i].varname)
            return i;
    }
    ERROR_FLAG = true;
    return -1;
}

void LDA() {
    int pos = getDataPosition(program[programCounter].operand);
    checkError();
    strcpy(reg[0].value,dat[pos].value);
}

void LDX() {
    int pos = getDataPosition(program[programCounter].operand);
    strcpy(reg[1].value,dat[pos].value);
}

void LDS() {
    int pos = getDataPosition(program[programCounter].operand);
    checkError();
    int accumulator = atoi(reg[0].value);
    int index = atoi(reg[1].value);
    char newstring[100];
    strcpy(newstring,dat[pos].value);
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

    sprintf(reg[1].value,"%d",number);
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
    strcpy(dat[pos].value,reg[0].value);
    printf("sta : %s \n",dat[pos].value);
}

void STX() {
    int pos = getDataPosition(program[programCounter].operand);
    checkError();
    strcpy(dat[pos].value,reg[1].value);
}

void ADD() {
    int pos = getDataPosition(program[programCounter].operand);
    checkError();
    int addend = atoi(dat[pos].value);
    int accumulator = atoi(reg[0].value);
    int sum = addend + accumulator;
    sprintf(reg[0].value,"%d",sum);
}

void SUB() {
    int pos = getDataPosition(program[programCounter].operand);
    checkError();
    int subtrahend = atoi(dat[pos].value);
    int accumulator = atoi(reg[0].value);
    int difference = accumulator - subtrahend;
    sprintf(reg[0].value,"%d",difference);
}

void MUL() {
    int pos = getDataPosition(program[programCounter].operand);
    checkError();
    int multiplicand = atoi(dat[pos].value);
    int accumulator = atoi(reg[0].value);
    printf("mul : %d acc : %d\n",multiplicand,accumulator);
    int product = accumulator * multiplicand;
    printf("Product : %d",product);
    snprintf(reg[0].value,sizeof(reg[0].value),"%d",product);
}

void ADDF() {
    int pos = getDataPosition(program[programCounter].operand);
    checkError();
    float addend = atof(dat[pos].value);
    float accumulator = atof(reg[0].value);
    float sum = accumulator + addend;
    ftoa(sum);
}

void SUBF() {
    int pos = getDataPosition(program[programCounter].operand);
    checkError();
    float subtrahend = atof(dat[pos].value);
    float accumulator = atof(reg[0].value);
    float difference = accumulator - subtrahend;
    ftoa(difference);
}

void MULF() {
    int pos = getDataPosition(program[programCounter].operand);
    checkError();
    float multiplicand = atof(dat[pos].value);
    float accumulator = atof(reg[0].value);
    float product = accumulator * multiplicand;
    ftoa(product);
}

void CMP() {
    int pos = getDataPosition(program[programCounter].operand);
    int reg_val = atoi(reg[1].value);
    int data_val = atoi(dat[pos].value);
    COMPARE_FLAG = reg_val <= data_val ? (reg_val == data_val ? 0 : -1) : 1;
}

void CMPS() {
    COMPARE_FLAG = 1;
    int pos = getDataPosition(program[programCounter].operand);
    COMPARE_FLAG = strcmp(reg[2].value, dat[pos].value);
}

void READ() {
    int read;
    read = mem[memoryPointer++].number;
    cout<<"READ : "<<read<<endl;
}

void READMEM() {
    int pos = getDataPosition(program[programCounter].operand);
    int read = mem[12].number;
    printf("Memory read : %d",read);
    sprintf(dat[pos].value,"%d",read);
}

void WRITE() {
    int pos = getDataPosition(program[programCounter].operand);
    printf("%s", dat[pos].value);
}

void SORT() {
    int pos = getDataPosition(program[programCounter].operand);
    int startVal = atoi(dat[pos].value);
    sort(mem + startVal, mem + memoryPointer, &numberSort);
}

void STAM() {
    int pos = getDataPosition(program[programCounter].operand);
    int location = atoi(reg[0].value);
    sprintf(dat[pos].value,"%d",mem[location].number);
    printf("STAM : %d", dat[pos].value);
}

void STXM() {
    int pos = getDataPosition(program[programCounter].operand);
    int location = atoi(reg[1].value);
    sprintf(dat[pos].value,"%d",mem[location].number);
    printf("STXM : %d", dat[pos].value);
}

void WRITEMEM() {
    int pos = getDataPosition(program[programCounter].operand);
    int location = atoi(reg[1].value);
    mem[location].number = atoi(dat[pos].value);
}

void executeCode() {
    while(true) {
        if(ERROR_FLAG == true) {
            printErrorMessage(programCounter);
            return;
        }
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
            printf("X : %s\n",reg[1].value);
        }
        else if(program[programCounter].opcode == "JEQ") {
            JEQ(program[programCounter].operand);
            normal_code = false;
        }
        else if(program[programCounter].opcode == "JLT") {
            JLT(program[programCounter].operand);
            cout<<"Data Lines : "<<endl;
            printDataLines();
            cout<<endl<<endl;
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
        else if(program[programCounter].opcode == "READMEM") {
            READMEM();
        }
        else if(program[programCounter].opcode == "WRITE") {
            WRITE();
        }
        else if(program[programCounter].opcode == "SORT") {
            SORT();
        }
        else if(program[programCounter].opcode == "STAM") {
            STAM();
        }
        else if(program[programCounter].opcode == "STXM") {
            STXM();
        }
        else if(program[programCounter].opcode == "WRITEMEM") {
            WRITEMEM();
        }
        else {
            if(programCounter == length)
                break;
            ERROR_FLAG = true;
            OPCODE_ERROR = true;
            printErrorMessage(programCounter);
            return;
        }
        if(programCounter == length)
            break;
        if(normal_code)
            programCounter++;
    }
}

int debugCode() {
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
        printf("X : %s\n",reg[1].value);
    }
    else if(program[programCounter].opcode == "JEQ") {
        JEQ(program[programCounter].operand);
        normal_code = false;
    }
    else if(program[programCounter].opcode == "JLT") {
        JLT(program[programCounter].operand);
        cout<<"Data Lines : "<<endl;
        printDataLines();
        cout<<endl<<endl;
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
    else if(program[programCounter].opcode == "READMEM") {
        READMEM();
    }
    else if(program[programCounter].opcode == "WRITE") {
        WRITE();
    }
    else if(program[programCounter].opcode == "SORT") {
        SORT();
    }
    else if(program[programCounter].opcode == "STAM") {
        STAM();
    }
    else if(program[programCounter].opcode == "STXM") {
        STXM();
    }
    else if(program[programCounter].opcode == "WRITEMEM") {
        WRITEMEM();
    }
    else {
        if(programCounter == length)
            return 0;
    }

    if(programCounter == length)
        return 0;
    if(normal_code)
        programCounter++;
    return 1;
}

Simulator::Simulator(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Simulator)
{
    ui->setupUi(this);
    for(int i = 0; i < 40; i++) {
            char *buff = "0";
            QTableWidgetItem *item = new QTableWidgetItem(buff);
            item->setTextAlignment(Qt::AlignCenter);
            ui->Memory->setItem(i,0,item);
   }
    for(int i=0; i < 20; i++){
        char *buff = "NULL";
        char *buff1 = "NULL";
        QTableWidgetItem *item = new QTableWidgetItem(buff);
        QTableWidgetItem *item1 = new QTableWidgetItem(buff1);
        item->setTextAlignment(Qt::AlignCenter);
        item1->setTextAlignment(Qt::AlignCenter);
        ui->Symtab->setItem(i,0,item);
        ui->Symtab->setItem(i,1,item1);

    }

}

Simulator::~Simulator()
{
    delete ui;
}

void Simulator::on_Open_triggered()
{

    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), QString(),
               tr("Text Files (*.txt);;C++ Files (*.cpp *.h)"));

       if (!fileName.isEmpty()) {
           QFile file(fileName);
           if (!file.open(QIODevice::ReadOnly)) {
               QMessageBox::critical(this, tr("Error"), tr("Could not open file"));
               return;
           }
           QTextStream in(&file);
           ui->Prog->setFontPointSize(10);
           ui->Prog->setText(in.readAll());
           file.close();
       }

       ui->Title->setText(fileName);
}

void Simulator::on_Save_triggered()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"), QString(),
               tr("Text Files (*.txt);;C++ Files (*.cpp *.h)"));

       if (!fileName.isEmpty()) {
           QFile file(fileName);
           if (!file.open(QIODevice::WriteOnly)) {
               // error message
               QMessageBox::critical(this,tr("Error"),tr("Could not save file"));
               return;
           } else {
               QTextStream stream(&file);
               stream << ui->Prog->toPlainText();
               stream.flush();
               file.close();
           }
       }

}

void Simulator::on_Quit_triggered()
{
    qApp->quit();
}

void Simulator::on_Clear_clicked()
{
    foreach(QLineEdit *widget, this->findChildren<QLineEdit*>()) {
            widget->clear();
            widget->update();
        }
        ui->Prog->clear();
        ui->Title->clear();
        ui->Title->update();
        length = 0;
        dataLength = 0;
        programCounter = 0;
        memoryPointer = 0;
        ERROR_FLAG = false;
        ERROR_LINE = -1;
        OPCODE_ERROR = false;
        first = true;
        COMPARE_FLAG = 0;
        //SYMTAB.clear();
        qApp->processEvents();
}

void Simulator::on_Run_triggered()
{

    QString text = ui->Title->text();
    QByteArray array = text.toLocal8Bit();
    char* buffer = array.data();
    cout<<buffer;
    PASS1(buffer);
    int local_memory = 0;
    for(int i = 0; i < 12 ; i++) {
        QString str = ui->Memory->item(i,0)->text();
        QByteArray qbyte = str.toLocal8Bit();
        char *byte = qbyte.data();
        mem[local_memory++].number = atoi(byte);
    }/*
    QString t = ui->Number->text();
    QByteArray a11 = t.toLocal8Bit();
    char *b11 = a11.data();

    mem[12].number = atoi(b11);
*/
    executeCode();
    if(ERROR_FLAG == false) {
        ui->Status->setText("                   Executed Successfully  ");
    }
    else {
        ui->Status->setText("Error in code");
        if(OPCODE_ERROR == true) {
            const char* mes = program[programCounter].opcode.c_str();
            char mess[25] = "Line Number : ";
            strcat(mess,message);
            strcat(mess,"\nOpcode : ");
            strcat(mess,mes);
            strcat(mess," NOT FOUND");
            QMessageBox::critical(this,tr("OPCODE ERROR"),tr(mess));
        }
        else {
            const char* mes = program[programCounter].operand.c_str();
            char mess[25] = "Line Number : ";
            strcat(mess,message);
            strcat(mess,"\nLabel : ");
            strcat(mess,mes);
            strcat(mess," NOT FOUND");
            QMessageBox::critical(this,tr("LABEL ERROR"),tr(mess));

        }
        return;
    }

    ui->Acc_Val->setText(reg[0].value);
    ui->Index_Val->setText(reg[1].value);

    /*  SET VARIABLES   */
    for(int i = 0; i < dataLength; i++) {
        if(dat[i].varname == "RESULT") {
            ui->Result->setText(dat[i].value);
        }
        else if(dat[i].varname == "STR") {
            ui->Number/*String*/->setText(dat[i].value);
        }
        else if(dat[i].varname == "NUMBER") {
            ui->Number->setText(dat[i].value);
        }
    }
    for(int i = 0; i < dataLength; i++) {
        char *buff = new char[dat[i].varname.length()+1];
        strcpy(buff,dat[i].varname.c_str());
        char *buff1 = (char*)dat[i].value;

        QTableWidgetItem *item = new QTableWidgetItem(buff);
        QTableWidgetItem *item1 = new QTableWidgetItem(buff1);
        item->setTextAlignment(Qt::AlignCenter);
        item1->setTextAlignment(Qt::AlignCenter);
        ui->Symtab->setItem(i,0,item);
        ui->Symtab->setItem(i,1,item1);
    }
    mem[12].number = 0;
    for(int i = 0; i < memoryPointer; i++) {
        char buff[10];
        sprintf(buff,"%d",mem[i].number);
        QTableWidgetItem *item = new QTableWidgetItem(buff);
        item->setTextAlignment(Qt::AlignCenter);
        ui->Memory->setItem(i,0,item);
    }
}

void Simulator::on_Clear_Mem_clicked()
{
    for(int i = 0; i < memoryPointer ; i++) {
            mem[i].number = 0;
        }
        for(int i = 0; i < memoryPointer; i++) {
            char buff[10];
            sprintf(buff,"%d",mem[i].number);
            QTableWidgetItem *item = new QTableWidgetItem(buff);
            item->setTextAlignment(Qt::AlignCenter);
            ui->Memory->setItem(i,0,item);
        }

}

void Simulator::on_Clr_Sym_clicked()
{
    for(int i = 0; i < dataLength; i++) {
        char *buff = "NULL";
        //strcpy(buff,dat[i].varname.c_str());
        char *buff1 = "NULL";//(char*)dat[i].value;

        QTableWidgetItem *item = new QTableWidgetItem(buff);
        QTableWidgetItem *item1 = new QTableWidgetItem(buff1);
        item->setTextAlignment(Qt::AlignCenter);
        item1->setTextAlignment(Qt::AlignCenter);
        ui->Symtab->setItem(i,0,item);
        ui->Symtab->setItem(i,1,item1);
    }

}
