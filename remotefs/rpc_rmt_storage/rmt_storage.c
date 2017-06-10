/*
 * ----------------------------------------------------------------------------
 *  Copyright (c) 2009-2010 Qualcomm Technologies, Inc.  All Rights Reserved.
 *  Qualcomm Technologies Proprietary and Confidential.
 *  ---------------------------------------------------------------------------
 *
 */

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <pthread.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <private/android_filesystem_config.h>

#include "rmt_storage.h"

#define RMT_STORAGE_CHECK_WORD   0X12345678
#ifdef ANDROID
  #define MMC_BLOCK_DEV_NAME       "/dev/block/mmcblk0"
#else
  #define MMC_BLOCK_DEV_NAME       "/dev/mmcblk0"
#endif

struct rmt_storage_client
{
   int fd;
   uint32_t check_word;
   pthread_t th_id;
   pthread_mutex_t th_mutex;
   pthread_cond_t cond;
   uint32_t xfer_dir;
   struct rmt_storage_iovec_desc xfer_desc[RMT_STORAGE_MAX_IOVEC_XFR_CNT];
   uint32_t xfer_cnt;
   uint32_t driveno;
   int fs_error;
   uint32_t error_code;
   unsigned close;
   uint32_t usr_data;
   uint32_t sid;
   struct rmt_shrd_mem_param *shrd_mem;
};

int rmt_storage_fd;
static struct rmt_storage_client clients[MAX_NUM_CLIENTS];
static struct rmt_shrd_mem_param rmt_shrd_mem[MAX_SHRD_MEM_ENTRIES];
static struct partition_lookup_entry part_lookup_table[] = {
   {0x4A, "/boot/modem_fs1", "", -1},
   {0x4B, "/boot/modem_fs2", "", -1},
   {0x58, "/boot/modem_fsg", "", -1},
   {0x59, "/q6_fs1_parti_id_0x59", "", -1},
   {0x5A, "/q6_fs2_parti_id_0x5A", "", -1},
   {0x5B, "/q6_fsg_parti_id_0x5B", "", -1},
   {0x5D, "ssd", "", -1}
};

/*===========================================================================

FUNCTION: find_partition_entry

DESCRIPTION: Find partition entry in lookup table

DEPENDENCIES: None

RETURN VALUE: Pointer to partiton_lookup_entry if found, otherwise NULL

SIDE EFFECTS: None

============================================================================*/
static struct partition_lookup_entry *find_partition_entry(uint8_t part_type)
{
   unsigned int i;
   for (i = 0;
        i < sizeof(part_lookup_table)/sizeof(struct partition_lookup_entry);
        i++)
            if (part_lookup_table[i].partition_type == part_type)
               return &part_lookup_table[i];
   return NULL;
}

/*===========================================================================

FUNCTION: open_partition

DESCRIPTION: Find file handle for path

DEPENDENCIES: None

RETURN VALUE: File handle if found, -1 otherwise

SIDE EFFECTS: None

============================================================================*/
static int open_partition(char *path)
{
   unsigned int i;

   if (!path)
      return -1;
   for (i = 0;
        i < sizeof(part_lookup_table)/sizeof(struct partition_lookup_entry);
        i++)
            if (!strncmp(part_lookup_table[i].path, path, MAX_PATH_NAME))
               return part_lookup_table[i].fd;
   return -1;
}

/*===========================================================================

FUNCTION: is_valid_boot_rec

DESCRIPTION: Validate boot record

DEPENDENCIES: None

RETURN VALUE: 1 if boot record is valid, o otherwise

SIDE EFFECTS: None

============================================================================*/
static int is_valid_boot_rec(const void *boot_rec)
{
   const struct boot_rec *br = boot_rec;

   if (br->sig != BOOT_REC_SIG) {
      LOGE("Invalid boot rec\n");
      return 0;
   }
   return 1;
}

