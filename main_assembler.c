#include<stdio.h>
#include<string.h>
#include<stdlib.h>

#define MAXLENGTH        20
#define MAXSTRLENGTH     30
#define MAXSYMTABLE      30
#define MAXMNEMONIC      48
#define REGCOUNT         16
#define COMPARISONCOUNT  14
#define MAXOPERANDS       3
#define INSTRUCLENGTH     33

#define FALSE -1
#define TRUE 0

#define LABELREPEAT     0
#define INCOMPLETE      1
#define COLONTWICE      2
#define FALSEREGISTER   3
#define FALSEMNEMONIC   4
#define FALSEIMMEDIATE  5
#define MULTILABEL      6
#define NOFILE          7
#define FALSECONDITIONAL 8
#define HIPHENTWICE      9
#define EXTRAOPERAND    10
#define NOLABEL         11
#define FALSELABEL      12

int symTabCount=0;
char binary[MAXLENGTH];
char instrucLen[INSTRUCLENGTH];
int indexBin;

struct Directive
{
    char name[MAXLENGTH];
    int opcode;
}listDir[2];
typedef struct Directive Directive;

struct Mnemonic
{
    char name[MAXLENGTH];
    int opcode;
    int numOperands;
    int addressMode; // 0 means not immediate , 1 means immediate
}listMnemonic[]={"ADD",0,3,0, "SUB",1,3,0,"MULT",2,3,0,"DIVD",3,3,0,"OR",4,3,0,"AND",5,3,0,"NAND",6,3,0,"MOD",7,3,0,"XOR",8,0,3,"NOR",9,3,0,"ADDI",10,3,1,"SUBI",11,3,1,"MULTI",12,3,1,"DIVDI",13,3,1,"ORI",14,3,1,"ANDI",15,3,1,"NANDI",16,3,1,"MODI",17,3,1,"XORI",18,3,1,"NORI",19,3,1,"FADD",20,3,0,"FSUB",21,3,0,"FMULT",22,3,0,"FDIVD",23,3,0,"MOV",1,2,0,"XCHG",2,2,0,"CMP",3,2,0,"LSHIFT",4,2,0,"RSHIFT",5,2,0,"LROT",6,2,0,"RROT",7,2,0,"LDR",8,2,0,"STR",9,2,0,"LDRI",10,2,1,"STRI",11,2,1,"IN",1,1,0,"OUT",2,1,0,"INCR",3,1,0,"DERC",4,1,0,"LDPC",5,1,0,"PUSH",6,1,0,"POP",7,1,0,"JUMP",8,1,1,"CALL",9,1,1,"HLT",1,0,0,"NOP",2,0,0,"EI",3,0,0,"DI",4,0,0};
typedef struct Mnemonic Mnemonic;

struct IntReg
{
    char name[MAXLENGTH];
    int opcode;
}listIntReg[]={"R0",0,"R1",1,"R2",2,"R3",3,"R4",4,"R5",5,"R6",6,"R7",7,"R8",8,"R9",9,"R10",10,"R11",11,"R12",12,"R13",13,"R14",14,"R15",15};
typedef struct IntReg IntReg;

struct FloatReg
{
    char name[MAXLENGTH];
    int opcode;
}listFloatReg[]={"F0",16,"F1",17,"F2",18,"F3",19,"F4",20,"F5",21,"F6",22,"F7",23,"F8",24,"F9",25,"F10",26,"F11",27,"F12",28,"F13",29,"F14",30,"F15",31};
typedef struct FloatReg FloatReg;

struct Comparison{
     char name[5];
     int code;
}listComp[]={"GT",0,"LT",1,"EQ",2,"GE",3,"LE",4,"NG",5,"NL",6,"NE",7,"CP",8,"CN",9,"CZ",10,"NP",11,"NN",12,"NZ",13};
typedef struct Comparison Comparson;

