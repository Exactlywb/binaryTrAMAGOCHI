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

    JMP = 0xE9,
    JE  = 0x84,
    JNE = 0x85,
    JA  = 0x87,
    JB  = 0x82

};

#define CALL_BYTE_CODE                  1, 0xE8
#define PUSH_BYTE_CODE                  1, 0x6A
#define JMP_IN_QWORD                    2, 0xEB, 0x08

#define inputBuffer                     _byteCodeStruct->_byteCode
#define inputSize                       _byteCodeStruct->_size

#define SUB_RSP_FOR_XMM                 PutCommandsIntoByteCode (JITBuffer, 4, 0x48, 0x83, 0xEC, 0x08);
#define PUSH_XMM_INTO_STACK(xmmPostfix)  PutCommandsIntoByteCode (JITBuffer, 5, 0xF2, 0x0F, 0x11, 0x04 + 8 * xmmPostfix, 0x24);

#define GET_XMM_FROM_STACK(xmmPostfix)   PutCommandsIntoByteCode (JITBuffer, 5, 0xF2, 0x0F, 0x10, 0x04 + 8 * xmmPostfix, 0x24);     

#define PUSH_NUMBER                                                                                                 \
                                        PutCommandsIntoByteCode (JITBuffer, JMP_IN_QWORD);                          \
                                        size_t labelBeforeNum = programSize + PROGRAM_START + 0x80;                 \
                                        for (size_t byteNum = 0; byteNum < sizeof (double); byteNum++) {            \
                                                                                                                    \
                                            JITBuffer [programSize++] = inputBuffer [byteNum];                      \
                                                                                                                    \
                                        }                                                                           \
                                                                                                                    \
                                        inputBuffer += sizeof (double);                                             \
                                        inputSize   -= sizeof (double);                                             \
                                                                                                                    \
                                        PutCommandsIntoByteCode (JITBuffer, 5, 0xF2, 0x0F, 0x10, 0x3C, 0x25);       \
                                        *(size_t*)(JITBuffer + programSize) = labelBeforeNum;                       \
                                        programSize += 4;                                                           \
                                        SUB_RSP_FOR_XMM                                                             \
                                        PUSH_XMM_INTO_STACK (7)

#define POP_REGULAR                                                                                                 \
                                        PutCommandsIntoByteCode (JITBuffer, 4, 0x48, 0x83, 0xC4, 0x08);             //rsp += 8

#define PUT_CONDITION                                                                                               \
                                        PutCommandsIntoByteCode (JITBuffer, 4, 0x66, 0x0F, 0x2F, 0xF7);

#define IMPLEMENT_JMP_CONDITION                                                                                     \
                                        if (jmpNum != JMP) {                                                        \
                                                                                                                    \
                                            GET_XMM_FROM_STACK (6)                                                  \
                                            POP_REGULAR                                                             \
                                            GET_XMM_FROM_STACK (7)                                                  \
                                            POP_REGULAR                                                             \
                                            PUT_CONDITION                                                           \
                                            PutCommandsIntoByteCode (JITBuffer, 1, 0x0F);                           \
                                                                                                                    \
                                        }   

#define FILL_SPACE_ADDRESS                                                                                          \
                                        PutCommandsIntoByteCode (JITBuffer, 4, 0x0, 0x0, 0x0, 0x0);
