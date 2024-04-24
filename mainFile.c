#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include  <ctype.h>

#define MAX_INPUT_LENGTH 1025

// special words in the system
const char *keyWords[] = {"buy", "sell", "go", "to", "from",
                          "at", "has", "more", "less", "than",
                          "and", "NOTHING", "NOWHERE", "NOBODY", "where", "who",
                          "total", "if", "exit"};
// these reserved words are not allowed to be used in the input

const char *reservedWords[] = {"NOTHING", "NOWHERE", "NOBODY","exit"};
const char *actionWords[] = {"buy", "sell", "go"};

////////////////////////////////////////////////////

// ALL STRUCTURES FOR PROJECT

// struct of 3 entities

typedef struct {
    char *name; // we need  to allocate memory
    int number;  // we need to allocate memory
} item;

typedef struct {
    char *name;  // we need  to allocate memory
    item items[100];  // we need to allocate memory
    char *location;
} subject;

typedef struct {
    char *name;  // we need  to allocate memory
    char subjectNames[][200];   // we need to allocate memory
} location;


// structures of the conditions
typedef enum {
    HAS,
    MORE,
    LESS,
    AT
} ConditionType;

typedef struct {
    int subjectIndex1;
    int subjectIndex2;
    int itemIndex1;
    int itemIndex2;
    int locationIndex;
    ConditionType conditionType;
    int conditionResult; // -1 (false), 0 (not evaluated), 1 (true)
} Condition;


// structures of  actions
typedef enum {
    SELL,
    BUY,
    GO,
    TO,
    FROM,
} ActionType;

typedef struct {
    int subjectIndex1;
    int subjectIndex2;
    int itemIndex1;
    int itemIndex2;
    int conditionIndex1;
    int conditionIndex2;
    int locationIndex;
    ActionType actionType;
    int actionResult;
} Action;

//END OF THE STRUCTURES


/*
 * These 5 pointer to pointer to "x" are used to parse the line and keep the syntax and meaning of the line.
 * After finishing the parsing and giving meaning to line, there memory will be free.
 *
 */
Action **actionsReal;
subject **pSubjects;
item **pItems;
location **pLocations;
Condition **pConditions;

/*These helper methods are for checking the validness or some other things for tokens.
 */

// validness of the  subject
int is_valid_subject_or_item(const char *token) {
    while (*token) {
        if (!isalpha(*token) && *token != '_') {
            return 0; // it is not valid
        }
        token++;
    }
    return 1;
}

int is_numeric(const char *str) {
    while (*str) {
        if (!isdigit(*str)) // If any character is not a digit, return 0
            return 0;
        str++;
    }
    return 1; // If all characters are digits, return 1
}

int isActionWord(char *currentWord) {
    // Iterate through forbidden words array
    for (int j = 0; j < sizeof(actionWords) / sizeof(actionWords[0]); j++) {
        // if the entity is equal to a forbidden word, return 1
        if (strcmp(currentWord, actionWords[j]) == 0) {
            return 1;
        }
    }
    // no forbidden words found, return 0
    return 0;
}

// check if the line is valid, returns 0 if it is not valid, returns 1 if it is valid
int isReservedWordUsed(char *currentLine[], int size) {
    int reservedWordCount = 4;

    for (int i = 0; i < size; i++) {
        // get the current token and check if it is a reserved word
        char *token = currentLine[i];
        for (int j = 0; j < reservedWordCount; j++) {
            if (strcmp(token, reservedWords[j]) == 0) {
                return 0; // indicates that the line is invalid
            }
        }
    }

    // check other invalid cases
    return 1; // indicates that no reserved word is used
}

int containsForbiddenWords(const char *word) {
    // Iterate through forbidden words array
    for (int j = 0; j < sizeof(keyWords) / sizeof(keyWords[0]); j++) {

        // If the entity is equal to a forbidden word, return 1
        if (strcmp(word, keyWords[j]) == 0) {

            return 1;
        }

    }
    // No forbidden words found, return 0
    return 0;
}


