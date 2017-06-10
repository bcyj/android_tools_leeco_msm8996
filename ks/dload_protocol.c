/*
 *  Implements the DMSS-DL "DLOAD" protocol used by the primary bootloader
 *
 *  Copyright (C) 2012 Qualcomm Technologies, Inc. All rights reserved.
 *                  Qualcomm Technologies Proprietary/GTDR
 *
 *  All data and information contained in or disclosed by this document is
 *  confidential and proprietary information of Qualcomm Technologies, Inc. and all
 *  rights therein are expressly reserved.  By accepting this material the
 *  recipient agrees that this material and the information contained therein
 *  is held in confidence and in trust and will not be used, copied, reproduced
 *  in whole or in part, nor its contents revealed in any manner to others
 *  without the express written permission of Qualcomm Technologies, Inc.
 *
 *
 *  dload_protocol.c : Implements the DMSS-DL "DLOAD" protocol used by the primary bootloader
 * ==========================================================================================
 *   $Header: //source/qcom/qct/core/storage/tools/kickstart/dload_protocol.c#9 $
 *   $DateTime: 2010/05/03 23:18:04 $
 *   $Author: niting $
 *
 *  Edit History:
 *  YYYY-MM-DD		who		why
 *  -----------------------------------------------------------------------------
 *
 *  Copyright 2012 by Qualcomm Technologies, Inc.  All Rights Reserved.
 *
 *==========================================================================================
 */
#include <sys/select.h>
#include <unistd.h>
#include "dload_protocol.h"
#include "kickstart_utils.h"

/* Currently, PBL allows only 256 bytes upload size at a given instance,
 * although it can support 1.5 kilobytes buffer size
 * safely. WRITE_SIZE_THRESHOLD is defined to allow the dload protocol to use
 * 1.5kilobytes buffer size to get better upload rate */

#define WRITE_SIZE_THRESHOLD 1536

extern unsigned int malloc_count;
extern unsigned int free_count;

/* Helper function to upload the dbl image */
static int upload_dbl_image (struct com_state *m_comm, char *dload_filename, int max_write_size);

char *szDloadCommands[25]={ "ILLEGAL",
    "WRITE",
    "ACK",
    "NAK",
    "ERASE",
    "GO",
    "NOOP",
    "PARAMREQUEST",
    "PARAMRESPONSE",
    "MEMDUMP",
    "RESET",
    "UNLOCK",
    "SWVERREQUEST",
    "SWVERRESPONSE",
    "POWERDOWN",
    "WRITEWITH32BITADDR",
    "MEMDEBUGQUERY",
    "MEMDEBUGINFO",
    "MEMREADREQUEST",
    "MEMREADRESPONSE"};

extern int old_pbl;

/*
 *  Load the input image using DMSS download protocol
 *
 *  PARAMETERS
 *  m_comm         [ I ] - Pointer to the comm port
 *  dload_filename [ I ] - Image file to download
 *
 *  RETURN VALUE: SUCCESS/EFAILED
 */
int dload_image (struct com_state *m_comm, char *dload_filename, int noclosedevnode)
{
    unsigned short max_write_size = 0;

    /* Query the target for maximum write size */
    dbg (EVENT, "Sending DLOAD PARAM_REQ to get the Maximum Write Size");
    max_write_size = dload_max_write_bytes (m_comm);

    if (EFAILED == max_write_size) {
        dbg (ERROR, "Get the device parameter's maximum write size option failed, failing the test");
        deinit_com_port (m_comm, 0);
        return EFAILED;
    }
    /* Log the maximum write size */
    dbg (EVENT, "Max Write Size returned by PBL: %d", max_write_size);

    /* Check if the maximum write size returned by the target is less than WRITE_SIZE_THRESHOLD,
     * if so then set the maximum write size to WRITE_SIZE_THRESHOLD for better upload rate*/
    if (max_write_size < WRITE_SIZE_THRESHOLD) {
        dbg (INFO, "Setting max_write_size to WRITE_SIZE_THRESHOLD:%d for better upload rate ", WRITE_SIZE_THRESHOLD);
        max_write_size = WRITE_SIZE_THRESHOLD;
    }

    /*Upload the Dbl Image */
    if (SUCCESS != upload_dbl_image (m_comm, dload_filename, max_write_size)) {
        dbg (ERROR, "Uploading the Dbl Image failed");
        deinit_com_port (m_comm, 0);
        return EFAILED;
    }

    dbg (EVENT, "\nUploaded the Dbl Image - SUCCESS\n");
    deinit_com_port (m_comm, noclosedevnode);
    return SUCCESS;
}


