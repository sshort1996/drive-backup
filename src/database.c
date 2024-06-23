#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include "cJSON.h" // Make sure you have cJSON library installed

// Function to initialize SQLite Database
int init_db(sqlite3 **db) {
    int rc = sqlite3_open("file_metadata.db", db);
    if(rc) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(*db));
        return rc;
    } else {
        fprintf(stdout, "Opened database successfully\n");
    }

    const char *sql = "CREATE TABLE IF NOT EXISTS files ("
                      "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                      "filename TEXT NOT NULL,"
                      "path TEXT NOT NULL,"
                      "mimetype TEXT,"
                      "created_time TEXT,"
                      "modified_time TEXT,"
                      "owner TEXT,"
                      "shared INTEGER,"
                      "permissions TEXT);";
    
    char *errMsg = 0;
    rc = sqlite3_exec(*db, sql, 0, 0, &errMsg);
    if(rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", errMsg);
        sqlite3_free(errMsg);
        return rc;
    } else {
        fprintf(stdout, "Table created successfully\n");
    }

    return SQLITE_OK;
}

// Function to insert file metadata into SQLite DB
int insert_file_metadata(sqlite3 *db, cJSON *file) {
    const char *sql = "INSERT INTO files (filename, path, mimetype, created_time, modified_time, owner, shared, permissions) VALUES (?, ?, ?, ?, ?, ?, ?, ?)";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return rc;
    }

    sqlite3_bind_text(stmt, 1, cJSON_GetObjectItem(file, "name")->valuestring, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, "", -1, SQLITE_STATIC);  // Filesystem path is empty as it's not part of API response
    sqlite3_bind_text(stmt, 3, cJSON_GetObjectItem(file, "mimeType")->valuestring, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, cJSON_GetObjectItem(file, "createdTime")->valuestring, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 5, cJSON_GetObjectItem(file, "modifiedTime")->valuestring, -1, SQLITE_STATIC);
    cJSON *owners = cJSON_GetObjectItem(file, "owners");
    if (owners && cJSON_IsArray(owners) && cJSON_GetArraySize(owners) > 0) {
        sqlite3_bind_text(stmt, 6, cJSON_GetObjectItem(cJSON_GetArrayItem(owners, 0), "emailAddress")->valuestring, -1, SQLITE_STATIC);
    } else {
        sqlite3_bind_text(stmt, 6, "", -1, SQLITE_STATIC);
    }
    sqlite3_bind_int(stmt, 7, cJSON_GetObjectItem(file, "shared")->valueint);
    sqlite3_bind_text(stmt, 8, cJSON_PrintUnformatted(cJSON_GetObjectItem(file, "permissions")), -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
    }

    sqlite3_finalize(stmt);
    return rc;
}

int main(void) {
    sqlite3 *db;
    int rc = init_db(&db);
    if (rc != SQLITE_OK) {
        return rc;
    }

    // Read content from metadata.json
    FILE *f = fopen("metadata.json", "r");
    if (f == NULL) {
        perror("Error opening file");
        return EXIT_FAILURE;
    }

    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *json_data = malloc(fsize + 1);
    fread(json_data, fsize, 1, f);
    fclose(f);

    json_data[fsize] = 0;

    // Parse JSON response
    cJSON *json = cJSON_Parse(json_data);
    if (json == NULL) {
        fprintf(stderr, "Error parsing JSON response.\n");
    } else {
        cJSON *files = cJSON_GetObjectItem(json, "files");
        if (files != NULL && cJSON_IsArray(files)) {
            int num_files = cJSON_GetArraySize(files);
            for (int i = 0; i < num_files; i++) {
                cJSON *file = cJSON_GetArrayItem(files, i);
                insert_file_metadata(db, file);
            }
        }
        cJSON_Delete(json);
    }

    free(json_data);
    sqlite3_close(db);

    return 0;
}
