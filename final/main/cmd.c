#include <string.h>
#include "../include/cmd.h"
#include "../include/manifest.h"
#include "esp_log.h"
#include "../include/bt.h"
#include "../include/crypto.h"

// one function per command

static int running = 1;

int getRunning() {
    return running;
}

int cmd_led_red(int status) {
    return gpio_set_level(GPIO_RED, status);
}

int cmd_led_green(int status) {
    return gpio_set_level(GPIO_GREEN, status);
}

int cmd_request_entries(int mode) {
    writeManifestToFile();
    FILE* fp = fopen(MANIFEST_FILENAME, "r");
    if (fp == NULL)
        return CMD_FAILURE;
    fseek(fp, 0, SEEK_END);
    size_t filesize = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    char* buffer = calloc(filesize, sizeof(char));
    fread(buffer, sizeof(char), filesize, fp);
    if (mode == UART_MODE)
        uart_write_bytes(PORT_NUM, buffer, filesize);
    else if (mode == BT_MODE)
        btSendData((uint8_t*) buffer, filesize);
    free(buffer);
    fclose(fp);
    return CMD_SUCCESS;
}

int cmd_request_credential(char* displayName, char* username, int mode) {
    if (getManifestEntry(displayName, username) == NULL)
        return CMD_FAILURE;
    char path[256] = {'\0'};
    strcat(path, "/sdcard/");
    strcat(path, displayName); 
    FILE* fp = fopen(path, "r");
    if (fp == NULL)
        return CMD_FAILURE;
    fseek(fp, 0, SEEK_END);
    size_t filesize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char* buffer = calloc(filesize + 1, sizeof(char));
    fread(buffer, sizeof(char), filesize, fp);
    buffer[filesize] = '\n';
    if (mode == UART_MODE)
        uart_write_bytes(PORT_NUM, buffer, filesize + 1);
    else if (mode == BT_MODE)
        btSendData((uint8_t*) buffer, filesize + 1);
    free(buffer);
    fclose(fp);
    return CMD_SUCCESS;
}

int cmd_modify_credential(char* displayName, char* username, char* pw) {
    if (getManifestEntry(displayName, username) == NULL)
        return MANIFEST_FAILURE;
        
    char path[256] = {'\0'};
    strcat(path, "/sdcard/");
    strcat(path, displayName);
    FILE* fp = fopen(path, "w");
    if (fp == NULL)
        return CMD_FAILURE;
    fprintf(fp, pw);
    fclose(fp);
    return CMD_SUCCESS;
}

int cmd_store_credential(char* displayName, char* username, char* url, char* pw) {
    addManifestEntry(displayName, username, url);
    char path[256] = {'\0'};
    strcat(path, "/sdcard/");
    strcat(path, displayName);
    FILE* fp = fopen(path, "w");
    if (fp == NULL)
        return CMD_FAILURE;
    fprintf(fp, pw);
    fclose(fp);
    return CMD_SUCCESS;
}

int cmd_delete_credential(char* displayName, char* userName) {
    if(!removeManifestEntry(displayName, userName))
        return CMD_FAILURE;
    char path[256] = {'\0'};
    strcat(path, "/sdcard/");
    strcat(path, displayName);
    remove(path);
    return CMD_SUCCESS;
}

void doCMD(uint8_t* data, int mode) {
    // debug
    if (mode == BT_MODE) {
        for (int i = 0; i < 20; i++)
            ESP_LOGI("debug-msg", "%d - %c", data[i], data[i]);
    }

    int returnStatus = 0;
    char *displayName, *username, *url, *pw;
    switch (data[1]) {
        case CMD_LED_RED:
            returnStatus = cmd_led_red(data[3]);
            break;
        case CMD_LED_GREEN:
            returnStatus = cmd_led_green(data[3]);
            break;
        case CMD_REQUEST_ENTRIES:
            returnStatus = cmd_request_entries(mode);
            break;
        case CMD_REQUEST_CREDENTIAL:
            displayName = strtok((char*) &data[3], ",");
            username = strtok(NULL, ",");
            returnStatus = cmd_request_credential(displayName, username, mode);
            break;
        case CMD_STORE_CREDENTIAL:
            displayName = strtok((char*) &data[3], ",");
            username = strtok(NULL, ",");
            url = strtok(NULL, ",");
            pw = strtok(NULL, ",");
            returnStatus = cmd_store_credential(displayName, username, url, pw);
            break;
        case CMD_MODIFY_CREDENTIAL:
            displayName = strtok((char*) &data[3], ",");
            username = strtok(NULL, ",");
            pw = strtok(NULL, ",");
            returnStatus = cmd_modify_credential(displayName, username, pw);
            break;
        case CMD_DELETE_CREDENTIAL:
            displayName = strtok((char*) &data[3], ",");
            username = strtok(NULL, ",");
            returnStatus = cmd_delete_credential(displayName, username);
            break;
        case CMD_POWER_OFF:
            running = 0;
            break;
        default:
            returnStatus = 0;
    }

    char toSend[2];

    switch (data[1]) {
        case CMD_REQUEST_CREDENTIAL:
        case CMD_REQUEST_ENTRIES:
            if (returnStatus == CMD_FAILURE) {
                toSend[0] = '0';
                toSend[1] = '\n';
                if (mode == BT_MODE)
                    btSendData((uint8_t*) toSend, 2);
                else if (mode == UART_MODE)
                    uart_write_bytes(PORT_NUM, toSend, 2);
            }
            break;
        case CMD_LED_RED:
        case CMD_LED_GREEN:
        case CMD_STORE_CREDENTIAL:
        case CMD_MODIFY_CREDENTIAL:
        case CMD_DELETE_CREDENTIAL:
            toSend[0] = returnStatus + '0';
            toSend[1] = '\n';
            if (mode == BT_MODE)
                btSendData((uint8_t*) toSend, 2);
            else if (mode == UART_MODE)
                uart_write_bytes(PORT_NUM, toSend, 2);
    }
}