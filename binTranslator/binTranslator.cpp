#include "binTranslator.h"

#define PRINT_ERR(message) printf ("%s in function %s. The error was catched on line %d\n", message, __FUNCTION__, __LINE__)

ProgramHeader   pHeader                         = {};
Label           labelTable [MAX_LABEL_COUNT]    = {};

static int      labelNum        = 0;

void BinaryTranslation (const char* input, const char* elfOutput) {

    FILE* elfFile = CreateELFFile (elfOutput);
    if (!elfFile) {

        PRINT_ERR ("Can't open FILE* elfFile");
        return;

    }

    char JITBuffer [MAX_CODE_SIZE] = "";
    
    FILE* inputFile = fopen (input, "rb");
    if (!inputFile) {

        PRINT_ERR ("Can't open FILE* inputFile");
        return;

    }
    
    HandleInputByteCode (inputFile, JITBuffer);
    pHeader.P_FILES = pHeader.P_MEMSZ = 0x80 + PROGRAM_SIZE;
    PrintELFHeader      (JITBuffer);

    fclose (elfFile);
    fclose (inputFile);

}

void HandleInputByteCode (FILE* input, char* JITBuffer) {

    if (!input) {

        PRINT_ERR ("Null pointer on FILE* input");
        return;

    }
    if (!JITBuffer) {

        PRINT_ERR ("Null pointer on char* JITBuffer");
        return;

    }

    IncludeStdLib (JITBuffer);

    InputByteCode _myByteCode = {};
    ReadInputByteCode (input, &_myByteCode);

    char* bufferPointerForFree = _myByteCode._byteCode;

    BytePassage (JITBuffer, FirstPassage, _myByteCode);
    LabelSort ();
    labelNum--;

    BytePassage (JITBuffer, SecondPassage, _myByteCode);
    BytePassage (JITBuffer, ThirdPassage , _myByteCode);

    free (bufferPointerForFree);

}

void BytePassage (char* JITBuffer, size_t passageNum, InputByteCode _myByteCode) {

    size_t firstInputSize   = _myByteCode._size;
    PROGRAM_SIZE             = STDLIB_SIZE;
    
    while (_myByteCode._size != 0) {

        if (passageNum == 2 && labelNum >= 0) {

            if ((firstInputSize - _myByteCode._size + 1) == labelTable [labelNum].inputVal) {
                
                labelTable [labelNum].outputVal = PROGRAM_SIZE - STDLIB_SIZE;
                labelNum--;

            }
        }
        
        char command = *(_myByteCode._byteCode);
        _myByteCode._byteCode++;

        switch (command) {

            case hlt: {
                CallForStdLib (JITBuffer, hltJmp);
                break;
            }    
            case push: {
                ImplementPush (JITBuffer, &(_myByteCode));
                break;
            }    
            case pop: {
                ImplementPop  (JITBuffer, &(_myByteCode));
                break;
            }
            case in: {
                CallForStdLib (JITBuffer, inJmp);
                break;
            }
            case out: {
                CallForStdLib (JITBuffer, outJmp);
                break;
            }
            case add: {
                ImplementMath (JITBuffer, ADD);
                break;
            }
            case sub: {
                ImplementMath (JITBuffer, SUB);
                break;
            }
            case mul: {
                ImplementMath (JITBuffer, MUL);
                break;
            }
            case sep: {
                ImplementMath (JITBuffer, DIV);
                break;
            }
            case sqrt: {
                ImplementMath (JITBuffer, SQRT);
                break;
            }
            case jmp: {
                ImplementJmp (JITBuffer, passageNum, &_myByteCode, JMP);
                break;
            }
            case je: {
                ImplementJmp (JITBuffer, passageNum, &_myByteCode, JE);
                break;
            }
            case ja: {
                ImplementJmp (JITBuffer, passageNum, &_myByteCode, JA);
                break;
            }
            case jb: {
                ImplementJmp (JITBuffer, passageNum, &_myByteCode, JB);
                break;
            }
            case jne: {
                ImplementJmp (JITBuffer, passageNum, &_myByteCode, JNE);
                break;
            }
            default: {
                PRINT_ERR ("Undefined command");
                printf ("%c = %d\n", *(_myByteCode._byteCode), *(_myByteCode._byteCode));
                return;
            }

        }

        _myByteCode._size--;

    }

}

