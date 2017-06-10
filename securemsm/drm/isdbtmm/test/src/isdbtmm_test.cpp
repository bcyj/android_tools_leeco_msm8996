/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

                          ISDB-Tmm Test Application

GENERAL DESCRIPTION
  Test ISDB-Tmm BKM functions.

EXTERNALIZED FUNCTIONS
  None

INITIALIZATION AND SEQUENCING REQUIREMENTS

Copyright (c) 2012 Qualcomm Technologies, Inc.
All Rights Reserved.
Qualcomm Technologies Confidential and Proprietary

*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/*===========================================================================

                            EDIT HISTORY FOR MODULE

$Header:

when       who    what, where, why
--------   ---    ----------------------------------------------------------
11/27/2013  rz    Handling deprecated mkdir() and rmdir() in SFS
03/07/2013  gs	  Switched to gtest
08/27/2012  sz    Added multi-threaded test
07/15/2012  ib    Re-enabled provocative test cases, added fd_hashing tests
06/20/2012  ib    Remove provocative test cases which results in system crash
02/14/2012  ib    Porting to QSEE
01/16/2012  ib    Created
===========================================================================*/

/** adb log */
#define LOG_TAG "IsdbtmmTest"
#define LOG_NDDEBUG 0 //Define to enable LOGD
#define LOG_NDEBUG  0 //Define to enable LOGV

#include <unistd.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <utils/Log.h>
#include <dlfcn.h>
#include <QSEEComAPI.h>
#include <pthread.h>

#include "isdbtmm_entry.h"
#include "tzcommon_entry.h"
#include "isdbtmm_clnt.h"

#include "gtest/gtest.h"
#include <errno.h>

using namespace std;

static const char *FILESYSTEM_LIB = "libdrmfs.so";
//static const char *TIMESYSTEM_LIB = "libdrmtime.so";

static struct QSEECom_handle *g_QSEEComHandle = NULL;
static void *g_FSHandle                       = NULL;
static void *g_TIMEHandle                     = NULL;

void *isdbt_single_thread_test( void *ptr )
{
	long ret;
	int fd=0;
	static uint8 data_write[20] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20};
	static uint8 data_read[20];
	uint8 *u8Arg = (uint8 *)ptr;
	uint8 retVal=0;  // 0 = no error, unless proven otherwise

	do {

		fd = BKMCL_Open(u8Arg, O_TRUNC | O_CREAT | O_RDWR );

		if(fd == -1 )
		{
			printf("BKMCL_Open file %s failed fd=%d\n", u8Arg, fd);
			retVal = 1;
			break;
		}

		ret = BKMCL_Write(fd, data_write, 20);

		if (ret != 20)
		{
			printf("BKMCL_Write failed to write to the file %s ret=%ld\n", u8Arg, ret);
			retVal = 1;
			break;
		}

		ret = BKMCL_Seek(fd, 0, SEEK_SET);

		if (ret == -1)
		{
			printf ("BKMCL_Seek failed to seek to file %s, ret=%ld\n", u8Arg, ret);
			retVal = 1;
			break;
		}


		ret = BKMCL_Read(fd, data_read, 20);

		ret = memcmp (data_read, data_write, 20);
		if (ret != 0)
		{
			printf ("BKMCL_Read failed in memcmp of file %s\n",u8Arg);
			retVal = 1;
			break;
		}


	} while(0);

	BKMCL_Close(fd);
	BKMCL_Remove(u8Arg);

	*u8Arg = retVal;
	return ptr;
}

#define ISDBT_MAX_INPUT_LEN    20
#define STR_LEN(str)    (sizeof((str)) - 1)

