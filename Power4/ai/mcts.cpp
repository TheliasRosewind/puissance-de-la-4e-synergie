//
// Created by Alexis on 16/02/2020.
//
#include "mcts.h"
#include <cstdlib>
#include <cstdio>
#include <cfloat>
#include <cmath>
#include <ctime>

#define REWARD_IA_WIN 1
#define REWARD_DRAW 0.5
#define REWARD_PLAYER_WIN 0

#define CONSTANTE_C 1.4142

int totalSimusByProgramm = 0;

Node * newNode (Node * parent, Action * action) {
    Node * node = (Node *)malloc(sizeof(Node));

    if ( parent != nullptr && action != nullptr ) {
        node->state = copyState (parent->state );
        playAction (node->state, action );
        node->action = action;
        node->player = OTHER_PLAYER(parent->player);
    }
    else {
        node->state = nullptr;
        node->action = nullptr;
        node->player = 0;
    }
    node->parent = parent;
    node->nb_children = 0;

    //FOR MCTS:
    node->nb_victory = 0;
    node->sumReward = 0;
    node->nb_simus = 0;

    return node;
}

Node * newChild(Node * parent, Action * action) {
    Node * child = newNode(parent, action) ;
    parent->childrens[parent->nb_children] = child;
    parent->nb_children++;
    return child;
}

void freeNode (Node * node) {
    if (node->state != nullptr) free(node->state);

    while (node->nb_children > 0) {
        freeNode(node->childrens[node->nb_children - 1]);
        node->nb_children --;
    }
    if (node->action != nullptr) free(node->action);

    free(node);
}

double calculateBValueNode(Node * node) {
    if (node->nb_simus == 0)   return DBL_MAX;

    double rewardAverage = (double)node->sumReward / node->nb_simus;

    if (node->parent->player == 1) rewardAverage *= -1;

    return rewardAverage + CONSTANTE_C * sqrt(log(node->parent->nb_simus) / node->nb_simus );
}

Node * selectUCB(Node * racine) {
    Node * currentNode = racine;
    int i = 0;

    if (endTest(currentNode->state) != NO || currentNode->nb_children != numberOfPossibleAction(currentNode->state))
        return currentNode;
    Node * nodeMaxBValue = currentNode->childrens[0];
    double maxBValue = calculateBValueNode(nodeMaxBValue);
    for (i = 1 ; i < currentNode->nb_children ; i++) {
        double bValueCurrent = calculateBValueNode(currentNode->childrens[i]);
        if (maxBValue < bValueCurrent) {
            nodeMaxBValue = currentNode->childrens[i];
            maxBValue = bValueCurrent;
        }
    }
    return selectUCB(nodeMaxBValue);
}

Node * extendNode(Node * node) {
    if (endTest(node->state) != NO)  return node;
    Action ** actions = possibleAction(node->state);

    int k = 0;
    while (actions[k] != nullptr) {
        bool alreadyDevelopped = false;
        int i;
        for (i = 0 ; i < node->nb_children ; i++) {
            if (actions[k]->column == node->childrens[i]->action->column) {
                alreadyDevelopped = true;
                break;
            }
        }
        if (alreadyDevelopped) {
            free(actions[k]);
            int j = k;
            while (actions[j] != nullptr) {
                actions[j] = actions[j+1];
                j++;
            }
        }
        else k++;
    }
    int choice = rand() % k;
    k = 0;
    while (actions[k] != nullptr) {
        if (k != choice) free(actions[k]);
        k++;
    }
    Node * child = newChild(node, actions[choice]);
    free(actions);
    return child;
}

EndGame simulateGame(State * state, bool chooseWinAction) {
    EndGame endGameResult;
    while ((endGameResult = endTest(state)) == NO) {
        Action** actions = possibleAction(state);
        Action* actionToPlay = nullptr;
        if (chooseWinAction && state->player == 1) {
            int k = 0;
            while (actions[k] != nullptr && actionToPlay == nullptr) {
                State * stateToState = copyState(state);
                playAction(stateToState, actions[k]);
                if (endTest(stateToState) == PLAYER_WIN) actionToPlay = actions[k];
                free(stateToState);
                k++;
            }
            if (actionToPlay == nullptr)  actionToPlay = actions[rand() % k];
        }
        else {
            int k = 0;
            while (actions[k] != nullptr)    k++;
            actionToPlay = actions[rand() % k];
        }
        playAction(state, actionToPlay);
        int k = 0;
        while (actions[k] != nullptr) {
            free(actions[k]);
            k++;
        }
        free(actions);
    }
    return endGameResult;
}

