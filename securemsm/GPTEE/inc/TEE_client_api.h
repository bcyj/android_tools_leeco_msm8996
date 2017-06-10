/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

	HLOS Global Platform TEE Clientt Library Functions Header

GENERAL DESCRIPTION
  Contains the library functions for accessing QSEECom driver.

EXTERNALIZED FUNCTIONS
  None

INITIALIZATION AND SEQUENCING REQUIREMENTS

Copyright (c) 2014 Qualcomm Technologies, Inc.
All Rights Reserved.
Qualcomm Technologies Confidential and Proprietary

*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
#ifndef __TEE_CLIENT_API_H_
#define __TEE_CLIENT_API_H_

typedef uint32_t TEEC_Result;

enum TEEC_ParamType {
	TEEC_VALUE_NONE,
	TEEC_VALUE_INPUT,
	TEEC_VALUE_OUTPUT,
	TEEC_VALUE_INOUT,
	TEEC_MEMREF_TEMP_INPUT,
	TEEC_MEMREF_TEMP_OUTPUT,
	TEEC_MEMREF_TEMP_INOUT,
	TEEC_MEMREF_WHOLE,
	TEEC_MEMREF_PARTIAL_INPUT,
	TEEC_MEMREF_PARTIAL_OUTPUT,
	TEEC_MEMREF_PARTIAL_INOUT,
};

#define MAX_NUM_PARAMS 4

/*----------------------------------------------------------------------------
 * Return Codes Macros
 * -------------------------------------------------------------------------*/

/* The maximum size of a single Shared Memory block, in bytes, of both API
 * allocated and API registered memory. This version of the standard requires
 * that this maximum size is greater than or equal to 512kB. In systems where
 * there is no limit imposed by the Implementation then this definition should
 * be defined to be the size of the address space. */
#define TEEC_CONFIG_SHAREDMEM_MAX_SIZE (0x80000 * 32)

/* Return Codes */
/* The operation was successful. */
#define TEEC_SUCCESS  0x00000000

/* Non-specific cause. */
#define TEEC_ERROR_GENERIC  0xFFFF0000

/* Access privileges are not sufficient. */
#define TEEC_ERROR_ACCESS_DENIED  0xFFFF0001

/* The operation was cancelled. */
#define TEEC_ERROR_CANCEL  0xFFFF0002

/* Concurrent accesses caused conflict. */
#define TEEC_ERROR_ACCESS_CONFLICT  0xFFFF0003

/* Too much data for the requested operation was passed. */
#define TEEC_ERROR_EXCESS_DATA  0xFFFF0004

/* Input data was of invalid format. */
#define TEEC_ERROR_BAD_FORMAT  0xFFFF0005

/* Input parameters were invalid. */
#define TEEC_ERROR_BAD_PARAMETERS  0xFFFF0006

/* Operation is not valid in the current state. */
#define TEEC_ERROR_BAD_STATE  0xFFFF0007

/* The requested data item is not found. */
#define TEEC_ERROR_ITEM_NOT_FOUND  0xFFFF0008

/* The requested operation should exist but is not yet implemented. */
#define TEEC_ERROR_NOT_IMPLEMENTED  0xFFFF0009

/* The requested operation is valid but not supported in this Implementation. */
#define TEEC_ERROR_NOT_SUPPORTED  0xFFFF000A

/* Expected data was missing.*/
#define TEEC_ERROR_NO_DATA  0xFFFF000B

/* System ran out of resources. */
#define TEEC_ERROR_OUT_OF_MEMORY  0xFFFF000C

/* The system is busy working on something else. */
#define TEEC_ERROR_BUSY 0xFFFF000D

/* Communication with a remote party failed. */
#define TEEC_ERROR_COMMUNICATION 0xFFFF000E

/* A security fault was detected. */
#define TEEC_ERROR_SECURITY 0xFFFF000F

/* The supplied buffer is too short for the generated output. */
#define TEEC_ERROR_SHORT_BUFFER 0xFFFF0010


/*----------------------------------------------------------------------------
 * Return Code Origins Macros
 * -------------------------------------------------------------------------*/

/* The return code is an error that originated within the TEE Client API
 * implementation.
 */