/*===========================================================================*/
/* Main Function. Calls the requested test suite.                            */
/*===========================================================================*/

	#define ETEST_BKMCL_VALID_DIR      "test_dir"
	#define ETEST_BKMCL_VALID_DIR2     "test_dir2"
	#define ETEST_BKMCL_WRONG_DIR1     "./test_dir"
	#define ETEST_BKMCL_WRONG_DIR2     "../test_dir"
	#define ETEST_BKMCL_UNEXISTING_DIR "wrong_test_dir"
	#define ETEST_BKMCL_TOO_LONG_DIR "this_is_a_too_long_directory_name_for_a_folder_sdjkfhgsjdfhgskjfhgfuhgejfhdjfkaskajhdkajsdhkajshkajsdhakjsdhaksjdhaksjdhaksdjhaksdjhaksdjhaksjdhaksdjhaksdjhakdjhakdjhaksdjhaksdjhaksjdhakjsdhaksjdhaksdjhaksjdhaksjdhaksjdhakjsdhakjsdhakjsdhakjsdhaksjdhakjdhaksjdhaksdjhaksdjhakdjhakdjhakdjhakdjhaskdjha"
	#define ETEST_BKMCL_NO_NAME_DIR            ""

	#define ETEST_BKMCL_FILE            "bkm_file1.dat"
	#define ETEST_BKMCL_FILE2           "bkm_file2.dat"
	#define ETEST_BKMCL_INVALID_FILE1     "./bkm_file1.dat"
	#define ETEST_BKMCL_INVALID_FILE2     "../bkm_file1.dat"
	#define ETEST_BKMCL_UNEXISTING_FILE "abra_cadabra.dat"
	#define ETEST_BKMCL_TOO_LONG_FILE "this_is_a_too_long_file_name_for_a_file_sdjkfhgsjdfhgskjfhgfuhgejfhdjfkaskajhdkajsdhkajshkajsdhakjsdhaksjdhaksjdhaksdjhaksdjhaksdjhaksjdhaksdjhaksdjhakdjhakdjhaksdjhaksdjhaksjdhakjsdhaksjdhaksdjhaksjdhaksjdhaksjdhakjsdhakjsdhakjsdhakjsdhaksjdhakjdhaksjdhaksdjhaksdjhakdjhakdjhakdjhakdjhaskdjha"
	#define ETEST_BKMCL_NO_NAME_FILE            ""
	#define ETEST_BKMCL_NUM_THREADS  (10)
	#define ETEST_BKMCL_FILE_NAME_LEN (5)
	#define BKMCL_FAILURE     -1
  #define BKMCL_DEPRECATED  -2

	long   ret;
	int fd=0;
	int fd2=0;
	int cnt=-1;
	int i;
	static uint8 data_write[20] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20};
	static uint8 data_read[50];
	char fullName[256];
	uint32 size;
	int error=0;

//
//    Initialize BKM
//

TEST(BKM, Initialize) {

	ASSERT_NE(BKMCL_Initialize(), BKMCL_FAILURE) << "BKMCL can't be initialized";
}

//
//    Multi-threaded test
//

TEST(BKM, MultiThread) {

	pthread_t thread_arr[ETEST_BKMCL_NUM_THREADS]; //array of 20 thread descriptors
	char param_arr[ETEST_BKMCL_NUM_THREADS][ETEST_BKMCL_FILE_NAME_LEN];
	int i;
	int numErr=0;

	//Start all threads
	for (i=0; i<ETEST_BKMCL_NUM_THREADS; i++)
	{
		int retVal;
		sprintf(param_arr[i],"%d",i);
		retVal = pthread_create( &(thread_arr[i]), NULL, isdbt_single_thread_test, (void*) param_arr[i]);

		EXPECT_EQ(retVal, 0) << "Error in pthread_create(), i=" << i;

	}

	//Join all threads
	for (i=0; i<ETEST_BKMCL_NUM_THREADS; i++)
	{
		pthread_join(thread_arr[i], NULL);
		if (param_arr[i][0] != 0)
		{
			numErr++;
		}
	}

	EXPECT_EQ(numErr, 0) << "Multi-threaded test FAILED";

}

//
//    MkDir valid dir test
//