/*===========================================================================

FUNCTION: read_partition_rec

DESCRIPTION: Read boot record and verify signature

DEPENDENCIES: None

RETURN VALUE: 0 if valid boot record is found

SIDE EFFECTS: None

============================================================================*/
static int read_partition_rec(int fd, void *buf, loff_t sector_offset, size_t size)
{
   loff_t ret, offset;
   ssize_t num_bytes;
   int i;

   if (!buf)
      return -1;
   offset = sector_offset * SECTOR_SIZE;
   ret = lseek64(fd, offset, SEEK_SET);
   if (ret < 0 || ret != offset) {
      LOGE("Error seeking 0x%llx bytes in partition. ret=0x%llx errno=%d\n",
           offset, ret, errno);
      return -1;
   }
   num_bytes = read(fd, buf, size);
   if ((num_bytes < 0) || ((size_t)num_bytes != size)) {
      LOGE("Error reading 0x%x bytes < 0x%x\n",
           (unsigned int)num_bytes, size);
      return -1;
   }
   if (!is_valid_boot_rec(buf))
      return -1;
   return 0;
}

/*===========================================================================

FUNCTION: parse_partition

DESCRIPTION: Walk partition table for supported partiton types

DEPENDENCIES: None

RETURN VALUE: Number of supported partitions found

SIDE EFFECTS: None

============================================================================*/
static int parse_partition(const char *path)
{
   struct boot_rec mbr;
   struct partition_lookup_entry *part_entry;
   int fd, i, part_num, part_type, parse_done, ret, parts_found;
   off_t ebr_offset, local_ebr_offset;

   if (!path)
      return -1;
   if (sizeof(mbr) != SECTOR_SIZE) {
      LOGE("MBR struct is not %d bytes\n", SECTOR_SIZE);
      return -1;
   }
   fd = open(path, O_RDONLY);
   if (fd < 0) {
      LOGE("Unable to open %s\n", path);
      return -1;
   }
   ret = read_partition_rec(fd, &mbr, 0, sizeof(mbr));
   if (ret < 0) {
      close(fd);
      return -1;
   }
   for (i = 0; i < MAX_MBR_ENTRIES; i++) {
      if (mbr.part_entry[i].type == EXTENDED_PARTITION_TYPE) {
         part_num = i + 1;
         ebr_offset = mbr.part_entry[i].start_sector;
         break;
      }
   }
   if (i == MAX_MBR_ENTRIES) {
      LOGE("No EBR found\n");
      close(fd);
      return -1;
   }
   parse_done = 0;
   parts_found = 0;
   local_ebr_offset = 0;
   do {
      ret = read_partition_rec(fd, &mbr, ebr_offset + local_ebr_offset,
                               sizeof(mbr));
      if (ret < 0) {
         close(fd);
         return parts_found;
      }
      for (i = 0; i < MAX_EBR_ENTRIES; i++) {
         part_type = mbr.part_entry[i].type;
         if (!part_type) {
            parse_done = 1;
            break;
         }
         if (part_type == EXTENDED_PARTITION_TYPE) {
            local_ebr_offset = mbr.part_entry[i].start_sector;
            break;
         }
         part_num++;
         part_type = mbr.part_entry[i].type;
         part_entry = find_partition_entry(part_type);
         if (part_entry) {
            parts_found++;
            snprintf(part_entry->devpath,
                     MAX_PATH_NAME,
                     "%sp%d", MMC_BLOCK_DEV_NAME, part_num);
	    part_entry->fd = open(part_entry->devpath, O_RDWR|O_SYNC);
	    if (part_entry->fd < 0)
		LOGE("Unable to open %s\n", part_entry->devpath);
            LOGV("Registering p%d: 0x%x %s %s\n", part_num,
                 part_entry->partition_type,
                 part_entry->path,
                 part_entry->devpath);
         }
      }
   } while(!parse_done);

   close(fd);
   return parts_found;
}

/*===========================================================================

FUNCTION: rmt_shrd_mem_param

DESCRIPTION: Get clients shared memory parameters from its storage id.

DEPENDENCIES: None

RETURN VALUE: Pointer to shared memory parameter

SIDE EFFECTS: None

============================================================================*/
static struct rmt_shrd_mem_param *get_shrd_mem(uint32_t sid)
{
   int i;