#define TEEC_ORIGIN_API  0x00000001

/* The return code is an error that originated within the underlying
 * communications stack linking the rich OS with the TEE.
 */
#define TEEC_ORIGIN_COMMS 0x00000002

/*The return code is an error that originated within the common TEE code. */
#define TEEC_ORIGIN_TEE 0x00000003

/* The return code originated within the Trusted Application code. This include
 * the case where the return code is a success.
 */
#define TEEC_ORIGIN_TRUSTED_APP  0x00000004


/*----------------------------------------------------------------------------
 * API Shared Memory Control Flag Macros :
 * All other flag values Reserved for Future Use
 * -------------------------------------------------------------------------*/

/* Application to the Trusted Application. The Shared Memory can carry data
 * from the Client
 */
#define TEEC_MEM_INPUT  0x00000001

/* The Shared Memory can carry data from the Trusted Application to the
 * Client Application.
 */
#define TEEC_MEM_OUTPUT  0x00000002


/*----------------------------------------------------------------------------
 * API Parameter TypesL: All other values Reserved for Future Use Macros :
 * -------------------------------------------------------------------------*/

/*The Parameter is not used */
#define TEEC_NONE  0x00000000

/* The Parameter is a TEEC_Value tagged as input. */
#define TEEC_VALUE_INPUT  0x00000001

/* The Parameter is a TEEC_Value tagged as output. */
#define TEEC_VALUE_OUTPUT  0x00000002

/* The Parameter is a TEEC_Value tagged as both as input and output, i.e.,
 * for which both the behaviors of TEEC_VALUE_INPUT and TEEC_VALUE_OUTPUT apply.
 */
#define TEEC_VALUE_INOUT  0x00000003

/* The Parameter is a TEEC_TempMemoryReference describing a region of memory
 * which needs to be temporarily registered for the duration of the Operation
 * and is tagged as input.
 */
#define TEEC_MEMREF_TEMP_INPUT  0x00000005

/* Same as TEEC_MEMREF_TEMP_INPUT, but the Memory Reference is tagged as
 * output. The Implementation may update the size field to reflect the required
 * output size in some use cases.
 */
#define TEEC_MEMREF_TEMP_OUTPUT  0x00000006

/* A Temporary Memory Reference tagged as both input and output, i.e., for which
 * both the behaviors of TEEC_MEMREF_TEMP_INPUT & TEEC_MEMREF_TEMP_OUTPUT apply.
 */
#define TEEC_MEMREF_TEMP_INOUT  0x00000007


/* The Parameter is a Registered Memory Reference that refers to the entirety of
 * its parent Shared Memory block. The parameter struct is TEEC_MemoryReference.
 * In this structure, the Implementation MUST read only the parent field and MAY
 * update the size field when the operation completes.
 */
#define TEEC_MEMREF_WHOLE  0x0000000C

/* A Registered Memory Reference structure that refers to a partial region of
 * its parent Shared Memory block and is tagged as input.
 */
#define TEEC_MEMREF_PARTIAL_INPUT  0x0000000D

/* A Registered Memory Reference structure that refers to a partial region of
 * its parent Shared Memory block and is tagged as output.
 */
#define TEEC_MEMREF_PARTIAL_OUTPUT  0x0000000E

/* The Registered Memory Reference structure that refers to a partial region of
 * its parent Shared Memory block and is tagged as both input and output, i.e.,
 * for which both the behaviors of TEEC_MEMREF_PARTIAL_INPUT and
 * TEEC_MEMREF_PARTIAL_OUTPUT apply
 */
#define TEEC_MEMREF_PARTIAL_INOUT  0x0000000F

/*----------------------------------------------------------------------------
 *  API Session Login Methods Macros
 * -------------------------------------------------------------------------*/

/* No login data is provided. */
#define TEEC_LOGIN_PUBLIC  0x00000000

/* Login data about user running the Client Application process is provided. */
#define TEEC_LOGIN_USER  0x00000001

/* Login data about the group running the Client Application process is
 * provided.
 */
#define TEEC_LOGIN_GROUP  0x00000002

/* Login data about the running Client Application itself is provided */
#define TEEC_LOGIN_APPLICATION  0x00000004