TEST(BKM, MKDirValid) {
	int retVal = BKMCL_MkDir((uint8*)ETEST_BKMCL_VALID_DIR);
	EXPECT_TRUE(retVal >= 0 || BKMCL_DEPRECATED == retVal) << "BKMCL_MkDir valid dir test failed";
}

//
//    MkDir re-create valid dir test
//

TEST(BKM, MKDirReCreateValid) {
  int retVal = BKMCL_MkDir((uint8*)ETEST_BKMCL_VALID_DIR);
  EXPECT_TRUE(retVal >= 0 || BKMCL_DEPRECATED == retVal) << "BKMCL_MkDir valid dir test failed";
	BKMCL_RmDir((uint8*)ETEST_BKMCL_VALID_DIR);
}

//
//    MkDir invalid dir test 1
//

TEST(BKM, MKDirInvalid1) {
	EXPECT_LT(BKMCL_MkDir((uint8*)ETEST_BKMCL_WRONG_DIR1), 0) << "BKMCL_MkDir invalid dir test 1 failed";
}

//
//    MkDir invalid dir test 2
//

TEST(BKM, MKDirInvalid2) {
	EXPECT_LT(BKMCL_MkDir((uint8*)ETEST_BKMCL_WRONG_DIR2), 0) << "BKMCL_MkDir invalid dir test 2 failed";
}

//
//    MkDir with dir name too long
//

TEST(BKM, MKDirNameTooLong) {
	EXPECT_LT(BKMCL_MkDir((uint8*)ETEST_BKMCL_TOO_LONG_DIR), 0) << "BKMCL_MkDir invalid dir name too long failed";
}


//
//    RmDir valid dir test
//

TEST(BKM, RmDirValid) {
	int retVal;
	retVal = BKMCL_MkDir((uint8*)ETEST_BKMCL_VALID_DIR2);
	EXPECT_TRUE(retVal >= 0 || BKMCL_DEPRECATED == retVal) << "BKMCL_RmDir valid dir test failed in creating the dir";
	retVal = BKMCL_RmDir((uint8*)ETEST_BKMCL_VALID_DIR2);
	EXPECT_TRUE(0 == retVal || BKMCL_DEPRECATED == retVal) << "BKMCL_RmDir valid dir test failed";
}


//
//    RmDir invalid dir test 1
//
TEST(BKM, RmDirInvalidDir1) {
	EXPECT_LT(BKMCL_RmDir((uint8*)ETEST_BKMCL_WRONG_DIR1), 0) << "BKMCL_RmDir invalid dir test 1 failed";
}

//
//    RmDir invalid dir test 2
//
TEST(BKM, RmDirInvalidDir2) {
	EXPECT_LT(BKMCL_RmDir((uint8*)ETEST_BKMCL_WRONG_DIR2), 0) << "BKMCL_RmDir invalid dir test 2 failed";
}

//
//    RmDir with dir name too long
//

TEST(BKM, RmDirNameTooLong) {
	EXPECT_LT(BKMCL_RmDir((uint8*)ETEST_BKMCL_TOO_LONG_DIR), 0) << "BKMCL_RmDir invalid dir name too long failed";
}

//
//    RmDir unexisting dir name
//

TEST(BKM, RmDirUnexistingDirName) {
	EXPECT_LT(BKMCL_RmDir((uint8*)ETEST_BKMCL_UNEXISTING_DIR), 0) << "BKMCL_RmDir for unexisting dir name test failed";
}


//
//    Open non existing file test
//

TEST(BKM, OpenUnexistingFile) {
	EXPECT_EQ(BKMCL_Open((uint8*)ETEST_BKMCL_UNEXISTING_FILE, O_RDWR ), -1) << "BKMCL_Open non existing file test failed";
}

//
//    Open invalid file test 1
//

TEST(BKM, OpenInvalidFile1) {
	EXPECT_EQ(BKMCL_Open((uint8*)ETEST_BKMCL_INVALID_FILE1, O_RDWR ), -1) << "BKMCL_Open invalid file test 1 failed";
}

