#include <sqlite3.h>

typedef struct Row_desc Row;
struct Row_desc {
    uint8_t stat;
    uint32_t ip;
    uint16_t port;
    char *id;
} Row_desc;

int myInitDB(char *path);
int myCloseDB();
int mySetRow(Row row);
int mySelectRow(char *id);
int myFetchRow(Row *row);
