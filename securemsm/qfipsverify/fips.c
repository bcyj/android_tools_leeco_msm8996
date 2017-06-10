/********************************************************************
---------------------------------------------------------------------
 Copyright (c) 2014 Qualcomm Technologies, Inc.
 All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
----------------------------------------------------------------------
FIPS140-2 Integrity Test Application
*********************************************************************/

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <cutils/properties.h>
#include <linux/qcedev.h>
#include <linux/fips_status.h>
#include "bootimg.h"

#define STANDARD_PAGE_SIZE 2048
#define QCE_DEVICE "/dev/qce"
#define EMMC_BOOT_DEVICE "/dev/block/platform/msm_sdcc.1/by-name/boot"
#define KERNEL_HMAC_FILE "/system/usr/qfipsverify/bootimg.hmac"
#define FIPS_BINARY "/system/bin/qfipsverify"
#define QFIPSVERIFY_HMAC_FILE "/system/usr/qfipsverify/qfipsverify.hmac"
#define KERNEL_FILE_NAME "zImage"
#define SHA256_DIGEST_SIZE 32
#define CHUNK_SIZE 0xFF00

static char *hmac_key = "\xb6\x17\x31\x86\x55\x05\x72\x64"
                "\xe2\x8b\xc0\xb6\xfb\x37\x8c\x8e\xf1"
                "\x46\xbe";

unsigned numberofchunks(unsigned x, unsigned y)
{
    unsigned quotient = x/y;
    unsigned remainder = x%y;

    if(remainder == 0)
        return quotient;
    else
        return quotient + 1;
}

int verify_kernel_image(boot_img_hdr *img, FILE *stream)
{
    struct qfips_verify_t data;
    struct qcedev_sha_op_req  req;
    FILE *hmac;
    int fd;
    size_t result = 0;
    size_t rb;
    char hmac_stored[2 * SHA256_DIGEST_SIZE],hmac_generated[2 * SHA256_DIGEST_SIZE];
    unsigned hmac_generated_index = 0;
    unsigned i;
    unsigned number_of_chunks;
    unsigned final_chunk;
    unsigned psize;

    psize = img->page_size;
    data.kernel_size = img->kernel_size;

    number_of_chunks = numberofchunks(data.kernel_size, CHUNK_SIZE);
    data.kernel = malloc(CHUNK_SIZE);
    if (!data.kernel)
    {
        printf("Memory allocation failed - creating kernel image!\n");
        return -1;
    }

    if (fseek(stream, psize, SEEK_SET))
    {
        printf("Fseek failure!\n");
        free(data.kernel);
        return -1;
    }

    fd = open(QCE_DEVICE, O_RDWR);
    if (fd < 0)
    {
        printf("Cannot open device QCEDEV!!!\n");
        free(data.kernel);
        return -1;
    }

    hmac = fopen(KERNEL_HMAC_FILE, "r");
    if (!hmac)
    {
        printf("Opening bootimg.hmac Failed!!!\n");
        free(data.kernel);
        close(fd);
        return -1;
    }

    req.data[0].vaddr = (uint8_t *)data.kernel;
    req.data[0].len = CHUNK_SIZE;
    req.data_len = CHUNK_SIZE;
    req.diglen = SHA256_DIGEST_SIZE;
    req.entries = 1;
    req.authklen = strlen(hmac_key);
    req.authkey = (uint8_t *)hmac_key;
    req.alg = QCEDEV_ALG_SHA256_HMAC;
    memset(&req.digest[0], 0, QCEDEV_MAX_SHA_DIGEST);

    if (ioctl(fd, QCEDEV_IOCTL_SHA_INIT_REQ, &req))
    {
        printf("QCEDEV_IOCTL_SHA_INIT_REQ Failed!!\n");
        result = -1;
        goto Cleanup;
    }

    for(i = 0;i < number_of_chunks - 1;i++)
    {
        rb = fread(data.kernel, CHUNK_SIZE, 1, stream);
        if ((rb != 1) || ferror(stream))
        {
            printf("Fread failure!!!\n");
            result = -1;
            goto Cleanup;
        }

        req.data[0].vaddr = (uint8_t *)data.kernel;
        req.data[0].len = CHUNK_SIZE;
        req.data_len = CHUNK_SIZE;
        req.diglen = SHA256_DIGEST_SIZE;
        req.entries = 1;
        if (ioctl(fd, QCEDEV_IOCTL_SHA_UPDATE_REQ, &req))
        {
            printf("QCEDEV_IOCTL_SHA_UPDATE_REQ Failed!!\n");
            result = -1;
            goto Cleanup;
        }

    }

    final_chunk = data.kernel_size - (number_of_chunks - 1) * CHUNK_SIZE;
    rb = fread(data.kernel, final_chunk, 1, stream);
    if ((rb!=1) || ferror(stream))
    {
        printf("Fread failure!\n");
        result = -1;
        goto Cleanup;
    }

    req.data[0].vaddr = (uint8_t *)data.kernel;
    req.data[0].len = final_chunk;
    req.data_len = final_chunk;
    req.diglen = SHA256_DIGEST_SIZE;
    req.entries = 1;
    if (ioctl(fd, QCEDEV_IOCTL_SHA_UPDATE_REQ, &req))
    {
        printf("QCEDEV_IOCTL_SHA_UPDATE_REQ Failed!!\n");
        result = -1;
        goto Cleanup;
    }

    if (ioctl(fd, QCEDEV_IOCTL_SHA_FINAL_REQ, &req))
    {
        printf("QCEDEV_IOCTL_SHA_FINAL_REQ Failed!!\n");
        result = -1;
        goto Cleanup;
    }

    rb = fread(&hmac_stored[0], 2 * SHA256_DIGEST_SIZE, 1, hmac);
    if ((rb!=1) || ferror(hmac))
    {
        printf("Fread failure!\n");
        result = -1;
        goto Cleanup;;
    }
    for (i=0; i < SHA256_DIGEST_SIZE; i++)
    {
        snprintf(&hmac_generated[hmac_generated_index], 3, "%02x", req.digest[i]);
        hmac_generated_index += 2;
    }

    if (memcmp(hmac_stored, hmac_generated, 2 * SHA256_DIGEST_SIZE))
    {
        printf("Memcmp Failed \n");
        result = -1;
    goto Cleanup;
    }

Cleanup:
    close(fd);
    fclose(hmac);
    free(data.kernel);
    return result;
}