/* Login data about the user running the  Client Application and about the
   Client Application itself is provided. */
#define TEEC_LOGIN_USER_APPLICATION  0x00000005

/* Login data about the group running the Client Application and about the
 * Client Application itself is provided.
 * Reserved for implementation-defined connection methods
 * 0x80000000 – 0xFFFFFFFF Behavior is implementation-defined.
 */
#define TEEC_LOGIN_GROUP_APPLICATION  0x00000006


/*----------------------------------------------------------------------------
 *  Macro for param type
 * -------------------------------------------------------------------------*/
#define TEEC_PARAM_TYPES(P0, P1, P2, P3) \
	((P0) | ((P1) << 4) | ((P2) << 8) | ((P3) << 12))

#define TEEC_PARAM_TYPE_GET(t, i) (((t) >> ((i)*4)) & 0xF)

/*----------------------------------------------------------------------------
 *  Data Structures
 * -------------------------------------------------------------------------*/

typedef struct {
	uint32_t timeLow;
	uint16_t timeMid;
	uint16_t timeHiAndVersion;
	uint8_t clockSeqAndNode[8];
}TEEC_UUID;

typedef struct  {
	/*<Implementation-Defined Type>*/
	void *imp;
}TEEC_Context;

typedef struct  {
	/*<Implementation-Defined Type>*/
	void *imp;
}TEEC_Session;

/* This type denotes a Shared Memory block which has either been registered
 * with the Implementation or allocated by it.
 * [IN] buffer: pointer to the memory buffer shared with the TEE
 * [IN] size: is the size of the memory buffer, in bytes
 * [IN] flags: bit-vector which can contain the following flags:
 *      o  TEEC_MEM_INPUT: the memory used to transfer Client Application->TEE
 *      o  TEEC_MEM_OUTPUT: the memory used to transfer TEE ->Client Application
 *      o  All other bits in this field SHOULD be set to 0, rsvd for future use
*/
typedef struct {
	void *buffer;
	size_t size;
	uint32_t flags;
	/* <Implementation-Defined Type> */
	void *imp;
}TEEC_SharedMemory;

/* This type defines a Registered Memory Reference, i.e., that uses
 * pre-registered or pre-allocated Shared Memory block. It is used as a
 * TEEC_Operation  parameter when the corresponding parameter type is one
 * of TEEC_MEMREF_WHOLE, TEEC_MEMREF_PARTIAL_INPUT, TEEC_MEMREF_PARTIAL_OUTPUT,
 * or TEEC_MEMREF_PARTIAL_INOUT.
 * [IN] parent: points to a  TEEC_SharedMemory  structure. The memory reference
 *      refers either to the whole Shared Memory or to a partial region within
 *      the Shared Memory block, depending of the parameter type. The data flow
 *      direction of the memory reference must be consistent with the flags
 *      defined in the parent Shared Memory Block. Note that the parent field
 *      MUST NOT be NULL. To encode a null Memory Reference, the Client App
 *      must use a Temporary Memory Reference with the buffer field set to NULL.
 * [IN/OUT] size: size of the referenced memory region, in bytes:
 *      This field is valid only if the Memory Reference type in the operation
 *      structure  is  not  TEEC_MEMREF_WHOLE. Otherwise, the size is read from
 *      the parent Shared Memory structure.
 *      o  When an operation completes, and if the Memory Reference is tagged as
 *      “output”, this is updated by trusted appluication to reflect the actual
 *      or required size of the output. This applies even if the parameter type
 *      is TEEC_MEMREF_WHOLE:
 *      If the Trusted Application has actually written some data in the
 *      output buffer, then the size  field reflects the actual number of
 *      bytes written (updated by Trusted Application).
 *      If the output buffer was not large enough to contain the whole
 *      output, Trusted application will update the size field with the
 *      size of the output buffer requested by the Trusted Application.
 *      In this case, no data has been written into the output buffer.
 * [IN] offset: offset in bytes, of the referenced memory region from the start
 *      of the Shared Memory block:
 *       - This is only interpreted as valid if the Memory Reference type in the
 *      operation structure is not TEEC_MEMREF_WHOLE. Otherwise, the base addr
 *      of the Shared Memory block is used (offset of 0).
 */
