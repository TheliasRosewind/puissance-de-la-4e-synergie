#ifndef POWER4_H_INCLUDED
#define POWER4_H_INCLUDED

// Game param
#define LARGEUR_MAX 7 		// max number of child for a node

// Macros
#define OTHER_PLAYER(i) (1-(i))

/** Endpoint for end game */
typedef enum { NO, DRAW, AI_WIN, PLAYER_WIN } EndGame;

/** Definition of State State (état/position du jeu) */
typedef struct {

    int player; // Who's play

    // 6 lines - 7 row
    char boardGame[6][7];

} State;

/** Definition of Action type */
typedef struct {

    int column;

} Action;

/** Copy a State */
State * copyState(State * src);

/** Initial State */
State * initialState();

/** Display the boardGame */
void displayGame(State * state);

/** New Action */
Action * newAction(int col);

/** Ask to the player wich action play
    Return NULL if input is incorrect */
Action * askAction();

/** Modify the state in play an action
    Return 0 if the action is impossible*/
int playAction(State * state, Action * action);

/** Return o list of possible actions at start of a state
    (array of pointer of action ended by NULL) */
Action ** possibleAction(State * state);

/** Count the number of actions were possible */
int numberOfPossibleAction(State * state);

/** Test if the state is an termina state
    Return NO, DRAW, AI_WIN ou PLAYER_WIN */
EndGame endTest(State * state );

int whoWin(EndGame end);

#endif
