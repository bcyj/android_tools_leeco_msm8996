
/*
    Face Recognition Library API
*/

#ifndef FACEPROCFRAPI_H__
#define FACEPROCFRAPI_H__

#define FACEPROC_API

#include    "FaceProcDef.h"

#ifndef FACEPROC_DEF_HALBUM
#define FACEPROC_DEF_HALBUM
    typedef void*           HALBUM;         /* Album Data Handle */
#endif /* FACEPROC_DEF_HALBUM */

#ifndef FACEPROC_DEF_HFEATURE
#define FACEPROC_DEF_HFEATURE
    typedef void*           HFEATURE;       /* Face Feature Data Handle */
#endif /* FACEPROC_DEF_HFACEDATA */

#ifndef FACEPROC_DEF_HPTRESULT
#define FACEPROC_DEF_HPTRESULT
    typedef void*           HPTRESULT;
#endif /* FACEPROC_DEF_HPTRESULT */

#define SERIALIZED_FEATUR_MEM_SIZE 176   /* Size of Serialized Feature Memory */

typedef INT32 FR_ERROR;

#define     FR_NORMAL                         0      /* Normal End              */
#define     FR_ERR_INVALIDPARAM              -3      /* Invalid Parameter Error */
#define     FR_ERR_ALLOCMEMORY               -4      /* Memory Allocation Error */
#define     FR_ERR_NOHANDLE                  -7      /* Handle Error            */
#define     FR_ERR_UNKOWN_FORMAT           -100      /* Unknown Format          */
#define     FR_ERR_INVALID_DATA_VERSION    -101      /* Invalid Data Version    */
#define     FR_ERR_INVALID_DATA            -102      /* Invalid Data            */

/* Facial Parts Detection Point Index */
enum FR_PT_PARTS_POINT {
    FR_PTPOINT_LEFT_EYE = 0,              /* Left Eye Center    */
    FR_PTPOINT_RIGHT_EYE,                 /* Right Eye Center   */
    FR_PTPOINT_MOUTH,                     /* Mouth Center       */
    FR_PTPOINT_LEFT_EYE_IN,               /* Left Eye In        */
    FR_PTPOINT_LEFT_EYE_OUT,              /* Left Eye Out       */
    FR_PTPOINT_RIGHT_EYE_IN,              /* Right Eye In       */
    FR_PTPOINT_RIGHT_EYE_OUT,             /* Right Eye Out      */
    FR_PTPOINT_MOUTH_LEFT,                /* Mouth Left         */
    FR_PTPOINT_MOUTH_RIGHT,               /* Mouth Right        */
    FR_PTPOINT_NOSE_LEFT,                 /* Nose Left          */
    FR_PTPOINT_NOSE_RIGHT,                /* Nose Right         */
    FR_PTPOINT_MOUTH_UP,                  /* Mouth Up           */
    FR_PTPOINT_KIND_MAX                   /* The number of Feature Points */
};