typedef struct  {
	TEEC_SharedMemory *parent;
	size_t size;
	size_t offset;
}TEEC_RegisteredMemoryReference;

/* This type defines a Temporary Memory Reference. It is used as a
 * TEEC_Operation parameter when the corresponding parameter type is one
 * of  TEEC_MEMREF_TEMP_INPUT, TEEC_MEMREF_TEMP_OUTPUT, or
 * TEEC_MEMREF_TEMP_INOUT.

 * [IN] buffer: Pointer to the first byte of memory which needs to be
 *              temporarily registered for the duration of the Operation.
 *              This field can be NULL to specify a null Memory reference
 * [IN/OUT] size: size of the referenced memory region, in bytes:
 *      When the operation completes, and unless the parameter type is
 *      TEEC_MEMREF_TEMP_INPUT, the Implementation must update this field
 *      to reflect the actual or required size of the output:
 *      - If the Trusted Application has actually written some data in the
 *        output  buffer, then the Implementation MUST update the size field
 *        with the actual number of bytes written.
 *      - If the output buffer was not large enough to contain the whole output,
 *        or if it is null, the Implementation MUST update the size field with
 *        the size of the output buffer requested by the Trusted Application.
 *        In this case, no data has been written into the output buffer.
 */
typedef struct  {
	void *buffer;
	size_t size;
}TEEC_TempMemoryReference;

/* This  type defines a parameter that is not referencing shared memory, but
 * instead small raw data passed by value. It is used as a TEEC_Operation
 * parameter when the corresponding parameter type is one of TEEC_VALUE_INPUT,
 * TEEC_VALUE_OUTPUT, or TEEC_VALUE_INOUT.
 * a,b: The two fields of this structure do not have a particular meaning. It
 * is up to the protocol between the Client Application and the Trusted
 * Application to assign a semantic to those two integers.
 */
typedef struct  {
	uint32_t a;
	uint32_t b;
}TEEC_Value;

/* This type defines a Parameter of a TEEC_Operation. It can be a Temporary
 * Memory Reference, a Registered Memory Reference, or a Value Parameter.
 * (see definitions above)
 * The field to select in this union depends on the type of the parameter
 * specifiedin the  paramTypes field of the TEEC_Operation structure.
 * (see enum TEEC_ParamType)
 */
typedef struct  {
	union {
		TEEC_TempMemoryReference tmpref;
		TEEC_RegisteredMemoryReference memref;
		TEEC_Value value;
	};
}TEEC_Parameter;

/* This type defines the payload of either an open Session operation or an
 * invoke Command operation. It is also used for cancellation of operations,
 * which may be desirable even if no payload is passed.
 * [IN] started:  MUST be initialized to zero by the Client Application before
 *      each use in an operation if the Client Application may need to cancel
 *      the operation about to be performed.
 * [IN] paramTypes: encodes the type of each of the Parameters in the operation.
 *      The layout of these types within a 32-bit integer and the Client
 *      Application MUST use the macro TEEC_PARAMS_TYPE  to construct a constant
 *      value for this field. As a special case, if the Client Application sets
 *      paramTypes  to 0, then the Implementation MUST interpret it as meaning
 *      that the type for each Parameter is set to TEEC_NONE.
 * [IN] params: array of four Parameters. For each parameter, one of the memref,
 *      tmpref, or value fields must be used depending on the corresponding
 *      parameter type passed in paramTypes as described in the specification of
 *      TEEC_Parameter.
 */
typedef struct  {
	uint32_t   started;
	uint32_t paramTypes;
	TEEC_Parameter params[MAX_NUM_PARAMS];
	/* <Implementation-Defined Type> imp; */
	void *imp;
}TEEC_Operation;

/*----------------------------------------------------------------------------
 * FUNCTION DECLARATIONS AND DOCUMENTATION
 * -------------------------------------------------------------------------*/

