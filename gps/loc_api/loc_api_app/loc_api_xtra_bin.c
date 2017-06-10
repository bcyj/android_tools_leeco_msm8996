/******************************************************************************
  @file:  loc_api_xtra_bin.c
  @brief: loc_api_test XTRA binary data services

  DESCRIPTION

  INITIALIZATION AND SEQUENCING REQUIREMENTS

  -----------------------------------------------------------------------------
  Copyright (c) 2009 Qualcomm Technologies, Inc.
  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
  -----------------------------------------------------------------------------
 ******************************************************************************/

/*=====================================================================
                        EDIT HISTORY FOR MODULE

  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order.

when       who      what, where, why
--------   ---      -------------------------------------------------------
03/17/09   dx       Android version
01/10/09   dx       Initial version, derived from cl's GPS XTRA client

$Id: //source/qcom/qct/modem/gps/cgps/dev/locationmiddleware/locapi/app-linux/source/test/loc_api_test/loc_api_xtra_bin.c#8 $
======================================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>         /* struct sockaddr_in */
#include <netdb.h>              /* struct hostent     */
#include <arpa/inet.h>
#include <sys/time.h>
#include <time.h>
#include <string.h>

#include "loc_api_rpc_glue.h"
#include "loc_api_test.h"
#include "loc_api_data.h"
#include "loc_api_xtra_bin.h"

#define MIN(a,b) ( (a) <= (b) ? (a) : (b) )

/**
 * File downloading data
 */
FILE   *xtra_bin_fp = NULL;

int     gps_xtra_bin_socket_fd = -1;
int     gps_xtra_sntp_socket_fd = -1;
char    gps_xtra_write_buffer[GPS_XTRA_READ_WRITE_BUFFER_SIZE];
int     gps_xtra_write_buffer_offset = 0;

char    gps_xtra_read_buffer[GPS_XTRA_READ_WRITE_BUFFER_SIZE + 1];
char    *gps_xtra_read_buffer_start_ptr;
char    *gps_xtra_read_buffer_end_ptr;

/*===========================================================================

FUNCTION gps_xtra_safe_socket_partial_flush

===========================================================================*/
static int gps_xtra_safe_socket_partial_flush(void)
{
   int i;

   if(!gps_xtra_write_buffer_offset)
   {
      return 0;
   }

   i = write(gps_xtra_bin_socket_fd, gps_xtra_write_buffer, gps_xtra_write_buffer_offset);

   if(i < 0)
   {
      loc_write_error("Socket write failed\n");
      return -1;
   }

   if (i < gps_xtra_write_buffer_offset)
   {
      memmove(gps_xtra_write_buffer, gps_xtra_write_buffer + i, gps_xtra_write_buffer_offset - i);
      gps_xtra_write_buffer_offset -= i;
   }
   else
   {
      gps_xtra_write_buffer_offset = 0;
   }
   return gps_xtra_write_buffer_offset;
}

/*===========================================================================

FUNCTION gps_xtra_safe_socket_write

===========================================================================*/
static void gps_xtra_safe_socket_write(char* str)
{
   int l,i;

   l = strlen(str);

   while(l)
   {
      if(gps_xtra_write_buffer_offset + l < GPS_XTRA_READ_WRITE_BUFFER_SIZE)
      {
         /* str will fit in wbuf */
         memcpy(&(gps_xtra_write_buffer[gps_xtra_write_buffer_offset]), str, l);
         gps_xtra_write_buffer_offset += l;
         l = 0;
      }
      else if(gps_xtra_write_buffer_offset)
      {
         /* wbuf is not empty and str can fill it */
         if (gps_xtra_write_buffer_offset < GPS_XTRA_READ_WRITE_BUFFER_SIZE)
         { 
            /* if wbuf not full, fill it */
            i = GPS_XTRA_READ_WRITE_BUFFER_SIZE - gps_xtra_write_buffer_offset;     
            memcpy(&(gps_xtra_write_buffer[gps_xtra_write_buffer_offset]), str, i);
            str += i;
            gps_xtra_write_buffer_offset+=i;
            l -= i;
         }
         gps_xtra_safe_socket_partial_flush();
      }
      else
      {
         /* str is too big and wbuf is empty--write in place */
         while (l > GPS_XTRA_READ_WRITE_BUFFER_SIZE)
         {
            /* write as much as possible */
            i = write(gps_xtra_bin_socket_fd, str, l);      
            if(i < 0)
            {
               loc_write_error("Socket write error\n");
               return;
            }
            str += i;
            l -= i;
         }
      }
   }
}

/*===========================================================================

FUNCTION gps_xtra_safe_socket_flush

===========================================================================*/
static void gps_xtra_safe_socket_flush(void)
{
   while(gps_xtra_safe_socket_partial_flush());
}