//
//    Open invalid file test 2
//

TEST(BKM, OpenInvalidFile2) {
	EXPECT_EQ(BKMCL_Open((uint8*)ETEST_BKMCL_INVALID_FILE2, O_RDWR ), -1) << "BKMCL_Open invalid file test 2 failed";
}


//
//    Open file with name too long  test
//

TEST(BKM, OpenNameTooLong) {
	EXPECT_EQ(BKMCL_Open((uint8*)ETEST_BKMCL_TOO_LONG_FILE, O_RDWR ), -1) << "BKMCL_Open with name too long test failed";
}

//
//    Open file in root folder test
//

TEST(BKM, OpenInRootFolder) {
	EXPECT_NE(fd = BKMCL_Open((uint8*)ETEST_BKMCL_FILE, O_TRUNC | O_CREAT | O_RDWR ), -1) << "BKMCL_Open file in root folder test failed";
	BKMCL_Close(fd);
}

//
//    Open file in a subfolder test
//

TEST(BKM, OpenInSubfolder) {

	strlcat (fullName, ETEST_BKMCL_VALID_DIR, sizeof(fullName));
	strlcat (fullName, "/", sizeof(fullName));
	strlcat (fullName, ETEST_BKMCL_FILE, sizeof(fullName));

	fd = BKMCL_Open((uint8*)fullName, O_TRUNC | O_CREAT | O_RDWR );
	EXPECT_NE(fd, -1) << "BKMCL_Open file in  a subfolder test failed";
	if(fd != -1) {
		BKMCL_Close(fd);
	}
}

//
//    Open 2 files in a row test
//

TEST(BKM, OpenTwoInARow) {

	fd = BKMCL_Open((uint8*)ETEST_BKMCL_FILE, O_TRUNC | O_CREAT | O_RDWR );
	fd2 = BKMCL_Open((uint8*)ETEST_BKMCL_FILE2, O_TRUNC | O_CREAT | O_RDWR );

	EXPECT_GE(fd+fd2, 0) << "BKMCL_Open 2 files test failed";

	if(fd != -1) {
		EXPECT_EQ(BKMCL_Close(fd),0) << "BKMCL_Open 2 files test failed";
	}
	if(fd2 != -1) {
		EXPECT_EQ(BKMCL_Close(fd2),0) << "BKMCL_Open 2 files test failed";
	}
}

//
//    Close file test
//

TEST(BKM, CloseFile) {

	fd = BKMCL_Open((uint8*)ETEST_BKMCL_FILE, O_TRUNC | O_CREAT | O_RDWR );
	EXPECT_NE(fd, -1) << "In close file test, failed to open the file first";
	if (fd != -1) {
		EXPECT_GE(BKMCL_Close(fd), 0) << "BKMCL_Close test failed";
	}
}

//
//    Close wrong file test
//

TEST(BKM, CloseWrongFile) {
	EXPECT_LT(BKMCL_Close(0x7eadbeef), 0) << "BKMCL_Close file with wrong fd test failed";
}

//
//    Valid file Get Size test
//

TEST(BKM, ValidFileGetSize) {

	fd = BKMCL_Open((uint8*)ETEST_BKMCL_FILE, O_TRUNC | O_CREAT | O_RDWR );
	EXPECT_NE(fd, -1) << "Valid file Get Size test failed to open the file";

	if (fd != -1) {
		ret = BKMCL_Write(fd, data_write, 20);
		EXPECT_EQ(ret, 20) << "Valid file Get Size test failed to write to the file";
		if (ret == 20) {
			ret = BKMCL_GetSize(fd, &size);
			EXPECT_GE(ret, 0) << "BKMCL_GetSize valid file test failed";
			if (ret >= 0) {
				EXPECT_EQ((int)size, 20) << "BKMCL_GetSize valid file test failed, size=	" << size;
			}
		}
		BKMCL_Close(fd);
		BKMCL_Remove((uint8*)ETEST_BKMCL_FILE);
	}
}