struct Buffer
{
    int changeFlag;// will be TRUE or FALSE
    char conditional[MAXLENGTH];
    char label[MAXLENGTH];
    char mnemonic[MAXLENGTH];
    char operand[MAXOPERANDS][MAXLENGTH];
};
typedef struct Buffer Buffer;
Buffer buf;

struct symTab
{
    char label[MAXLENGTH];
    int address;
}listSymTab[MAXSYMTABLE];
typedef struct symTab symTab;

void cleanBuffer()
{
    buf.changeFlag=FALSE;
    strcpy(buf.conditional,"");
    strcpy(buf.label,"");
    strcpy(buf.mnemonic,"");
    int index;
    for(index=0;index<MAXOPERANDS;index++)
    {
        strcpy(buf.operand[index],"");
    }
}

void printError(int error, char *line, int lineNumber)
{
    switch(error)
    {
        case 0 : printf("Error in line %s at %d\n",line,lineNumber);
            break;
        case 1 : printf("Incomplete operands at line: %d \n : %s\n",lineNumber,line);
            break;
        case 2 : printf("Number of Colons(:) exist more than 1 at line : %d\n : %s\n",lineNumber,line);
            break;
        case 3 : printf("Invalid Register name at line : %d : %s\n",lineNumber,line);
            break;
        case 4 : printf("Invalid Mnemonic used at line : %d : %s\n",lineNumber,line);
            break;
        case 5 : printf("Invalid Data at line : %d : %s\n",lineNumber,line);
            break;
        case 6 : printf("Label %s already defined at line number %d\n",line,lineNumber);
            break;
        case 7 : printf("File name %s does not exist\n",line);
            break;
        case 8 : printf("Invalid Conditional field: %d : %s\n",lineNumber,line);
            break;
        case 9 : printf("Hiphen occuring twice: %d : %s\n",lineNumber,line);
            break;
        case 10 : printf("Extra operands entered: %d : %s\n",lineNumber,line);
            break;
        case 11 : printf("No label for JUMP/CALL : %d : %s\n",lineNumber,line);
            break;
        case 12 : printf("Enter valid label name ,only uppercase alphabets allowed : %d : %s\n",lineNumber,line);
            break;
    }
}

int updateSymbolTable(char s[ ],int lineCount){
        int i,len;
        len=strlen(s);
        for(i=0;i<symTabCount;i++)
        {
            if(!strcmp(s,listSymTab[i].label)) {
                    printError(MULTILABEL,listSymTab[i].label ,listSymTab[i].address);
                    return FALSE;
             }
        }
        strcpy(listSymTab[symTabCount].label,s); // Copy label to next location in symbol table
        listSymTab[symTabCount].address=lineCount;          // symTabCount is the no. of symbol in table.
        symTabCount++;
        return TRUE;
}

int getSymbol(char s[]){
    int i,len=strlen(s);
    for(i=0;i<symTabCount;i++)
    {
        if(!strcmp(s,listSymTab[i].label)) {
                return i;
         }
    }
    return FALSE;
}

int checkLabel(char s[])
{
    int length,i;
    length = strlen(s);
    for(i = 0; i < length; i++)
    {
        if(s[i] < 'A' || s[i] > 'Z')return FALSE;
    }
    return TRUE;
}
int getMnemonic(char s[],int lineCount)
{
    int i;
    for(i=0;i<MAXMNEMONIC;i++)
    {
        if(!strcmp(s,listMnemonic[i].name))
        {
            return i;
        }
    }
    return FALSE;
}

int getIntReg(char s[])
{
    int i;
    for(i=0;i<REGCOUNT;i++)
    {
        if(!strcmp(s,listIntReg[i].name))
        {
            return listIntReg[i].opcode;
        }
    }
    return FALSE;
}
int getFloatReg(char s[])
{
    int i;
    for(i=0;i<REGCOUNT;i++)
    {
        if(!strcmp(s,listFloatReg[i].name))
        {
            return listFloatReg[i].opcode;
        }
    }
    return FALSE;
}
int getCompCode(char s[]){
      int i;
      for(i=0;i<COMPARISONCOUNT;i++) {
            if(!strcmp(listComp[i].name,s))
                   return i;
      }
      return FALSE;
}