void spreadResult(Node * node, EndGame result) {

    while (node != nullptr) {
        node->nb_simus++;
        switch(result) {
            case AI_WIN :
                node->nb_victory++;
                node->sumReward += REWARD_IA_WIN;
                break;
            case PLAYER_WIN :
                node->sumReward += REWARD_PLAYER_WIN;
                break;
            case DRAW :
                node->sumReward += REWARD_DRAW;
                break;
            default:
                break;
        }
        node = node->parent;
    }

}

Node * findNodeForBestAction(Node * root, MethodActionChoice method) {

    Node * nodeBestAction = root->childrens[0];
    int i = 1, maxSimus;
    double maxValue, currentValue;

    switch(method) {
        case ROBUSTE :
            maxSimus = nodeBestAction->nb_simus;

            for (i = 1 ; i < root->nb_children ; i++) {
                if (maxSimus < root->childrens[i]->nb_simus) {
                    nodeBestAction = root->childrens[i];
                    maxSimus = nodeBestAction->nb_simus;
                }
            }
            break;

        case MAX :
            if (nodeBestAction->nb_simus == 0) maxValue = 0;
            else maxValue = (double)nodeBestAction->sumReward / nodeBestAction->nb_simus;

            for (i = 1 ; i < root->nb_children ; i++) {
                if (root->childrens[i]->nb_simus == 0) currentValue = 0;
                else currentValue = (double)root->childrens[i]->sumReward / root->childrens[i]->nb_simus;

                if (maxValue < currentValue) {
                    nodeBestAction = root->childrens[i];
                    maxValue = currentValue;
                }
            }
            break;
    }
    return nodeBestAction;
}

void mcts(State * state, double timeMax, int iterationMax, MethodActionChoice method, int optimisationLevel, int verboseLevel) {

    if (timeMax <= 0 && iterationMax <= 0) {
        fprintf(stderr, "Stop condition is inexistent for MCTS : the time or the number of max iteration must be specified.");
        exit(EXIT_FAILURE);
    }
    clock_t tic, toc;
    tic = clock();
    double time = 0;
    Action ** actions;
    Node * nodeBestAction = nullptr;
    Action * bestAction;
    Node * root = newNode(nullptr, nullptr);
    root->state = copyState(state);
    actions = possibleAction(root->state);
    int k = 0;
    Node * child;
    while ( actions[k] != nullptr) {
        child = newChild(root, actions[k]);
        k++;
        if (optimisationLevel >= 2 && endTest(child->state) == PLAYER_WIN)  nodeBestAction = child;
    }
    int iter = 0;
    if (nodeBestAction == nullptr) {
        do {
            Node * selectedNode = selectUCB(root);
            child = extendNode(selectedNode);
            State * stateCopie = copyState(child->state);
            bool chooseWwinAction = optimisationLevel >= 1;
            EndGame result = simulateGame(stateCopie, chooseWwinAction);
            free(stateCopie);
            spreadResult(child, result);
            toc = clock();
            time = ((double) (toc - tic)) / CLOCKS_PER_SEC;
            iter ++;
        } while ((timeMax <= 0 || time < timeMax) && (iterationMax <= 0 || iter < iterationMax) );
        nodeBestAction = findNodeForBestAction(root, method);
    }
    bestAction = nodeBestAction->action;
    if (verboseLevel >= 2)
        printf("\nTime used       : %0.4fs"
               "\nNumber of iterations : %d\n", time, iter);

    if (verboseLevel >= 3) {
        int i;
        for (i=0 ; i < root->nb_children ; i++) {
            Node * noeud = root->childrens[i];
            printf("\nFor the action in column %d :  Number of simulations   : %d", noeud->action->column, noeud->nb_simus);
            if (verboseLevel >= 4) {
                printf("\n                              Average for reward : ");
                if (noeud->nb_simus > 0) printf("%0.4f", (double)noeud->sumReward / noeud->nb_simus);
                else printf("None");
            }
        }
        printf("\n");
    }
    if (verboseLevel >= 1) {
        printf("\nAction play in column %d", nodeBestAction->action->column);
        printf("\nTotal number of simulations : %d", root->nb_simus);
        printf("\nEstimate probability of victory for AI : ");
        if (nodeBestAction->nb_simus > 0) printf("%0.2f %%", (double)nodeBestAction->nb_victory / nodeBestAction->nb_simus * 100);
        else printf("None");
        totalSimusByProgramm += root->nb_simus;
        printf("\nSum of simulations : %d", totalSimusByProgramm);
        printf("\n");

    }
    playAction(state, bestAction);
    freeNode(root);
    free (actions);
}
