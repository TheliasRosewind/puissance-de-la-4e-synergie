#include "game/power4.h"
#include "ai/mcts.h"
#include "utils/utils.h"
#include <cstdlib>
#include <cstdio>
#include <getopt.h>

void conflictArgument(int opt);

int main(int argc, char **argv) {

    //------- DISPLAY PARAMETERS ----------
    int verboseLevel = 1;

    // Defaul tvalue
    // time : 5s
    // iterations : non limité
    // méthode : robuste

    //------- ALGORITM PARAMETERS ----------
    int optimisationLevel = 1;
    MethodActionChoice methodChoice = ROBUSTE;                    // Choice method of best / action at the end of MCTS
    double time = 5;                                            // Compute time for an action with  MCTS (in seconds)
    int iterations = -1;                                        // Highest number of itreration of MCTS algorithm
    bool robustFlag = false, maxFlag = false, timeFlag = false;

    //------- PROGRAM PARAMETERS ----------
    static struct option long_options[] = {
            {"max",          no_argument,       0, 'm'},
            {"robust",       no_argument,       0, 'r'},
            {"time",         required_argument, 0, 't'},
            {"iterations",   required_argument, 0, 'i'},
            {"optimization", required_argument, 0, 'o'},
            {0,              0,                 0, 0}
    };
    int option_index = 0;
    opterr = 0;
    int opt = 0;


    // ------ READ PROGRAM PARAMETERS INPUT
    while ((opt = getopt_long(argc, argv, "hmrt:i:o:v:", long_options, &option_index)) != -1) {
        int intResult = 0;
        double doubleResult = 0;

        switch (opt) {
            //time limit for the MCTS algo
            case 't' :
                if (convertStringToDouble(optarg, &doubleResult) && doubleResult > 0) {
                    time = doubleResult;
                    timeFlag = true;
                } else {
                    fprintf(stderr, "Incorrect argument : %s.\n", optarg);
                    fprintf(stderr,"-t option require a decimal number (positive except zero).\n "
                                   "Use -h option for more information.\n");
                    return 1;
                }
                break;

                //number of iteration fot MCTS algo
            case 'i' :
                if (convertStringToInt(optarg, &intResult) && intResult > 0)
                    iterations = intResult;
                else {
                    fprintf(stderr, "Incorrect argument : %s.\n", optarg);
                    fprintf(stderr, "-i option requiere a positive integer (except zero).\n "
                                    "Use -h option for more information.\n");
                    return 1;
                }
                break;

            case 'm' :
                if (robustFlag) {   // Si le choix de robuste a déjà été fait
                    conflictArgument(opt);
                    return 1;
                }
                maxFlag = true;
                methodChoice = MAX;
                break;

                //Robust methode for the MCTS algo    //
            case 'r' :
                if (maxFlag) {   // Si le choix de max a déjà été fait
                    conflictArgument(opt);
                    return 1;
                }
                robustFlag = true;
                methodChoice = ROBUSTE;
                break;

                //Level of optimisationn of the MCTS algo
            case 'o' :
                if (convertStringToInt(optarg, &intResult) && intResult >= 0)
                    optimisationLevel = intResult;
                else {
                    fprintf(stderr, "Incorrect argument : %s.\n", optarg);
                    fprintf(stderr, "-o option require a positiv integer (zero include).\n "
                                    "Use -h option for more information.\n");
                    return 1;
                }
                break;

            case 'v' :
                if (convertStringToInt(optarg, &intResult) && intResult >= 0)
                    verboseLevel = intResult;
                else {
                    fprintf(stderr, "Incorrect argument : %s.\n", optarg);
                    fprintf(stderr, "-v option require a positive integer (zero include).\n "
                                    "Use -h option for more information.\n");
                    return 1;
                }
                break;

        }
    }

    // if iteration numbers is precise but not the time
    if (!timeFlag && iterations >= 0)
        time = -1;

    Action *action = nullptr;
    EndGame end;

    State *state = initialState();

    char c;

    do {
        printf("Who start? (Human : 0, IA : 1) ? ");
    } while (((scanf("%d%c", &(state->player), &c) != 2 || c != '\n') && clean_stdin()) ||
             (state->player != 0 && state->player != 1));

    do {
        printf("\n");
        displayGame(state);

        if (state->player != 0) {
            mcts(state, time, iterations, methodChoice, optimisationLevel, verboseLevel);
        } else {
            //Player turn
            int possibleAction = 0;
            do {
                action = askAction();
                if (action != nullptr) {
                    possibleAction = playAction(state, action);
                    free(action);
                }
            } while (!possibleAction);
        }

        end = endTest(state);
    } while (end == NO);

    printf("\n");
    displayGame(state);

    //Display the message in case of who win
    whoWin(end);

    free(state);

    return 0;
}

void conflictArgument(int opt) {
    fprintf(stderr, "Arguments Conflict : -%c.\n", opt);
    fprintf(stderr, "-r and -m option can't be used at the same time.\n "
                    "Use -h option for more information.\n");
}




