#ifndef _DM_PL_FS_H_
#define _DM_PL_FS_H_

#include "vdm_pl_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 *  File Handle type define
 */
typedef int DM_FILE_HANDLE;

/**
 *  File macro define
 */
#define DM_INVALID_FILE_HANDLE           ((DM_FILE_HANDLE)-1)

/**
 *  struct for larger object storage map.
 */
typedef struct uri_filename_tag {
    const IS8 *uri;     ///< uri in DM's tree
    const IS8 *filename;    ///< filename to save to, after download

} uri_filename_t;

/**
 *  OpenMode for devFS_openFile().
 */
typedef enum _DM_FILE_OPEN_MODE_ENUM {
    DM_FOM_OPEN_FOR_READ, ///< Opens for reading. If the file does not exist or cannot be found, the fopen call fails.
    DM_FOM_OPEN_FOR_WRITE, ///< Opens an empty file for writing. If the given file exists, its contents are destroyed.
    DM_FOM_OPEN_FOR_READ_WRITE, ///< Opens for both reading and writing. (The file must exist.)
    DM_FOM_OPEN_FOR_APPEND ///< Opens for writing at the end of the file (appending) without removing the EOF marker
                           // before writing new data to the file; creates the file first if it doesn¡¯t exist.

} DM_FILE_OPEN_MODE;

/**
 *  Seek Mode for devFS_seek().
 */
typedef enum _DM_SEEKMODE_ENUM {
    DM_SEEK_SET,    ///< Beginning of file.
    DM_SEEK_CUR,    ///< Current position of file pointer.
    DM_SEEK_END     ///< End of file.

} DM_SEEK_MODE_E;

/**
 *  @brief  File name for Current Server ID.
 *  @param  none.
 *  @return The file path name.
 */
const IS8 * devFS_getServerIdFileName(void);

/**
 *  @brief  File name for OMA DM object management tree.
 *  @param  none.
 *  @return The file path name.
 */
const IS8* devFS_getObjTreeFileName(void);

/**
 *  @brief  File name for OMA DM account object management tree.
 *  @param  none.
 *  @return The file path name.
 */
const IS8* devFS_getAccTreeFileName(void);

/**
 *  @brief  File name for SMS Notify.
 *  @param  none.
 *  @return The file path name.
 */
const IS8* devFS_getSMSNotifyFileName(void);

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
DM_FILE_HANDLE devFS_openFile(const IS8 *fileName, DM_FILE_OPEN_MODE openMode);

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
IS32 devFS_closeFile(DM_FILE_HANDLE fh);

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
IU32 devFS_readFile(DM_FILE_HANDLE fh, void *buffer, IU32 size);

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
IU32 devFS_writeFile(DM_FILE_HANDLE fh, const void *buffer, IU32 size);

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
IS32 devFS_getFileSize(const IS8 *fileName);

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
IS32 devFS_seek(DM_FILE_HANDLE fh, IS32 offset, DM_SEEK_MODE_E origin);

/**
 *  @brief   This function determines if a file of a particular name exists
 *           within the file system.
 *
 *  @param   fileName  [I]name of the file to be checked.
 *
 *  @return  Function return bf_true, If exist; \n
 *           Otherwise, it return bf_false.
 *
 *  @note    none.
 */
IBOOL devFS_fileExist(const IS8 *fileName);

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
IS32 devFS_removeFile(const IS8 *fileName);

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
IS32 devFS_saveDataToFile(const IS8 *fileName, const void *data, IS32 dataSize);

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
IS32 devFS_readDataFromFile(const IS8 *fileName, void *data, IS32 dataSize);

#ifdef __cplusplus
}
#endif

#endif
