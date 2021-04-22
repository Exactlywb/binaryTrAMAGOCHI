#include "binTranslator.h"

#define PRINT_ERR(message) printf ("%s in function %s. The error was catched on line %d\n", message, __FUNCTION__, __LINE__)

ProgramHeader pHeader = {};

static size_t programSize = 0;

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

    while (_myByteCode._size != 0) {

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
            default:
                PRINT_ERR ("Undefined command");
                printf ("%c\n", *(_myByteCode._byteCode));
                return;

        }

        _myByteCode._size--;

    }

    free (bufferPointerForFree);

}
void PutCommandsIntoByteCode (char* JITBuffer, size_t byteCount, ...) {

    va_list byteList;
    va_start (byteList, byteCount);

    for (size_t byte = 0; byte < byteCount; byte++) {

        printf ("here %zu\n", programSize);
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
    printf ("Size of file %zu\n", sizeOfStdLib);
    
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

void ImplementPush   (char* JITBuffer, InputByteCode* _byteCodeStruct) {

    char modeMem = *((inputBuffer)++);
    char modeReg = *((inputBuffer)++);
    char modeNum = *((inputBuffer)++);
    inputSize -= 3;

    printf ("After mode %d\n", *(_byteCodeStruct->_byteCode));

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