int getImmediate(char s[])
{
    int num,i;
    if(s[0]!='#')
        return FALSE;
    else
    {
        if(s[1]=='\0')
            return FALSE;
        for(i=1;s[i]!='\0';i++)
        {
            if(s[i]<'0' || s[i]>'9')
                return FALSE;
        }
       num=atoi(s+1);
       if(num>=0 && num<32)
            return num;
        else
            return FALSE;
    }
}

char* intToBin(int a)
{
    int m=a%2;
    if(a==0){
        binary[0]='0';
        return binary;
    }
    intToBin(a/2);
    binary[indexBin] = m + '0';
    indexBin++;
     binary[indexBin]='\0';
    return binary;
}

int parser(char line[MAXLENGTH],int lineCount)
{
    int flag = TRUE;
    int length;
    char tempRep[MAXLENGTH];
    char *ptr;
    char tempLine[MAXLENGTH];
    char tempOperand[MAXLENGTH];
    char firstChar;
    strcpy(tempLine,line);
    int index;
    int index2;
    cleanBuffer();
    ptr = strchr(tempLine,':');
    if(ptr != NULL)
    {
        ptr = strtok(tempLine,":");
        strcpy(buf.label,ptr);
        flag = updateSymbolTable(buf.label,lineCount);
        if(flag == FALSE){printError(LABELREPEAT,line,lineCount);return flag;}
        ptr = strtok(NULL,":");
        if(ptr == NULL){printError(INCOMPLETE,line,lineCount);return FALSE;}
        strcpy(tempRep,ptr);
        strcpy(tempLine,tempRep);
        ptr = strchr(tempLine,':');
        if(ptr != NULL){printError(COLONTWICE,line,lineCount);return flag;}
    }

    ptr = strchr(tempLine,'-');
    if(ptr != NULL)
    {
        ptr = strtok(tempLine,"-");
        length = strlen(ptr);
        if(length == 3)
        {
            if(ptr[0] == 'S')buf.changeFlag = TRUE;
            else {printError(FALSECONDITIONAL,line,lineCount);return FALSE;}
            strcpy(tempRep,ptr + 1);
        }
        else if(length == 2)
        {
            strcpy(tempRep,ptr);
        }
        else
        {
            printError(FALSECONDITIONAL,line,lineCount);
            return FALSE;
        }
        flag = getCompCode(tempRep);
        if(flag == FALSE){printError(FALSECONDITIONAL,line,lineCount);return flag;}
        strcpy(buf.conditional,tempRep);
        ptr = strtok(NULL,"-");
        if(ptr == NULL){printError(INCOMPLETE,line,lineCount);return FALSE;}
        strcpy(tempRep,ptr);
        strcpy(tempLine,tempRep);
        ptr = strchr(tempLine,'-');
        if(ptr != NULL){printError(HIPHENTWICE,line,lineCount);return flag;}
    }

    ptr = strtok(tempLine," ");
    strcpy(buf.mnemonic,ptr);
    flag = getMnemonic(buf.mnemonic,lineCount);
    if(flag == FALSE){printError(FALSEMNEMONIC,line,lineCount);return flag;}
    index = flag;
    firstChar = tempLine[0];

    if(listMnemonic[index].addressMode == 0)// Parses for non immediate addressing mode
    {
        for(index2 = 1; index2 <= listMnemonic[index].numOperands; index2++)
        {
            ptr = strtok(NULL,", ");
            if(ptr == NULL){printError(INCOMPLETE,line,lineCount);return FALSE;}
            strcpy(tempOperand,ptr);
            flag = (firstChar == 'F') ? getFloatReg(tempOperand) : getIntReg(tempOperand);
            if(flag == FALSE){printError(FALSEREGISTER,line,lineCount);return flag;}
            else strcpy(buf.operand[index2 - 1],tempOperand);
        }
    }

    if(listMnemonic[index].addressMode == 1) //Parses for immediate addressing mode
    {
        if( (strcmp(buf.mnemonic,"JUMP") != 0) && (strcmp(buf.mnemonic,"JUMP") != 0) )
        {
            printf("Entered ");
            ptr = strtok(NULL,", ");
            if(ptr == NULL){printError(INCOMPLETE,line,lineCount);return FALSE;}
            strcpy(tempOperand,ptr);
            flag = (firstChar == 'F') ? getFloatReg(tempOperand) : getIntReg(tempOperand);
            if(flag == FALSE){printError(FALSEREGISTER,line,lineCount);return flag;}
            else strcpy(buf.operand[0],tempOperand);

            for(index2 = 2; index2 <= listMnemonic[index].numOperands; index2++)
            {
                ptr = strtok(NULL,", ");
                if(ptr == NULL){printError(INCOMPLETE,line,lineCount);return FALSE;}
                strcpy(tempOperand,ptr);
                flag = getImmediate(tempOperand);
                if(flag == FALSE){printError(FALSEIMMEDIATE,line,lineCount);return flag;}
                else strcpy(buf.operand[index2 - 1],tempOperand + 1);
            }
        }
        else
        {
            ptr = strtok(NULL," ");
            if(ptr == NULL){printError(INCOMPLETE,line,lineCount);return FALSE;}
            strcpy(tempOperand,ptr);
            flag = checkLabel(tempOperand);
            if(flag == FALSE){printError(FALSELABEL,line,lineCount);return FALSE;}
            else strcpy(buf.label,tempOperand);
        }
    }
        ptr = strtok(NULL,", ");
        if(ptr != NULL){printError(EXTRAOPERAND,line,lineCount);return FALSE;}
}
void truncate(char str[])
{
    int length = strlen(str);
    int index,first,second;
    for(index = 0;index < length ;index++)
    {
        if(str[index]!=' '){first=index;break;}
    }
    strcpy(str,str+first);
    for(index = length - 1; index >=0 ; index--)
    {
        if(str[index] != ' ' && str[index] != '\n'){second = index ;break;}
    }
    str[index+1]='\0';
}