int creatingListsFromInStr(char *token[], int size) {
    /* This method is the main parsing method of the project. It takes the tokens of the line and
     * parses it to actions, conditions, locations, subjects and items.
     *
     */
    int actionNum = 0;
    int conditionNum = 0;
    int locationNum = 0;
    int subjectNum = 0;
    int itemNum = 0;

    int subjectWhereWeLeave = 0;
    int itemWhereWeLeave = 0;
    int locationWhereWeLeave = 0;
    int actionWhereWeLeave = 0;
    int conditionWhereWeLeave = 0;

    // Allocate memory for arrays
    actionsReal = (Action **) malloc(size * sizeof(Action *));
    pItems = (item **) malloc(size * sizeof(item *));
    pSubjects = (subject **) malloc(size * sizeof(subject *));
    pLocations = (location **) malloc(size * sizeof(location *));
    pConditions = (Condition **) malloc(size * sizeof(Condition *));

    actionsReal[0] = NULL;
    pItems[0] = NULL;
    pSubjects[0] = NULL;
    pLocations[0] = NULL;
    pConditions[0] = NULL;

    for (int i = 0; i < size; i++) { // iterate through the tokens
        if (strcmp((token[i]), "sell") == 0) { // if the action is sell
            actionsReal[actionNum++] = (Action *) malloc(sizeof(Action));
            actionsReal[actionNum - 1]->actionType = SELL;
            actionsReal[actionNum - 1]->subjectIndex1 = subjectWhereWeLeave;
            actionsReal[actionNum - 1]->subjectIndex2 = subjectNum - 1;
            actionsReal[actionNum - 1]->conditionIndex1 = -1;
            actionsReal[actionNum - 1]->conditionIndex2 = -1;
            actionsReal[actionNum - 1]->locationIndex = -1;
            subjectWhereWeLeave = subjectNum;
            actionsReal[actionNum] = NULL;
            // for items
            actionsReal[actionNum - 1]->itemIndex1 = itemWhereWeLeave;
            while (i < size) { // if there are multiple items
                if (i + 2 < size && is_numeric(token[++i])) {
                    pItems[itemNum++] = (item *) malloc(sizeof(item));
                    pItems[itemNum - 1]->name = token[i + 1];
                    pItems[itemNum - 1]->number = atoi(token[i]);
                    pItems[itemNum] = NULL;
                    if ((i + 3 < size) && is_numeric(token[i + 3])) { // if there are more items
                        if (strcmp(token[i + 2], "and") == 0) {

                            i += 2;
                            continue;
                        } else return 0;
                    } else if ((i + 3 < size) && strcmp(token[i + 2], "if") == 0) { // if there is a condition
                        itemWhereWeLeave = itemNum;
                        actionsReal[actionNum - 1]->itemIndex2 = itemNum - 1;
                        i += 1;
                        break;
                    } else if ((i + 3 < size) && strcmp(token[i + 2], "and") == 0) { // if there are multiple sentences
                        itemWhereWeLeave = itemNum;
                        actionsReal[actionNum - 1]->itemIndex2 = itemNum - 1;
                        i += 1;
                        break;
                    } else if ((i + 3 < size) && strcmp(token[i + 2], "to") == 0) { // if there is a buyer
                        itemWhereWeLeave = itemNum;
                        actionsReal[actionNum - 1]->itemIndex2 = itemNum - 1;
                        i += 1;
                        break;
                    } else { // if the sentence is finished
                        itemWhereWeLeave = itemNum;
                        i += 1; // to end the sentence
                        actionsReal[actionNum - 1]->itemIndex2 = itemNum - 1;
                        break;
                    }
                } else {
                    return 0; // it should be invalid
                }
            }
        } else if (strcmp(token[i], "buy") == 0) {
            actionsReal[actionNum++] = (Action *) malloc(sizeof(Action));
            actionsReal[actionNum - 1]->actionType = BUY;
            actionsReal[actionNum - 1]->subjectIndex1 = subjectWhereWeLeave;
            actionsReal[actionNum - 1]->subjectIndex2 = subjectNum - 1;
            actionsReal[actionNum - 1]->conditionIndex1 = -1;
            actionsReal[actionNum - 1]->conditionIndex2 = -1;
            actionsReal[actionNum - 1]->locationIndex = -1;
            subjectWhereWeLeave = subjectNum;
            actionsReal[actionNum] = NULL;
            // for items
            actionsReal[actionNum - 1]->itemIndex1 = itemWhereWeLeave;
            while (i < size) {
                if ((i + 2 < size) && is_numeric(token[++i])) {
                    pItems[itemNum++] = (item *) malloc(sizeof(item));
                    pItems[itemNum - 1]->name = token[i + 1];
                    pItems[itemNum - 1]->number = atoi(token[i]);
                    pItems[itemNum] = NULL;
                    if ((i + 3 < size) && is_numeric(token[i + 3])) {
                        if (strcmp(token[i + 2], "and") == 0) {
                            i += 2;
                            continue;
                        } else return 0;
                    } else if ((i + 3 < size) && strcmp(token[i + 2], "if") == 0) {
                        itemWhereWeLeave = itemNum;
                        actionsReal[actionNum - 1]->itemIndex2 = itemNum - 1;
                        i += 1;
                        break;
                    } else if ((i + 2 < size) && strcmp(token[i + 2], "and") == 0) {
                        itemWhereWeLeave = itemNum;
                        actionsReal[actionNum - 1]->itemIndex2 = itemNum - 1;
                        i += 1;
                        break;
                    } else if ((i + 2 < size) && strcmp(token[i + 2], "from") == 0) {
                        itemWhereWeLeave = itemNum;
                        actionsReal[actionNum - 1]->itemIndex2 = itemNum - 1;
                        i += 1;
                        break;
                    } else { // if the sentence is finished
                        itemWhereWeLeave = itemNum;
                        i += 1; // to end the sentence
                        actionsReal[actionNum - 1]->itemIndex2 = itemNum - 1;
                        break;
                    }
                } else {
                    return 0; // it should be invalid
                }
            }
        } else if (strcmp(token[i], "go") == 0) {
            actionsReal[actionNum++] = (Action *) malloc(sizeof(Action));
            actionsReal[actionNum - 1]->actionType = GO;
            actionsReal[actionNum - 1]->subjectIndex1 = subjectWhereWeLeave;
            actionsReal[actionNum - 1]->subjectIndex2 = subjectNum - 1;
            actionsReal[actionNum - 1]->conditionIndex1 = -1;
            actionsReal[actionNum - 1]->conditionIndex2 = -1;
            subjectWhereWeLeave = subjectNum;
            actionsReal[actionNum] = NULL;
            actionsReal[actionNum - 1]->itemIndex1 = -1;
            actionsReal[actionNum - 1]->itemIndex2 = -1;
            // location
            actionsReal[actionNum - 1]->locationIndex = locationWhereWeLeave;
            pLocations[locationNum++] = (location *) malloc(sizeof(location));
            pLocations[locationNum] = NULL;
            if (i + 2 < size && strcmp(token[i + 1], "to") == 0) {
                pLocations[locationNum - 1]->name = token[i + 2];
            } else return 0;

            locationWhereWeLeave = locationNum;
            i += 2;

        } else if (strcmp(token[i], "to") == 0 && actionNum != 0 && actionsReal[actionNum - 1]->actionType == SELL) {
            actionsReal[actionNum++] = (Action *) malloc(sizeof(Action));
            actionsReal[actionNum - 1]->actionType = TO;
            // subject of the "to" which is buyer of sell action
            pSubjects[subjectNum++] = (subject *) malloc(sizeof(subject));
            if (i + 1 < size) {
                pSubjects[subjectNum - 1]->name = token[i + 1];
            } else return 0;
            pSubjects[subjectNum] = NULL;
            actionsReal[actionNum - 1]->subjectIndex1 = subjectWhereWeLeave;
            actionsReal[actionNum - 1]->subjectIndex2 = subjectNum - 1;
            actionsReal[actionNum - 1]->itemIndex1 = -1;
            actionsReal[actionNum - 1]->itemIndex2 = -1;
            actionsReal[actionNum - 1]->conditionIndex1 = -1;
            actionsReal[actionNum - 1]->conditionIndex2 = -1;
            subjectWhereWeLeave = subjectNum;
            actionsReal[actionNum] = NULL;
            i++;

        } else if (strcmp(token[i], "from") == 0 && actionNum != 0 && actionsReal[actionNum - 1]->actionType == BUY) {

            actionsReal[actionNum++] = (Action *) malloc(sizeof(Action));
            actionsReal[actionNum - 1]->actionType = FROM;
            // subject of the "from" which is seller of buy action
            pSubjects[subjectNum++] = (subject *) malloc(sizeof(subject));

            if (i + 1 < size) {
                pSubjects[subjectNum - 1]->name = token[i + 1];
            } else return 0;
            pSubjects[subjectNum] = NULL;
            actionsReal[actionNum - 1]->subjectIndex1 = subjectWhereWeLeave;
            actionsReal[actionNum - 1]->subjectIndex2 = subjectNum - 1;
            actionsReal[actionNum - 1]->itemIndex1 = -1;
            actionsReal[actionNum - 1]->itemIndex2 = -1;
            actionsReal[actionNum - 1]->conditionIndex1 = -1;
            actionsReal[actionNum - 1]->conditionIndex2 = -1;
            subjectWhereWeLeave = subjectNum;
            actionsReal[actionNum] = NULL;
            i++;

        } else if (strcmp(token[i], "if") == 0) {
            // conditionIndex1 and conditionIndex2 are inclusive
            int isActionDetected = 0;
            while (i < size) {
                if ((i + 1 < size) && strcmp(token[++i], "at") == 0) {
                    pConditions[conditionNum++] = (Condition *) malloc(sizeof(Condition));
                    pConditions[conditionNum - 1]->conditionType = AT;
                    pConditions[conditionNum - 1]->subjectIndex1 = subjectWhereWeLeave;
                    pConditions[conditionNum - 1]->subjectIndex2 = subjectNum - 1;
                    pConditions[conditionNum - 1]->itemIndex1 = -1;
                    pConditions[conditionNum - 1]->itemIndex2 = -1;
                    pConditions[conditionNum - 1]->conditionResult = 0; // to indicate that it is not evaluated yet

                    subjectWhereWeLeave = subjectNum;
                    pConditions[conditionNum] = NULL;
                    // location
                    pConditions[conditionNum - 1]->locationIndex = locationWhereWeLeave;
                    pLocations[locationNum++] = (location *) malloc(sizeof(location));
                    pLocations[locationNum] = NULL;
                    if (i + 1 < size) {
                        pLocations[locationNum - 1]->name = token[i + 1];
                    } else return 0;

                    locationWhereWeLeave = locationNum;
                    i += 2;
                } else if (strcmp(token[i], "has") == 0) {
                    if ((i + 2 < size) && strcmp(token[i + 1], "more") == 0) {
                        if (strcmp(token[i + 2], "than") != 0) {
                            return 0;
                        }
                        pConditions[conditionNum++] = (Condition *) malloc(sizeof(Condition));
                        pConditions[conditionNum - 1]->conditionType = MORE;
                        pConditions[conditionNum - 1]->subjectIndex1 = subjectWhereWeLeave;
                        pConditions[conditionNum - 1]->subjectIndex2 = subjectNum - 1;
                        pConditions[conditionNum - 1]->locationIndex = -1;
                        pConditions[conditionNum - 1]->conditionResult = 0; // to indicate that it is not evaluated yet
                        subjectWhereWeLeave = subjectNum;
                        pConditions[conditionNum] = NULL;
                        // for items
                        pConditions[conditionNum - 1]->itemIndex1 = itemWhereWeLeave;
                        i += 2;
                        while (i < size) {
                            if ((i + 2 < size) && is_numeric(token[++i])) {
                                pItems[itemNum++] = (item *) malloc(sizeof(item));
                                pItems[itemNum - 1]->name = token[i + 1];
                                pItems[itemNum - 1]->number = atoi(token[i]);
                                pItems[itemNum] = NULL;
                                if ((i + 3 < size) && is_numeric(token[i + 3])) {
                                    if (strcmp(token[i + 2], "and") == 0) {
                                        i += 2;
                                        continue;
                                    } else return 0;
                                } else if (i + 2 == size) {
                                    itemWhereWeLeave = itemNum;
                                    pConditions[conditionNum - 1]->itemIndex2 = itemNum - 1;
                                    i += 2;
                                    break;
                                } else if ((i + 2 < size) && strcmp(token[i + 2], "and") == 0) {
                                    itemWhereWeLeave = itemNum;
                                    pConditions[conditionNum - 1]->itemIndex2 = itemNum - 1;
                                    i += 1;
                                    break;
                                } else {
                                    i += 2;
                                }
                            } else return 0;
                        }
                    } else if ((i + 1 < size) && strcmp(token[i + 1], "less") == 0) {
                        if (!((i + 2 < size) && strcmp(token[i + 2], "than") == 0)) {
                            return 0;
                        }
                        pConditions[conditionNum++] = (Condition *) malloc(sizeof(Condition));
                        pConditions[conditionNum - 1]->conditionType = LESS;
                        pConditions[conditionNum - 1]->subjectIndex1 = subjectWhereWeLeave;
                        pConditions[conditionNum - 1]->subjectIndex2 = subjectNum - 1;
                        pConditions[conditionNum - 1]->locationIndex = -1;
                        pConditions[conditionNum - 1]->conditionResult = 0; // to indicate that it is not evaluated yet
                        subjectWhereWeLeave = subjectNum;
                        pConditions[conditionNum] = NULL;
                        // for items
                        pConditions[conditionNum - 1]->itemIndex1 = itemWhereWeLeave;
                        i += 2;
                        while (i < size) {
                            if ((i + 2 < size) && is_numeric(token[++i])) {
                                pItems[itemNum++] = (item *) malloc(sizeof(item));
                                pItems[itemNum - 1]->name = token[i + 1];
                                pItems[itemNum - 1]->number = atoi(token[i]);
                                pItems[itemNum] = NULL;
                                if ((i + 3 < size) && is_numeric(token[i + 3])) {
                                    if (strcmp(token[i + 2], "and") == 0) {
                                        i += 2;
                                        continue;
                                    } else return 0;
                                } else if (i + 2 == size) {
                                    itemWhereWeLeave = itemNum;
                                    pConditions[conditionNum - 1]->itemIndex2 = itemNum - 1;
                                    i += 2;
                                    break;
                                } else if ((i + 2 < size) && strcmp(token[i + 2], "and") == 0) {
                                    itemWhereWeLeave = itemNum;
                                    pConditions[conditionNum - 1]->itemIndex2 = itemNum - 1;
                                    i += 1;
                                    break;
                                } else {
                                    i += 2;
                                }
                            } else return 0;
                        }
                    } else { // if the condition is "has" and there is no "more" or "less", our subject
                        if (i + 1 >= size) {
                            return 0;
                        }
                        pConditions[conditionNum++] = (Condition *) malloc(sizeof(Condition));
                        pConditions[conditionNum - 1]->conditionType = HAS;
                        pConditions[conditionNum - 1]->subjectIndex1 = subjectWhereWeLeave;
                        pConditions[conditionNum - 1]->subjectIndex2 = subjectNum - 1;
                        pConditions[conditionNum - 1]->locationIndex = -1;
                        pConditions[conditionNum - 1]->conditionResult = 0; // to indicate that it is not evaluated yet
                        subjectWhereWeLeave = subjectNum;
                        pConditions[conditionNum] = NULL;
                        // for items
                        pConditions[conditionNum - 1]->itemIndex1 = itemWhereWeLeave;
                        while (i < size) {
                            if ((i + 2 < size) && is_numeric(token[++i])) {
                                pItems[itemNum++] = (item *) malloc(sizeof(item));
                                pItems[itemNum - 1]->name = token[i + 1];
                                pItems[itemNum - 1]->number = atoi(token[i]);
                                pItems[itemNum] = NULL;
                                if ((i + 3 < size) && is_numeric(token[i + 3])) {
                                    if (strcmp(token[i + 2], "and") == 0) {
                                        i += 2;
                                        continue;
                                    } else return 0;
                                } else if (i + 2 == size) {
                                    itemWhereWeLeave = itemNum;
                                    pConditions[conditionNum - 1]->itemIndex2 = itemNum - 1;
                                    i += 2;
                                    break;
                                } else if ((i + 2 < size) && strcmp(token[i + 2], "and") == 0) {
                                    itemWhereWeLeave = itemNum;
                                    pConditions[conditionNum - 1]->itemIndex2 = itemNum - 1;
                                    i += 1;
                                    break;
                                } else {
                                    i += 2;
                                }
                            } else return 0;
                        }
                    }
                } else if (isActionWord(token[i])) {
                    for (int y = actionWhereWeLeave; y < actionNum; y++) {
                        actionsReal[y]->conditionIndex1 = conditionWhereWeLeave;
                        actionsReal[y]->conditionIndex2 = conditionNum - 1;
                    }
                    actionWhereWeLeave = actionNum;
                    conditionWhereWeLeave = conditionNum;
                    i -= 1;
                    isActionDetected = 1;
                    break;

                } else if (strcmp(token[i], "and") == 0) {
                    if (i == size - 1) {
                        return 0;
                    } else continue;
                } else {
                    if (!containsForbiddenWords(token[i])) {
                        pSubjects[subjectNum++] = (subject *) malloc(sizeof(subject));
                        pSubjects[subjectNum - 1]->name = token[i];
                        pSubjects[subjectNum] = NULL;
                    } else {
                        return 0;
                    }

                }
            }
            if (!isActionDetected) {
                for (int y = actionWhereWeLeave; y < actionNum; y++) {
                    actionsReal[y]->conditionIndex1 = conditionWhereWeLeave;
                    actionsReal[y]->conditionIndex2 = conditionNum - 1;
                }
                actionWhereWeLeave = actionNum;
                conditionWhereWeLeave = conditionNum;
                i -= 1;
            }
        } else if (strcmp(token[i], "and") == 0) { // if the token is and
            if (i == size - 1) {
                return 0;
            } else if (i + 1 < size && (strcmp(token[i + 1], "and") == 0)) {
                return 0;
            } else continue;
        } else if (is_valid_subject_or_item(token[i]) && i + 1 < size && containsForbiddenWords(token[i + 1])) {
            // if the token is a valid subject or item
            pSubjects[subjectNum++] = (subject *) malloc(sizeof(subject));
            pSubjects[subjectNum - 1]->name = token[i];
            pSubjects[subjectNum] = NULL;
        } else { // if the token is not valid
            return 0;
        }
    }
    // if we return 1, it means that the line is valid otherwise it is not valid
    return 1;
}


