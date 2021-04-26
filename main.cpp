#include "binTranslator/binTranslator.h"

int main (int argc, char* argv []) {

    if (argc > 1) {
        
        BinaryTranslation (argv [1], "a.out");
        return 0;

    } else {

        printf ("There is no fileName to translate\n");
        return 1;

    }

}