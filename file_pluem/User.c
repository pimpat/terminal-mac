//
//  User.c
//  newTran
//
//  Created by Kasidej Bungbrakearti on 8/4/2557 BE.
//
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "User.h"

int getIndexById(User *self, char *id) {
    int i;
    for (i=0; i<self->len_device; i++) {
        if (strcmp(self->list_device[i]->id, id) == 0) {
            return i;
        }
    }
    return -1;
}
Device* getDeviceById(User *self, char *id) {
    int i;
    for (i=0; i<self->len_device; i++) {
        if (strcmp(self->list_device[i]->id, id) == 0) {
            return self->list_device[i];
        }
    }
    return NULL;
}

void AddDevice(User *self, Device *device) {
    self->list_device[self->len_device++] = device;
}
void AddCopyDevice(User *self, Device *device) {
    Device *new = newDevice(device->id);
    new->func->setStat(new, device->stat);
    new->func->setIP(new, device->ip);
    new->func->setPort(new, device->port);
    self->list_device[self->len_device++] = new;
}
void RemoveAtDevice(User *self, int index) {
    int i;
    if (index >= self->len_device) {
        return;
    }
    self->list_device[index]->func->free(self->list_device[index]);
    for (i=index+1; i<self->len_device; i++) {
        self->list_device[i-1] = self->list_device[i];
    }
    self->list_device[i-1] = NULL;
    self->len_device--;
}
void RemoveDevice(User *self, Device *device) {
    int i;
    for (i=0; i<self->len_device; i++) {
        if (self->list_device[i] == device || (strcmp(self->list_device[i]->id, device->id)==0)) {
            self->list_device[i]->func->free(self->list_device[i]);
            for (i=i+1; i<self->len_device; i++) {
                self->list_device[i-1] = self->list_device[i];
            }
            self->list_device[i-1] = NULL;
            self->len_device--;
            return;
        }
    }
}
void setUsername(User *self, char *username) {
    strcpy(self->username, username);
}

void freeDeviceInUser(User *self) {
    int i;
    Device *device;
    for (i=0; i<self->len_device; i++) {
        device = self->list_device[i];
        device->func->free(device);
    }
}
void freeUser(User *self) {
    free(self->func);
    free(self);
}

User* newUser(char *username) {
    User *user = malloc(sizeof(User_desc));
    if (user == NULL) {
        fprintf(stderr, "Error: can't malloc user\n");
        return NULL;
    }
    user->func = malloc(sizeof(UserFunc_desc));
    if (user->func == NULL) {
        fprintf(stderr, "Error: can't malloc user->func\n");
        free(user);
        return NULL;
    }
    user->len_device = 0;
    setUsername(user, username);
    
    user->func->getIndexById = getIndexById;
    user->func->AddDevice = AddDevice;
    user->func->AddCopyDevice = AddCopyDevice;
    user->func->RemoveAtDevice = RemoveAtDevice;
    user->func->RemoveDevice = RemoveDevice;
    user->func->setUsername = setUsername;
    user->func->freeDevice = freeDeviceInUser;
    user->func->free = freeUser;
    
    return user;
}


// ListUser
User *listUser[MAX_LISTUSER];
int lenUser;

int initListUser() {
    int i;
    for (i=0; i<MAX_LISTUSER; i++) {
        listUser[i] = NULL;
    }
    lenUser = 0;
    return RET_OK;
}
int addUserToList(User *user) {
    if (lenUser >= MAX_LISTUSER) {
        fprintf(stderr, "Error: lenUser >= MAX_LISTUSER\n");
        return RET_ERR;
    }
    listUser[lenUser++] = user;
    return RET_OK;
}
int RemoveUserAtFromList(int index) {
    int i;
    if (index >= lenUser) {
        return RET_ERR;
    }
    for (i=index+1; i<lenUser; i++) {
        listUser[i-1] = listUser[i];
    }
    listUser[i-1] = NULL;
    lenUser--;
    return RET_OK;
}
int RemoveUserFromList(User *user) {
    int i;
    for (i=0; i<lenUser; i++) {
        if (listUser[i] == user || (strcmp(listUser[i]->username, user->username)==0)) {
            for (i=i+1; i<lenUser; i++) {
                listUser[i-1] = listUser[i];
            }
            listUser[i-1] = NULL;
            lenUser--;
            return RET_OK;
        }
    }
    return RET_ERR;
}
User* getUserByNameFromList(char *name) {
    int i;
    for (i=0; i<lenUser; i++) {
        if (strcmp(listUser[i]->username, name) == 0 ) {
            return listUser[i];
        }
    }
    return NULL;
}