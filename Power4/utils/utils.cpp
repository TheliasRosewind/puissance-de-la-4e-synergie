//
// Created by Alexis on 16/02/2020.
//

#include <cstdlib>
#include <cstdio>
#include <climits>
#include <cerrno>


bool convertStringToInt(char * string, int * result) {
    char* end = nullptr;
    errno = 0;
    long temp = strtol(string, &end, 10);

    if (*end == '\0' && errno != ERANGE && temp >= INT_MIN && temp <= INT_MAX) {
        *result = (int)temp;
        return true;
    }
    return false;
}

int clean_stdin() {
    while (getchar()!='\n');
    return 1;
}

bool convertStringToDouble(char* string, double* result) {
    char * end = nullptr;
    errno = 0;
    long temp = strtod(string, &end);

    if (*end == '\0' && errno != ERANGE) {
        *result = temp;
        return true;
    }
    return false;
}
