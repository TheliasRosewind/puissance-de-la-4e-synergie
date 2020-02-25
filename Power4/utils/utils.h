//
// Created by Alexis on 16/02/2020.
//

#ifndef POWER4_UTILS_H
#define POWER4_UTILS_H

/**
   Tools function.
*/

/** Convert an string (char*) in integer and store the result in result.
    Retourn true if the conversion end with no error, otherwise false. */
bool convertStringToInt(char * string, int * result);

/** Clean standard entry */
int clean_stdin();

/** Convertit un string (char*) en double et stocke le résultat dans result.
    Retourne true si la conversion s'est déroulée correctement, false sinon. */
bool convertStringToDouble(char * string, double * result);

#endif //POWER4_UTILS_H