void freeHeapArrays() {
    // Free the memory of the arrays
    int j, k, l, m, n;
    j = 0;
    k = 0;
    l = 0;
    m = 0;
    n = 0;

    while (actionsReal[j] != NULL) {
        free(actionsReal[j]);
        j++;
    }
    while (pConditions[k] != NULL) {
        free(pConditions[k]);
        k++;
    }
    while (pSubjects[l] != NULL) {
        free(pSubjects[l]);
        l++;
    }
    while (pItems[m] != NULL) {
        free(pItems[m]);
        m++;
    }
    while (pLocations[n] != NULL) {
        free(pLocations[n]);
        n++;
    }

    free(actionsReal);
    free(pSubjects);
    free(pItems);
    free(pLocations);
    free(pConditions);
}

int is_valid_list() {
    int i = 0;
    while (pSubjects[i] != NULL) {
        if (!is_valid_subject_or_item(pSubjects[i]->name)) {
            return 0;
        }
        i++;
    }
    i = 0;
    while (pItems[i] != NULL) {
        if (!is_valid_subject_or_item(pItems[i]->name)) {
            return 0;
        }
        i++;
    }
    return 1;
}


/// BEGINNING OF THE SEMANTIC PART

/*
 * These two pointer to pointer to "x" are for inventory of subjects and locations.
 */
