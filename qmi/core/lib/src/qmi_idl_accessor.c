/*===========================================================================

                    Q M I _ I D L _ A C C E S O R . C

  Accesor APIs for encode/decode library.


  Copyright (c) 2011-2013 Qualcomm Technologies, Inc. All Rights Reserved.
 Qualcomm Technologies Proprietary and Confidential.
===========================================================================*/
#include "qmi_idl_lib_target.h"
#include <stdint.h>
#include <string.h>

#include "qmi_idl_lib.h"
#include "qmi_idl_lib_internal.h"

/***********************************************************************

FUNCTION qmi_idl_find_msg

DESCRIPTION
  Search list of messages for message with matching message ID.

ARGUMENTS p_service - pointer to service object
          message_type - Request/Response/Indication (different list for
              each message type.
          message_id - ID to search for
          out_type_table - Output param,Pointer to referenced type table object
                           in case the message is defined in the included IDL.
                           Pointer to local type table object in case the
                           message defined is local.

RETURN VALUE pointer to start of message table entry

***********************************************************************/
extern const qmi_idl_message_table_entry *qmi_idl_find_msg
(
  const qmi_idl_service_object_type p_service,
  qmi_idl_type_of_message_type message_type,
  uint16_t message_id,
  qmi_idl_lib_exception_type *exc,
  const qmi_idl_type_table_object **out_type_table
);

/*===========================================================================
  FUNCTION  qmi_idl_get_idl_version
===========================================================================*/
int32_t qmi_idl_get_idl_version
(
  const qmi_idl_service_object_type p_service,
  uint32_t *idl_version
)
{
  int32_t ret = QMI_IDL_LIB_NO_ERR;

  if( p_service == NULL || idl_version == NULL ) {
    return QMI_IDL_LIB_PARAMETER_ERROR;
  }
  switch( p_service->library_version ) {
  case 1:
  case 2:
  case 3:
  case 4:
  case 5:
  case 6:
    *idl_version = p_service->idl_version;
    break;


  default:
    ret = QMI_IDL_LIB_UNRECOGNIZED_SERVICE_VERSION;
    break;
  }

  return ret;
} /* qmi_idl_get_idl_version */

/*===========================================================================
  FUNCTION  qmi_idl_get_idl_minor_version
===========================================================================*/
int32_t qmi_idl_get_idl_minor_version
(
  const qmi_idl_service_object_type p_service,
  uint32_t *idl_version
)
{
  int32_t ret = QMI_IDL_LIB_NO_ERR;
  if( p_service == NULL || idl_version == NULL ) {
    return QMI_IDL_LIB_PARAMETER_ERROR;
  }
  switch( p_service->library_version ) {
  case 1:
  case 2:
  case 3:
  case 4:
    ret = QMI_IDL_LIB_INCOMPATIBLE_SERVICE_VERSION;
    break;
  case 5:
  case 6:
    *idl_version = p_service->idl_minor_version;
    break;

  default:
    ret = QMI_IDL_LIB_UNRECOGNIZED_SERVICE_VERSION;
    break;
  }

  return ret;
} /* qmi_idl_get_idl_version */

/*===========================================================================
  FUNCTION  qmi_idl_get_service_id
===========================================================================*/
/*!
@brief
  Accessor function for getting the service ID from a service object.

@param[in]  service        Pointer to service object, result from service
                           object accessor function from service header file.
@param[out] service_id     Pointer to return value, the service ID

@retval    QMI_IDL_LIB_NO_ERR     Success
@retval    QMI_IDL_...    Error, see error codes defined in qmi_idl_lib.h

*/
/*=========================================================================*/
int32_t qmi_idl_get_service_id
(
  const qmi_idl_service_object_type p_service,
  uint32_t *service_id
)
{
  int32_t ret = QMI_IDL_LIB_NO_ERR;

  if( p_service == NULL || service_id == NULL ) {
    return QMI_IDL_LIB_PARAMETER_ERROR;
  }
  switch( p_service->library_version ) {
  case 1:
  case 2:
  case 3:
  case 4:
  case 5:
  case 6:
    *service_id = p_service->service_id;
    break;

  default:
    ret = QMI_IDL_LIB_UNRECOGNIZED_SERVICE_VERSION;
    break;
  }

  return ret;
} /* qmi_idl_get_service_id */

/*===========================================================================
  FUNCTION  qmi_idl_get_max_service_len
===========================================================================*/
/*!
@brief
  Accessor function for getting the maximum message length for a particular
  service.

@param[in]  service        Pointer to service object, result from service
                           object accessor function from service header file.
@param[out] service_len    Pointer to return value, the maximum message
                           length for the service.

@retval    QMI_IDL_LIB_NO_ERR     Success
@retval    QMI_IDL_...    Error, see error codes defined in qmi_idl_lib.h

*/
/*=========================================================================*/
int32_t qmi_idl_get_max_service_len
(
  const qmi_idl_service_object_type p_service,
  uint32_t *service_len
)
{
  int32_t ret = QMI_IDL_LIB_NO_ERR;

  if( p_service == NULL || service_len == NULL ) {
    return QMI_IDL_LIB_PARAMETER_ERROR;
  }

  switch( p_service->library_version ) {
  case 1:
  case 2:
  case 3:
  case 4:
  case 5:
  case 6:
    *service_len = p_service->max_msg_len;
    break;


  default:
    ret = QMI_IDL_LIB_UNRECOGNIZED_SERVICE_VERSION;
    break;
  }

  return ret;
} /* qmi_idl_get_max_service_len */