void PutCommandsIntoByteCode (char* JITBuffer, size_t byteCount, ...) {

    va_list byteList;
    va_start (byteList, byteCount);

    for (size_t byte = 0; byte < byteCount; byte++) {

        JITBuffer [PROGRAM_SIZE++] = (char)(va_arg (byteList, int));

    }

    va_end (byteList);

}

void IncludeStdLib (char* JITBuffer) {

    if (!JITBuffer) {

        PRINT_ERR ("Null pointer on char* JITBuffer");
        return;

    }

    FILE* stdLib            = fopen ("fr_stdlib/stdlib_no_header", "rb");
    if (!stdLib) {

        PRINT_ERR ("Can't open FILE* stdLib");
        return;

    }
    
    size_t sizeOfStdLib     = GetSizeOfFile (stdLib);
    STDLIB_SIZE = sizeOfStdLib;
    
    char* stdLibBinaryCode  = (char*)calloc (sizeOfStdLib, sizeof (char));
    if (!stdLibBinaryCode) {

        PRINT_ERR ("Problems with memory char* stdLibBinaryCode");
        return;

    }

    fread (JITBuffer, sizeOfStdLib, 1, stdLib);

    PROGRAM_SIZE += sizeOfStdLib;

    free (stdLibBinaryCode);
    fclose (stdLib);

}

void ReadInputByteCode (FILE* input, InputByteCode* _byteCodeStruct) {

    if (!input) {

        PRINT_ERR ("Null pointer on FILE* input");
        return;

    }

    size_t  fileSize = GetSizeOfFile (input);

    char*   buffer   = (char*)calloc (fileSize, sizeof (char));
    if (!buffer) {

        PRINT_ERR ("Memory trouble with char* buffer");
        return;

    }

    fread (buffer, sizeof (char), fileSize, input);

    _byteCodeStruct->_byteCode = buffer;
    _byteCodeStruct->_size     = fileSize; 

} 

size_t GetSizeOfFile (FILE* input) {

    if (!input) {

        PRINT_ERR ("Null pointer on FILE* input");
        return 0;

    }

    fseek (input, 0, SEEK_END);
    size_t fileSize = ftell (input);
    fseek (input, 0, SEEK_SET);

    return fileSize;

}

FILE* CreateELFFile (const char* elfOutput) {

    FILE* output = fopen (elfOutput, "wb");
    if (!output) {

        PRINT_ERR ("Can't open FILE* output");
        return nullptr;

    }

    return output;

}

void PrintELFHeader (char* buffer) {
    
    FILE* output = fopen ("a.out", "wb");
    if (!output) {

        PRINT_ERR ("Can't open FILE* output");
        return;

    }

    ELFHeader elfHeader = {};

    fwrite (&elfHeader, sizeof (elfHeader), 1              , output);
    fwrite (&pHeader  , sizeof (pHeader)  , 1              , output);

    fwrite (buffer    , sizeof (char)     , PROGRAM_SIZE    , output);

    fclose (output);

}

//=============================================================================
//========================TRANSLATE INTO BYTE CODE=============================

void CallForStdLib (char* JITBuffer, int jmpTableNum) {

    PutCommandsIntoByteCode (JITBuffer, CALL_BYTE_CODE);
    
    int adress = jmpTableNum - PROGRAM_SIZE;
    *(int*)(JITBuffer + PROGRAM_SIZE) = adress;
    PROGRAM_SIZE += 4;

}

void ImplementMath (char* JITBuffer, int mathNum) {

    GetXMMFromStack (JITBuffer, 7);
    PopRegular (JITBuffer);
    
    if (mathNum == SQRT) {

        PutCommandsIntoByteCode (JITBuffer, 4, 0xF2, 0x0F, SQRT, 0xFF); //maybe it's better to write mathNum instead of SQRT here

    } else {

        GetXMMFromStack (JITBuffer, 6);
        PopRegular (JITBuffer);

        PutCommandsIntoByteCode (JITBuffer, 5, 0xF2, 0x0F, mathNum, 0xFE, 0x48); 

    }
    
    SubRspForXMM (JITBuffer);
    PushXMMIntoStack (JITBuffer, 7);

}

void ReadModeFromInputBuffer (char* mode, InputByteCode* _byteCodeStruct) {

    mode [0]    = *((inputBuffer)++);
    mode [1]    = *((inputBuffer)++);
    mode [2]    = *((inputBuffer)++);
    inputSize  -= 3;

}