#ifdef  __cplusplus
extern "C" {
#endif

/*********************************************
* Version Information
*********************************************/
/*       Gets Version       */
FACEPROC_API INT32 FACEPROC_FR_GetVersion(UINT8 *pbyMajor, UINT8 *pbyMinor);


/*********************************************
* Face Feature Data Handle Creation/Deletion
*********************************************/
/*      Creates Face Feature Data Handle    */
FACEPROC_API HFEATURE FACEPROC_FR_CreateFeatureHandle( void );
/*      Deletes Face Feature Data Handle    */
FACEPROC_API INT32 FACEPROC_FR_DeleteFeatureHandle(HFEATURE hFeature);


/*********************************************
* Face Feature Extraction
*********************************************/
/*      Extracts Face Feature from Facial Part Positions and the Image                  */
FACEPROC_API INT32 FACEPROC_FR_ExtractFeature(HFEATURE hFeature, RAWIMAGE *pImage, INT32 nWidth, INT32 nHeight,
                                                     INT32 nPointNum, POINT aptPoint[], INT32 anConfidence[]);
/*      Extracts Face Feature from Facial Parts Detection Result Handle ans the Image   */
FACEPROC_API INT32 FACEPROC_FR_ExtractFeatureFromPtHdl(HFEATURE hFeature, RAWIMAGE *pImage, INT32 nWidth, INT32 nHeight, HPTRESULT hPtResult);


/*********************************************
* Face Feature Data Reading/Writing
*********************************************/
/*      Writes Face Feature Data into Memory     */
FACEPROC_API INT32 FACEPROC_FR_WriteFeatureToMemory(HFEATURE hFeature, UINT8 *pbyBuffer, UINT32 unBufSize);
/*      Reads Face Feature Data from Memory      */
FACEPROC_API INT32 FACEPROC_FR_ReadFeatureFromMemory(HFEATURE hFeature, UINT8 *pbyBuffer, UINT32 unBufSize, FR_ERROR *pError);


/*********************************************
* Album Data Handle Creation/Deletion
*********************************************/
/*      Creates Album Data Handle   */
FACEPROC_API HALBUM FACEPROC_FR_CreateAlbumHandle(INT32 nMaxUserNum, INT32 nMaxDataNumPerUser);
/*      Deletes Album Data Handle   */
FACEPROC_API INT32 FACEPROC_FR_DeleteAlbumHandle(HALBUM hAlbum);


/*********************************************
* Acquisition of Album Data configuration
*********************************************/
/*      Gets the maximal numbers of Users and Feature Data per User */
FACEPROC_API INT32 FACEPROC_FR_GetAlbumMaxNum(HALBUM hAlbum, INT32 *pnMaxUserNum, INT32 *pnMaxDataNumPerUser);


/*********************************************
* Data Registration In Album
*********************************************/
/*      Registers Face Feature Data into Album Data */
FACEPROC_API INT32 FACEPROC_FR_RegisterData(HALBUM hAlbum, HFEATURE hFeature, INT32 nUserID, INT32 nDataID);


/*********************************************
* Acquisition of Album Data Registration Status
*********************************************/
/*      Gets the total number of Data registered in Album Data          */
FACEPROC_API INT32 FACEPROC_FR_GetRegisteredAllDataNum(HALBUM hAlbum, INT32 *pnAllDataNum);
/*      Gets the number of Users with registered Data                   */
FACEPROC_API INT32 FACEPROC_FR_GetRegisteredUserNum(HALBUM hAlbum, INT32 *pnUserNum);
/*      Gets the number of registered Feature Data of a specified User  */
FACEPROC_API INT32 FACEPROC_FR_GetRegisteredUsrDataNum(HALBUM hAlbum, INT32 nUserID, INT32 *pnUserDataNum);
/*      Gets Data Registration Status of a specified User               */
FACEPROC_API INT32 FACEPROC_FR_IsRegistered(HALBUM hAlbum, INT32 nUserID, INT32 nDataID, BOOL *pIsRegistered);


/*********************************************
* Clearance of Album Data
*********************************************/
/*      Clears all the Data from Album Data                         */
FACEPROC_API INT32 FACEPROC_FR_ClearAlbum(HALBUM hAlbum);
/*      Clears all the Data of a specified User from Album Data     */
FACEPROC_API INT32 FACEPROC_FR_ClearUser(HALBUM hAlbum, INT32 nUserID);
/*      Clears a specified Data of a specified User from Album Data */
FACEPROC_API INT32 FACEPROC_FR_ClearData(HALBUM hAlbum, INT32 nUserID, INT32 nDataID);


/*********************************************
* Album Serialization/Restoration
*********************************************/
/*      Gets the size of Serialized Album Data  */
FACEPROC_API INT32 FACEPROC_FR_GetSerializedAlbumSize(HALBUM hAlbum, UINT32 *punSerializedAlbumSize);
/*      Serializes Album Data                   */
FACEPROC_API INT32 FACEPROC_FR_SerializeAlbum(HALBUM hAlbum, UINT8 *pbyBuffer, UINT32 unBufSize);
/*      Restores Album Data                     */
FACEPROC_API HALBUM FACEPROC_FR_RestoreAlbum(UINT8 *pbyBuffer, UINT32 unBufSize, FR_ERROR *pError);


/*********************************************
* Acquisition of Feature Data from Album Data
*********************************************/
/*      Gets Feature Data from Album Data      */
FACEPROC_API INT32 FACEPROC_FR_GetFeatureFromAlbum(HALBUM hAlbum, INT32 nUserID, INT32 nDataID, HFEATURE hFeature);


/*********************************************
* Verification/Identification
*********************************************/
/*      Verification      */
FACEPROC_API INT32 FACEPROC_FR_Verify(HFEATURE hFeature, HALBUM hAlbum, INT32 nUserID, INT32 *pnScore );
/*      Identification    */
FACEPROC_API INT32 FACEPROC_FR_Identify(HFEATURE hFeature, HALBUM hAlbum, INT32 nMaxResultNum,
                                        INT32 anUserID[], INT32 anScore[], INT32 *pnResultNum);

#ifdef  __cplusplus
}
#endif

#endif  /* FACEPROCFRAPI_H__ */

