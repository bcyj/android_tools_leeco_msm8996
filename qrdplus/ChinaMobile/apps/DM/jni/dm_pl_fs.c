#include "dm_pl_fs.h"
#include "dm_error.h"
#include "dm_pl_debug.h"
#include "sci_types.h"

#ifdef WIN32
#include "std_header.h"
#include "XSR_Partition.h"
#endif

#define MMIFILE_FULL_PATH_MAX_LEN 512
#define SEEK_END 2
/**
 *  @brief  File name for Current Server ID.
 *  @param  none.
 *  @return The file path name.
 */
const IS8 * devFS_getServerIdFileName(void) {
    const IS8* SRV_ID_FILE_NAME = "/data/data/com.android.dm/files/serverid.dat";

    return SRV_ID_FILE_NAME;
}

/**
 *  @brief  File name for OMA DM object management tree.
 *  @param  none.
 *  @return The file path name.
 */
const IS8* devFS_getObjTreeFileName(void) {
    const IS8* DMOBJ_TREE_FILE_NAME = "/data/data/com.android.dm/bfTreeObj.dat";

    return DMOBJ_TREE_FILE_NAME;
}

/**
 *  @brief  File name for OMA DM account object management tree.
 *  @param  none.
 *  @return The file path name.
 */
const IS8* devFS_getAccTreeFileName(void) {
    const IS8* DMACC_TREE_FILE_NAME = "/data/data/com.android.dm/bfTreeAcc.dat";

    return DMACC_TREE_FILE_NAME;
}

/**
 *  @brief  File name for SMS Notify.
 *  @param  none.
 *  @return The file path name.
 */
const IS8* devFS_getSMSNotifyFileName(void) {
    const IS8* SMS_NOTIFY_FILE_NAME =
            "/data/data/com.android.dm/files/smsnotify.bin";

    return SMS_NOTIFY_FILE_NAME;
}

/**
 *  @brief   This function open a file according to opening mode
 *
 *  @param   fileName   [I]The name of the file to be opened.
 *  @param   openMode   [I]Opening mode.
 *
 *  @return  The file handle. \n
 *           If failed, return DM_INVALID_FILE_HANDLE;
 *
 *  @note    when using DM_FOM_OPEN_FOR_WRITE, the old file will be removed if it existed. \n
 *           The option DM_FOM_OPEN_FOR_APPEND is never chosed by SDK, only by wrappers. \n
 *           So, you do not need to support it directly in DM_FILE_HANDLE devFS_openFile. But in \n
 *           IS32 devFS_lgrObjBuffer and IS32 devFS_writePkgToStorage, append mode is \n
 *           required. Binary mode should be used whenever we call this function to open \n
 *           a file.
 */
DM_FILE_HANDLE devFS_openFile(const IS8 *fileName, DM_FILE_OPEN_MODE openMode) {
    DM_FILE_HANDLE file_handle = NULL;
    uint32 optMode = 0;
    wchar ucs2_name_arr[MMIFILE_FULL_PATH_MAX_LEN] = { 0 };
    wchar dm_full_path[MMIFILE_FULL_PATH_MAX_LEN] = { 0 };
    uint16 dm_full_path_len = MMIFILE_FULL_PATH_MAX_LEN;
    char str[100];

    DM_FILE_HANDLE fh;
    char szMode[8];
    int i = 0;

    if (openMode == DM_FOM_OPEN_FOR_READ_WRITE) {
        szMode[i++] = 'r';
        szMode[i++] = '+';
    } else if (openMode == DM_FOM_OPEN_FOR_APPEND) {
        szMode[i++] = 'a';
    } else if (openMode == DM_FOM_OPEN_FOR_WRITE) {
        szMode[i++] = 'w';
    } else    // DM_FOM_OPEN_FOR_READ
    {
        szMode[i++] = 'r';
    }

    szMode[i++] = 'b';

    // end of string
    szMode[i++] = 0;

    // really open the file
    fh = (DM_FILE_HANDLE) fopen(fileName, szMode);
    if (fh == NULL ) {
        return DM_INVALID_FILE_HANDLE ;
    }

    return fh;
}

/**
 *  @brief   This function close the file handle which is opened by devFS_openFile().
 *
 *  @param   fh   [I]The file handle to be closed.
 *
 *  @return  Returns DM_OK if the stream is successfully closed. \n
 *           Returns DM_ERROR if the stream is failed closed.
 *
 *  @note    none.
 */
IS32 devFS_closeFile(DM_FILE_HANDLE fh) {
    dm_debug_macro(DM_DEBUG_WRAPPER_MASK, "*** devFS_closeFile, fh=[%d]",
            (int) fh);
    if (fh != DM_INVALID_FILE_HANDLE && fh != NULL ) {
        fclose((void*) fh);
    }
    return DM_OK;
}

/**
 *  @brief   This function read data to buffer according to the size to be read.
 *
 *  @param   fh     [I]The file handle to be read.
 *  @param   buffer [I]Storage location for data.
 *  @param   size   [I]Size (bytes) to be read.
 *
 *  @return  Function returns the number of full items actually read. which may
 *           be less than count if an error occurs or if the end of the file is
 *           encountered before reaching count. \n
 *           If size or count is 0, fread returns 0 and the buffer contents are unchanged.
 *
 *  @note    none.
 */
IU32 devFS_readFile(DM_FILE_HANDLE fh, void *buffer, IU32 size) {
    uint32 readsize;
    char str[100];

    return (IU32) fread(buffer, 1, size, (void*) fh);
}

