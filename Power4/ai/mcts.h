//
// Created by Alexis on 16/02/2020.
//

#ifndef POWER4_MCTS_H
#define POWER4_MCTS_H

#include "../game/power4.h"


/**
    Fonctions d'implémentation de l'algorithme MCTS avec UCB (UCT).
*/

/** Méthode du choix du action à jouer pour MCTS */
typedef enum { MAX, ROBUSTE } MethodActionChoice;

/** Definition du type Node */
typedef struct NodeSt {

    int player; // player who played for arrived here
    Action * action;   // action played by this playyer to arrived here

    State * state;

    struct NodeSt * parent;
    struct NodeSt * childrens[LARGEUR_MAX]; // childrens list rach children associate to a possible action
    int nb_children;	// number of children in the list

    // FOR MCTS:
    int nb_victory;
    double sumReward;
    int nb_simus;

} Node;

/** Create a new node when play an action from a parent
 * Note: use newNode(NULL, NULL) for create root */
Node * newNode (Node * parent, Action * action);

/** Add a child to a parent when play an action
 * retourn the pointer on the added child */
Node * newChild(Node * parent, Action * action);

/** Free the memory of a node and his children recursively */
void freeNode (Node * node);

/** Calculate the B-value of a node */
double calculateBValueNode(Node * noeud);

/** Select recursively at start of the root (in param) the Node with the biggest B-Value
 * until arrived to a terminal node or one of all children will nt be developed */
Node * selectUCB(Node * racine);

/** Realize the extends of a node. Develop one of his child randomly and return this child
    If the node is a terminal node, return itself */
Node * extendNode(Node * node);

/** Simulate a game from a state until the end of the simulate game.
 *  return the terminal state
 *  if chooseWinAction, increase the simulation (use win-action when it's possible) */
EndGame simulateGame(State * state, bool chooseWinAction);

/** Spread the result from a node and ride upp the result of the game to the parent of the node */
void spreadResult(Node * node, EndGame result);

/** Find the node who match with the best possible action with use the specified method (start at root) */
Node * findNodeForBestAction(Node * root, MethodActionChoice method);

/** MCTS-UCT Algorithm
    timeMax : (seconds) : max time to executethe algorithm
    iterationMax : max iteration of the algorithm
    Only the more limited argument is use.
    If an argument was NULL or negativ, it was ignored
    methodChoice : method for choose the action

    ------ Optimisation level of the Algorithm ------
        - 1 (Default) : (Q3) : increase the simulation -> always choose win-actions when it's possible
        - 0 : basis use of the MCTS Algorithm (the simulation was realized randomly)
        - 2 : Algorithe was not execute when a win-action is possible. The action was played directly.

     ------ Verbose level of the Program ------
        - 0 : Dislplay only the game and the action question (for player)
        - 1 (Default) : (QUESTION 1:) Display the current action played, the number of total simulation realised
                and an estimation of the probability of win for the AI.
        - 2 : Display  the time used in the principal iteration of the algorithm MCTS and the number of iteration realised.
        - 3 : Display  the number of simulation realised for each action
        - 4 : Display  the average of the reward for each action
    */
void mcts(State * etat, double tempsmax, int iterationsmax, MethodActionChoice methodeChoix, int optimisationLevel, int verboseLevel);


#endif //POWER4_MCTS_H
