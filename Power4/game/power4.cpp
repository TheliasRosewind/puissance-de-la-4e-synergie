#include "power4.h"
#include "../utils/utils.h"
#include <cstdio>
#include <cstdlib>

// Macros
#define ARRAY_LENGTH(x)  (sizeof(x) / sizeof((x)[0]))

State * copyState(State * src) {
    State * state = (State *)malloc(sizeof(State));

    state->player = src->player;

    int i, j;
    for (i=0; i < ARRAY_LENGTH(state->boardGame); i++)
        for ( j=0; j < ARRAY_LENGTH(state->boardGame[0]); j++)
            state->boardGame[i][j] = src->boardGame[i][j];

    return state;
}

State * initialState(void) {
    State * state = (State *)malloc(sizeof(State));

    int i, j;
    for (i=0; i < ARRAY_LENGTH(state->boardGame); i++)
        for ( j=0; j < ARRAY_LENGTH(state->boardGame[0]); j++)
            state->boardGame[i][j] = ' ';

    return state;
}

void displayGame(State * state) {

    int i, j;
    printf("   |");
    for ( j = 0; j < ARRAY_LENGTH(state->boardGame[0]); j++)
        printf(" %d |", j);
    printf("\n");
    printf("--------------------------------");
    printf("\n");

    for(i=0; i < ARRAY_LENGTH(state->boardGame); i++) {
        printf(" %d |", i);
        for ( j = 0; j < ARRAY_LENGTH(state->boardGame[0]); j++)
            printf(" %c |", state->boardGame[i][j]);
        printf("\n");
        printf("--------------------------------");
        printf("\n");
    }
}

Action * newAction(int col) {
    Action * action = (Action *)malloc(sizeof(Action));

    action->column = col;

    return action;
}

Action * askAction(void) {
    int col = -1;
    char c;
    printf(" Wich column ? ") ;
    if ( (scanf("%d%c", &col, &c)!=2 || c!='\n') && clean_stdin() )
        return NULL;

    return newAction(col);
}

int playAction(State * state, Action * action) {

    // The Action is impossible if it's not in the limit
    // ou si la column est rempli (donc premiére ligne occupée)
    if (action->column < 0 || action->column >= ARRAY_LENGTH(state->boardGame[0]) ||
        state->boardGame[0][action->column] != ' ')
        return 0;

    // Find the line where the pawn where go
    int playedLine = -1;
    int row = ARRAY_LENGTH(state->boardGame) - 1; // start of the bottom of the boardGame (derniére ligne donc)
    while (row >=0 && playedLine == -1) {
        if (state->boardGame[row][action->column] == ' ') // if this place is free
            playedLine = row;
        else
            row--;
    }

    // cas d'erreur impossible en temps normal
    if (playedLine == -1)
        return 0;

    state->boardGame[playedLine][action->column] = state->player ? 'O' : 'X';

    //Next player
    state->player = OTHER_PLAYER(state->player);

    return 1;
}

Action ** possibleAction(State * state) {
    Action ** action = (Action **) malloc((1 + LARGEUR_MAX) * sizeof(Action *) );
    int k = 0;
    int column;
    // on parcourt les colonnes
    for(column=0; column < ARRAY_LENGTH(state->boardGame[0]); column++) {
        // on vérifie que la column courante n'est pas remplie (donc première ligne non occupée)
        if (state->boardGame[0][column] == ' ' ) {
            action[k] = newAction(column);
            k++;
        }
    }
    action[k] = NULL;
    return action;
}

int numberOfPossibleAction(State * state) {
    int count = 0;
    int column;
    // on parcourt les colonnes
    for(column=0; column < ARRAY_LENGTH(state->boardGame[0]); column++)
        // on vérifie que la column courante n'est pas remplie (donc premiére ligne non occupée)
        if (state->boardGame[0][column] == ' ' )
            count++;
    return count;
}

EndGame endTest(State * state) {

    int largeurPlateau = ARRAY_LENGTH(state->boardGame);
    int hauteurPlateau = ARRAY_LENGTH(state->boardGame[0]);

    // tester si un player a gagné
    int i, j, k, n = 0;
    for (i=0; i < largeurPlateau; i++) {
        for(j=0; j < hauteurPlateau; j++) {
            if (state->boardGame[i][j] != ' ') {
                n++;	// nb coups joués

                // lignes
                k=0;
                while ( k < 4 && i+k < largeurPlateau && state->boardGame[i + k][j] == state->boardGame[i][j] )
                    k++;
                if ( k == 4 )
                    return state->boardGame[i][j] == 'O' ? AI_WIN : PLAYER_WIN;

                // colonnes
                k=0;
                while ( k < 4 && j+k < hauteurPlateau && state->boardGame[i][j + k] == state->boardGame[i][j] )
                    k++;
                if ( k == 4 )
                    return state->boardGame[i][j] == 'O' ? AI_WIN : PLAYER_WIN;

                // diagonales
                k=0;
                while ( k < 4 && i+k < largeurPlateau && j+k < hauteurPlateau && state->boardGame[i + k][j + k] == state->boardGame[i][j] )
                    k++;
                if ( k == 4 )
                    return state->boardGame[i][j] == 'O' ? AI_WIN : PLAYER_WIN;

                k=0;
                while ( k < 4 && i+k < largeurPlateau && j-k >= 0 && state->boardGame[i + k][j - k] == state->boardGame[i][j] )
                    k++;
                if ( k == 4 )
                    return state->boardGame[i][j] == 'O' ? AI_WIN : PLAYER_WIN;
            }
        }
    }

    // test if macth is draw
    if ( n == ARRAY_LENGTH(state->boardGame) * ARRAY_LENGTH(state->boardGame[0]) )
        return DRAW;

    return NO;
}

int whoWin(EndGame end){
    if (end == AI_WIN )
        printf( "** IA WIN **\n");
    else if (end == DRAW )
        printf(" It's a DRAW !  \n");
    else
        printf( "** WELL DONE - YOU WIN  **\n");

    return 0;
}