/**
 *  @brief   This function save buffer to a file according to the size to be written.
 *
 *  @param   fh     [I]The file handle to be write.
 *  @param   buffer [O]Pointer to data to be written.
 *  @param   size   [I]Size (bytes) to be written.
 *
 *  @return  Function returns the number of full items actually written, which may
 *           be less than count if an error occurs. Also, if an error occurs, the
 *           file-position indicator cannot be determined.
 *
 *  @note    none.
 */
IU32 devFS_writeFile(DM_FILE_HANDLE fh, const void *buffer, IU32 size) {
    uint32 writesize;
    char str[100];

    return (IU32) fwrite(buffer, 1, size, (void*) fh);
}

/**
 *  @brief   This function get a file's length in bytes.
 *
 *  @param   fileName [I]The name of the file to be checked.
 *
 *  @return  If the function succeeds, the return value is the file size. \n
 *           If the function fails, the return value is -1.
 *
 *  @note    none.
 */
IS32 devFS_getFileSize(const IS8 *fileName) {
    uint32 size;
    char str[100];
    wchar ucs2_name_arr[MMIFILE_FULL_PATH_MAX_LEN] = { 0 };
    wchar dm_full_path[MMIFILE_FULL_PATH_MAX_LEN] = { 0 };
    uint16 dm_full_path_len = MMIFILE_FULL_PATH_MAX_LEN;

    void * fh;

    // Open the file
    fh = fopen(fileName, "rb");
    if (fh == NULL ) {
        return -1;
    }

    // Get the size of the file.
    fseek(fh, 0, SEEK_END);
    size = ftell(fh);
    fclose(fh);

    return size;
}

/**
 *  @brief   Moves the file pointer to a specified location.
 *
 *  @param   fh     [I]File handle ,it should have be opened by other function call.
 *  @param   offset [I]Number of bytes from origin.
 *  @param   origin [I]Initial position.
 *
 *  @return  If successful, function returns 0. \n
 *           Otherwise, it returns a nonzero value.
 *
 *  @note    none.
 */
IS32 devFS_seek(DM_FILE_HANDLE fh, IS32 offset, DM_SEEK_MODE_E origin) {
    int retval;
    char str[100];

    return fseek((void*) fh, offset, (int) origin);
}

/**
 *  @brief   This function determines if a file of a particular name exists
 *           within the file system.
 *
 *  @param   fileName  [I]name of the file to be checked.
 *
 *  @return  Function return TRUE, If exist; \n
 *           Otherwise, it return FALSE.
 *
 *  @note    none.
 */
IBOOL devFS_fileExist(const IS8 *fileName) {
    wchar ucs2_name_arr[MMIFILE_FULL_PATH_MAX_LEN] = { 0 };
    wchar full_path_name[MMIFILE_FULL_PATH_MAX_LEN + 2] = { 0 };
    uint16 full_path_len = MMIFILE_FULL_PATH_MAX_LEN;
    char str[100];

    DM_FILE_HANDLE f;

    f = devFS_openFile(fileName, DM_FOM_OPEN_FOR_READ);
    if (f != DM_INVALID_FILE_HANDLE ) {
        // it existes
        devFS_closeFile(f);
        return TRUE;
    } else {
        // not exist
        return FALSE;
    }
}

/**
 *  @brief   This function delete a file.
 *           If the file exists, the file will been deleted; otherwise it do nothing.
 *
 *  @param   fileName  [I]name of the file to be deleted.
 *
 *  @return  Each of these functions returns 0 if the file is successfully deleted.
 *           Otherwise, it returns -1.
 *
 *  @note    none.
 */
IS32 devFS_removeFile(const IS8 *fileName) {
    char str[100];
    DM_FILE_HANDLE file_handle = NULL;
    wchar ucs2_name_arr[MMIFILE_FULL_PATH_MAX_LEN] = { 0 };
    wchar dm_full_path[MMIFILE_FULL_PATH_MAX_LEN] = { 0 };
    uint16 dm_full_path_len = MMIFILE_FULL_PATH_MAX_LEN;

    return remove(fileName);
}

/**
 *  @brief    This function save data to a file according to the size of data
 *            if file exists before, the file will be re-written.
 *
 *  @param    fileName  - the name of the file to save data to
 *  @param    data      - the data structure that contains its value
 *  @param    dataSize  - size of the data structure
 *
 *  @return   DM_OK or DM_ERROR
 *
 *  @note     none.
 */
IS32 devFS_saveDataToFile(const IS8 *fileName, const void *data, IS32 dataSize) {
    IS32 ret;
    DM_FILE_HANDLE f;

    f = devFS_openFile(fileName, DM_FOM_OPEN_FOR_WRITE);
    if (f != DM_INVALID_FILE_HANDLE ) {
        /* Write the data structure */
        ret = devFS_writeFile(f, (const void *) data, dataSize);
        devFS_closeFile(f);

        return ((ret == dataSize) ? DM_OK : DM_ERROR);
    } else {
        return DM_ERROR;
    }
}

/**
 *  @brief    This function reads data from an existing file.
 *
 *  @param    fileName - the name of the file to read data from
 *  @param    data     - the data structure
 *  @param    dataSize - size of the data structure
 *
 *  @return   DM_OK
 *            DM_ERROR if file operation fails
 *
 *  @note     none.
 */
IS32 devFS_readDataFromFile(const IS8 *fileName, void *data, IS32 dataSize) {
    DM_FILE_HANDLE f;
    IS32 ret;

    f = devFS_openFile(fileName, DM_FOM_OPEN_FOR_READ);
    if (f != DM_INVALID_FILE_HANDLE ) {
        ret = devFS_readFile(f, data, dataSize);
        devFS_closeFile(f);

        return ((ret == dataSize) ? DM_OK : DM_ERROR);
    } else {
        return DM_ERROR;
    }
}
