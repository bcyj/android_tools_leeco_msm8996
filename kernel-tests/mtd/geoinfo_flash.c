/*********************************File Header************************************
 * File Name             : geoinfo_flash.c
 * Brief Description     : Geometric Information of Flash 
 * Sub System Test       : MTD Driver
 * Pre Requisite         : Char Device Nodes like /dev/mtd0 or /dev/mtd1... 
 * Date                  :
 *******************************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <string.h>
#include <mtd/mtd-user.h>

#define ERR_VAL		-1

 int main(int argc, char* argv[])
{
	struct mtd_info_user info_user;
	int fd = ERR_VAL, ret_val = ERR_VAL;

	if(argc < 2)
	{
		printf("Usage Programe <device> for e.q. /dev/mtd0..\n");
		goto END;
	}
	
	memset(&info_user, 0, sizeof(info_user));
	fd = open(argv[1], O_RDWR);
	if(fd == ERR_VAL)
	{
		printf("Failed To Open Device %s\n", argv[1]);
		goto END;
	}
	
    ret_val = ioctl(fd, MEMGETINFO, &info_user);
	if(ret_val == ERR_VAL)
	{
		printf("IOCTL mtd_ioctl Failure\n");
		goto CLOSE;
	}


	printf(" Type 		: 0x%x\n", info_user.type);
	printf(" Flags		: %d\n", info_user.flags);
	printf(" Size 		: %d\n", info_user.size);
	printf(" Erase Size	: %d\n", info_user.erasesize);
	printf(" Write Size	: %d\n", info_user.writesize);
	printf(" OOB Size	: %d\n", info_user.oobsize);


CLOSE:	close(fd);	
END:	return 0;
}