/*===========================================================================

FUNCTION gps_xtra_get_recv_len

===========================================================================*/
static int gps_xtra_get_recv_len(void)
{
   int i = 0; /* data length in the buffer */

   if(gps_xtra_read_buffer_end_ptr <= gps_xtra_read_buffer_start_ptr)
   {
      gps_xtra_read_buffer_end_ptr = gps_xtra_read_buffer_start_ptr = gps_xtra_read_buffer;
   }
   else
   {
      i = gps_xtra_read_buffer_end_ptr - gps_xtra_read_buffer_start_ptr;

      memmove(gps_xtra_read_buffer, 
            gps_xtra_read_buffer_start_ptr, 
            i);

      gps_xtra_read_buffer_start_ptr = gps_xtra_read_buffer;
      gps_xtra_read_buffer_end_ptr = gps_xtra_read_buffer + i;
   }
   return i;
}

/*===========================================================================

FUNCTION gps_xtra_safe_socket_read_line

===========================================================================*/
static char *gps_xtra_safe_socket_read_line(void)
{
   char *s,*r;
   int i;

   while(1)
   {
      if(gps_xtra_read_buffer_start_ptr < gps_xtra_read_buffer_end_ptr)
      {
         /* gps_xtra_read_buffer not empty */
         for(s = gps_xtra_read_buffer_start_ptr; s < gps_xtra_read_buffer_end_ptr; s++)
         {
            if (*s=='\r' || *s=='\n')
            {
               break;
            }
         }

         if(*s == '\n' || (*s == '\r' && s < gps_xtra_read_buffer_end_ptr - 1))
         {
            /* found a complete line */
            r = gps_xtra_read_buffer_start_ptr;

            if(*s == '\n')
            {
               gps_xtra_read_buffer_start_ptr = s + 1;
               *s = 0;
            } 
            else if(s[1] == '\n')
            {
               *s = s[1] = 0;
               gps_xtra_read_buffer_start_ptr = s + 2;
            }
            return r;
         }
         /* else, there isn't a complete line in the buffer */
         if (gps_xtra_read_buffer_start_ptr == gps_xtra_read_buffer && gps_xtra_read_buffer_end_ptr == gps_xtra_read_buffer + GPS_XTRA_READ_WRITE_BUFFER_SIZE)
         {
            /* buffer is full, and no complete line */
            /* this line is too long--pretend it is complete */
            gps_xtra_read_buffer[GPS_XTRA_READ_WRITE_BUFFER_SIZE] = 0;                /* mark the end */
            gps_xtra_read_buffer_start_ptr = gps_xtra_read_buffer_end_ptr = gps_xtra_read_buffer;     /* mark as emtpy */
            return gps_xtra_read_buffer;              /* return the whole thing */
         }
      }
      else
      {
         /* buffer is empty */
         gps_xtra_read_buffer_start_ptr = gps_xtra_read_buffer_end_ptr = gps_xtra_read_buffer;
      }

      if (gps_xtra_read_buffer_start_ptr != gps_xtra_read_buffer)
      {
         /* move buffer to front */
         i = gps_xtra_read_buffer_end_ptr - gps_xtra_read_buffer_start_ptr;

         memmove(gps_xtra_read_buffer,
               gps_xtra_read_buffer_start_ptr,
               i);

         gps_xtra_read_buffer_start_ptr = gps_xtra_read_buffer;
         gps_xtra_read_buffer_end_ptr = gps_xtra_read_buffer + i;
      }

      i = read(gps_xtra_bin_socket_fd,
            gps_xtra_read_buffer_end_ptr,
            GPS_XTRA_READ_WRITE_BUFFER_SIZE - (gps_xtra_read_buffer - gps_xtra_read_buffer_end_ptr)); /* fill the buffer */

      if(i < 0)
      {
         loc_write_error("Socket read error\n");
         return NULL;
      }

      gps_xtra_read_buffer_end_ptr += i;

      if(i == 0)
      {                   /* nothing read--socket dead? */
         if(gps_xtra_read_buffer_end_ptr >= gps_xtra_read_buffer + GPS_XTRA_READ_WRITE_BUFFER_SIZE)
         {
            /* mark end of buffer */
            gps_xtra_read_buffer[GPS_XTRA_READ_WRITE_BUFFER_SIZE] = 0;
         }
         else
         {
            *gps_xtra_read_buffer_end_ptr = 0;
         }

         if (gps_xtra_read_buffer_end_ptr > gps_xtra_read_buffer_start_ptr)
         {
            /* return what we have */
            gps_xtra_read_buffer_end_ptr = gps_xtra_read_buffer_start_ptr;
            return gps_xtra_read_buffer_start_ptr;
         }
         else
         {
            return 0;               /* return NULL */
         }
      }
   }
}