/*===========================================================================
  FUNCTION  qmi_idl_get_max_message_len
===========================================================================*/
/*!
@brief
  Accessor function for getting the maximum message length for a particular
  message.

@param[in]  service       Pointer to service object, result from service
                          object accessor function from service header file.
@param[in]  message_type  The type of message: request, response, or indication.
@param[in]  message_id    Message ID from IDL.
@param[out] message_len   Pointer to the return value, the maximum message
                          length for the service, message type, and message ID.

@retval    QMI_IDL_LIB_NO_ERR     Success
@retval    QMI_IDL_...    Error, see error codes defined in qmi_idl_lib.h

*/
/*=========================================================================*/
int32_t qmi_idl_get_max_message_len
(
  const qmi_idl_service_object_type p_service,
  qmi_idl_type_of_message_type message_type,
  uint16_t message_id,
  uint32_t *message_len
)
{
  const qmi_idl_service_message_table_entry *p_srvmsg = NULL;
  qmi_idl_service_object_type inheritance_obj;
  uint32_t i = 0;

  /* Exception handling variables */
  qmi_idl_lib_exception_type exc;

  /* Set up exception handling */
  QMI_IDL_LIB_TRY(&exc)
  {
    if( p_service == NULL ||
        message_type >= QMI_IDL_NUM_MSG_TYPES ||
        message_len == NULL )
    {
      QMI_IDL_HANDLE_ERROR(&exc, QMI_IDL_LIB_PARAMETER_ERROR, 0, 0, 0 );
    }
    inheritance_obj = p_service;
    switch( p_service->library_version )
    {
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
      while (inheritance_obj)
      {
        /* Linear search for the table entry for the message ID */
        p_srvmsg = inheritance_obj->msgid_to_msg[message_type];
        i = inheritance_obj->n_msgs[message_type];
        while (i != 0 && p_srvmsg->qmi_message_id != message_id)
        {
          ++p_srvmsg;
          --i;
        }
        if (i != 0)
        {
          break;
        }
        inheritance_obj = qmi_idl_get_inherited_service_object(inheritance_obj);
      }
      if (0 == i)
      {
        QMI_IDL_HANDLE_ERROR(&exc, QMI_IDL_LIB_MESSAGE_ID_NOT_FOUND,
                          message_id, 0, 0 );
      }

      *message_len = (uint32_t) p_srvmsg->max_msg_len;
      break;

    default:
      QMI_IDL_HANDLE_ERROR(&exc, QMI_IDL_LIB_UNRECOGNIZED_SERVICE_VERSION,
                          p_service->library_version, 0, 0 );
      break;
    }
  } QMI_IDL_LIB_CATCH(&exc) {
    return QMI_IDL_LIB_GET_ERROR(&exc);
  }

  return QMI_IDL_LIB_NO_ERR;
} /* qmi_idl_get_max_message_len */


/*===========================================================================
  FUNCTION  qmi_idl_get_message_c_struct_len
===========================================================================*/
/*!
@brief
  Accessor function for getting the c struct size for a particular
  message.

@param[in]  service       Pointer to service object, result from service
                          object accessor function from service header file.
@param[in]  message_type  The type of message: request, response, or indication.
@param[in]  message_id    Message ID from IDL.
@param[out] c_struct_len  Pointer to the return value, the c struct size for
                          structure corresponding to the service,message type
                          and message_id.

@retval    QMI_IDL_LIB_NO_ERR     Success
@retval    QMI_IDL_...    Error, see error codes defined in qmi_idl_lib.h

*/
/*=========================================================================*/
int32_t qmi_idl_get_message_c_struct_len
(
  const qmi_idl_service_object_type p_service,
  qmi_idl_type_of_message_type message_type,
  uint16_t message_id,
  uint32_t *c_struct_len
)
{
  const qmi_idl_message_table_entry *p_srvmsg;

  /* Exception handling variables */
  qmi_idl_lib_exception_type exc;

  /* Set up exception handling */
  QMI_IDL_LIB_TRY(&exc)
  {
    if( p_service == NULL ||
        message_type >= QMI_IDL_NUM_MSG_TYPES ||
        c_struct_len == NULL )
    {
      QMI_IDL_HANDLE_ERROR( &exc, QMI_IDL_LIB_PARAMETER_ERROR, 0, 0, 0 );
    }

    switch( p_service->library_version )
    {
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
      /* Find the appropriate message to return the desired c struct size */
      p_srvmsg = qmi_idl_find_msg( p_service, message_type, message_id, &exc,
                                 NULL );

      *c_struct_len = (uint32_t) p_srvmsg->c_struct_sz;
      break;
    default:
      QMI_IDL_HANDLE_ERROR( &exc, QMI_IDL_LIB_UNRECOGNIZED_SERVICE_VERSION,
                          p_service->library_version, 0, 0 );
      break;
    }
  } QMI_IDL_LIB_CATCH(&exc) {
    return QMI_IDL_LIB_GET_ERROR(&exc);
  }

  return QMI_IDL_LIB_NO_ERR;
}

