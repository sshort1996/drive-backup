#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>

int header_printed = 0; // Flag to check if headers are printed

/* Callback function to print the query results */
static int callback(void *data, int argc, char **argv, char **azColName) {
    int i;

    // Print the headers for the first row only
    if(!header_printed) {
        for(i = 0; i < argc; i++) {
            printf("%-15s", azColName[i]);
        }
        printf("\n");
        for(i = 0; i < argc; i++) {
            printf("---------------");
        }
        printf("\n");
        header_printed = 1;
    }

    // Print the table content
    for(i = 0; i < argc; i++) {
        printf("%-15s", argv[i] ? argv[i] : "NULL");
    }
    printf("\n");

    return 0;
}

int main(int argc, char* argv[]) {
    sqlite3 *db;
    char *zErrMsg = 0;
    int rc;
    const char* data = "Result";

    /* Open database */
    rc = sqlite3_open("file_metadata.db", &db);

    if(rc) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        return(0);
    } else {
        fprintf(stderr, "Opened database successfully\n");
    }

    /* Create SQL statement */
    const char* sql = "SELECT * from files;";

    /* Execute SQL statement */
    rc = sqlite3_exec(db, sql, callback, (void*)data, &zErrMsg);

    if(rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    } else {
        fprintf(stdout, "Operation done successfully\n");
    }
    sqlite3_close(db);
    return 0;
}
