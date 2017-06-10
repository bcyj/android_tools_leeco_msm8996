/*
* Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
* Qualcomm Technologies Proprietary and Confidential.
*
*/
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
using namespace std;

typedef unsigned char uint8 ;
typedef unsigned short uint16;
typedef unsigned int uint32;

static int parseData(uint8 *dest, uint16 bufLength, FILE *file, char **type)
{
    uint16 k=0, i=0;
    size_t length=0;
    char *data = NULL;
    while( getline(&data, &length, file) > 0) {
        if(!strncmp(data, "  ", 2) || !strncmp(data, "\t", 1)) {
            char *temp = &data[0];
            while(*temp == ' ' || *temp == '\t')
                temp++;
            for(i =0; (k < bufLength && temp) &&  (i < 16); k+=2, i++) {
                while(*temp == ' ')
                    temp++;
                if(sscanf(temp, "%c%c", &dest[k], &dest[k+1]) != 2) {
                    break;
                }
                temp++;
                temp++;
                dest[k+2] = '\0';
            }
            length = 0;
        } else {
            *type = data;
            data = NULL;
            break;
        }
    }
    return 0;
}

int main(int argc, char *argv[])
{
    if(argc < 2)
    {
        fprintf(stderr, "Please pass file name as an argument\n");
        fprintf(stderr, "Usage : \ndisaDaemon file.txt\n");
        return 0;
    }

    FILE *file = fopen(argv[1], "r");

    if (!file) {
        fprintf(stderr, "Unable to open the file %s\n", argv[1]);
        return 0;
    }


    char *type = (char *)malloc(256* sizeof(char));
    char *dest = NULL;
    uint8 header[32];
    uint8 payload[512];
    uint16 i=0, j=0, bufLength = 0, k=0;
    size_t length=0;
    char cmd[1024];
    uint16 cmdLength = 0;
    int bufLength1 = 0;
    memset(cmd, 0, 1024);
    strcpy(cmd, "iwpriv wlan0 encryptMsg ");
    cmdLength = 24;


    while(type || getline(&type, &length, file) > 0 )
    {
        i = 0, dest = NULL, k=0;
        j++;

        if( !strncmp(type, "KeyId = ", (size_t)8)) {
            sscanf(type, "KeyId = %c", &cmd[cmdLength]);

            cmdLength++;
            cmd[cmdLength] = '\0';

            if(type) {
                free(type);
                type = NULL;
            }
            continue;
        } else if( !strncmp(type, "TK ", (size_t)3)) {

            bufLength = 0x20;
            parseData(header, bufLength, file, &type);

            for(k=0; k< bufLength; k++) {
                cmd[cmdLength++] = header[k];
            }
            continue;
        } else if( !strncmp(type, "PN ", (size_t)3)) {

            bufLength = 0xc;
            parseData(header, bufLength, file, &type);

            for(k=0; k< bufLength; k++) {
                cmd[cmdLength++] = header[k];
            }
            continue;
        } else if( !strncmp(type, "Header", (size_t)6)) {
            sscanf(type, "Header %d", &bufLength1);

            bufLength = (uint16)bufLength1 * 2;

            parseData(header, bufLength, file, &type);

            char length[4];
            sprintf(length, "%x", bufLength1);

            for(k=0; k< strlen(length); k++) {
                cmd[cmdLength++] = length[k];
            }
            cmd[cmdLength] = '\0';

            for(k=0; k< bufLength; k++) {
                cmd[cmdLength++] = header[k];
            }
            continue;
        } else if (!strncmp(type, "EncHeader", 9)) {
        } else if (!strncmp(type, "Muted MAC Header", 16)) {
        } else if (!strncmp(type, "CCM Nounce", 10)) {
        } else if (!strncmp(type, "MIC", 3)) {
        } else if (!strncmp(type, "Data", 4)) {

            sscanf(type, "Data %d", &bufLength1);

            bufLength = (uint16)bufLength1 * 2;

            parseData(payload, bufLength, file, &type);

            char length[4];

            for(k=0; k< sizeof(uint16); k++) {
                memset(&length, 0, 4);
                sprintf(length, "%02x", (bufLength1 & 0xFF));
                cmd[cmdLength++] = length[0];
                cmd[cmdLength++] = length[1];
                bufLength1 >>= 8;
            }
            cmd[cmdLength] = '\0';

            for(k=0; k< bufLength; k++) {
                cmd[cmdLength++] = payload[k];
            }
            continue;
        } else if (!strncmp(type, "////", 4)) {
            break;
        } else {
        }

        if(type) {
            free(type);
            type = NULL;
        }
    }

   fprintf(stderr, "%s\n", cmd);
   if(type) {
       free(type);
   }
   system(cmd);
   return 0;
}
