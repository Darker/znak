#include <stdio.h>
#include <emscripten/emscripten.h>

#include "znak/znak.h"


#ifdef __INTELLISENSE__
    #define EMSCRIPTEN_KEEPALIVE
#endif

int main() {
    printf("fdsfgdhgsdhyt\n");
    return 0;
}

#ifdef __cplusplus
#define EXTERN extern "C"
#else
#define EXTERN
#endif

EXTERN EMSCRIPTEN_KEEPALIVE void myFunction(int argc, char ** argv) {
    printf("MyFunction Called\n");
}