/**
 * @brief Open a handle to the TEE device.
 *
 * This function initializes a new TEE Context, forming a connection between
 * this Client Application and the TEE identified by the string identifier name.
 * The Client Application MAY pass a NULL name, which means that a default TEE
 * will be connected to.
 * The caller MUST pass a pointer to a valid TEEC_Context in context. The API
 * assume all fields of the TEEC_Context structure are in an undefined state.
 *
 * @param[in] name: a zero-terminated string describing the TEE to connect to.
 *            If this  parameter is set to NULL default "QSEE" is selected.
 * @param[in/out] context: a TEEC_Context structure that is initialized this API
 *
 * @return: TEEC_SUCCESS: the initialization was successful.
 *          Another error code from above: initialization was not successful.
 *
 * Programmer Error: The following usage of the API is a programmer error:
 *       Attempting to initialize the same TEE Context structure concurrently
 *       from multiple threads. Multithreaded Client Applications must use
 *       platform-provided locking mechanisms to ensure that this case
 *       does not occur
 */
TEEC_Result TEEC_InitializeContext(const char *name, TEEC_Context *context);
/**
 * @brief
 *
 * This function finalizes an initialized TEE Context, closing the connection
 * between the Client Application and the TEE. The Client Application MUST only
 * call this function when ALL Sessions inside this TEE_Context have been closed
 * and all Shared Memory blocks have been released.
 * The functiona does not fail:  after this function returns the Client
 * Application must be able to consider that the Context has been closed.
 * The API does not do anything if context is NULL, simply returns.
 *
 * @param[in] context: an initialized TEEC_Context structure which is to be
 *                     finalized.
 * @return. NO retuirn value
 *
 * Programmer Error: The following usage of the API is a programmer error:
 *    Calling with a context which still has sessions opened.
 *    Calling with a context which contains unreleased Shared Memory blocks.
 *    Attempting to finalize the same TEE Context structure concurrently from
 *    multiple threads.
 *    Attempting to finalize the same TEE Context structure more than once,
 *    without an intervening call to TEEC_InitalizeContext.
 */
void TEEC_FinalizeContext(TEEC_Context *context);


/**
 * @brief
 *
 * This function registers a block of existing Client Application memory as a
 * block of Shared Memory within the scope of the specified TEE Context, in
 * accordance with the parameters which have been set by the Client Application
 * inside the sharedMem structure.
 * The parameter context MUST point to an initialized TEE Context.
 * The parameter sharedMem MUST point to the Shared Memory structure defining
 * the memory region to register. The Client Application MUST have populated the
 * following fields of the Shared Memory structure before calling this function:
 *   - The buffer field MUST point to the memory region to be shared, and MUST
 *           not be NULL.
 *   - The size field MUST contain the size of the buffer, in bytes.
 *          (0 is a valid size).
 *   - The flags  field indicates the intended directions of data flow between
 *          the ClientApplication and the TEE.
 *   - The API assumes that all other fields in the Shared Memory structure have
 *          undefined content.
 * The size of the buffer is limited defined by the constant
 * TEEC_CONFIG_SHAREDMEM_MAX_SIZE. However, note that this function may fail to
 * register a block smaller than this limit due to low resource condition
 * encountered at run-time.
 *
 * @param[in] context: a pointer to an initialized TEE Context
 * @param[in] sharedMem: a pointer to a Shared Memory structure to register:
 *     o  the  buffer, size, and flags fields of the sharedMem structure MUST be
 *        set in accordance with the specification described above
 * @return: TEEC_SUCCESS: the initialization was successful.
 *          TEEC_ERROR_OUT_OF_MEMORY: registration could not be completed due to
 *            lack of resources.
 *          Another error code from above: initialization was not successful for
 *             another reason.
 *
 * Programmer Error: The following usage of the API is a programmer error:
 *    Calling with a context which is not initialized.
 *    Calling with a sharedMem which is not populated in accordance with the
 *       specification.
 *    Attempting to initialize the same Shared Memory structure concurrently
 *       from multiple specification threads. Multi-threaded Client Applications
 *       must use platform -provided locking mechanisms to ensure that this case
 *       does not occur.
 */
TEEC_Result TEEC_RegisterSharedMemory(TEEC_Context *context,
					TEEC_SharedMemory *sharedMem);


