#include "../include/manifest.h"
#include <stdlib.h>
#include <string.h>
#include "esp_log.h"

#define TAG "MANIFEST"

static ManifestContent* content = NULL;

void _clearBuffer(char* buff, int size) {
    for (int i = 0; i < size; i++)
        buff[i] = '\0';
}

int readManifestToMemory() {
    if (content != NULL) {
        ESP_LOGE(TAG, "Manifest already read to memory. Deallocate before reading again");
        return MANIFEST_FAILURE;
    }
    
    // allocate data structure to store info
    content = malloc(sizeof(ManifestContent));
    content->numEntry = 0;
    content->head = NULL;
    content->tail = NULL;

    FILE* fp = fopen(MANIFEST_FILENAME, "r");
    if (fp == NULL) {
        ESP_LOGI(TAG, "Manifest file does not exist yet");
        return MANIFEST_SUCCESS;
    }

    // store manifest info to structure
    char lineBuffer[64];
    char* displayName = NULL;
    char* username = NULL;
    char* url = NULL;

    do {
        int i = 0;
        memset(lineBuffer, '\0', sizeof(lineBuffer) / sizeof(lineBuffer[0]));
        do {
            lineBuffer[i] = fgetc(fp);
            if (feof(fp))
                return MANIFEST_SUCCESS;
        } while(lineBuffer[i++] != '\n');
        lineBuffer[i-1] = '\0';

        displayName = strtok(lineBuffer, ",");
        username = strtok(NULL, ",");
        url = strtok(NULL, ",");

        addManifestEntry(displayName, username, url);
        // attempt to read past EOF to set EOF flag
        if (fgetc(fp) != EOF)
            fseek(fp, -1, SEEK_CUR);
    } while (!feof(fp));

    fclose(fp);

    ESP_LOGI(TAG, "Successfully read manifest to memory");
    return MANIFEST_SUCCESS;
}

int writeManifestToFile() {
    FILE* fp = fopen(MANIFEST_FILENAME, "w");
    if (fp == NULL) {
        ESP_LOGE(TAG, "Unable to open manifest file for writing");
        return MANIFEST_FAILURE;
    }

    ManifestEntry* currEntry = content->head;
    while (currEntry != NULL) {
        fprintf(fp, currEntry->displayName);
        fputc(',', fp);
        fprintf(fp, currEntry->username);
        fputc(',', fp);
        fprintf(fp, currEntry->url);
        fputc(',', fp);
        fputc('\n', fp);
        currEntry = currEntry->next;
    }
    fclose(fp);

    ESP_LOGI(TAG, "Successfully written manifest to file");
    return MANIFEST_SUCCESS;
}

int deallocateManifest() {
    if (content == NULL) {
        ESP_LOGE(TAG, "Attempting to deallocate manifest that was unallocated");
        return MANIFEST_FAILURE;
    }

    ManifestEntry* currEntry = content->head;
    while(currEntry != NULL) {
        ManifestEntry* nextEntry = currEntry->next;
        free(currEntry);
        currEntry = nextEntry;
    }
    free(content);
    content = NULL;

    // ESP_LOGI(TAG, "Successfully deallocated manifest on memory");
    return MANIFEST_SUCCESS;
}

int addManifestEntry(char* displayName, char* username, char* url) {
    if (content == NULL)
        return MANIFEST_FAILURE;

    ManifestEntry* newEntry = NULL;
    newEntry = getManifestEntry(displayName, username);
    if (newEntry == NULL) {
        newEntry = calloc(1, sizeof(ManifestEntry));
        if (content->head == NULL)
            content->head = newEntry;
        else
            content->tail->next = newEntry;
        content->tail = newEntry;
        newEntry->next = NULL;
        content->numEntry += 1;
    }

    strcpy(newEntry->displayName, displayName);
    strcpy(newEntry->username, username);
    strcpy(newEntry->url, url);

    // ESP_LOGI(TAG, "Successfully added manifest entry");
    return MANIFEST_SUCCESS;
}

ManifestEntry* getManifestEntry(char* displayName, char* userName) {
    if (content == NULL)
        return NULL;
    ManifestEntry* currEntry = content->head;
    while(currEntry != NULL) {
        if ( (strcmp(currEntry->displayName, displayName) == 0) && (strcmp(currEntry->username, userName) == 0) )
            break;
        currEntry = currEntry->next;
    };
    /*
    if (currEntry != NULL)
        ESP_LOGI(TAG, "Successfully retrieved manifest entry");
    else
        ESP_LOGE(TAG, "Failed to retrieve manifest entry. Not found");
    */
    
    return currEntry;
}

int removeManifestEntry(char* displayName, char* userName) {
    if (content == NULL)
        NULL;

    ManifestEntry* currEntry = content->head;
    if (currEntry == NULL)
        return MANIFEST_FAILURE;

    if ((strcmp(currEntry->displayName, displayName) == 0)
        && (strcmp(currEntry->username, userName) == 0)) {
        content->head = currEntry->next;
        free(currEntry);
        content->numEntry -= 1;
        return MANIFEST_SUCCESS;
    }
    
    while (currEntry->next != NULL) {
        ManifestEntry* prevEntry = currEntry;
        currEntry = currEntry->next;
        if ((strcmp(currEntry->displayName, displayName) == 0)
            && (strcmp(currEntry->username, userName) == 0)) {
            prevEntry->next = currEntry->next;
            if (currEntry->next == NULL)
                content->tail = prevEntry;
            free(currEntry);
            content->numEntry -= 1;
            // ESP_LOGI(TAG, "Successfully removed manifest entry");
            return MANIFEST_SUCCESS;
        }
    };

    // ESP_LOGE(TAG, "Failed to remove manifest entry");
    return MANIFEST_FAILURE;
}

int wipeStorageData() {
    while(content->head != NULL) {
        char path[256] = {'\0'};
        strcat(path, "/sdcard/");
        strcat(path, content->head->displayName);
        remove(path);
        ManifestEntry* tmp = content->head;
        content->head = content->head->next;
        free(tmp);
    }
    // wipe content of MANIFEST file
    writeManifestToFile();
    ESP_LOGI(TAG, "Successfully wiped SD storage data");
    return MANIFEST_SUCCESS;
}