#define modeNum mode [2]
#define modeReg mode [1]
#define modeMem mode [0]

void ImplementPop (char* JITBuffer, InputByteCode* _byteCodeStruct) {

    if (CallOptimizator (JITBuffer, _byteCodeStruct, O1))
        return;

    char mode [3] = "";
    ReadModeFromInputBuffer (mode, _byteCodeStruct);

    if (modeNum == 0) {

        if (modeReg == 1) {

            if (modeMem == 1) {

                GetXMMFromStack (JITBuffer, 7);
                char regNum = *(inputBuffer++);
                PushXMMIntoStack (JITBuffer, regNum);
                inputSize--;

                PopRegular (JITBuffer);

            } else {

                GetXMMFromStack (JITBuffer, *((inputBuffer)++));
                inputSize--;

                PopRegular (JITBuffer);

            }

        } else {

            PopRegular (JITBuffer);

        }

    }

}

void ImplementPush (char* JITBuffer, InputByteCode* _byteCodeStruct) {

    if (CallOptimizator (JITBuffer, _byteCodeStruct, O1))
        return;

    char mode [3] = "";
    ReadModeFromInputBuffer (mode, _byteCodeStruct);

    if (modeNum == 1) {

        PushNumber (JITBuffer, _byteCodeStruct);

    } else {

        if (modeReg == 1) {

            if (modeMem == 1) {

                SubRspForXMM (JITBuffer);
                PushXMMIntoStack (JITBuffer, *((inputBuffer)++));
                inputSize--;

            } else {

                SubRspForXMM (JITBuffer);
                PushXMMIntoStack (JITBuffer, *((inputBuffer)++));
                inputSize--;

            }

        }

    }

}

void ImplementJmp (char* JITBuffer, size_t passageNum, InputByteCode* _byteCodeStruct, int jmpNum) {

    ImplementJmpCondition (JITBuffer, jmpNum);
    PutCommandsIntoByteCode (JITBuffer, 1, jmpNum);

    if (passageNum == 1 || passageNum == 3) {

        size_t number = 0;

        for (size_t numByte = 0; numByte < sizeof (size_t); numByte++) {
            
            number *= 10;
            number += (unsigned char)inputBuffer [numByte];

        }
        inputBuffer += sizeof (size_t);
        inputSize   -= sizeof (size_t);

        if (passageNum == 3) {
            
            for (size_t labelCount = 0; labelCount < labelNum; labelCount++) {

                if (number == (labelTable [labelCount]).inputVal) {
                    
                    int adress = (labelTable [labelCount]).outputVal - PROGRAM_SIZE - 0x4 + STDLIB_SIZE;
                    *(int*)(JITBuffer + PROGRAM_SIZE) = adress;
                    PROGRAM_SIZE += 4;

                    return;

                }

            }
            
        }

        bool newLabel = true;
        for (size_t labelCount = 0; labelCount < labelNum; labelCount++) {

            if (number == (labelTable [labelCount]).inputVal) {
                
                newLabel = false;
                break;

            }

        }

        if (newLabel) {

            labelTable [labelNum++] = {number, 0};

        }

        FillSpaceAddress (JITBuffer);

    } else if (passageNum == 2) {

        inputBuffer += sizeof (size_t);
        inputSize   -= sizeof (size_t);

        FillSpaceAddress (JITBuffer);

    } else {

        PRINT_ERR ("Unexpected passageNum");
        return;

    }

}

int LabelCmp (const void* firstL, const void* secondL) {

    Label* firstLabel  = (Label*)firstL;
    Label* secondLabel = (Label*)secondL; 

    if (firstLabel->inputVal < secondLabel->inputVal)
        return 1;
    else if (firstLabel->inputVal > secondLabel->inputVal)
        return -1;
    else 
        return 0;

}

void LabelSort () {

    qsort (labelTable, labelNum, sizeof (Label), LabelCmp);

}

//=============================================================================
//========================TRANSLATE TECH FUNCTIONS=============================

void SubRspForXMM (char* JITBuffer) {

    PutCommandsIntoByteCode (JITBuffer, 4, 0x48, 0x83, 0xEC, 0x08);

}