   for (i = 0; i < MAX_SHRD_MEM_ENTRIES; i++)
      if (rmt_shrd_mem[i].sid == sid) {
         return &rmt_shrd_mem[i];
      }
   return NULL;
}

/*===========================================================================

FUNCTION: init_shrd_mem

DESCRIPTION: Get info on storage id from kernel.

DEPENDENCIES: None

RETURN VALUE: 0 if found

SIDE EFFECTS: None

============================================================================*/
static int init_shrd_mem(uint32_t sid)
{
   int i;

   for (i = 0; i < MAX_SHRD_MEM_ENTRIES; i++) {
        if (!rmt_shrd_mem[i].sid) {
           rmt_shrd_mem[i].sid = sid;
           if (ioctl(rmt_storage_fd,
                     RMT_STORAGE_SHRD_MEM_PARAM,
                     &rmt_shrd_mem[i]) < 0) {
               LOGE("rmt_storage shared memory ioctl failed\n");
               close(rmt_storage_fd);
               break;
           }
           rmt_shrd_mem[i].base = mmap(0, rmt_shrd_mem[i].size,
                            PROT_READ | PROT_WRITE,
                            MAP_SHARED,
                            rmt_storage_fd,
                            rmt_shrd_mem[i].start);
           if (rmt_shrd_mem[i].base == MAP_FAILED) {
              LOGE("mmap failed for sid=0x%08x", sid);
              close(rmt_storage_fd);
              break;
           }
           LOGV("New shrd mem: %d\n", sid);
           LOGV("start=0x%08x\n", rmt_shrd_mem[i].start);
           LOGV("size=0x%08x\n", rmt_shrd_mem[i].size);
           LOGV("base=%p\n", rmt_shrd_mem[i].base);
           return 0;
        }
   }
   return -1;
}


/*===========================================================================

FUNCTION: rmt_storage_send_status

DESCRIPTION: All write events will be deferred to the client's thread and will
be returned immediately. This function sends an ioctl with the status when
the write is done.

DEPENDENCIES: None

RETURN VALUE: None

SIDE EFFECTS: None

============================================================================*/
static void rmt_storage_send_status(struct rmt_storage_client *client)
{
   struct rmt_storage_send_sts args;
   int ret;

   args.err_code = client->error_code;
   args.data = client->usr_data;
   args.handle = (client - clients) + 1;
   args.xfer_dir = client->xfer_dir;

   ret = ioctl(rmt_storage_fd, RMT_STORAGE_SEND_STATUS, &args);
   if (ret < 0)
      LOGE("rmt_storage send status ioctl failed\n");
}

/*===========================================================================

FUNCTION: rmt_storage_init_client

DESCRIPTION: Initializes the client's variables.

DEPENDENCIES: None

RETURN VALUE: None

SIDE EFFECTS: None

============================================================================*/
static void rmt_storage_init_client(struct rmt_storage_client *client)
{
   client->check_word = RMT_STORAGE_CHECK_WORD;
   pthread_mutex_init(&client->th_mutex, NULL);
   pthread_cond_init(&client->cond, NULL);
   client->close = 0;
   client->fd = -1;
}

/*===========================================================================

FUNCTION: rmt_storage_free_client

DESCRIPTION: Unintializes the client's variables.

DEPENDENCIES: None

RETURN VALUE: None

SIDE EFFECTS: None

============================================================================*/
static void rmt_storage_free_client(struct rmt_storage_client *client)
{
   client->check_word = 0;
   pthread_mutex_destroy(&client->th_mutex);
   pthread_cond_destroy(&client->cond);
   client->close = 0;
   client->fd = -1;
}

