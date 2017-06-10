
#ifndef QCRIL_DB_H
#define QCRIL_DB_H

#include <stdio.h>
#include <sqlite3.h>

#include "qcril_log.h"
#include "qcril_qmi_nas.h"


#define QCRIL_MAX_EMERGENCY_NUMBERS_LEN 200
#define QCRIL_MAX_IMS_ADDRESS_LEN 81
#define QCRIL_DB_MAX_OPERATOR_TYPE_LEN (6)

int qcril_db_init(void);

/*===========================================================================

  FUNCTION  qcril_db_is_mcc_part_of_emergency_numbers_table

===========================================================================*/
/*!
    @brief
    Checks for mcc existence in db and retireves emergency number from db.

    @return
    0 if function is successful.
*/
/*=========================================================================*/
int qcril_db_is_mcc_part_of_emergency_numbers_table
(
    qmi_ril_custom_emergency_numbers_source_type source,
    char *mcc,
    char *emergency_num
);

/*===========================================================================

  FUNCTION  qcril_db_is_mcc_part_of_emergency_numbers_table_with_service_state

===========================================================================*/
/*!
    @brief
    Checks for mcc & service state existence in db and retrives
    emergency number from db.

    @return
    0 if function is successful.
*/
/*=========================================================================*/
int qcril_db_is_mcc_part_of_emergency_numbers_table_with_service_state
(
    qmi_ril_custom_emergency_numbers_source_type source,
    char *mcc,
    char *service_state,
    char *emergency_num
);

/*===========================================================================

  FUNCTION  qcril_db_is_number_mcc_part_of_emergency_numbers_table

===========================================================================*/
/*!
    @brief
    Checks for mcc and number existence in db

    @return
    0 if function is successful.
*/
/*=========================================================================*/
int qcril_db_is_number_mcc_part_of_emergency_numbers_table
(
    char *emergency_num,
    char *mcc,
    qmi_ril_custom_emergency_numbers_source_type source
);

/*===========================================================================

  FUNCTION  qcril_db_is_number_mcc_part_of_emergency_numbers_table_with_service_state

===========================================================================*/
/*!
    @brief
    Checks for mcc and number existence based on service state

    @return
    0 if function is successful.
*/
/*=========================================================================*/
int qcril_db_is_number_mcc_part_of_emergency_numbers_table_with_service_state
(
    char *emergency_num,
    char *mcc,
    qmi_ril_custom_emergency_numbers_source_type source,
    char *service
);

/*===========================================================================

  FUNCTION  qcril_db_is_ims_address_for_mcc_emergency_number_part_of_emergency_numbers_table

===========================================================================*/
/*!
    @brief
    Checks for mcc and emergency number existence in db and retrieves
    corresponding ims_address (if present) from db.

    @return
    TRUE if function is successful.
*/
/*=========================================================================*/
int qcril_db_is_ims_address_for_mcc_emergency_number_part_of_emergency_numbers_table
(
    qmi_ril_custom_emergency_numbers_source_type source,
    char *mcc,
    char *emergency_num,
    char *ims_address
);


/*===========================================================================

  FUNCTION  qcril_db_query_escv_type

===========================================================================*/
/*!
    @brief
    Query ESCV type nased upon iin or (mcc, mnc)

    @return
    escv type
*/
/*=========================================================================*/
int qcril_db_query_escv_type
(
    char *emergency_num,
    char *iin,
    char *mcc,
    char *mnc,
    char *roam
);

/*===========================================================================

  FUNCTION  qcril_db_query_properties_table

===========================================================================*/
/*!
    @brief
    Query property table

    @return
    None
*/
/*=========================================================================*/
void qcril_db_query_properties_table
(
    char *property_name,
    char *value
);


/*===========================================================================

  FUNCTION  qcril_db_query_operator_type

===========================================================================*/
/*!
    @brief
    Query operator type based upon (mcc, mnc)

    @output
    string 3gpp or 3gpp2
*/
/*=========================================================================*/
void qcril_db_query_operator_type
(
    char *mcc,
    char *mnc,
    char operator_type[QCRIL_DB_MAX_OPERATOR_TYPE_LEN]
);


/*===========================================================================

  FUNCTION  qcril_db_insert_operator_type

===========================================================================*/
/*!
    @brief
    Insert operator type, mcc and mnc

    @return
    E_SUCCESS or E_FAILURE
*/
/*=========================================================================*/
int qcril_db_insert_operator_type
(
    char *mcc,
    char *mnc,
    char operator_type[QCRIL_DB_MAX_OPERATOR_TYPE_LEN]
);


/*===========================================================================

  FUNCTION  qcril_db_reset_cleanup

===========================================================================*/
/*!
    @brief
    Reset all global vars and release database

    @return
    0 on success
*/
/*=========================================================================*/
int qcril_db_reset_cleanup();

#endif /* QCRIL_DB_H */