/*===========================================================================
 *  METHOD:
 *  dload_max_write_bytes
 *
 *  DESCRIPTION:
 *  Gets the maximum write size for the DMSS download protocol
 *
 *  PARAMETERS
 *  m_comm         [ I ] - Pointer to the comm port
 *
 *
 *  RETURN VALUE:
 *  int               - SUCCESS/EFAILED
 *  ===========================================================================*/
int dload_max_write_bytes (struct com_state *m_comm)
{
    char dload_param_req = 7;     /*parameter request code: 0x07*/
    int rc, size;
    struct timeval timeout;
    fd_set readfs;     /* file descriptor set */

    /*Convert to HDLC format */
    char *encoded_buffer = malloc ((sizeof(dload_param_req) * 2 + 4) * sizeof(char));
    if (NULL==encoded_buffer) {
        dbg (ERROR, "Memory allocation failure: %s", strerror(errno));
        return EFAILED;
    }
    malloc_count++;

    size = hdlc_encode (&dload_param_req, sizeof(dload_param_req), encoded_buffer);
    if (size == 0) {
        dbg (ERROR, "HDLC encoding returned false");
        if (NULL!=encoded_buffer) {
            free (encoded_buffer);
            encoded_buffer = NULL;
            free_count++;
        }
        return EFAILED;
    }

    /*Send the download request protocol*/
    if (SUCCESS != tx_data (encoded_buffer, size, m_comm)) {
        dbg (ERROR, "Download reques data transfer failed");
        if (NULL!=encoded_buffer) {
            free (encoded_buffer);
            encoded_buffer = NULL;
            free_count++;
        }
        return EFAILED;
    }

    /* Free the buffer used for creating the encoded buffer */
    if (NULL != encoded_buffer) {
        free (encoded_buffer);
        encoded_buffer = NULL;
        free_count++;
    }

    /* Wait for a maximum of 10 milliseconds */
    FD_ZERO(&readfs);
    FD_SET (m_comm->port_fd, &readfs);
    timeout.tv_usec = 10000;        /* microseconds */
    timeout.tv_sec = 0;         /* seconds */
    rc = select (m_comm->port_fd + 1, &readfs, NULL, NULL, &timeout);
    if (rc < 0) {
        dbg (ERROR, "Select on file descriptor returned failure %s", strerror (errno));
        return EFAILED;
    }

    if (rc == 0) {
        dbg (ERROR, "No data received from the target - timeout occured");
        return EFAILED;
    }

    int count = read (m_comm->port_fd, m_comm->recv_buffer, RX_BUFFER_SIZE);
    if (count < 0) {
        dbg (ERROR, "Read failed due to : %s", strerror (errno));
        return EFAILED;
    }

	dbg(INFO, "\nRecieved '%s' (0x%.2X) from PARAM_REQ",szDloadCommands[m_comm->recv_buffer[1]],m_comm->recv_buffer[1]);

    /*Check if parameter response is received by checking for 0x08 in the second field */
    if (0x08 != m_comm->recv_buffer[1]) 
    {
        dbg (ERROR, "Did not get PARAMRESPONSE (0x08) received for the PARM_REQ DMSS command: '%s' (0x%.2X)" ,  szDloadCommands[m_comm->recv_buffer[1]], m_comm->recv_buffer[1]);
        return EFAILED;
    }

    /*Get the maximum write size from the 5th and 6th bytes of the received payload */
    return m_comm->recv_buffer[4] << 8 | m_comm->recv_buffer[5];
}

/*===========================================================================
 *  METHOD:
 *  upload_dbl_image
 *
 *  DESCRIPTION:
 *  uploads the dbl image using dload protocol
 *
 *  PARAMETERS
 *  m_comm            - Pointer to the comm port
 *  max_write_size    - Maximum write size returned by the Download request
 *  dload_filename    - file to be uploaded
 *
 *  RETURN VALUE:
 *  int               - SUCCESS/EFAILED
 *  ===========================================================================*/