/*===========================================================================
  FUNCTION  qmi_idl_get_max_c_struct_len
===========================================================================*/
/*!
@brief
  Accessor function for getting the maximum c struct size for a particular
  service.

@param[in]  service       Pointer to service object, result from service
                          object accessor function from service header file.
@param[out] c_struct_len  Pointer to the return value, the c struct size for
                          structure corresponding to the service,message type
                          and message_id.

@retval    QMI_IDL_LIB_NO_ERR     Success
@retval    QMI_IDL_...    Error, see error codes defined in qmi_idl_lib.h

*/
/*=========================================================================*/
int32_t qmi_idl_get_max_c_struct_len
(
  const qmi_idl_service_object_type p_service,
  uint32_t *c_struct_len
)
{
  const qmi_idl_message_table_entry *p_srvmsg = NULL;
  qmi_idl_service_object_type inheritance_obj;
  int max_msgs, i;

  /* Exception handling variables */
  qmi_idl_lib_exception_type exc;

  /* Set up exception handling */
  QMI_IDL_LIB_TRY(&exc) {
    if( p_service == NULL ||
        c_struct_len == NULL ) {
      QMI_IDL_HANDLE_ERROR( &exc, QMI_IDL_LIB_PARAMETER_ERROR, 0, 0, 0 );
    }
    *c_struct_len = 0;
    inheritance_obj = p_service;
    switch( p_service->library_version ) {
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
      /* Loop through all messages to find the largest c struct size */
      while (inheritance_obj)
      {
        /* Calculate the number of total messages */
        max_msgs = inheritance_obj->n_msgs[QMI_IDL_REQUEST] +
          inheritance_obj->n_msgs[QMI_IDL_RESPONSE] + inheritance_obj->n_msgs[QMI_IDL_INDICATION];
        for( i=0;i<max_msgs;++i ) {
          p_srvmsg = &(inheritance_obj->p_type_table->p_messages[i]);
          if( *c_struct_len < (uint32_t) p_srvmsg->c_struct_sz ) {
            *c_struct_len = (uint32_t) p_srvmsg->c_struct_sz;
          }
        }
        inheritance_obj = qmi_idl_get_inherited_service_object(inheritance_obj);
      }
      break;

    default:
      QMI_IDL_HANDLE_ERROR( &exc, QMI_IDL_LIB_UNRECOGNIZED_SERVICE_VERSION,
                            p_service->library_version, 0, 0 );
      break;
    }
  } QMI_IDL_LIB_CATCH(&exc) {
    return QMI_IDL_LIB_GET_ERROR(&exc);
  }

  return QMI_IDL_LIB_NO_ERR;
}

/*===========================================================================
  FUNCTION  qmi_idl_inherit_service_object
===========================================================================*/
/*!
@brief
  Inherits a parent service object

@retval    length of the standard response

@param[in/out] child_service    The service object that will be used with QCCI/QCSI
@param[in]     parent_service   The service object to inherit messages from. Parent_service_obj
                                of this field MUST be NULL.

*/
/*=========================================================================*/
int32_t qmi_idl_inherit_service_object
(
  qmi_idl_service_object_type child_service,
  qmi_idl_service_object_type parent_service
)
{
  if( !child_service || !parent_service)
  {
    return QMI_IDL_LIB_PARAMETER_ERROR;
  }
  if( child_service->library_version < 5 || parent_service->library_version < 5 )
  {
    return QMI_IDL_LIB_INCOMPATIBLE_SERVICE_VERSION;
  }
  if( parent_service->parent_service_obj )
  {
    return QMI_IDL_LIB_PARAMETER_ERROR;
  }
  if(child_service->library_version > QMI_IDL_LIB_ENCDEC_VERSION ||
     parent_service->library_version > QMI_IDL_LIB_ENCDEC_VERSION)
  {
    return QMI_IDL_LIB_UNRECOGNIZED_SERVICE_VERSION;
  }

  child_service->parent_service_obj = parent_service;
  return QMI_IDL_LIB_NO_ERR;
}

/*===========================================================================
  FUNCTION  qmi_idl_inherit_service_object
===========================================================================*/
/*!
@brief
  Returns a parent service object that was previously inherited.

@param[in] service    The service object that will be used with QCCI/QCSI

@retval inherited service object, or NULL

*/
/*=========================================================================*/
qmi_idl_service_object_type qmi_idl_get_inherited_service_object
(
  qmi_idl_service_object_type service
)
{
  if( !service || service->library_version < 5 )
  {
    return NULL;
  }
  return service->parent_service_obj;
}