subject **subjectInventory;
location **locationInventory;

//  helper methods for semantics
int findSubject(char *name, subject ***pSubject) {
    /*
     * this method find the place of subject in inventory which is  given in the line.
     * If it is not in inventory it is created.
     */
    int i = 0;
    while ((*pSubject)[i] != NULL) {
        if (strcmp(name, (*pSubject)[i]->name) == 0) {
            return i;
        }
        i++;
    }
    (*pSubject)[i] = (subject *) malloc(sizeof(subject));
    (*pSubject)[i]->items[0].name = "0";
    (*pSubject)[i]->name = name;
    (*pSubject)[i]->location = "0";
    (*pSubject)[i + 1] = NULL;
    return i;
}

int addSubjectToLocation(char *name, int locationIndex, location ***pLocation) {
    int i = 0;  // person might be already there
    while (strcmp((*pLocation)[locationIndex]->subjectNames[i], "0") != 0) {
        if (strcmp((*pLocation)[locationIndex]->subjectNames[i], name) == 0) {
            return 0;
        }
        i++;
    }
    // if person is not there
    strcpy((*pLocation)[locationIndex]->subjectNames[i], name);
    strcpy((*pLocation)[locationIndex]->subjectNames[i + 1], "0");
    return 1;
}

int findItem(char *name, int subjectIndex, subject ***pSubject) {
    /*
    * this method find the place of item in subject's inventory which is  given in the line.
    * If it is not in inventory it is created.
    */
    int i = 0;
    while (strcmp((*pSubject)[subjectIndex]->items[i].name, "0") != 0) {
        if (strcmp(name, (*pSubject)[subjectIndex]->items[i].name) == 0) {
            return i;
        }
        i++;
    }
    (*pSubject)[subjectIndex]->items[i].name = name;
    (*pSubject)[subjectIndex]->items[i].number = 0;
    (*pSubject)[subjectIndex]->items[i + 1].name = "0";
    return i;
}

