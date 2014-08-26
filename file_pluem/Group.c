//
//  Group.c
//  newTran
//
//  Created by Kasidej Bungbrakearti on 8/8/2557 BE.
//
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Group.h"
#include "User.h"


Group* newGroup(char *groupname) {
    User *group = newUser(groupname);
    if (group == NULL) {
        fprintf(stderr, "Error: can't malloc group\n");
        return NULL;
    }
    
    return (Group *)group;
}

Group* getGroupByNameFromList(Group **listgroup, int len, char *name) {
    int i;
    for (i=0; i<len; i++) {
        if (strcmp(listgroup[i]->groupname, name) == 0 ) {
            return listgroup[i];
        }
    }
    return NULL;
}
void removeGroupFromList(Group **listgroup, int *len, char *name) {
    int i;
    for (i=0; i<*len; i++) {
        if (strcmp(listgroup[i]->groupname, name) == 0) {
            listgroup[i]->func->free(listgroup[i]);
            for (i=i+1; i<*len; i++) {
                listgroup[i-1] = listgroup[i];
            }
            listgroup[i-1] = NULL;
            (*len)--;
            break;
        }
    }
}