/*===========================================================================

FUNCTION resolve_in_addr

DESCRIPTION
   Translates a hostname to in_addr struct
   
DEPENDENCIES
   n/a
   
RETURN VALUE
   TRUE if successful
   
SIDE EFFECTS
   n/a
   
===========================================================================*/
boolean resolve_in_addr(char *host_addr, struct in_addr *in_addr_ptr)
{
   struct hostent             *hp;   
   hp = gethostbyname(host_addr);   
   if (hp != NULL) /* DNS OK */
   {
      memcpy(in_addr_ptr, hp->h_addr_list[0], hp->h_length);          
   }
   else
   {
      /* Try IP representation */
      if (inet_aton(host_addr, in_addr_ptr) == 0) 
      {
         /* IP not valid */
         loc_write_error("DNS query on '%s' failed\n", host_addr);
         return FALSE;
      }
   }

   return TRUE;
}

/*===========================================================================

FUNCTION loc_test_xtra_download_bin

DESCRIPTION
   Download and opens the XTRA data file
    
DEPENDENCIES

RETURN VALUE
   File pointer to read the downloaded data
   
SIDE EFFECTS
   The downloaded file is saved to disk
   
===========================================================================*/
FILE* loc_xtra_download_bin(int *filesize)
{
   struct sockaddr_in peer_addr;
   char   *str;
   int    file_size = 0, recv_size;
   int    read_len, j;

   /* Get data file pointer */
   xtra_bin_fp = LOC_TEST_XTRA_USE_TMPFILE ? tmpfile() : 
      fopen(GPS_XTRA_FILENAME, "wb+");
   
   if (!xtra_bin_fp)
   {
      loc_write_error("Failed to write to " GPS_XTRA_FILENAME " file.\n");
      return NULL;
   }   

   /* Set up buffers */
   gps_xtra_write_buffer_offset   = 0;
   gps_xtra_read_buffer_start_ptr = gps_xtra_read_buffer;
   gps_xtra_read_buffer_end_ptr   = gps_xtra_read_buffer;
   memset((void *)gps_xtra_read_buffer,
         0,
         GPS_XTRA_READ_WRITE_BUFFER_SIZE + 1);
   recv_size = 0;

   /* DNS query */
   loc_write_msg("Resolving hostname for XTRA server...\n");
   
   /* Resolve internet address */
   memset(&peer_addr,0, sizeof(peer_addr));
   peer_addr.sin_family = AF_INET;
   peer_addr.sin_port = htons(GPS_XTRA_FILE_SERVER_PORT);
   if (!resolve_in_addr(param.xtra_server_addr, &peer_addr.sin_addr))
   {
      fclose(xtra_bin_fp);
      return NULL; /* failed to resolve hostname */
   }
   
   /* Open the socket */
   gps_xtra_bin_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
   if(gps_xtra_bin_socket_fd < 0)
   {
      loc_write_error("Socket failed\n");
      fclose(xtra_bin_fp);
      return NULL;
   }
   
   /* Connect to XTRA server */
   loc_write_msg("Connecting to XTRA server\n");
   if(connect(gps_xtra_bin_socket_fd, (struct sockaddr*)&peer_addr, sizeof(peer_addr)) < 0)
   {
      loc_write_error("Connection failed\n");
      fclose(xtra_bin_fp);
      return NULL;
   }

   /* Write download request to socket */ 
   gps_xtra_safe_socket_write("GET /");
   gps_xtra_safe_socket_write(GPS_XTRA_FILENAME);
   gps_xtra_safe_socket_write(" HTTP/1.0\nAccept: */*\r\n");
   gps_xtra_safe_socket_write("\r\n");
   gps_xtra_safe_socket_flush();
   
   loc_write_msg("%s\n", gps_xtra_write_buffer);

   /* Read server response */
   str = gps_xtra_safe_socket_read_line();
   if (!str || strncmp(str ,"HTTP/", 5))
   {
      loc_write_error("HTTP server returns no header.\n");
      fclose(xtra_bin_fp);
      return NULL;
   }
   
   /* Process response */
   while ((str = gps_xtra_safe_socket_read_line()) && *str)
   {
      if (!strncasecmp(str, "Content-Length:", 15))
      {
         file_size = atoi(str + 15);
      }
   }

   /* Read XTRA file */ 
   read_len = gps_xtra_get_recv_len();

   do 
   {
      recv_size += read_len;

      if (file_size)
      {
         loc_write_msg("% 2.1f%% %d/%d\n",100.0 * recv_size / file_size, recv_size, file_size);
      }

      if (read_len)
      {
         j = fwrite(gps_xtra_read_buffer, 1, read_len, xtra_bin_fp);

         if(j != read_len)
         {
            loc_write_error("read!=write (%d!=%d)\n",read_len,j);
         }
      }

      read_len = read(gps_xtra_bin_socket_fd, gps_xtra_read_buffer, GPS_XTRA_READ_WRITE_BUFFER_SIZE);

   } while(read_len > 0);

   /* close socket */
   shutdown(gps_xtra_bin_socket_fd, 2);
   close(gps_xtra_bin_socket_fd);
   
   loc_write_msg("XTRA file downloaded, size %d.\n", file_size);

   /* rewind file for reading */
   if (xtra_bin_fp)
   {
      fflush(xtra_bin_fp);
      fseek(xtra_bin_fp, 0L, SEEK_SET);
   }
   *filesize = file_size;
   
   return xtra_bin_fp; /* successful */
}

