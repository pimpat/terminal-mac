//
//  User.h
//  newTran
//
//  Created by Kasidej Bungbrakearti on 8/4/2557 BE.
//
//

#ifndef newTran_User_h
#define newTran_User_h

#include "define.h"
#include "Device.h"

typedef struct User_desc User;
typedef struct UserFunc_desc UserFunc;

struct User_desc {
    Device *list_device[MAX_DEVICE_PER_USER];
    uint8_t len_device;
    char username[USERNAME_LENGTH];
    UserFunc *func;
} User_desc;

struct UserFunc_desc {
    int (*getIndexById)(User *self, char *id);
    void (*AddDevice)(User *self, Device *device);
    void (*AddCopyDevice)(User *self, Device *device);
    void (*RemoveAtDevice)(User *self, int index);
    void (*RemoveDevice)(User *self, Device *device);
    void (*setUsername)(User *self, char *username);
    void (*freeDevice)(User *self);
    void (*free)(User *self);
} UserFunc_desc;

User* newUser(char *username);


// ListUser

int initListUser();
int addUserToList(User *user);
int RemoveUserAtFromList(int index);
int RemoveUserFromList(User *user);
User* getUserByNameFromList(char *name);

#endif