int  upload_dbl_image (struct com_state *m_comm, char *dload_filename, int max_write_size)
{
    unsigned long bytes_remaining = 0;
    unsigned long bytes_written = 0;
    unsigned short write_size = 0;
    const unsigned char WRITE_CMD32 = 15;       /*Command code for DMSS write command */
    int write_cmd_hdr_length = 7;               /*length of the header that each write packet*/

    int fd  = open_file(dload_filename);
    if(fd <  0)
    {
       dbg (ERROR, "input file not opened successfully");
       return EFAILED;
    }
    const unsigned long image_size = get_file_size (fd);
    dbg (INFO, "ImageSize: %ld bytes", image_size);

    unsigned long exec_address;
    if (old_pbl) {
        /*Get the Execution address from the image header */
        struct old_mi_boot_image_header_type *image_hdr = malloc (sizeof(struct old_mi_boot_image_header_type));
        if (NULL==image_hdr) {
            dbg (ERROR, "Memory allocation failure: %s", strerror(errno));
            return EFAILED;
        }
        malloc_count++;
        if(read(fd, image_hdr, sizeof(struct old_mi_boot_image_header_type)) < 0)
        {
            dbg (ERROR, "file read operation failed: Linux system error code: %s", strerror(errno));
            if (NULL!=image_hdr) {
                free (image_hdr);
                image_hdr = NULL;
                free_count++;
            }
            return EFAILED;
        }
        exec_address = image_hdr->image_dest_ptr - (sizeof(struct old_mi_boot_image_header_type));
        if (NULL!=image_hdr) {
            free (image_hdr);
            image_hdr = NULL;
            free_count++;
        }
    }
    else {
        /*Get the Execution address from the image header */
        struct mi_boot_image_header_type *image_hdr = malloc (sizeof(struct mi_boot_image_header_type));
        if (NULL==image_hdr) {
            dbg (ERROR, "Memory allocation failure: %s", strerror(errno));
            return EFAILED;
        }
        malloc_count++;

        if (read(fd, image_hdr, sizeof(struct mi_boot_image_header_type)) < 0)
        {
            dbg (ERROR, "file read operation failed: Linux system error code: %s", strerror(errno));
            if (NULL!=image_hdr) {
                free (image_hdr);
                image_hdr = NULL;
                free_count++;
            }
            return EFAILED;
        }

        exec_address = image_hdr->image_dest_ptr - (sizeof(struct mi_boot_image_header_type));
        if (NULL!=image_hdr) {
            free (image_hdr);
            image_hdr = NULL;
            free_count++;
        }
    }

    dbg (INFO, "ExecAddress: 0x%lx", exec_address);

    /* Convert to HDLC format */
    char *encoded_buffer = malloc ((write_cmd_hdr_length + WRITE_SIZE_THRESHOLD * 2 + 4) * sizeof(char));
    if (NULL==encoded_buffer) {
        dbg (ERROR, "Memory allocation failure: %s", strerror(errno));
        return EFAILED;
    }
    malloc_count++;

    unsigned long current_address = exec_address;
    char packet[MAX_DMPACKET] = { 0 };
    int rc;
    struct timeval timeout;
    fd_set readfs;

    /* Write the image data to the device's RAM, one record at a time */
    do {
        bytes_remaining = image_size - bytes_written;
        write_size = (unsigned short)(__min (WRITE_SIZE_THRESHOLD, bytes_remaining));
        if (write_cmd_hdr_length + write_size > sizeof(packet))
            goto clean_and_exit;

        /*Send the write command*/
        packet[0] = WRITE_CMD32;
        packet[1] = (char)(current_address >> 24);         /* Address of write MSB first */
        packet[2] = (char)(current_address >> 16) & 0xFF;
        packet[3] = (char)(current_address >> 8) & 0xFF;
        packet[4] = (char)(current_address & 0xFF);
        packet[5] = (char)(write_size >> 8);         /* Size of write MSB first */
        packet[6] = (char)(write_size & 0xFF);
        lseek(fd, bytes_written, SEEK_SET);
        if (read(fd, &packet[7], write_size) < 0) {
            dbg (ERROR, "Device read descriptor returned error: %s", strerror (errno));
            goto clean_and_exit;
        }

        /*Encode the data*/
        int size = hdlc_encode (packet, write_cmd_hdr_length + write_size, encoded_buffer);
        if (size == 0) {
            dbg (ERROR, "HDLC encoding not successfully made");
            goto clean_and_exit;
        }

        /*Increment current address and bytes written*/
        current_address += write_size;
        bytes_written   += write_size;

        /*Transfer the encoded Data*/
        if (SUCCESS != tx_data (encoded_buffer, size, m_comm)) {
            dbg (ERROR, "Not sent, write error");
            goto clean_and_exit;
        }

        FD_ZERO(&readfs);
        FD_SET (m_comm->port_fd, &readfs);
        timeout.tv_usec = 0;        /* microseconds */
        timeout.tv_sec = 10;         /* seconds */
        rc = select (m_comm->port_fd + 1, &readfs, NULL, NULL, &timeout);
        if (rc < 0) {
            dbg (ERROR, "Select on file descriptor returned failure %s", strerror (errno));
            goto clean_and_exit;
        }

        if (rc == 0) {
            dbg (ERROR, "No data received from the target - timeout occured");
            goto clean_and_exit;
        }
        /*Wait for the acknowledgement from the target*/
        if (read (m_comm->port_fd, m_comm->recv_buffer, RX_BUFFER_SIZE) < 0) {
            dbg (ERROR, "Rx from device failed with error: %s", strerror (errno));
            goto clean_and_exit;
        }

        /*Check if the received payload contains the acknowledgement*/
        if (NULL != m_comm->recv_buffer) {
            if (0x02 != m_comm->recv_buffer[1]) {
                dbg (ERROR, "ACK not received, received instead : %x", m_comm->recv_buffer[1]);
                goto clean_and_exit;
            }
        }
    } while ((bytes_written < image_size));

    /*Free the buffer used for hdlc encoding*/
    if (NULL != encoded_buffer) {
        free (encoded_buffer);
        encoded_buffer = NULL;
        free_count++;
    }
    dbg (INFO, "Total Bytes Uploaded: %ld", bytes_written);

    /*Execute the uploaded dbl image*/
    if (SUCCESS != dload_exec (m_comm, exec_address)) {
        dbg (ERROR, "Execution of dbl image failed");
        goto clean_and_exit;
    }
    return SUCCESS;

clean_and_exit:
    if (NULL != encoded_buffer) {
        free (encoded_buffer);
        encoded_buffer = NULL;
        free_count++;
    }
    return EFAILED;
}
/*===========================================================================
 *  METHOD:
 *  dload_exec
 *
 *  DESCRIPTION:
 *  Construct object/load resource based file into memory
 *
 *  PARAMETERS
 *  m_comm         [ I ] - Pointer to the comm port object
 *  address        [ I ] - Destination address to jump to
 *
 *  RETURN VALUE:
 *  int                  SUCCESS/EFAILED
 *  ===========================================================================*/