int findLocation(char *name, location ***pLocation) {
    /*
    * this method find the place of location in inventory which is  given in the line.
    * If it is not in inventory it is created.
    */
    int i = 0;
    while ((*pLocation)[i] != NULL) {
        if (strcmp(name, (*pLocation)[i]->name) == 0) {
            return i;
        }
        i++;
    }
    (*pLocation)[i] = (location *) malloc(sizeof(location));
    (*pLocation)[i]->name = name;
    strcpy((*pLocation)[i]->subjectNames[0], "0");
    (*pLocation)[i + 1] = NULL;
    return i;
}

int evaluateCondition(int conditionIndex, subject ***pSubject) {
    /* This method evaluates the given condition and return the value of the condition.
     * If the condition is true it returns 1 , if not it returns -1
     */



    int conditionResult = 1;
    int subjectIndex1 = pConditions[conditionIndex]->subjectIndex1; // find the subject range
    int subjectIndex2 = pConditions[conditionIndex]->subjectIndex2;

    int itemIndex1 = pConditions[conditionIndex]->itemIndex1; // find the item range
    int itemIndex2 = pConditions[conditionIndex]->itemIndex2;

    int locationIndex = pConditions[conditionIndex]->locationIndex; // find the location index

    if (pConditions[conditionIndex]->conditionType == AT) {// if the condition type is "AT"
        for (int i = subjectIndex1; i <= subjectIndex2; i++) { // iterate through the subjects
            int subjectFoundAt = findSubject(pSubjects[i]->name, &subjectInventory);
            if (strcmp((*pSubject)[subjectFoundAt]->location, pLocations[locationIndex]->name) != 0) {
                // if the location of the subject is not the same with the location in the condition
                conditionResult = -1; // it is not valid
                break;
            }
        }
        return conditionResult;
    } else if (pConditions[conditionIndex]->conditionType == HAS) {
        for (int i = subjectIndex1; i <= subjectIndex2; i++) { // iterate through the subjects
            int subjectFoundAt = findSubject(pSubjects[i]->name,
                                             &subjectInventory); // by calling this, we insert the subject if it is not present
            for (int j = itemIndex1; j <= itemIndex2; j++) { // iterate through the items
                int itemFoundAt = findItem(pItems[j]->name, subjectFoundAt, &subjectInventory);
                if ((*pSubject)[subjectFoundAt]->items[itemFoundAt].number != pItems[j]->number) {
                    // if the number of the item in the subject inventory is not the same with the number in the condition
                    conditionResult = -1;
                    break;
                }
            }
            if (conditionResult == -1) {
                break;
            }
        }
        return conditionResult;
    } else if (pConditions[conditionIndex]->conditionType == MORE) { // if the condition type is "MORE"
        for (int i = subjectIndex1; i <= subjectIndex2; i++) {
            int subjectFoundAt = findSubject(pSubjects[i]->name, &subjectInventory);
            for (int j = itemIndex1; j <= itemIndex2; j++) {
                int itemFoundAt = findItem(pItems[j]->name, subjectFoundAt, &subjectInventory);
                if ((*pSubject)[subjectFoundAt]->items[itemFoundAt].number <= pItems[j]->number) {
                    conditionResult = -1;
                    break;
                }
            }
            if (conditionResult == -1) {
                break;
            }
        }
        return conditionResult;
    } else if (pConditions[conditionIndex]->conditionType == LESS) { // if the condition type is "LESS"
        for (int i = subjectIndex1; i <= subjectIndex2; i++) {
            int subjectFoundAt = findSubject(pSubjects[i]->name, &subjectInventory);
            for (int j = itemIndex1; j <= itemIndex2; j++) {
                int itemFoundAt = findItem(pItems[j]->name, subjectFoundAt, &subjectInventory);
                if ((*pSubject)[subjectFoundAt]->items[itemFoundAt].number >= pItems[j]->number) {
                    conditionResult = -1;
                    break;
                }
            }
            if (conditionResult == -1) {
                break;
            }
        }
        return conditionResult;
    }
    return conditionResult;
}