int firstPass(FILE *input)
{
   char line[MAXSTRLENGTH];
   char tempLine[MAXSTRLENGTH];
   int flag=TRUE;
   int lineCount=0,length;
   do
   {
        lineCount++;
        fgets(line,MAXSTRLENGTH,(FILE*)input);
        truncate(line);
        strcpy(tempLine,line);
        length = strlen(line);
        if(length == 0)continue;
        if(line[0]=='/' && line[1]=='/')continue;
        flag = parser(tempLine,lineCount);
        if(flag == FALSE)break;

   }
   while( (!feof(input) ) );
   fclose(input);
   return flag;
}

void secondPass(FILE* input,FILE* output)
{
   char line[MAXSTRLENGTH];
   char tempLine[MAXSTRLENGTH];
   int flag=TRUE;
   int lineCount=0,length;
   int index,index1,temp;
   int regOpcode;
   char *tempBin;
   int i;
    while(fgets(line,MAXSTRLENGTH,(FILE*)input)){
        memset(instrucLen,'0',INSTRUCLENGTH*sizeof(int));
        instrucLen[32]='\0';
        lineCount++;
        truncate(line);
        printf("Extracting line : '%s'\n",line);
        strcpy(tempLine,line);
        // Checking if contains Line
        length = strlen(line);
        if(strcmp("\n",line) != 0)flag = parser(tempLine,lineCount);
        if(flag == FALSE)break;
        indexBin=0;
        //if((index=getSymbol(buf.label))!=FALSE)
          //  fprintf(output,"%s ",intToBin(listSymTab[index].address));
          if(buf.changeFlag==TRUE)
          {
              instrucLen[0]='1';
          }
          if(strcmp(buf.conditional,"")){
            index=getCompCode(buf.conditional);
            tempBin=intToBin(listComp[index].code);
            for(i=0;i<strlen(tempBin);i++)
            {
                instrucLen[5-strlen(tempBin)+i]=tempBin[i];
            }
          }
        index=getMnemonic(buf.mnemonic,lineCount);
        indexBin=0;
        tempBin=intToBin(listMnemonic[index].opcode);
        //strcpy((instrucLen+17-strlen(tempBin)),tempBin);
        for(i=0;i<strlen(tempBin);i++)
        {
            instrucLen[17-strlen(tempBin)+i]=tempBin[i];
        }

        if(listMnemonic[index].numOperands==2)
            instrucLen[11]='1';
        else if(listMnemonic[index].numOperands==1)
            instrucLen[10]='1';
        else if(listMnemonic[index].numOperands==0)
            instrucLen[10]=instrucLen[11]='1';

        if(listMnemonic[index].addressMode==0)
        {
            if(listMnemonic[index].opcode>=10 && listMnemonic[index].opcode<=19)
            {
                for(index1=0;index1<listMnemonic[index].numOperands;index1++)
                {
                    indexBin=0;
                    regOpcode=getFloatReg(buf.operand[index1]);
                    tempBin=intToBin(regOpcode);
                    for(i=0;i<strlen(tempBin);i++)
                    {
                        instrucLen[22+(index1*5)-strlen(tempBin)+i]=tempBin[i];
                    }
                    //strcpy((instrucLen+22+(index1*5)-strlen(tempBin)),tempBin);
                }
            }
            else
            {
                for(index1=0;index1<listMnemonic[index].numOperands;index1++)
                {
                    indexBin=0;
                    regOpcode=getIntReg(buf.operand[index1]);
                    tempBin=intToBin(regOpcode);
                    //strcpy((instrucLen+22+(index1*5)-strlen(tempBin)),tempBin);
                    for(i=0;i<strlen(tempBin);i++)
                    {
                        instrucLen[22+(index1*5)-strlen(tempBin)+i]=tempBin[i];
                    }
                }
            }

        }else
        {
            indexBin=0;
            regOpcode=getIntReg(buf.operand[0]);
            tempBin=intToBin(regOpcode);
            //strcpy((instrucLen+22-strlen(tempBin)),tempBin);
            for(i=0;i<strlen(tempBin);i++)
            {
                instrucLen[22-strlen(tempBin)+i]=tempBin[i];
            }
            for(index1=1;index1<listMnemonic[index].numOperands;index1++)
            {
                indexBin=0;
                temp=atoi(buf.operand[index1]);
                tempBin=intToBin(regOpcode);
                for(i=0;i<strlen(tempBin);i++)
                {
                    instrucLen[22+(index1*5)-strlen(tempBin)+i]=tempBin[i];
                }
                //strcpy((instrucLen+22+(index1*5)-strlen(tempBin)),tempBin);
            }
        }
        fprintf(output,"%s",instrucLen);
    }
    fclose(input);
    fclose(output);
}

int main()
{
    FILE *input,*output;
    int flag;
    char filename[MAXSTRLENGTH];
    char outFilename[MAXSTRLENGTH+3];
    printf("Enter file name : ");
    scanf("%s",filename);
    input = fopen(filename, "r");
    if(input==NULL)
    {
        printError(NOFILE,filename,-1);
        return 0;
    }
    flag=firstPass(input);
    if(flag==FALSE)
    {
        return 0;
    }
    input = fopen(filename, "r");
    strcpy(outFilename,filename);
    strcpy(outFilename+strlen(filename)-4,"Out.txt");
    output = fopen(outFilename, "w");
    secondPass(input,output);
    return 0;
}