/*===========================================================================

FUNCTION: rmt_storage_client_thread

DESCRIPTION: The function that gets executed when a pthread is created for
each client. The thread will be blocked upon the events from RPC server. When
an event is received, the thread will be unblocked and it executes the request.

DEPENDENCIES: None

RETURN VALUE: None

SIDE EFFECTS: None

============================================================================*/
static void * rmt_storage_client_thread(void *data)
{
   uint32_t i;
   struct rmt_storage_iovec_desc *xfer;
   ssize_t ret = 0;
   struct rmt_storage_client *client = data;
   char *buf;

   LOGI("rmt_storage client thread started \n");

   pthread_mutex_lock(&client->th_mutex);
   while(1) {
      if (client->close)
         break;

      if(client->xfer_cnt && !client->shrd_mem) {
         LOGE("No shared mem for sid=0x%08x\n", client->sid);
         break;
      }

      for(i=0; i<client->xfer_cnt; i++) {
         xfer = &client->xfer_desc[i];
         lseek(client->fd, xfer->sector_addr * (off_t)RAMFS_BLOCK_SIZE, SEEK_SET);
         buf = (char *) ((uint32_t) client->shrd_mem->base + (xfer->data_phy_addr - client->shrd_mem->start));

         if(client->xfer_dir == RMT_STORAGE_WRITE)
            ret = write(client->fd, buf, xfer->num_sector * 512);
         else
            ret = read(client->fd, buf, xfer->num_sector * 512);

         if ((int) ret < 0) {
            LOGE("rmt_storage fop(%d) failed with error = %d \n", \
                 client->xfer_dir, (int) ret);
            break;
         }
         LOGI("rmt_storage fop(%d): bytes transferred = %d\n", \
              client->xfer_dir, (int) ret);
      }
      if (client->xfer_cnt) {
         client->error_code = ((int) ret > 0) ? 0 : ret;
         rmt_storage_send_status(client);
         client->xfer_cnt = 0;
      }
      /* Wait for subsequent events and process them */
      pthread_cond_wait(&client->cond, &client->th_mutex);
      LOGI("unblock rmt_storage client thread\n");
   }
   pthread_mutex_unlock(&client->th_mutex);
   /* free client structure */
   rmt_storage_free_client(client);
   LOGI("rmt_storage client thread exited\n");

   return NULL;
}

/*===========================================================================

FUNCTION: rmt_storage_process_event

DESCRIPTION: Processes the events open, close, write and register callback.
A client thread will be created for each client's open request. Signal the
thread when any event on this client is available.

DEPENDENCIES: None

RETURN VALUE: None

SIDE EFFECTS: None

============================================================================*/
static void rmt_storage_process_event(struct rmt_storage_event *event)
{
   struct rmt_storage_client *client;
   struct rmt_shrd_mem_param *shrd_mem;
   uint32_t result = RMT_STORAGE_NO_ERROR;
   int i, ret;

   switch(event->id) {
     case RMT_STORAGE_OPEN: {
        LOGI("rmt_storage open event: handle=%d\n", event->handle);

        client = &clients[event->handle - 1];
        client->sid = event->sid;

        rmt_storage_init_client(client);
	if (client->fd >= 0) {
		LOGV("%s already opened\n", event->path);
		break;
	}
	client->fd = open_partition(event->path);

        if (client->fd < 0) {
           LOGE("Unable to open %s\n", event->path);
           rmt_storage_free_client(client);
           break;
        }
        LOGI("Opened %s\n", event->path);
        client->shrd_mem = NULL;
        ret = pthread_create(&client->th_id, NULL, rmt_storage_client_thread, (void *)client);
        if (ret) {
           LOGE("Unable to create a pthread\n");
           close(client->fd);
           rmt_storage_free_client(client);
           break;
        }
        break;
     }

     case RMT_STORAGE_CLOSE: {
        LOGI("rmt_storage close event\n");
        client = &clients[event->handle - 1];

        if (client->check_word != RMT_STORAGE_CHECK_WORD) {
           LOGE("Invalid rmt_storage client\n");
           break;
        }

        pthread_mutex_lock(&client->th_mutex);
        client->close = 1;
        pthread_cond_signal(&client->cond);
        pthread_mutex_unlock(&client->th_mutex);
        close(client->fd);
        break;
     }

     case RMT_STORAGE_WRITE: /* fall through*/
     case RMT_STORAGE_READ: {
        if(event->id == RMT_STORAGE_WRITE)
           LOGI("rmt_storage write event\n");
        else
           LOGI("rmt_storage read event\n");

        client = &clients[event->handle - 1];

        if (client->check_word != RMT_STORAGE_CHECK_WORD) {
           LOGE("Invalid rmt_storage client\n");
           break;
        }
        /* Shared mem may be allocated at runtime, before a file op,
           after an open */
        client->shrd_mem = get_shrd_mem(client->sid);
	if (!client->shrd_mem) {
           ret = init_shrd_mem(client->sid);
           if (ret)
              break;
           client->shrd_mem = get_shrd_mem(client->sid);
           if (!client->shrd_mem) {
              LOGE("Shared mem entry for 0x%x not found\n", client->sid);
              break;
           }
        }

        pthread_mutex_lock(&client->th_mutex);
        client->xfer_dir = event->id;
        memcpy((void *)client->xfer_desc, (void *)event->xfer_desc,
              event->xfer_cnt * sizeof(struct rmt_storage_iovec_desc));
        client->xfer_cnt = event->xfer_cnt;

        pthread_cond_signal(&client->cond);
        pthread_mutex_unlock(&client->th_mutex);
        break;
     }

     case RMT_STORAGE_SEND_USER_DATA: {
        LOGI("rmt_storage send user data event\n");
        client = &clients[event->handle - 1];

        if (client->check_word != RMT_STORAGE_CHECK_WORD) {
           LOGE("Invalid rmt_storage client\n");
           break;
        }

        client->usr_data = event->usr_data;
        break;
     }

     default:
        LOGI("unable to handle the event\n");
        break;
     }
     return;
}