int dload_exec (struct com_state *port, unsigned long address)
{
    int go_cmd_length = 5;      /*size of the DLOAD go command*/
    char packet[go_cmd_length];

    memset (packet, 0, go_cmd_length);

    const unsigned short segment = address >> 16;
    const unsigned short offset = address & 0x0000ffff;

    packet[0] = 0x05;                   /*Go command id*/
    FLOPW (&(packet[1]), segment);      /* Code segment */
    FLOPW (&(packet[3]), offset);       /* Code offset */

    /* Convert to HDLC format */
    char *encoded_buffer = malloc ((go_cmd_length * 2 + 4) * sizeof(char));
    if (NULL==encoded_buffer) {
        dbg (ERROR, "Memory allocation failure: %s", strerror(errno));
        return EFAILED;
    }
    malloc_count++;
    /* Encode buffer */
    int size = hdlc_encode (packet, go_cmd_length, encoded_buffer);
    if (size == 0) {
        dbg (ERROR, "Not encoded");
        if (NULL!=encoded_buffer) {
            free (encoded_buffer);
            encoded_buffer = NULL;
            free_count++;
        }
        return EFAILED;
    }

    /*send the execution command*/
    if (SUCCESS != tx_data (encoded_buffer, size, port)) {
        dbg (ERROR, "Not sent, write error");
        if (NULL!=encoded_buffer) {
            free (encoded_buffer);
            encoded_buffer = NULL;
            free_count++;
        }
        return EFAILED;
    }
    /*free the buffer used for encoding*/
    if (NULL != encoded_buffer) {
        free (encoded_buffer);
        encoded_buffer = NULL;
        free_count++;
    }

    /*Check if the acknowledgement is received*/
    {
        int rc;
        struct timeval timeout;
        fd_set readfs;

        FD_ZERO(&readfs);
        FD_SET (port->port_fd, &readfs);
        timeout.tv_usec = 0;        /* microseconds */
        timeout.tv_sec = 10;         /* seconds */
        rc = select (port->port_fd + 1, &readfs, NULL, NULL, &timeout);
        if (rc < 0) {
            dbg (ERROR, "Select on file descriptor returned failure %s", strerror (errno));
            return EFAILED;
        }

        if (rc == 0) {
            dbg (ERROR, "No data received from the target - timeout occured");
            return EFAILED;
        }

        int count = read (port->port_fd, port->recv_buffer, RX_BUFFER_SIZE);
        if (count < 0) {
            dbg (ERROR, "Device read descriptor returned error: %s", strerror (errno));
            return EFAILED;
        }

        if (0x02 != port->recv_buffer[1]) {
            dbg (ERROR, "NAK received for GO command");
            return EFAILED;
        }
    }
    return SUCCESS;
}
