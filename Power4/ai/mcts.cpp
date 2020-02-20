//
// Created by Alexis on 16/02/2020.
//
#include "mcts.h"
#include <cstdlib>
#include <cstdio>
#include <cfloat>
#include <cmath>
#include <ctime>

// Constantes/paramètres Algo MCTS
#define REWARD_IA_WIN 1
#define REWARD_DRAW 0.5
#define REWARD_PLAYER_WIN 0

#define CONSTANTE_C 1.4142  // ~ Racine carré de 2

Node * newNode (Node * parent, Action * action) {
    Node * node = (Node *)malloc(sizeof(Node));

    if ( parent != NULL && action != NULL ) {
        node->state = copyState (parent->state );
        playAction (node->state, action );
        node->action = action;
        node->player = OTHER_PLAYER(parent->player);
    }
    else {
        node->state = NULL;
        node->action = NULL;
        node->player = 0;
    }
    node->parent = parent;
    node->nb_children = 0;

    // POUR MCTS:
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
    if (node->state != NULL)
        free(node->state);

    while (node->nb_children > 0) {
        freeNode(node->childrens[node->nb_children - 1]);
        node->nb_children --;
    }
    if (node->action != NULL)
        free(node->action);

    free(node);
}

double calculateBValueNode(Node * node) {
    if (node->nb_simus == 0)        //The node have any simulation
        return DBL_MAX;

    double rewardAverage = (double)node->sumReward / node->nb_simus;

    if (node->parent->player == 1)
        rewardAverage *= -1;    
    // *-1 si le node parent est un node Min = si le action joué pour arriver ici a été effectué par l'ordinateur

    return rewardAverage + CONSTANTE_C * sqrt(log(node->parent->nb_simus) / node->nb_simus );
}

Node * selectUCB(Node * racine) {
    Node * currentNode = racine;
    int i = 0;

    // Si on arrive à un noeud terminal ou un dont tous les fils n'ont pas été développés
    if (endTest(currentNode->state) != NO || currentNode->nb_children != numberOfPossibleAction(currentNode->state))
        return currentNode;

    // Sinon, on sélectionne le fils possédant la B-valeur maximale
    Node * nodeMaxBValue = currentNode->childrens[0];
    double maxBValue = calculateBValueNode(nodeMaxBValue);
    for (i = 1 ; i < currentNode->nb_children ; i++) {
        double bValueCurrent = calculateBValueNode(currentNode->childrens[i]);
        if (maxBValue < bValueCurrent) {
            nodeMaxBValue = currentNode->childrens[i];
            maxBValue = bValueCurrent;
        }
    }
    // Appel récursif sur le fils possédant la B-valeur maximale
    return selectUCB(nodeMaxBValue);
}

Node * extendNode(Node * node) {
    if (endTest(node->state) != NO)    // Si le node représente un état final
        return node;                   // on ne le développe pas

    Action ** actions = possibleAction(node->state);

    // On enlève les actions correspondant aux fils existants du node
    int k = 0;
    while (actions[k] != NULL) {

        bool alreadyDevelopped = false;

        int i;
        for (i = 0 ; i < node->nb_children ; i++) {
            // Si il s'agit du même action
            if (actions[k]->column == node->childrens[i]->action->column) {
                alreadyDevelopped = true; // ce action a déjà un fils correspondant
                break;              // on arrête le parcours des fils
            }
        }

        if (alreadyDevelopped) {  // si ce action a déjà un fils correspondant
            // On supprime ce action et on décale la liste de actions
            free(actions[k]);
            int j = k;
            while (actions[j] != NULL) {
                actions[j] = actions[j+1];
                j++;
            }
        }
        else
            k++;
    }

    // On développe un fils au hasard
    int choice = rand() % k;

    // On libère la mémoire des actions inutilisés
    k = 0;
    while (actions[k] != NULL) {
        if (k != choice)
            free(actions[k]); // sauf celle du action joué qui sera libérée dans freenoeud
        k++;
    }

    Node * child = newChild(node, actions[choice]);
    free(actions);    // On libère la mémoire de la liste de actions
    return child;
}

EndGame simulateGame(State * state, bool chooseWinAction) {
    EndGame endGameResult;
    // Tant que la partie n'est pas terminée
    while ((endGameResult = endTest(state)) == NO) {
        Action** actions = possibleAction(state);
        Action* actionToPlay = NULL;

        // Si on doit choisir un action gagnant quand cela est possible (si c'est le tour de l'ordinateur)
        if (chooseWinAction && state->player == 1) {
            int k = 0;
            // On teste tous les actions pour voir si un des actions est gagnant
            while (actions[k] != NULL && actionToPlay == NULL) {
                State * stateToState = copyState(state);
                playAction(stateToState, actions[k]);

                if (endTest(stateToState) == PLAYER_WIN)
                    actionToPlay = actions[k];

                free(stateToState);

                k++;
            }
            // Si aucun action gagnant n'est possible
            if (actionToPlay == NULL)
                actionToPlay = actions[rand() % k];  // On joue un action aléatoirement
        }
            // Sinon, on choisit le action aléatoirement
        else {
            int k = 0;
            while (actions[k] != NULL)    k++;
            actionToPlay = actions[rand() % k];  // On joue un action aléatoirement
        }

        playAction(state, actionToPlay);  // On joue le action

        // On libère la mémoire
        int k = 0;
        while (actions[k] != NULL) {
            free(actions[k]);
            k++;
        }
        free(actions);
    }

    return endGameResult;
}

