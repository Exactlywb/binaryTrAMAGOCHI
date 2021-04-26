#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include "commands.h"
#include <stdarg.h>

typedef const unsigned char c_u_char; 

struct ELFHeader {

    c_u_char      EI_MAG      [4] = {0x7f, 0x45, 0x4C, 0x46};
    c_u_char      EI_CLASS        =  0x02;
    c_u_char      EI_DATA         =  0x01;
    c_u_char      EI_VERSION      =  0x01;
    c_u_char      EI_OSABI        =  0x00;
    c_u_char      EI_OSABIVER [8] = {0x00};
    c_u_char      E_TYPE      [2] = {0x02, 0x00};
    c_u_char      E_MACHINE   [2] = {0x3E, 0x00};
    c_u_char      E_VERSION   [4] = {0x01, 0x00, 0x00, 0x00};
    c_u_char      E_ENTRY     [8] = {0x80, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00};
    c_u_char      E_PHOFF     [8] = {0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    c_u_char      E_SHOFF     [8] = {0x00};
    c_u_char      E_FLAGS     [4] = {0x00};
    c_u_char      E_EHSIZE    [2] = {0x40, 0x00};
    c_u_char      E_PHENTSIZE [2] = {0x38, 0x00};
    c_u_char      E_PHNUM     [2] = {0x01, 0x00};
    c_u_char      E_SHENTSIZE [2] = {0x40, 0x00};
    c_u_char      E_SHNUM     [2] = {0x00};
    c_u_char      E_SHSTRNDX  [2] = {0x00};

};

struct ProgramHeader {

    const int       P_TYPE      = 0x00000001;
    const int       P_FLAGS     = 0x00000004;
    const size_t    P_OFFSET    = 0x0000000000000000;
    const size_t    P_VADDR     = 0x0000000000400000;
    const size_t    P_PADDR     = 0x0000000000400000;
    size_t          P_FILES;                            // Are these two fields equal?
    size_t          P_MEMSZ;                            //
    const size_t    P_ALIGN     = 0x0000000000001000;
    const size_t    P_SPAC      = 0x0000000500000001;

};

struct InputByteCode {

    char*  _byteCode;
    size_t _size;

};

struct Label {

    size_t inputVal     = 0;
    size_t outputVal    = 0;

};

enum CommandsNum {

    #define DEF_CMD(name, asmFunc, cpuFunc) name, 
    #include "../../../aleria_asm/commands"
    #undef  DEF_CMD

};

enum PassageNums {

    FirstPassage    = 1 ,
    SecondPassage       ,
    ThirdPassage

};

const   size_t MAX_CODE_SIZE    = 0x00010000;
const   size_t PROGRAM_START    = 0x00400000;
const   size_t MAX_LABEL_COUNT  = 100;
static  size_t STDLIB_SIZE      = 0;

int             HandleInputMode         (int argc, char* argv []);

//=============================KING - FUNCTION==============================
void            BinaryTranslation       (const char* input, const char* elfOutput);

//=================================PARSING==================================
void            ReadInputByteCode       (FILE* input, InputByteCode* _byteCodeStruct);
void            BytePassage             (char* JITBuffer, size_t passageNum, InputByteCode _myByteCode);


//========================TRANSLATE INTO BYTE CODE==========================
void            HandleInputByteCode     (FILE* input, char* JITBuffer);

void            PutCommandsIntoByteCode (char* JITBuffer, size_t countBytes, ...);

void            CallForStdLib           (char* JITBuffer, int jmpNum);
void            ImplementMath           (char* JITBuffer, int mathNum);
void            ImplementJmp            (char* JITBuffer, size_t passageNum, InputByteCode* _byteCodeStruct, int jmpNum);
void            ImplementPush           (char* JITBuffer, InputByteCode* _byteCodeStruct);
void            ImplementPop            (char* JITBuffer, InputByteCode* _byteCodeStruct);

//=======================BUILDING EXECUTABLE FILE==========================
FILE*           CreateELFFile           (const char* elfOutput);
void            IncludeStdLib           (char* JITBuffer);
void            PrintELFHeader          (char* buffer);

//==================================OTHER==================================
size_t          GetSizeOfFile           (FILE* input);
void            LabelSort               ();