//
//    Invalid file fd Get Size test
//

TEST(BKM, InvalidFileGetSize) {
	EXPECT_LT(BKMCL_GetSize(0x2000100, &size), 0) << "BKMCL_GetSize invalid file test failed";
}

//
//    Valid file Seek test - use case SEEK_SET (seek from start pos)
//

TEST(BKM, ValidFileSeekFromStartPos) {

	fd = BKMCL_Open((uint8*)ETEST_BKMCL_FILE, O_TRUNC | O_CREAT | O_RDWR );
	EXPECT_NE(fd, -1) << "Valid file use Seek test use case SEEK_SET failed to open the file";

	if (fd != -1) {
		ret = BKMCL_Write(fd, data_write, 20);
		EXPECT_EQ(ret, 20) << "Valid file Seek test use case SEEK_SET failed to write to the file";
		if (ret == 20) {
			ret = BKMCL_Seek(fd, 5, SEEK_SET);
			EXPECT_NE(ret, -1) << 	"BKMCL_Seek valid file use case SEEK_SET test failed to seek";
			if (ret != -1) {
				ret = BKMCL_Read(fd, data_read, 10);
				ret = memcmp (data_read, &data_write[5], 10);
				EXPECT_EQ(ret, 0) << "BKMCL_Seek valid file use case SEEK_SET test failed in memcmp";
			}
		}
		ret = BKMCL_Close(fd);
		ret = BKMCL_Remove((uint8*)ETEST_BKMCL_FILE);
	}
}

//
//    Valid file Seek test - use case SEEK_CUR (seek from current pos)
//

TEST(BKM, ValidFileSeekFromCurrPos) {

	fd = BKMCL_Open((uint8*)ETEST_BKMCL_FILE, O_TRUNC | O_CREAT | O_RDWR );
	EXPECT_NE(fd, -1) << "Valid file use Seek test use case SEEK_CUR failed to open the file";

	if (fd != -1) {
		ret = BKMCL_Write(fd, data_write, 20);
		EXPECT_EQ(ret, 20) << "Valid file Seek test use case SEEK_CUR failed to write to the file";
		if (ret == 20) {
			ret = BKMCL_Seek(fd, -10, SEEK_CUR);
			EXPECT_NE(ret, -1) << "BKMCL_Seek valid file use case SEEK_CUR test failed to seek";
			if (ret != -1) {
			ret = BKMCL_Read(fd, data_read, 10);
			ret = memcmp (data_read, &data_write[10], 10);
				EXPECT_EQ(ret, 0) << "BKMCL_Seek valid file use case SEEK_CUR test failed in memcmp";
			}
		}
		ret = BKMCL_Close(fd);
		ret = BKMCL_Remove((uint8*)ETEST_BKMCL_FILE);
	}
}

//
//    Valid file Seek test - use case SEEK_END (seek from end pos)
//

TEST(BKM, ValidFileSeekFromEndPos) {
	fd = BKMCL_Open((uint8*)ETEST_BKMCL_FILE, O_TRUNC | O_CREAT | O_RDWR );
	EXPECT_NE(fd, -1) << "Valid file use Seek test use case SEEK_END failed to open the file";

	if (fd != -1) {
		ret = BKMCL_Write(fd, data_write, 20);
		EXPECT_EQ(ret, 20) << "Valid file Seek test use case SEEK_END failed to write to the file";
		if (ret == 20) {
			ret = BKMCL_Seek(fd, -10, SEEK_END);
			EXPECT_NE(ret, -1) << "BKMCL_Seek valid file use case SEEK_END test failed to seek";
			if (ret != -1) {
				ret = BKMCL_Read(fd, data_read, 10);
				ret = memcmp (data_read, &data_write[10], 10);
				EXPECT_EQ(ret, 0) << "BKMCL_Seek valid file use case SEEK_END test failed in memcmp";
			}
		}
		ret = BKMCL_Close(fd);
		ret = BKMCL_Remove((uint8*)ETEST_BKMCL_FILE);
	}
}