/**
 * @brief
 *
 * This function allocates a new block of memory as a block of Shared Memory
 * within the scope of the specified TEE Context, in accordance with the
 * parameters which have been set by the Client Application inside the
 * sharedMem structure.
 * The context parameter MUST point to an initialized TEE Context.
 * The sharedMem parameter MUST point to the Shared Memory structure defining
 * the region to allocate.
 * Client Applications MUST have populated the following fields of the
 * Shared Memory structure:
 *    The size field MUST contain the desired s ize of the buffer, in bytes.
 *       The size is allowedto be zero. In this case memory is allocated and the
 *       pointer written in to the buffer field on return MUST not be NULL but
 *       MUST never be de-referenced by the Client Application. In this case
 *         however, the Shared Memory block can be used in Registered Memory
 *         References.
 *       The flags field indicates the allowed directions of data flow between
 *         the ClientApplication and the TEE.
 *       The API assumes that all other fields in the Shared Memory structure
 *          have undefined content.
 * The size of the buffer is limited defined by the constant
 * TEEC_CONFIG_SHAREDMEM_MAX_SIZE. However, note that this function may fail to
 * register a block smaller than this limit due to low resource condition
 * encountered at run-time.
 * If this function returns any code other than TEEC_SUCCESS, The API will
 *  return with buffe field of sharedMem to NULL.
 *
 * @param[in] context: a pointer to an initialized TEE Context
 * @param[in] sharedMem: a pointer to a Shared Memory structure to allocate:
 *      o  Before calling this API, Client Application MUST set the size,
 *         and flags fields
 *      o  On return, for a successful allocation the API will set the
 *         pointer buffer to the address of the allocated block, otherwise it
 *          will set buffer to NULL.
 * @return: TEEC_SUCCESS: the initialization was successful.
 *          TEEC_ERROR_OUT_OF_MEMORY: allocation could not be completed due to
 *          lack of resources.
 *          Another error code from above: initialization was not successful
 *          for another reason.
 *
 * Programmer Error: The following usage of the API is a programmer error:
 *    Calling with a context which is not initialized.
 *    Calling with a sharedMem which is not populated in accordance with the
 *       specification.
 *    Attempting to initialize the same Shared Memory structure concurrently
 *       from multiple specification threads. Multi-threaded Client Applications
 *       must use platform -provided locking mechanisms to ensure that this case
 *       does not occur.
 */
TEEC_Result TEEC_AllocateSharedMemory(TEEC_Context *context,
					TEEC_SharedMemory *sharedMem);

/**
 * @brief
 *
 * This function deregisters or deallocates a previously initialized block of.
 * Shared Memory
 * For a memory buffer allocated using TEEC_AllocateSharedMemory, this API will
 * free the underlying memory. It will set the buffer and size fields of the
 * sharedMem structure to NULL and 0 respectively before returning.
 * Client Application MUST NOT access this region after this function has been
 * called.
 *
 * For memory registered using TEEC_RegisterSharedMemory this API deregister
 * the underlying memory from the TEE, but the memory region will stay available
 * to the Client Application for other purposes as the memory is owned by it.
 *
 * This API does nothing if the sharedMem parameter is NULL.
 *
 * @param[in] context: a pointer to an initialized TEE Context
 * @param[in] sharedMem: haredMem: a pointer to a valid Shared Memory structure
 * @return: No return value
 *
 * Programmer Error: The following usage of the API is a programmer error:
 *    Calling with a context which is not initialized.
 *    Calling with a sharedMem which is not populated in accordance with the
 *       specification.
 *    Attempting to initialize the same Shared Memory structure concurrently
 *       from multiple specification threads. Multi-threaded Client Applications
 *       must use platform -provided locking mechanisms to ensure that this case
 *       does not occur.
 */
void TEEC_ReleaseSharedMemory(TEEC_SharedMemory *sharedMem);