/*===========================================================================

FUNCTION: main

DESCRIPTION: The function opens the device "/dev/rmt_storage", mmaps the shared
memory region and then makes an ioctl to get the RPC events from the server.

DEPENDENCIES: None

RETURN VALUE: None

SIDE EFFECTS: None

============================================================================*/
int main(int argc, char **argv)
{
   uint32_t result;
   int ret;
   struct rmt_storage_event event;
   char filename[64];
   int oom_fd;
   int err_no;
   int i;

   LOGI("rmt_storage user app start.. ignoring cmd line parameters\n");

   snprintf(filename, sizeof(filename), "/proc/%d/oom_adj", getpid());
   oom_fd = open(filename, O_WRONLY);
   if (oom_fd >= 0) {
      write(oom_fd, "-16", 3);
      close(oom_fd);
   } else {
      LOGE("unable to write into /proc/%d/oom_adj", getpid());
      return -1;
   }

   rmt_storage_fd = open("/dev/rmt_storage", O_RDWR);
   if (rmt_storage_fd < 0) {
      LOGE("Unable to open /dev/rmt_storage\n");
      return -1;
   }
   LOGI("rmt_storage open success\n");

   ret = parse_partition(MMC_BLOCK_DEV_NAME);
   if (ret < 0) {
      LOGE("Error (%d) parsing partitions\n", ret);
      return -1;
   }
   LOGI("%d supported partitions found\n", ret);

   ret = setgid(AID_NOBODY);
   if (ret < 0) {
      LOGE("Error changing gid (%d)\n", ret);
      return -1;
   }

   ret = setuid(AID_NOBODY);
   if (ret < 0) {
      LOGE("Error changing uid (%d)\n", ret);
      return -1;
   }

   while(1) {
      ret = ioctl(rmt_storage_fd, RMT_STORAGE_WAIT_FOR_REQ, &event);
      if (ret < 0) {
         err_no = errno;
         LOGE("rmt_storage wait event ioctl failed errno=%d\n", err_no);
         break;
      }

      rmt_storage_process_event(&event);
      LOGI("rmt_storage events processing done\n");
   }

   close(rmt_storage_fd);
   return 0;
}