//
//    Valid file multiple seek test
//

TEST(BKM, ValidFileMultipleSeek) {
	fd = BKMCL_Open((uint8*)ETEST_BKMCL_FILE, O_TRUNC | O_CREAT | O_RDWR );
	EXPECT_NE(fd, -1) << "Valid file multiple Seek test failed to open the file";

	if (fd != -1) {
		ret = BKMCL_Write(fd, data_write, 20);
		EXPECT_EQ(ret, 20) << "Valid file multiple Seek test failed to write to the file";
		if (ret == 20) {
			ret = BKMCL_Seek(fd, 2, SEEK_SET);  // pos = 2
			EXPECT_NE(ret, -1) << "BKMCL_Seek valid file multiple seek test failed to seek(SEEK_SET)";
			if (ret != -1) {
				ret = BKMCL_Read(fd, data_read, 10);  // pos = 2+10=12
				ret = memcmp (data_read, &data_write[2], 10);
				EXPECT_EQ(ret, 0) << "BKMCL_Seek valid file multiple seek test failed in memcmp #1";

				ret = BKMCL_Seek(fd, -2, SEEK_CUR);  // pos = 12-2=10
				ret = BKMCL_Read(fd, data_read, 5); // pos = 10+5=15
				ret = memcmp (data_read, &data_write[10], 5);
				EXPECT_EQ(ret, 0) << "BKMCL_Seek valid file multiple seek test failed in memcmp #2";

				ret = BKMCL_Seek(fd, -6, SEEK_CUR);  // pos = 15-6=9
				ret = BKMCL_Read(fd, data_read, 10); // pos = 7+10=17
				ret = memcmp (data_read, &data_write[9], 10);
				EXPECT_EQ(ret, 0) << "BKMCL_Seek valid file multiple seek test failed in memcmp #3";

				ret = BKMCL_Seek(fd, -5, SEEK_END);  // pos = end-5=20-5=15
				ret = BKMCL_Read(fd, data_read, 5);
				ret = memcmp (data_read, &data_write[15], 5);
				EXPECT_EQ(ret, 0) << "BKMCL_Seek valid file multiple seek test failed in memcmp #4";
			}
		}
		ret = BKMCL_Close(fd);
		ret = BKMCL_Remove((uint8*)ETEST_BKMCL_FILE);
	}
}

//
//    Invalid file fd Seek test
//

TEST(BKM, InvalidFilefdSeek) {
	EXPECT_EQ(BKMCL_Seek(0x2000777, 10, SEEK_CUR), -1) << "BKMCL_Seek invalid file test failed";
}

//
//    Remove valid file test
//

TEST(BKM, RemoveValidFile) {
	fd = BKMCL_Open((uint8*)ETEST_BKMCL_FILE, O_TRUNC | O_CREAT | O_RDWR );
	EXPECT_NE(fd, -1) << "Valid file Remove test failed to open the file";
	if (fd != -1) {
		ret = BKMCL_Write(fd, data_write, 20);
		EXPECT_EQ(ret, 20) << "Valid file Remove test failed to write to the file";
		BKMCL_Close(fd);
		if (ret == 20) {
			EXPECT_GE(BKMCL_Remove((uint8*)ETEST_BKMCL_FILE), 0) << "BKMCL_Remove valid file test failed";
		}
	}
}

//
//    Non existing file Remove test
//

TEST(BKM, RemoveNonExistingFile) {
	EXPECT_LT(BKMCL_Remove((uint8*)ETEST_BKMCL_UNEXISTING_FILE), 0) << "BKMCL_Remove non existing file test failed";
}

//
//    Invalid file name file Remove test 1
//

TEST(BKM, InvalidFileNameRemove1) {
	EXPECT_LT(BKMCL_Remove((uint8*)ETEST_BKMCL_INVALID_FILE1), 0) << "BKMCL_Remove invalid file name test 1 failed";
}