void PushXMMIntoStack (char* JITBuffer, unsigned char xmmPostfix) {

    PutCommandsIntoByteCode (JITBuffer, 5, 0xF2, 0x0F, 0x11, 0x04 + 8 * xmmPostfix, 0x24);

}

void GetXMMFromStack (char* JITBuffer, unsigned char xmmPostfix) {

    PutCommandsIntoByteCode (JITBuffer, 5, 0xF2, 0x0F, 0x10, 0x04 + 8 * xmmPostfix, 0x24);  //movsd xmm*, [rsp]

}

size_t PushNumberLikeBytes (char* JITBuffer, InputByteCode* _byteCodeStruct) {

    PutCommandsIntoByteCode (JITBuffer, JMP_IN_QWORD);                          
    size_t labelBeforeNum = PROGRAM_SIZE + PROGRAM_START + 0x80;                 
    for (size_t byteNum = 0; byteNum < sizeof (double); byteNum++) {            
                                                                                
        JITBuffer [PROGRAM_SIZE++] = inputBuffer [byteNum];                      
                                                                                
    }                                                                           
                                                                                
    inputBuffer += sizeof (double);                                             
    inputSize   -= sizeof (double);

    return labelBeforeNum;

}

void PushNumber (char* JITBuffer, InputByteCode* _byteCodeStruct) {

    size_t labelBeforeNum = PushNumberLikeBytes (JITBuffer, _byteCodeStruct);                              
                                                                                
    /*push [rsp], xmm7*/                                                        
    PutCommandsIntoByteCode (JITBuffer, 5, 0xF2, 0x0F, 0x10, 0x3C, 0x25);       
    *(size_t*)(JITBuffer + PROGRAM_SIZE) = labelBeforeNum;                       
    PROGRAM_SIZE += 4;       

    SubRspForXMM (JITBuffer);                                                             
    PushXMMIntoStack (JITBuffer, 7);    

}

void PopRegular (char* JITBuffer) {

    PutCommandsIntoByteCode (JITBuffer, 4, 0x48, 0x83, 0xC4, 0x08); //rsp += 8

}

void PutCondition (char* JITBuffer) {

    PutCommandsIntoByteCode (JITBuffer, 4, 0x66, 0x0F, 0x2F, 0xF7);

}

void PutRet (char* JITBuffer) {

    PutCommandsIntoByteCode (JITBuffer, 1, 0xC3);

}

void ImplementJmpCondition (char* JITBuffer, int jmpNum) {

    if (jmpNum != JMP && jmpNum != CALL) {                                      
                                                                                                                    
        GetXMMFromStack (JITBuffer, 6);                                                  
        PopRegular (JITBuffer);  

        GetXMMFromStack (JITBuffer, 7);
        PopRegular (JITBuffer);

        PutCondition (JITBuffer);                                                          
        PutCommandsIntoByteCode (JITBuffer, 1, 0x0F); //xmm7                          
                                                                                
    } 

}

void FillSpaceAddress (char* JITBuffer) {

    PutCommandsIntoByteCode (JITBuffer, 4, 0x00, 0x00, 0x00, 0x00);

}

//=============================================================================
//============================= Optimizator ===================================

char ReadCharFromInputByteCode (InputByteCode* _byteCodeStruct) {

    inputSize--;
    return *(inputBuffer++);

}

bool CheckMode (char* mode, char num, char reg, char mem) {

    if (modeNum == num &&
        modeReg == reg &&
        modeMem == mem) {

        return true;
    
    }

    return false;

}

void SkipNumberInByteCode (InputByteCode* _byteCodeStruct) {

    int doubleSize  = sizeof (double);

    inputBuffer     += doubleSize;
    inputSize       -= doubleSize;

}

void ReturnSkippedBytes (InputByteCode* _byteCodeStruct, size_t skippedBytes) {

    inputBuffer -= skippedBytes;
    inputSize   += skippedBytes;

}