int assignEvaluationConditions() {
    // with evaluation conditions method all conditions are evaluated here and assign their value in pConditions pointer.
    int i = 0;
    while (pConditions[i] != NULL) {
        int boolean = evaluateCondition(i, &subjectInventory);
        if (pConditions[i]->conditionResult == 0) {
            pConditions[i]->conditionResult = boolean;
        }
        i++;
    }
    return 0;
}

int willActionBeDone(int actionIndex, subject ***pSubject) {
    //Actions are evaluated here according to their external condition and also their internal feasibility.
    assignEvaluationConditions();
    // if at least one of the condition is false, do not do anything.
    if (actionsReal[actionIndex]->conditionIndex1 != -1) {
        for (int i = actionsReal[actionIndex]->conditionIndex1; i <= actionsReal[actionIndex]->conditionIndex2; i++) {
            if (pConditions[i]->conditionResult == -1) {
                return 0;
            }
        }
    }
    // if the method can pass the for loop it means that we can do the action
    int subjectIndex1 = actionsReal[actionIndex]->subjectIndex1;
    int subjectIndex2 = actionsReal[actionIndex]->subjectIndex2;

    int itemIndex1 = actionsReal[actionIndex]->itemIndex1;
    int itemIndex2 = actionsReal[actionIndex]->itemIndex2;

    //SELL and TO
    if (actionsReal[actionIndex]->actionType == SELL) {
        for (int i = subjectIndex1; i <= subjectIndex2; i++) { // iterate through the subjects
            int subjectFoundAt = findSubject(pSubjects[i]->name,
                                             &subjectInventory); // by calling this, we insert the subject if it is not present
            for (int j = itemIndex1; j <= itemIndex2; j++) { // iterate through the items
                int itemFoundAt = findItem(pItems[j]->name, subjectFoundAt, &subjectInventory);
                if ((*pSubject)[subjectFoundAt]->items[itemFoundAt].number < pItems[j]->number) {
                    // if the number of the item in the subject inventory is not as much as with the number in the sentence do not do it.
                    return 0;
                }
            }

        }
        return 1; // it means that sell function can pass the condition of itself

    } else if (actionsReal[actionIndex]->actionType == TO) {
        return actionsReal[actionIndex - 1]->actionResult;
    }
    // BUY and FROM
    if (actionsReal[actionIndex]->actionType == BUY) {
        int addition = subjectIndex2 - subjectIndex1 + 1;
        if (actionsReal[actionIndex + 1] != NULL && actionsReal[actionIndex + 1]->actionType == FROM) {
            subjectIndex1 = actionsReal[actionIndex + 1]->subjectIndex1;
            subjectIndex2 = actionsReal[actionIndex + 1]->subjectIndex2;
            for (int i = subjectIndex1; i <= subjectIndex2; i++) { // iterate through the subjects
                int subjectFoundAt = findSubject(pSubjects[i]->name,
                                                 &subjectInventory); // by calling this, we insert the subject if it is not present
                for (int j = itemIndex1; j <= itemIndex2; j++) { // iterate through the items
                    int itemFoundAt = findItem(pItems[j]->name, subjectFoundAt, &subjectInventory);
                    if ((*pSubject)[subjectFoundAt]->items[itemFoundAt].number < pItems[j]->number * addition) {
                        // if the number of the item in the subject inventory is not as much as with the number in the sentence do not do it.
                        return 0;
                    }
                }
            }
            return 1; // it means that sell function can pass the condition of itself
        } else {
            return 1;
        }
    } else if (actionsReal[actionIndex]->actionType == FROM) {
        return actionsReal[actionIndex - 1]->actionResult;
    }
        //go to
    else if (actionsReal[actionIndex]->actionType == GO) {
        return 1;
    }
}

int doAction(int actionIndex, subject ***pSubject) {
    // if the action could be done, this method do it.
    int willActionDone = willActionBeDone(actionIndex, &subjectInventory);

    if (willActionDone == 1) {

        int subjectIndex1 = actionsReal[actionIndex]->subjectIndex1;
        int subjectIndex2 = actionsReal[actionIndex]->subjectIndex2;

        int itemIndex1 = actionsReal[actionIndex]->itemIndex1;
        int itemIndex2 = actionsReal[actionIndex]->itemIndex2;

        int locationIndex = actionsReal[actionIndex]->locationIndex;


        if (actionsReal[actionIndex]->actionType == SELL) {
            int addition = subjectIndex2 - subjectIndex1 + 1;
            for (int i = subjectIndex1; i <= subjectIndex2; i++) { // iterate through the subjects
                int subjectFoundAt = findSubject(pSubjects[i]->name,
                                                 &subjectInventory); // by calling this, we insert the subject if it is not present

                for (int j = itemIndex1; j <= itemIndex2; j++) { // iterate through the items
                    int itemFoundAt = findItem(pItems[j]->name, subjectFoundAt, &subjectInventory);
                    (*pSubject)[subjectFoundAt]->items[itemFoundAt].number -= pItems[j]->number;//  decrease the number of items
                }
            }
            if (actionsReal[actionIndex + 1] != NULL && actionsReal[actionIndex + 1]->actionType == TO) {
                subjectIndex1 = actionsReal[actionIndex + 1]->subjectIndex1;
                subjectIndex2 = actionsReal[actionIndex + 1]->subjectIndex2;
                for (int i = subjectIndex1; i <= subjectIndex2; i++) { // iterate through the subjects
                    int subjectFoundAt = findSubject(pSubjects[i]->name, &subjectInventory);
                    // by calling this, we insert the subject if it is not present
                    for (int j = itemIndex1; j <= itemIndex2; j++) { // iterate through the items
                        int itemFoundAt = findItem(pItems[j]->name, subjectFoundAt, &subjectInventory);
                        (*pSubject)[subjectFoundAt]->items[itemFoundAt].number += (pItems[j]->number *
                                                                                   addition); //  decrease the number of items
                    }
                }
            }
        } else if (actionsReal[actionIndex]->actionType == BUY) {
            int subtraction = subjectIndex2 - subjectIndex1 + 1;
            for (int i = subjectIndex1; i <= subjectIndex2; i++) { // iterate through the subjects
                int subjectFoundAt = findSubject(pSubjects[i]->name,
                                                 &subjectInventory); // we insert the subject if it is not present
                for (int j = itemIndex1; j <= itemIndex2; j++) { // iterate through the items
                    int itemFoundAt = findItem(pItems[j]->name, subjectFoundAt, &subjectInventory);
                    (*pSubject)[subjectFoundAt]->items[itemFoundAt].number += pItems[j]->number;//  decrease the number of items
                }
            }
            if (actionsReal[actionIndex + 1] != NULL && actionsReal[actionIndex + 1]->actionType == FROM) {
                subjectIndex1 = actionsReal[actionIndex + 1]->subjectIndex1;
                subjectIndex2 = actionsReal[actionIndex + 1]->subjectIndex2;
                for (int i = subjectIndex1; i <= subjectIndex2; i++) { // iterate through the subjects
                    int subjectFoundAt = findSubject(pSubjects[i]->name, &subjectInventory);
                    // by calling this, we insert the subject if it is not present
                    for (int j = itemIndex1; j <= itemIndex2; j++) { // iterate through the items
                        int itemFoundAt = findItem(pItems[j]->name, subjectFoundAt, &subjectInventory);
                        (*pSubject)[subjectFoundAt]->items[itemFoundAt].number -= (subtraction * pItems[j]->number);
                        //  decrease the number of items
                    }
                }
            }

        } else if (actionsReal[actionIndex]->actionType == TO || actionsReal[actionIndex]->actionType == FROM) {
            return 0;
        } else if (actionsReal[actionIndex]->actionType == GO) {
            for (int i = subjectIndex1; i <= subjectIndex2; i++) { // iterate through the subjects
                int subjectFoundAt = findSubject(pSubjects[i]->name, &subjectInventory);
                (*pSubject)[subjectFoundAt]->location = pLocations[locationIndex]->name;
                int locationFoundAt = findLocation(pLocations[locationIndex]->name, &locationInventory);
                addSubjectToLocation((*pSubject)[subjectFoundAt]->name, locationFoundAt, &locationInventory);
            }
        }
    } else return 0;
}

