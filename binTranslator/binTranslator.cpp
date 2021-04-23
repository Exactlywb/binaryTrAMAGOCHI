#include "binTranslator.h"

#define PRINT_ERR(message) printf ("%s in function %s. The error was catched on line %d\n", message, __FUNCTION__, __LINE__)

ProgramHeader   pHeader                         = {};
Label           labelTable [MAX_LABEL_COUNT]    = {};

static int      labelNum      = 0;
static size_t   programSize   = 0;

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
    pHeader.P_FILES = pHeader.P_MEMSZ = 0x80 + programSize;
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

    BytePassage (JITBuffer, 1, _myByteCode);
    LabelSort ();
    labelNum--;

    BytePassage (JITBuffer, 2, _myByteCode);

    BytePassage (JITBuffer, 3, _myByteCode);

    free (bufferPointerForFree);

}

void BytePassage (char* JITBuffer, size_t passageNum, InputByteCode _myByteCode) {

    size_t firstInputSize   = _myByteCode._size;
    programSize             = STDLIB_SIZE;
    
    while (_myByteCode._size != 0) {

        if (passageNum == 2 && labelNum >= 0) {

            if ((firstInputSize - _myByteCode._size + 1) == labelTable [labelNum].inputVal) {
                
                labelTable [labelNum].outputVal = programSize - STDLIB_SIZE;
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
            default:
                PRINT_ERR ("Undefined command");
                printf ("%c = %d\n", *(_myByteCode._byteCode), *(_myByteCode._byteCode));
                return;

        }

        _myByteCode._size--;

    }

    if (passageNum == 2 && labelNum >= 0) {
            if ((firstInputSize - _myByteCode._size + 1) == labelTable [labelNum].inputVal) {
                
                labelTable [labelNum].outputVal = programSize - STDLIB_SIZE;
                labelNum--;

            }
    }

}

void PutCommandsIntoByteCode (char* JITBuffer, size_t byteCount, ...) {

    va_list byteList;
    va_start (byteList, byteCount);

    for (size_t byte = 0; byte < byteCount; byte++) {

        JITBuffer [programSize++] = (char)(va_arg (byteList, int));

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

    programSize += sizeOfStdLib;

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

    fwrite (buffer    , sizeof (char)     , programSize    , output);

    fclose (output);

}

//=============================================================================
//========================TRANSLATE INTO BYTE CODE=============================

void CallForStdLib (char* JITBuffer, int jmpTableNum) {

    PutCommandsIntoByteCode (JITBuffer, CALL_BYTE_CODE);
    
    int adress = jmpTableNum - programSize;
    *(int*)(JITBuffer + programSize) = adress;
    programSize += 4;

}

void ImplementMath (char* JITBuffer, int mathNum) {

    GET_XMM_FROM_STACK (7)
    POP_REGULAR
    
    if (mathNum == SQRT) {

        PutCommandsIntoByteCode (JITBuffer, 4, 0xF2, 0x0F, SQRT, 0xFF); //maybe it's better to write mathNum instead of SQRT here

    } else {

        GET_XMM_FROM_STACK (6)
        POP_REGULAR

        PutCommandsIntoByteCode (JITBuffer, 5, 0xF2, 0x0F, mathNum, 0xFE, 0x48); 

    }
    
    SUB_RSP_FOR_XMM
    PUSH_XMM_INTO_STACK (7)

}

void ImplementPop (char* JITBuffer, InputByteCode* _byteCodeStruct) {

    char modeMem = *((inputBuffer)++);
    char modeReg = *((inputBuffer)++);
    char modeNum = *((inputBuffer)++);
    inputSize -= 3;

    if (modeNum == 0) {

        if (modeReg == 1) {

            if (modeMem == 1) {



            } else {

                GET_XMM_FROM_STACK (*((inputBuffer)++))
                inputSize--;

                POP_REGULAR

            }

        } else {

            if (modeMem == 1) {



            } else {

                POP_REGULAR

            }

        }

    }

}

void ImplementPush (char* JITBuffer, InputByteCode* _byteCodeStruct) {

    char modeMem = *((inputBuffer)++);
    char modeReg = *((inputBuffer)++);
    char modeNum = *((inputBuffer)++);
    inputSize -= 3;

    if (modeNum == 1) {

        if (modeReg == 1) {

            

        } else {

            PUSH_NUMBER

        }

    } else {

        if (modeReg == 1) {

            if (modeMem == 1) {



            } else {

                SUB_RSP_FOR_XMM
                PUSH_XMM_INTO_STACK (*((inputBuffer)++))
                inputSize--;

            }

        } else {



        }

    }

}

void ImplementJmp (char* JITBuffer, size_t passageNum, InputByteCode* _byteCodeStruct, int jmpNum) {

    IMPLEMENT_JMP_CONDITION

    PutCommandsIntoByteCode (JITBuffer, 1, jmpNum);

    printf ("1) Program size %zu\n", programSize - STDLIB_SIZE);

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
                    
                    int adress = (labelTable [labelCount]).outputVal - programSize - 0x4 + STDLIB_SIZE;
                    *(int*)(JITBuffer + programSize) = adress;
                    programSize += 4;

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

        PutCommandsIntoByteCode (JITBuffer, 4, 0x0, 0x0, 0x0, 0x0);

    } else if (passageNum == 2) {

        inputBuffer += sizeof (size_t);
        inputSize   -= sizeof (size_t);

        PutCommandsIntoByteCode (JITBuffer, 4, 0x0, 0x0, 0x0, 0x0);

    } else {

        PRINT_ERR ("Unexpected passageNum");
        return;

    }

}

void LabelSort () {

    for (size_t i = 0; i < labelNum; i++) {

        for (size_t j = i; j < labelNum; j++) {

            if (labelTable [i].inputVal < labelTable [j].inputVal) {

                size_t temp = labelTable [i].inputVal;
                labelTable [i].inputVal = labelTable [j].inputVal;
                labelTable [j].inputVal = temp;

            }

        }

    }

}