bool PushPopOptimization (char* JITBuffer, InputByteCode* _byteCodeStruct) {

    size_t skippedBytes = 0;

    char mode [3] = "";
    ReadModeFromInputBuffer (mode, _byteCodeStruct);
    skippedBytes += 3;

    if (CheckMode (mode, 1, 0, 0)) {                                    

        SkipNumberInByteCode (_byteCodeStruct);                         //skip number
        skippedBytes += sizeof (double);

    } else if (CheckMode (mode, 0, 1, 0)) {

        ReadCharFromInputByteCode (_byteCodeStruct);                    //skip register
        skippedBytes++;

    }

    skippedBytes++;
    if (ReadCharFromInputByteCode (_byteCodeStruct) == pop) {

        ReadModeFromInputBuffer (mode, _byteCodeStruct);
        skippedBytes += 3;

        if (CheckMode (mode, 0, 0, 0)) {                                 //pop

            return true;

        } else
            ReturnSkippedBytes (_byteCodeStruct, skippedBytes);

    } else
        ReturnSkippedBytes (_byteCodeStruct, skippedBytes);

    return false;

}

bool PushNumPopRegOptimization (char* JITBuffer, InputByteCode* _byteCodeStruct) {

    size_t skippedBytes = 0;

    char mode [3] = "";
    ReadModeFromInputBuffer (mode, _byteCodeStruct);
    skippedBytes += 3;

    size_t labelBeforeNumber = 0;

    if (CheckMode (mode, 1, 0, 0)) {                                    //push num

        labelBeforeNumber = PushNumberLikeBytes (JITBuffer, _byteCodeStruct);      
        skippedBytes += sizeof (double);
    
    } else {

        ReturnSkippedBytes (_byteCodeStruct, skippedBytes);
        return false;

    }

    skippedBytes++;
    if (ReadCharFromInputByteCode (_byteCodeStruct) == pop) {

        ReadModeFromInputBuffer (mode, _byteCodeStruct);
        skippedBytes += 3;

        if (CheckMode (mode, 0, 1, 0)) {                                 //pop reg

            char xmmPostfix = ReadCharFromInputByteCode (_byteCodeStruct);
            PutCommandsIntoByteCode (JITBuffer, 5, 0xF2, 0x0F, 0x10, 0x4 + 8 * xmmPostfix, 0x25);

            *(size_t*)(JITBuffer + PROGRAM_SIZE) = labelBeforeNumber;                       
            PROGRAM_SIZE += 4;

            return true;

        } else {

            ReturnSkippedBytes (_byteCodeStruct, skippedBytes);
            PROGRAM_SIZE -= sizeof (double);

            return false;

        }

    } else {

        ReturnSkippedBytes (_byteCodeStruct, skippedBytes);
        PROGRAM_SIZE -= sizeof (double) + 2;
        return false;

    }

}

bool PushOptimizator (char* JITBuffer, InputByteCode* _byteCodeStruct) {

    if (PushPopOptimization (JITBuffer, _byteCodeStruct))
        return true;

    if (PushNumPopRegOptimization (JITBuffer, _byteCodeStruct))
        return true;

    return false;    

}

bool PopOptimizator (char* JITBuffer, InputByteCode* _byteCodeStruct) {

    char mode [3] = "";
    ReadModeFromInputBuffer (mode, _byteCodeStruct);

    if (CheckMode (mode, 0, 1, 0)) {

        char popRegNum = ReadCharFromInputByteCode (_byteCodeStruct);                //skip name of register
        
        if (ReadCharFromInputByteCode (_byteCodeStruct) == push) {

            ReadModeFromInputBuffer (mode, _byteCodeStruct);
            if (CheckMode (mode, 0, 1, 0)) {
                
                char pushRegNum = ReadCharFromInputByteCode (_byteCodeStruct);

                if (pushRegNum == popRegNum) {
                    
                    GetXMMFromStack (JITBuffer, popRegNum);
                    return true;

                } else {

                    inputBuffer -= 9;
                    inputSize   += 9;

                }

            } else {

                inputBuffer -= 8;
                inputSize   += 8;
                
            }

        } else {

            inputBuffer -= 5;
            inputSize   += 5;
            
        }

    } else {

        inputBuffer -= 3;
        inputSize   += 3;

    }

    return false;

}

bool CallOptimizator (char* JITBuffer, InputByteCode* _byteCodeStruct, int OLvl) {

    if (OLvl == 1) {

        if (*(inputBuffer - 1) == push)
            return PushOptimizator (JITBuffer, _byteCodeStruct);    
        else if (*(inputBuffer - 1) == pop)
            return PopOptimizator  (JITBuffer, _byteCodeStruct);

    } else {

        printf ("No any optimization!\n");
        return false;

    }

    return false;

}