//
//    Invalid file name file Remove test 2
//

TEST(BKM, InvalidFileNameRemove2) {
	EXPECT_LT(BKMCL_Remove((uint8*)ETEST_BKMCL_INVALID_FILE2), 0) << "BKMCL_Remove invalid file name test 2 failed";
}

//
//    Too long file name file Remove test
//

TEST(BKM, TooLongFileNameRemove) {
	EXPECT_LT(BKMCL_Remove((uint8*)ETEST_BKMCL_TOO_LONG_FILE), 0) << "BKMCL_Remove too long file name test failed";
}

//
//    Empty name file Remove test
//

TEST(BKM, EmptyFileNameRemove) {
	EXPECT_LT(BKMCL_Remove((uint8*)ETEST_BKMCL_NO_NAME_FILE), 0) << "BKMCL_Remove empty file name test failed";
}

//
//    Valid Write test
//

TEST(BKM, ValidWrite) {
	fd = BKMCL_Open((uint8*)ETEST_BKMCL_FILE, O_TRUNC | O_CREAT | O_RDWR );
	EXPECT_NE(fd, -1) << "Valid Write test failed to open the file";
	if (fd != -1) {
		ret = BKMCL_Write(fd, data_write, 20);
		EXPECT_EQ(ret, 20) << "BKMCL_Write valid test failed";
		ret = BKMCL_Close(fd);
		ret = BKMCL_Remove((uint8*)ETEST_BKMCL_FILE);
	}
}

//
//    Valid Write in APPEND mode test
//

TEST(BKM, ValidWriteInAPPENDMode) {
	fd = BKMCL_Open((uint8*)ETEST_BKMCL_FILE, O_TRUNC | O_CREAT | O_RDWR );
	EXPECT_NE(fd, -1) << "Valid Write in APPEND mode test failed to open the file";
	if (fd != -1) {
		ret = BKMCL_Write(fd, data_write, 20);
		EXPECT_EQ(ret, 20) << "Valid Write in APPEND mode test failed in first write ret=" << ret << " instead of 20";
		if (ret == 20) {
			ret = BKMCL_Close(fd);
			fd = BKMCL_Open((uint8*)ETEST_BKMCL_FILE, O_APPEND | O_RDWR );
			ret = BKMCL_Write(fd, data_write, 20);
			ret = BKMCL_GetSize(fd, &size);
			EXPECT_EQ(ret, 0) << "BKMCL_Write in APPEND mode test failed";
			EXPECT_EQ((int)size, 40) << "BKMCL_Write in APPEND mode test failed";
		}
		ret = BKMCL_Close(fd);
		ret = BKMCL_Remove((uint8*)ETEST_BKMCL_FILE);
	}
}

//
//    Valid Write in TRUNC mode test
//

TEST(BKM, ValidWriteInTRUNCMode) {
	fd = BKMCL_Open((uint8*)ETEST_BKMCL_FILE, O_TRUNC | O_CREAT | O_RDWR );
	EXPECT_NE(fd, -1) << "Valid Write in TRUNC mode test failed to open the file";
	if (fd != -1) {
		ret = BKMCL_Write(fd, data_write, 20);
		EXPECT_EQ(ret, 20) << "Valid Write in TRUNC mode test failed in first write ret=" << ret << " instead of 20";
		if (ret == 20) {
			ret = BKMCL_Close(fd);
			fd = BKMCL_Open((uint8*)ETEST_BKMCL_FILE, O_TRUNC | O_RDWR );
			ret = BKMCL_Write(fd, data_write, 15);
			ret = BKMCL_GetSize(fd, &size);
			EXPECT_EQ(ret, 0) << "BKMCL_Write in TRUNC mode test failed";
			EXPECT_EQ((int)size, 15) << "BKMCL_Write in TRUNC mode test failed";
		}
		ret = BKMCL_Close(fd);
		ret = BKMCL_Remove((uint8*)ETEST_BKMCL_FILE);
	}
}

