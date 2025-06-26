#include "../library.h"

int main() {
    init_handlers();
    int *p = NULL;
    *p = 42;
    //THROW(EXC_INVALID_ARGUMENT);
}