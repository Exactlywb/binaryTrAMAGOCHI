enum MathOperators {

    ADD  = 0x58,
    SUB  = 0x5C,
    MUL  = 0x59,
    DIV  = 0x5E,
    SQRT = 0x51

};

enum JmpTable {

    hltJmp  = 0x1,
    inJmp   = 0x3,
    outJmp  = 0x5

};

enum JmpNum {

    JMP  = 0xE9,
    JE   = 0x84,
    JNE  = 0x85,
    JA   = 0x87,
    JB   = 0x82,
    CALL = 0xE8

};

#define CALL_BYTE_CODE                  1, 0xE8

#define PUSH_BYTE_CODE                  1, 0x6A
#define JMP_IN_QWORD                    2, 0xEB, 0x08

#define inputBuffer                     _byteCodeStruct->_byteCode
#define inputSize                       _byteCodeStruct->_size