/**
 * @brief
 *
 * This function opens a new Session between the Client Application and the
 * specified Trusted Application.
 * The API assumes that all fields of this session structure are in an undefined
 * state.
 * When this function returns TEEC_SUCCESS this structure is populated with all
 * the information necessary for subsequent operations within the Session.
 * The target Trusted Application is identified by a UUID passed in the
 * parameter destination.
 * The Session MAY be opened using a specific connection method that can carry
 * additional connection data, such as data about the user or user-group
 * running the Client  Application, or about the Client Application itself. This
 * allows the Trusted Application to implement access control methods which
 * separate functionality or data accesses for different actors in the rich
 * environment outside of the TEE.
 *
 * The additional data associated with each connection method is passed in via
 * the pointer connectionData (Unsupported for QTEE context).
 * For the core login types the following connection data is required::
 *     TEEC_LOGIN_PUBLIC
 *        o  connectionData SHOULD be NULL.
 *     TEEC_LOGIN_USER
 *        o  connectionData SHOULD be NULL.
 *     TEEC_LOGIN_GROUP : Unsupported for QTEE/default context
 *        o  connectionData MUST point to a uint32_t which contains the group
 *           which this Client Application wants  to connect as. The
 *           Implementation is responsible for securely ensuring that the Client
 *           Application instance is actually a member of this group.
 *     TEEC_LOGIN_APPLICATION
 *        o  connectionData SHOULD be NULL.
 *     TEEC_LOGIN_USER_APPLICATION
 *        o  connectionData SHOULD be NULL.
 *     TEEC_LOGIN_GROUP_APPLICATION : Unsupported for QTEE/default context
 *        o  connectionData MUST point to a uint32_t which contains the group
 *           which this Client Application wants to connect as. The
 *           Implementation is responsible for securely ensuring that the Client
 *           Application instance is actually a member of this group.
 *
 * User MAY optionally send an Operation Payload, and  MAY also cancel it.
 * When the payload is present, the parameter operation MUST point to a
 * TEEC_Operation structure populated by the Client Application.
 * If operation is NULL then no data buffers are exchanged with the
 * Trusted Application, and the operation cannot be cancelled by the Client
 * Application.
 *
 * The result of this function is returned both in the function TEEC_Result
 * return code and the return orig stored in the variable pointed to by
 * returnOrigin.
 *
 * @param[in] context: pointer to an initialized TEE Context
 * @param[in] session: pointer to a Session structure to open.
 * @param[in] destination: pointer to a structure containing the UUID of the
 *                destination Trusted Application.
 * @param[in] connectionMethod: the method of connection to use.
 * @param[in] connectionData: any necessary data required to support the
 *            connection method chosen
 * @param[in] operation:  pointer to an Operation containing a set of Parameters
 *            to exchange with the Trusted Application, or NULL if no Parameters
 *            are to be exchanged or if the operation cannot be cancelled. Refer
 *            to TEEC_InvokeCommand for more details.
 * @param[in] returnOrigin: pointer to a variable which will contain the return
 *            origin. This field may be NULL if the return origin is not needed.
 * @return: If the returnOrigin is different from TEEC_ORIGIN_TRUSTED_APP,
 *          an error code  from above is returned.
 *          If the returnOrigin is equal to TEEC_ORIGIN_TRUSTED_APP, a return
 *          code defined by the protocol between the Client Application and the
 *          Trusted Application. In any case, a return code set to TEEC_SUCCESS
 *          means that the session was successfully opened and a return code
 *          different from TEEC_SUCCESS means that the session opening failed.
 *
 * Programmer Error: The following usage of the API is a programmer error:
 *    Calling with a context which is not initialized.
 *    Calling with a connectionData set to NULL if connection data is required
 *      by the specified connection method
 *    Calling with an operation containing an invalid  paramTypes field, i.e.,
 *      containing a reserved parameter type or where a parameter type that
 *      conflicts with the parent Shared Memory.
 *    Encoding Registered Memory References which refer to Shared Memory blocks
 *      allocated within the scope of a different TEE Context.
 *    Attempting to open a Session using the same Session structure concurrently
 *      from multiple threads. Multi-threaded Client Applications must use
 *      platform -provided locking mechanismso ensure that this case does
 *      not occur.
 *    Using the same Operation structure for multiple concurrent operations.
 */
TEEC_Result TEEC_OpenSession(TEEC_Context *context, TEEC_Session *session,
				const TEEC_UUID *destination,
				uint32_t connectionMethod,
				const void *connectionData,
				TEEC_Operation *operation,
				uint32_t *returnOrigin);