int semantic() {

    int i = 0;
    int g = 0;
    int h = 0;
    if (actionsReal[0] == NULL) {
        return 0;
    }
    while (actionsReal[g] != NULL) {
        for (int j = actionsReal[g]->subjectIndex1; j <= actionsReal[g]->subjectIndex2; j++) {
            for (int k = j + 1; k <= actionsReal[g]->subjectIndex2; k++) {
                if (pSubjects[k] != NULL && strcmp(pSubjects[j]->name, pSubjects[k]->name) == 0) {
                    return 0;
                }
            }
        }
        for (int j = actionsReal[g]->itemIndex1; j <= actionsReal[g]->itemIndex2; j++) {
            for (int k = j + 1; k <= actionsReal[g]->itemIndex2; k++) {
                if (pItems[k] != NULL && strcmp(pItems[j]->name, pItems[k]->name) == 0) {
                    return 0;
                }
            }
        }
        g++;

    }
    while (pConditions[h] != NULL) {
        for (int j = pConditions[h]->subjectIndex1; j <= pConditions[h]->subjectIndex2; j++) {
            for (int k = j + 1; k <= pConditions[h]->subjectIndex2; k++) {
                if (pSubjects[k] != NULL && strcmp(pSubjects[j]->name, pSubjects[k]->name) == 0) {
                    return 0;
                }
            }
        }
        for (int j = pConditions[h]->itemIndex1; j <= pConditions[h]->itemIndex2; j++) {
            for (int k = j + 1; k <= pConditions[h]->itemIndex2; k++) {
                if (pItems[k] != NULL && strcmp(pItems[j]->name, pItems[k]->name) == 0) {
                    return 0;
                }
            }
        }
        h++;
    }
    while (actionsReal[i] != NULL) {
        doAction(i, &subjectInventory);
        i++;
    }
    // free the memory
    freeHeapArrays();
    return 1;
}

// QUESTION PART

int is_it_question(char *last_token) {
    if (strcmp(last_token, "?") == 0) {
        return 1; // to indicate it is question
    }
    return 0;    // to indicate it is not
}

int is_it_quantity_asking(char *token) {
    if (strcmp(token, "total") == 0) {
        return 1; // to indicate it is a quantity asking question
    }
    return 0;
}

int is_it_location_asking(char *token) {
    if (strcmp(token, "where") == 0) {
        return 1; // to indicate it is a quantity asking question
    }
    return 0;
}

int is_it_presence_asking(char *token[], int size) {
    if (strcmp(token[1], "at") == 0 && strcmp(token[0], "who") == 0) {
        return 1; // to indicate it is a quantity asking question
    }
    return 0;
}

int is_it_inventory_asking(char *token) {
    if (strcmp(token, "total") == 0) {
        return 1; // to indicate it is a quantity asking question
    }
    return 0;
}


int whoAtQuestion(char *locationName, location ***pLocation) {
    int locationIndex = findLocation(locationName, &locationInventory);
    int i = 0;
    int isThereAnyPerson = 0;
    while (strcmp((*pLocation)[locationIndex]->subjectNames[i], "0") != 0) {
        printf("%s", (*pLocation)[locationIndex]->subjectNames[i]);
        isThereAnyPerson = 1;
        if (strcmp((*pLocation)[locationIndex]->subjectNames[i + 1], "0") != 0) {
            printf(" and ");
        }
        i++;
    }
    if (isThereAnyPerson == 0) {
        printf("NOBODY\n");
        return 0;
    }
    printf("\n");
    return 0;
}

int locationAsking(char *subjectName, subject ***pSubject) {
    int subjectIndex = findSubject(subjectName, &subjectInventory);
    if (strcmp((*pSubject)[subjectIndex]->location, "0") == 0) {
        printf("NOWHERE\n");
    } else {
        printf("%s\n", (*pSubject)[subjectIndex]->location);

    }
    return 0;
}

int quantityTotal(char *subjectName, char *itemName, subject ***pSubject) {
    int subjectIndex = findSubject(subjectName, &subjectInventory);
    int itemIndex = findItem(itemName, subjectIndex, &subjectInventory);
    return (*pSubject)[subjectIndex]->items[itemIndex].number;
}