int verify_fips_binary(FILE *stream)
{
    struct qcedev_sha_op_req  req;
    FILE *hmac;
    char *buf_fips_binary;
    int fd;
    size_t result = 0;
    size_t rb;
    long fips_binary_size;
    char hmac_stored[2 * SHA256_DIGEST_SIZE], hmac_generated[2 * SHA256_DIGEST_SIZE];
    unsigned hmac_generated_index = 0,i;

    if (fseek(stream, 0, SEEK_END))
    {
        printf("fseek failure!\n");
        return -1;
    }

    fips_binary_size = ftell(stream);
    if (fips_binary_size == -1)
    {
        printf("ftell failure!\n");
        return -1;
    }

    rewind(stream);

    buf_fips_binary = malloc(fips_binary_size);
    if (!buf_fips_binary)
    {
        printf("Memory allocation failed - creating buffer for fips binary\n");
        return -1;
    }

    rb = fread(buf_fips_binary, fips_binary_size, 1, stream);
    if ((rb!=1) || ferror(stream))
    {
        printf("Fread failure - fips binary!\n");
        free(buf_fips_binary);
        return -1;
    }

    fd = open(QCE_DEVICE, O_RDWR);
    if (fd < 0)
    {
        printf("Cannot open device QCEDEV!!\n");
        free(buf_fips_binary);
        return -1;
    }

    hmac = fopen(QFIPSVERIFY_HMAC_FILE, "r");
    if (!hmac)
    {
        printf("Opening qfipsverify.hmac Failed!\n");
        free(buf_fips_binary);
        close(fd);
        return -1;
    }

    req.data[0].vaddr = (uint8_t *)buf_fips_binary;
    req.data[0].len = fips_binary_size;
    req.data_len = fips_binary_size;
    req.diglen = SHA256_DIGEST_SIZE;
    req.entries = 1;
    req.authklen = strlen(hmac_key);
    req.authkey = (uint8_t *)hmac_key;
    req.alg = QCEDEV_ALG_SHA256_HMAC;
    memset(&req.digest[0], 0, QCEDEV_MAX_SHA_DIGEST);

    if (ioctl(fd, QCEDEV_IOCTL_SHA_INIT_REQ, &req))
    {
        printf("QCEDEV_IOCTL_SHA_INIT_REQ Failed!!\n");
        result = -1;
        goto Cleanup;
    }

    if (ioctl(fd, QCEDEV_IOCTL_SHA_UPDATE_REQ, &req))
    {
        printf("QCEDEV_IOCTL_SHA_UPDATE_REQ Failed!!\n");
        result = -1;
        goto Cleanup;
    }

    if (ioctl(fd, QCEDEV_IOCTL_SHA_FINAL_REQ, &req))
    {
        printf("QCEDEV_IOCTL_SHA_FINAL_REQ Failed!!\n");
        result = -1;
        goto Cleanup;
    }

    rb = fread(&hmac_stored[0], 2 * SHA256_DIGEST_SIZE, 1, hmac);
    if ((rb!=1) || ferror(hmac))
    {
        printf("Fread failure!\n");
        result = -1;
        goto Cleanup;
    }

    for (i=0; i < SHA256_DIGEST_SIZE; i++)
    {
        snprintf(&hmac_generated[hmac_generated_index], 3, "%02x", req.digest[i]);
        hmac_generated_index += 2;
    }

    if (memcmp(hmac_stored, hmac_generated, 2 * SHA256_DIGEST_SIZE))
    {
        printf("Memcmp Failed \n");
        result = -1;
        goto Cleanup;
    }

Cleanup:
    close(fd);
    fclose(hmac);
    free(buf_fips_binary);
    return result;
}