/**
 * @brief
 *
 * This function closes a Session which has been opened with a Trusted
 * Application.
 * All Commands within the Session MUST have completed before calling this
 * function. This API does nto do anything if the session parameter is NULL.
 * This API does not return any failure: after  this  function returns the
 * Client Application should consider that the Session has been closed.
 *
 * @param[in] session: pointer to a Session to close.
 * @return: No return value
 *
 * Programmer Error: The following usage of the API is a programmer error:
 *    Calling with a session which still has commands running.
 *    Attempting to close the same Session concurrently from multiple threads.
 *    Attempting to close the same Session more than once.
 */
void TEEC_CloseSession(TEEC_Session *session);


/**
 * @brief
 *
 * This function invokes a Command within the specified Session.
 * The parameter session MUST point to a valid open Session.
 * The parameter commandID is an identifier that is used to indicate which
 * of the exposed TrustedApplication functions should be invoked. The supported
 * command identifiers are defined by the Trusted Application?s protocol.
 *
 * Operation Handling
 * A Command MAY optionally carry an Operation Payload.  When the payload is
 * present the parameter operation MUST point to a TEEC_Operation structure
 * populated by the Client Application.
 * If operation is NULL then no parameters are exchanged with the Trusted
 * Application, and only the Command ID is exchanged.
 * The operation structure is also used to manage cancellation of the Command.
 * If cancellation is required then the operation pointer MUST be non-NULL and
 * the Client Application MUST have zeroed the started field of the operation
 * structure before calling this function.
 *
 * NOTE: Cancellation of pending operations is unsupported for QTEE/default
 *         context
 *
 * The operation structure MAY contain no Parameters if no data payload is to
 * be exchanged.
 *
 * The result of this function is returned both in the function TEEC_Result
 * return code and the return orig stored in the variable pointed to by
 * returnOrigin:
 * If the return origin is different from TEEC_ORIGIN_TRUSTED_APP, the return
 *    TEEC_ERROR_CANCEL code MUST be one of the error codes defined above.
 * If the return code is then means that the operation was cancelled
 *    before it reached the Trusted Application.
 * If the return origin is TEEC_ORIGIN_TRUSTED_APP, the meaning of the return
 *    code depends on the protocol between the Client Application and the
 *    Trusted  Application. If TEEC_SUCCESS is returned, it always means the
 *    operatin was succesful. However, and if the function returns a code
 *    different from TEEC_SUCCESS, it means that the operation failed.
 *
 * @param[in] session: the open Session in which the command will be invoked.
 * @param[in] commandID: the identifier of the Command within the Trusted
 *       Application to invoke.  The meaning of each Command Identifier must be
 *       defined in the protocol exposed by the Trusted Application
 * @param[in] operation:  pointer to an Operation containing a set of Parameters
 *       to exchange with the Trusted Application, or  NULL  if no Parameters
 *       are to be exchangedor if the operation cannot be cancelled. Refer to
 *       TEEC_InvokeCommand for more details.
 * @param[in] returnOrigin: pointer to a variable which will contain the return.
 *       origin. This field may be  NULL if the return origin is not needed.
 * @return: If the returnOrigin is different from TEEC_ORIGIN_TRUSTED_APP,
 *       an error code  from above is returned.
 *       If the returnOrigin is equal to TEEC_ORIGIN_TRUSTED_APP, a return
 *       code defined by  the protocol between the Client Application and
 *       the Trusted Application.
 *
 * Programmer Error: The following usage of the API is a programmer error:
 *    Calling with a session which is not an open Session.
 *    Calling with invalid content in the paramTypes field of the operation
 *       structure.
 *       This invalid behavior includes types which are RFU or which conflict
 *       with the flags of the parent Shared Memory block.
 *    Encoding Registered Memory References which refer to Shared Memory blocks
 *       allocated or registered within the scope of a different TEE Context.
 *    Using the same operation structure concurrently for multiple operations,
 *       whether open Session operations or Command invocations.
 */
TEEC_Result TEEC_InvokeCommand(TEEC_Session *session, uint32_t commandID,
		TEEC_Operation *operation, uint32_t *returnOrigin);


void TEEC_RequestCancellation(TEEC_Operation *operation);


#ifdef __cplusplus
}
#endif

#endif