void spreadResult(Node * node, EndGame result) {

    while (node != NULL) {
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

    switch(method) {   // max simulations
        case ROBUSTE :
            maxSimus = nodeBestAction->nb_simus;

            for (i = 1 ; i < root->nb_children ; i++) {
                if (maxSimus < root->childrens[i]->nb_simus) {
                    nodeBestAction = root->childrens[i];
                    maxSimus = nodeBestAction->nb_simus;
                }
            }
            break;

        case MAX :      // max valeurs
            if (nodeBestAction->nb_simus == 0)
                maxValue = 0;
            else
                maxValue = (double)nodeBestAction->sumReward / nodeBestAction->nb_simus;

            for (i = 1 ; i < root->nb_children ; i++) {
                if (root->childrens[i]->nb_simus == 0)
                    currentValue = 0;
                else
                    currentValue = (double)root->childrens[i]->sumReward / root->childrens[i]->nb_simus;

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

    // Condition d'arrêt de l'algorithme inexistante
    if (timeMax <= 0 && iterationMax <= 0) {
        fprintf(stderr, "Condition d'arrêt inexsitante pour l'algorithme MCTS : le time ou le nombre d'itérations maximal doit être précisé.");
        exit(EXIT_FAILURE);
    }

    clock_t tic, toc;
    tic = clock();
    double time = 0;

    Action ** actions;
    Node * nodeBestAction = NULL;
    Action * bestAction;

    // Créer l'arbre de recherche
    Node * root = newNode(NULL, NULL);
    root->state = copyState(state);

    // créer les premiers noeuds:
    actions = possibleAction(root->state);
    int k = 0;
    Node * child;
    while ( actions[k] != NULL) {
        child = newChild(root, actions[k]);
        k++;
        // Si le niveau d'optimisation et suffisant et qu'un action gagnant est possible
        if (optimisationLevel >= 2 && endTest(child->state) == PLAYER_WIN)
            nodeBestAction = child;   // on le joue tout de suite
    }

    /* Algorithme MCTS-UCS */
    int iter = 0;

    if (nodeBestAction == NULL) {    // Optimisation
        do {
            // Sélection
            Node * selectedNode = selectUCB(root);
            // Expansion
            child = extendNode(selectedNode);
            // Simulation
            State * stateCopie = copyState(child->state);
            bool chooseWwinAction = optimisationLevel >= 1;
            EndGame result = simulateGame(stateCopie, chooseWwinAction);
            free(stateCopie);
            // Propagation
            spreadResult(child, result);

            toc = clock();
            time = ((double) (toc - tic)) / CLOCKS_PER_SEC;
            iter ++;
        } while ((timeMax <= 0 || time < timeMax) && (iterationMax <= 0 || iter < iterationMax) );

        // On cherche le meilleur action possible
        nodeBestAction = findNodeForBestAction(root, method);
    }

    bestAction = nodeBestAction->action;

    /* fin de l'algorithme  */

    // Affichage du time passé dans la boucle principale de l'algorithme MCTS et du nombre d'itérations.
    if (verboseLevel >= 2)
        printf("\nTemps utilisé       : %0.4fs"
               "\nNombre d'itérations : %d\n", time, iter);

    // Affichage du nombre de simulations réalisées pour chaque action
    if (verboseLevel >= 3) {
        int i;
        for (i=0 ; i < root->nb_children ; i++) {
            Node * noeud = root->childrens[i];
            printf("\nPour le action en colonne %d :   Nombre de simulations   : %d", noeud->action->column, noeud->nb_simus);
            // et de la récompense moyenne pour chaque action
            if (verboseLevel >= 4) {
                printf("\n                              Moyenne des récompenses : ");
                if (noeud->nb_simus > 0)
                    printf("%0.4f", (double)noeud->sumReward / noeud->nb_simus);
                else
                    printf("aucune");
            }
        }
        printf("\n");
    }

    // Affichage du nombre de simulations réalisées pour calculer le meilleur action
    // et une estimation de la probabilité de victoire pour l'ordinateur
    if (verboseLevel >= 1) {
        printf("\nCoup joué en colonne %d", nodeBestAction->action->column);
        printf("\nNombre total de simulations : %d", root->nb_simus);
        printf("\nEstimation de probabilité de victoire pour l'ordinateur : ");
        if (nodeBestAction->nb_simus > 0)
            printf("%0.2f %%", (double)nodeBestAction->nb_victory / nodeBestAction->nb_simus * 100);
        else
            printf("aucune");
        printf("\n");
    }

    // Jouer le meilleur premier action
    playAction(state, bestAction);

    // Penser à libérer la mémoire :
    freeNode(root);
    free (actions);
}