/*===========================================================================

FUNCTION loc_xtra_inject_bin

DESCRIPTION
   Injects XTRA data into GPS modem by parts. This function does not close
   xtra_fp.
   
DEPENDENCIES
   loc_open                        - call first
   tmode.xtra_inject_incomplete    - changes injection behaviors
   tmode.xtra_inject_out_of_order  
   
RETURN VALUE
   0 if successful, otherwise LOC_API return code
   
SIDE EFFECTS
   Calls loc_ioctl which can generate callback. 
   
===========================================================================*/
int loc_xtra_inject_bin(FILE* xtra_fp, int file_size)
{
   if (file_size == 0)
   {
      return FALSE; /* no data to inject */
   }
   
   int part_size      = GPS_XTRA_INJECT_PART_SIZE;
   int total_parts      = (file_size - 1) / part_size + 1;
   int remain_size    = file_size;
   int i, rc;
   
   loc_write_msg("Begin to inject XTRA file to modem (size %d)...\n", file_size);
   
   rpc_loc_ioctl_data_u_type  data;                         /* ioctl data */
   rpc_loc_predicted_orbits_data_s_type *inject_info      /* xtra injection data */
         = &data.rpc_loc_ioctl_data_u_type_u.predicted_orbits_data;      
   /* Allocate memory for data reading */
   unsigned char* data_buf = (unsigned char*) malloc(part_size);
   
   if (data_buf == NULL)
   {
      loc_write_error("XTRA data: out of memory.\n");
      return FALSE; /* out of memory */
   }
   
   /* Set up inject info */
   inject_info->total_size = file_size;
   inject_info->data_ptr.data_ptr_len = 0; /* to be filled below */
   inject_info->data_ptr.data_ptr_val = (char*) data_buf;
   inject_info->total_parts = total_parts;   
   
   for (i = 0; i < inject_info->total_parts; i++)
   {
      int data_size = MIN(part_size, remain_size);      
      int actual_data_size = fread((void*) data_buf, 1, data_size, xtra_fp);
      if (data_size != actual_data_size)
      {
         loc_write_error("XTRA data file read error.\n");
         return FALSE;
      }
      
      /* Inject a part */
      inject_info -> part_len = actual_data_size;
      inject_info -> part = i + 1;  /* Part 1 is the first part */
      inject_info -> data_ptr.data_ptr_len = inject_info -> part_len; 
      
      /* EC: tests out of order injection; should be rejected */
      if (tmode.xtra_inject_out_of_order)
      {
         /* Note the data order is not swapped; just tests whether the server can 
            reject the request */ 
         if (inject_info->part == 2)
         { 
            loc_write_msg("EC: out-of-order injection\n");
            inject_info->part = 3; 
         }
         else if (inject_info->part == 3)
         { 
            loc_write_msg("EC: out-of-order injection\n");
            inject_info->part = 2; 
         }
      }
      
      loc_write_msg("Injecting part %d/%d (%d bytes remain)\n", inject_info -> part, 
            total_parts, remain_size);      
      if ((rc = loc_ioctl(sys.loc_handle, RPC_LOC_IOCTL_INJECT_PREDICTED_ORBITS_DATA, &data)) 
            != RPC_LOC_API_SUCCESS)
      {
         loc_write_error("XTRA data injection failed.\n");
         return rc;  /* error, abort injection */
      }
      
      /* Deduct remaining size */
      remain_size -= actual_data_size;
      
      /* EC: Aborts the injection if testing xtra_inject_incomplete */
      if (tmode.xtra_inject_incomplete)
      {
         loc_write_msg("EC: incomplete injection\n");         
         break;
      }
   }
   
   free(data_buf);
   
   return 0; /* successful */
}
