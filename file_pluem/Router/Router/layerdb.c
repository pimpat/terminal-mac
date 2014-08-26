#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sqlite3.h>
#include <sys/file.h>
#include "layerdb.h"

sqlite3 *db;
sqlite3_stmt *stmtAdd, *stmtSel;
int myInitDB(char *path) {
    int ret = 0;
    ret = access(path, F_OK);
    if (ret == -1) {
        ret = sqlite3_open(path, &db);
        if(ret){
            fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
            return ret;
        }
        ret = sqlite3_exec(db,
            "CREATE TABLE 'DATA'( " \
/*            	"row INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, " \
*/	            "stat INTEGER, " \
	            "ip INTEGER, " \
    	        "port INTEGER, " \
        	    "id TEXT PRIMARY KEY NOT NULL, " \
            	"timestamp DATETIME DEFAULT (strftime('%s','now')))" ,
            NULL, NULL, NULL);
    } else {
        ret = sqlite3_open(path, &db);
        if(ret){
            fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
            return ret;
        }
    }

//	fprintf(stdout, "Opened database successfully.\n");

	ret = sqlite3_prepare_v2(db, "INSERT OR REPLACE INTO 'DATA' (stat, ip, port, id) VALUES(?, ?, ?, ?)", -1, &stmtAdd, 0);
    if (ret != SQLITE_OK) {
        fprintf(stderr, "Error: prepare stmtAdd: Code %d\n", ret);
        return ret;
    }
	ret = sqlite3_prepare_v2(db, "SELECT * FROM 'DATA' WHERE id=?", -1, &stmtSel, 0);
    if (ret != SQLITE_OK) {
        fprintf(stderr, "Error: prepare stmtSel: Code %d\n", ret);
        return ret;
    }
    
    return 0;
}
int myCloseDB() {
	int ret;
	ret = sqlite3_finalize(stmtAdd);
	if (ret != SQLITE_OK) return ret;
	ret = sqlite3_finalize(stmtSel);
	if (ret != SQLITE_OK) return ret;
    return sqlite3_close(db);
}
int mySetRow(Row row) {
    int ret;
	ret = sqlite3_reset(stmtAdd);
    ret = sqlite3_bind_int(stmtAdd, 1, row.stat);
    ret = sqlite3_bind_int(stmtAdd, 2, row.ip);
    ret = sqlite3_bind_int(stmtAdd, 3, row.port);
    ret = sqlite3_bind_text(stmtAdd, 4, row.id, -1, SQLITE_TRANSIENT);
	ret = sqlite3_step(stmtAdd);
	if (ret != SQLITE_DONE) fprintf(stderr, "Could net step ppStmtAddUser.\n");

    return 0;
}

int mySelectRow(char *id) {
	int ret;
	ret = sqlite3_reset(stmtSel);
	ret = sqlite3_bind_text(stmtSel, 1, id, -1, SQLITE_TRANSIENT);
	return 0;
}
int myFetchRow(Row *row) {
	int ret = sqlite3_step(stmtSel);

	if (ret == SQLITE_ROW) {
        row->stat = sqlite3_column_int(stmtSel, 0);
        row->ip = sqlite3_column_int(stmtSel, 1);
        row->port = sqlite3_column_int(stmtSel, 2);
        row->id = (char*)sqlite3_column_text(stmtSel, 3);
	} else if (ret == SQLITE_DONE) {
		return ret;
	} else {
		fprintf(stderr, "Error: Fetch User: %d\n", ret);
		return ret;
	}
	return 0;
}