int read_kernel_image_header(boot_img_hdr *img, FILE *stream)
{
    size_t result;

    result = fread(img, sizeof(boot_img_hdr), 1, stream);
    if ((result!=1) || ferror(stream))
    {
        printf("Fread failed failed!\n");
        return -1;
    }
    else if (feof(stream))
    {
        printf("Cannot read image header!\n");
        return -1;
    }

    return 0;
}

int open_emmc_device(FILE **stream)
{
    *stream = fopen(EMMC_BOOT_DEVICE, "r");
    if (*stream == NULL)
    {
        printf("Opening boot partition for read failed!\n");
        return -1;
    }

    return 0;
}

int open_fips_binary(FILE **stream)
{
    *stream = fopen(FIPS_BINARY, "r");
    if (*stream == NULL)
    {
        printf("Opening fips binary for read failed!\n");
        return -1;
    }

    return 0;
}

int set_fips_state(enum fips_status fips_enabled)
{
    int fd, err;

    fd = open(QCE_DEVICE, O_RDWR);
    if (fd < 0)
    {
        printf("Cannot open device QCEDEV!!\n");
        return -1;
    }
    err = ioctl(fd, QCEDEV_IOCTL_UPDATE_FIPS_STATUS, &fips_enabled);
    close(fd);

    if(err)
        return -1;
    else
        return 0;
}

int main()
{

#ifdef FIPSENABLED
    FILE *stream1, *stream2;
    boot_img_hdr *img;
    size_t result;
    enum fips_status fips_enabled;
    unsigned ret;

    img = calloc(sizeof(boot_img_hdr), 1);
    if (!img)
    {
        printf("Memory allocation for image failed!\n");
        fips_enabled = FIPS140_STATUS_FAIL;
        result = set_fips_state(fips_enabled);
        return result;
    }

    memcpy(img->magic, BOOT_MAGIC, BOOT_MAGIC_SIZE);
    img->page_size = STANDARD_PAGE_SIZE;

    result = open_emmc_device(&stream1);
    if (result != 0)
    {
        fips_enabled = FIPS140_STATUS_FAIL;
        result = set_fips_state(fips_enabled);
        free(img);
        return result;
    }

    result = open_fips_binary(&stream2);
    if (result != 0)
    {
        fips_enabled = FIPS140_STATUS_FAIL;
        result = set_fips_state(fips_enabled);
        free(img);
        fclose(stream1);
        return result;
    }

    result = read_kernel_image_header(img, stream1);
    if (result != 0)
    {
        fips_enabled = FIPS140_STATUS_FAIL;
        result = set_fips_state(fips_enabled);
        goto Cleanup;
    }

    result = verify_kernel_image(img, stream1);
    if (result != 0)
    {
        fips_enabled = FIPS140_STATUS_FAIL;
        result = set_fips_state(fips_enabled);
        goto Cleanup;
    }

    result = verify_fips_binary(stream2);
    if (result != 0)
    {
        fips_enabled = FIPS140_STATUS_FAIL;
        result = set_fips_state(fips_enabled);
        goto Cleanup;
    }

    fips_enabled = FIPS140_STATUS_PASS;
    result = set_fips_state(fips_enabled);

Cleanup:
    free(img);
    fclose(stream2);
    fclose(stream1);
    return result;

#else
	return 0;

#endif /*FIPSENABLED*/

}