int inventoryInquiry(char *subjectName, subject ***pSubject) {
    int isThereAnyItem = 0;
    int subjectIndex = findSubject(subjectName, &subjectInventory);
    int i = 0;
    while (strcmp((*pSubject)[subjectIndex]->items[i].name, "0") != 0) {
        if ((*pSubject)[subjectIndex]->items[i].number != 0) {
            printf("%d %s", (*pSubject)[subjectIndex]->items[i].number, (*pSubject)[subjectIndex]->items[i].name);
            isThereAnyItem = 1;
        }
        if (strcmp((*pSubject)[subjectIndex]->items[i + 1].name, "0") != 0 &&
            (*pSubject)[subjectIndex]->items[i + 1].number != 0) {
            printf(" and ");
        }
        i++;
    }
    if (isThereAnyItem == 0) {
        printf("NOTHING\n");
        return 0;
    }
    printf("\n");
    return 0;
}
// END OF THE QUESTION PART


int parse_line(char *line, char *tokens[]) {
    /*
     * This parse line method includes approximately all necessary part to be used in main method.
     * it takes the line and parse it to tokens and first check some invalid cases.
     * If it is a question it does the compulsory patterns for questions.
     * If it is not a question it takes the line and do the jobs with creating lists and semantic methods.
     *
    */

    int num_tokens = 0;
    char *token = strtok(line, " ");
    while (token != NULL) {
        tokens[num_tokens] = strdup(token);
        num_tokens++;
        token = strtok(NULL, " "); // get the next token
    }
    if (num_tokens == 1 && strcmp(tokens[0], "exit") == 0) {
        exit(0);
    }
    if (!isReservedWordUsed(tokens, num_tokens)) {
        printf("INVALID\n");
        return 0;
    }

    if (num_tokens < 3) {
        printf("INVALID\n");
        return 0;
    }

    if (is_it_question(tokens[num_tokens - 1])) {
        // if it is a question

        if (is_it_quantity_asking(tokens[num_tokens - 3])) {
            // we need to take the item name which is in n-2 location
            int quantityTotalNumber = 0;
            if (num_tokens % 2 != 0) { // number of tokens must be even
                printf("INVALID\n");
                return 0;
            }

            for (int i = 0; i < num_tokens - 3; i++) { // check the subject and "and" words
                if (i % 2 == 1) {
                    if (strcmp(tokens[i], "and") != 0) {
                        printf("INVALID\n");
                        return 0;
                    } else {
                        continue;
                    }
                } else {
                    if (strcmp(tokens[i], "and") == 0) {
                        printf("INVALID\n");
                        return 0;
                    }
                    if (containsForbiddenWords(tokens[i])) {
                        printf("INVALID\n");
                        return 0;
                    }
                    quantityTotalNumber += quantityTotal(tokens[i], tokens[num_tokens - 2], &subjectInventory);
                }
            }
            printf("%d\n", quantityTotalNumber);
            return 0;
        } else if (is_it_location_asking(tokens[num_tokens - 2])) {
            // at 0 location there is tne name of the subject
            // We need to go and find its location
            if (num_tokens != 3) {
                printf("INVALID\n");
                return 0;
            }
            if (containsForbiddenWords(tokens[0])) {
                printf("INVALID\n");
                return 0;
            }
            locationAsking(tokens[0], &subjectInventory);
            return 0;

        } else if (is_it_presence_asking(tokens, num_tokens)) {
            // at n-2 index there is location name ,and we need to go and find who are there?
            if (num_tokens != 4) {
                printf("INVALID\n");
                return 0;
            }
            if (containsForbiddenWords(tokens[num_tokens - 2])) {
                printf("INVALID\n");
                return 0;
            }
            whoAtQuestion(tokens[num_tokens - 2], &locationInventory);
            return 0;

        } else if (is_it_inventory_asking(tokens[num_tokens - 2])) {
            // at 0 location there is a subject, we need to go its inventory and give the all items
            if (num_tokens != 3) {
                printf("INVALID\n");
                return 0;
            }
            if (containsForbiddenWords(tokens[0])) {
                printf("INVALID\n");
                return 0;
            }
            inventoryInquiry(tokens[0], &subjectInventory);
            return 0;

        } else {
            printf("INVALID\n");
        }
    } else {
        // if it is not a question
        int a = creatingListsFromInStr(tokens, num_tokens);
        int b = is_valid_list();
        int wy=0;
        int wz=0;
        int wv=0;
        while (pSubjects[wy] != NULL) {
            if(containsForbiddenWords(pSubjects[wy]->name)){
                printf("INVALID\n");
                return 0;
            }
            wy++;
        }
        while (pItems[wv] != NULL) {
            if(containsForbiddenWords(pItems[wv]->name)){
                printf("INVALID\n");
                return 0;
            }
            wv++;
        }
        while (pLocations[wz] != NULL) {
            if(containsForbiddenWords(pLocations[wz]->name)){
                printf("INVALID\n");
                return 0;
            }
            wz++;
        }
        if (a == 0 || b == 0) {
            printf("INVALID\n");
            freeHeapArrays();
            return 0;
        }
        int x = semantic();
        if (x == 0) {
            printf("INVALID\n");
        } else {
            printf("OK\n");
        }
    }
    return 0;
}

int main(int argc, char *argv[]) {
    // first malloc memory for inventories in heap and then make their first element Null to be able to keep the place of where the last element.
    subjectInventory = (subject **) malloc(1024 * sizeof(subject *));
    locationInventory = (location **) malloc(1024 * sizeof(location *));
    subjectInventory[0] = NULL;
    locationInventory[0] = NULL;

    char line[MAX_INPUT_LENGTH];
    char *tokens[515]; // Array to hold up to 514 tokens

    // main loop that will terminate when the user types "exit"
    while (1) {
        printf(">> ");
        fflush(stdout); // Flush stdout
        if (scanf(" %[^\n]", line) != 1) {
            printf("INVALID\n");
        } else {
            // we need to parse the line
            parse_line(line, tokens);
        }
    }

    return 0;
}