//
//    Write with wrong fd test
//

TEST(BKM, WriteWithWrongfd) {
	EXPECT_LT(BKMCL_Write(0x2000100, data_write, 10), 0) << "BKMCL_Write invalid file test failed";
}

//
//    Read with wrong fd test
//

TEST(BKM, ReadWithWrongfd) {
	EXPECT_LT(BKMCL_Read(0x2000100, data_read, 10), 0) << "BKMCL_Read invalid file test failed";
}

//
//    Valid Read 20 bytes from a file and compare to the expected results
//

TEST(BKM, ValidReadFromFile) {
	fd = BKMCL_Open((uint8*)ETEST_BKMCL_FILE, O_TRUNC | O_CREAT | O_RDWR );
	EXPECT_NE(fd, -1) << "Valid Read test failed to open the file";
	if (fd != -1) {
		ret = BKMCL_Write(fd, data_write, 20);
		EXPECT_EQ(ret, 20) << "Valid Read test, write step failed";
		if (ret == 20) {
			ret =  BKMCL_Close(fd);
			fd = BKMCL_Open((uint8*)ETEST_BKMCL_FILE, O_RDONLY );
			ret = BKMCL_Read (fd, data_read, 20);
			EXPECT_EQ(ret, 20) << "BKMCL_Read valid test failed to read data";
			if (ret == 20) {
				ret = memcmp (data_read, data_write, 20);
				EXPECT_EQ(ret, 0) << "BKMCL_Read valid test failed, data_read != data_write";
			}
		}
		ret = BKMCL_Close(fd);
		ret = BKMCL_Remove((uint8*)ETEST_BKMCL_FILE);
	}
}

//
//    Read exceeding data test
//

TEST(BKM, ReadExceedingData) {
	fd = BKMCL_Open((uint8*)ETEST_BKMCL_FILE, O_TRUNC | O_CREAT | O_RDWR );
	EXPECT_NE(fd, -1) << "Exceeding Read test failed to open the file";
	if (fd != -1) {
		ret = BKMCL_Write(fd, data_write, 20);
		EXPECT_EQ(ret, 20) << "Exceeding Read test, write step failed";
		if (ret == 20) {
			ret =  BKMCL_Close(fd);
			fd = BKMCL_Open((uint8*)ETEST_BKMCL_FILE, O_RDONLY );
			ret = BKMCL_Read (fd, data_read, 30);
			EXPECT_EQ(ret, 20) << "BKMCL_Read Exceeding test failed to read data";
		}
		ret = BKMCL_Close(fd);
		ret = BKMCL_Remove((uint8*)ETEST_BKMCL_FILE);
	}
}

//
//    Do the cleanup before exiting
//

TEST(BKM, Uninitialize) {
	ASSERT_NE(BKMCL_Uninitialize(), -1) << "BKMCL_Unitialize cleanup failed";
}



int main(int argc, char *argv[])
{

	::testing::InitGoogleTest(&argc, argv);

	char strUserInput[ISDBT_MAX_INPUT_LEN]    = {""};
	const char strExit[]                      = "exit";
	const char strBkmTest[]                   = "bkm";
	int len = 0;

	do
	{
	if( argc <= 1 )
	{
		printf("\n\n-------------------------------\n"
				"Isdb- Tmm Test Menu (Select one):\n\n"
				" bkm         - To test BKM API\n"
				" exit        - To exit this test application\n\n"
	);
		printf("Usage: isdbtmmtest <testcase1> <testcase2> <testcase3>\n");
		printf("eg: isdbtmmtest bkm exit\n\n");
		break;
	}

	while(argc-- >= 1)
	{
		if( strcmp(*argv, strExit) == 0 )
		{
		break;
		}
		else if( strcmp(*argv, strBkmTest) == 0 )
		{
			ret = RUN_ALL_TESTS();
		}

		*(argv++);
	}

	break;

	}while(0);
	return 0;

}

