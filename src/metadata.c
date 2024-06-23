#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

// Define a struct to hold the API response
struct ApiResponse {
    char *data;
    size_t size;
};

// Callback function to write data into our ApiResponse struct
static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    struct ApiResponse *mem = (struct ApiResponse *)userp;

    char *ptr = realloc(mem->data, mem->size + realsize + 1);
    if(ptr == NULL) {
        // out of memory
        printf("Not enough memory (realloc returned NULL)\n");
        return 0;
    }

    mem->data = ptr;
    memcpy(&(mem->data[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->data[mem->size] = 0;

    return realsize;
}

int main(void) {
    CURL *curl;
    CURLcode res;

    struct ApiResponse response;

    response.data = malloc(1);  // Will be grown by realloc in the callback
    response.size = 0;          // No data at this point

    curl_global_init(CURL_GLOBAL_DEFAULT);

    curl = curl_easy_init();
    if(curl) {
        struct curl_slist *headers = NULL;

        // Retrieve the token from an environment variable
        const char *token = getenv("API_TOKEN");
        if (!token) {
            fprintf(stderr, "API token is not set in environment variables.\n");
            return EXIT_FAILURE;
        }
        char auth_header[256];
        snprintf(auth_header, sizeof(auth_header), "Authorization: Bearer %s", token);

        headers = curl_slist_append(headers, auth_header);

        const char *url = "https://www.googleapis.com/drive/v3/files?fields=files(id,name,mimeType,kind,size,createdTime,modifiedTime,owners,shared,permissions)";
        
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&response);

        // Additional security options
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);

        // Perform the request, res will get the return code
        res = curl_easy_perform(curl);

        // Check for errors
        if(res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        } else {
            // Print the response safely (assuming safe data from trusted API source)
            FILE *f = fopen("metadata.json", "w");
            if (f == NULL) {
                perror("Error opening file");
                return EXIT_FAILURE;
            }
            fprintf(f, "%s", response.data);
            fclose(f);
        }

        // Clean up
        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
    }

    free(response.data);
    curl_global_cleanup();

    return 0;
}
