/*
 * Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>

#include "Zip.h"
#include "SysUtil.h"

#define PACK_PATH_LEN 256

// Where in the package we expect to find the origional update-binary.
#define BINARY_NAME "META-INF/com/google/android/ori-update-binary"

// Where to back up origional update-binary.
#define BACK_BINARY_NAME "/tmp/ori_update_binary"

// Stored new carrier name like "newcarrier=/sdcard/xxx.oat.zip".
#define ACTION_FILE "/cache/action"

// Where to load the public keys.
#define PUBLIC_KEYS_FILE "/res/keys"

// Get the path of new carrier package will be installed next.
char* getNewCarrierPack();

// Verify the carrier package will be installed.
int verifyCarrierPack(const char* path);

// These need be defined for verification function
// Here we define them to fake values.
RecoveryUI* ui = NULL;

class FakeUI : public RecoveryUI {
    void Init() { }
    void SetBackground(Icon icon) { }
    void SetProgressType(ProgressType determinate) { }
    void SetProgress(float fraction) { }
    void StartMenu(const char* const * headers,
            const char* const * items,
            int initial_selection) { }
    void ShowProgress(float portion, float seconds) { }
    void ShowText(bool visible) { }
    int SelectMenu(int sel) { return 0; }
    void EndMenu() { }
    bool IsTextVisible() { return false; }
    bool WasTextEverVisible() { return false; }
    void Print(const char* fmt, ...) { }
};

void ui_print(const char* format, ...) { }

int main(int argc, char** argv) {

    setbuf(stdout, NULL);

    setbuf(stderr, NULL);

    if (argc != 4) {
        printf("Input the wrong argument number for (%d)\n", argc);
        return 1;
    }

    char* version = argv[1];
    if ((version[0] != '1' &&
        version[0] != '2' &&
        version[0] != '3') ||
        version[1] != '\0') {
        printf("Use the wrong updater binary API for ver%s\n", argv[1]);
        return 2;
    }

    // Back up the ori-update-binary to tmp/ from the package.
    const char* carrierPack = (char*) argv[3];
    MemMapping mmap;
    if (sysMapFile(carrierPack, &mmap) != 0) {
        printf("Failed to map carrier package %s\n", carrierPack);
        return 3;
    }
    ZipArchive ZA;
    int error;
    error = mzOpenZipArchive(mmap.addr, mmap.length, &ZA);
    if (error != 0) {
        sysReleaseMap(&mmap);
        printf("Failed to open carrier package %s: %s\n", carrierPack, strerror(error));
        return 3;
    }

    const ZipEntry* binary_entry = mzFindZipEntry(&ZA, BINARY_NAME);
    if (binary_entry == NULL) {
        sysReleaseMap(&mmap);
        printf("Failed to find %s in carrier package %s\n", BINARY_NAME, carrierPack);
        return 4;
    }

    unlink(BACK_BINARY_NAME);
    int FD = creat(BACK_BINARY_NAME, 0755);
    if (FD < 0) {
        sysReleaseMap(&mmap);
        mzCloseZipArchive(&ZA);
        printf("Can't create file %s\n", BACK_BINARY_NAME);
        return 5;
    }
    bool success = mzExtractZipEntryToFile(&ZA, binary_entry, FD);
    close(FD);
    mzCloseZipArchive(&ZA);
    sysReleaseMap(&mmap);
    if (success) {
        printf("Write original %s to %s success!\n", BINARY_NAME, BACK_BINARY_NAME);
    } else {
        return 6;
    }

    // Begin to install ota package...
    argv[0] = (char*) BACK_BINARY_NAME;
    argv[3] = (char*) carrierPack;
    pid_t pid = fork();
    if (pid == 0) {
        umask(022);
        execv(BACK_BINARY_NAME, (char* const*)argv);
        printf("Can't run %s.\n", BACK_BINARY_NAME);
        return 7;
    }
    waitpid(pid, NULL, 0);

    char* newCarrierPack = getNewCarrierPack();
    if (newCarrierPack != NULL && strcmp(newCarrierPack, "")) {
        printf("\n\nThe new carrier package is %s.\n", newCarrierPack);
        // Switch to new carrier
        if (sysMapFile(newCarrierPack, &mmap) != 0) {
            printf("Failed to map new carrier package %s\n", newCarrierPack);
            return 8;
        }
        sysReleaseMap(&mmap);

        printf("Installing new carrier package %s...\n", newCarrierPack);
        int result = verifyCarrierPack(newCarrierPack);
        if (result != 0) {  // Verify faild!
            return 9;
        }

        argv[3] = (char*) newCarrierPack;
        pid_t pid = fork();
        if (pid == 0) {
            umask(022);
            execv(BACK_BINARY_NAME, (char* const*)argv);
            printf("E:Can't run %s.\n", BACK_BINARY_NAME);
            return 10;
        }
        waitpid(pid, NULL, 0);
    }
    free(newCarrierPack);

    return 0;
}

char* getNewCarrierPack() {

    FILE* fAction = NULL;
    char* path = (char*) malloc(sizeof(char) * PACK_PATH_LEN + 1);
    if (path == NULL) {
        return NULL;
    }

    if ((fAction = fopen(ACTION_FILE, "r")) == NULL) {
        return NULL;
    }

    memset(path, 0, sizeof(char) * PACK_PATH_LEN + 1);
    int count = fscanf(fAction, "%*[^=]=%s", path);

    fclose(fAction);
    if (remove(ACTION_FILE) == 0) {
        printf("Removed %s.", ACTION_FILE);
    }

    if (count < 1 || path[0] == '\0') {
        return NULL;
    }

    return path;
}

int verifyCarrierPack(const char* path) {

    ui = new FakeUI();

    MemMapping mmap;
    if (sysMapFile(path, &mmap) != 0) {
        printf("VerifyCarrierPack: Failed to map carrier package.\n");
        return 1;
    }

    int keysNum;
    Certificate* publicKeys = load_keys(PUBLIC_KEYS_FILE, &keysNum);
    if (publicKeys == NULL) {
        sysReleaseMap(&mmap);
        printf("VerifyCarrierPack: Failed to load public keys\n");
        return 2;
    }
    printf("VerifyCarrierPack: %d key(s) loaded from %s\n",
            keysNum, PUBLIC_KEYS_FILE);

    printf("VerifyCarrierPack: Verifying carrier package...%s\n", path);

    int error = verify_file(mmap.addr, mmap.length, publicKeys, keysNum);
    free(publicKeys);
    printf("VerifyCarrierPack: Verify_file returned %d\n", error);
    if (error != VERIFY_SUCCESS) {
        printf("VerifyCarrierPack: Signature verification failed\n");
        sysReleaseMap(&mmap);
        return 3;
    }

    return 0;
}
