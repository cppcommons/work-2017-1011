#include <stdio.h>

#include <iostream>
#include <utility>
#ifndef __dso_handle
//void*   __dso_handle = (void*) &__dso_handle;
#endif

int main() {
    try {
        float age;
        std::cout << "How old are you?\n";
        //std::cin >> age;
        //std::cout << "You are " << age << " years (or whatever) old\n";
    } catch ( std::ios::failure & ) {
        std::cout << "Sorry, didn't understand that.\n";
        throw;
    }
    return 0;
}
