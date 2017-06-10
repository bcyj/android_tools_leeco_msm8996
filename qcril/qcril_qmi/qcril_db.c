/******************************************************************************

  @file    qcril_db.c
  @brief   Provides interface to communicate with qcril db tables

  DESCRIPTION
    Initialize sqlite db
    Create qcril db tables
    Provides interface to query db tables

  ---------------------------------------------------------------------------

  Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

  ---------------------------------------------------------------------------
******************************************************************************/

#include "qcril_db.h"

#ifdef QMI_RIL_UTF
#define QCRIL_DATABASE_NAME "./qcril.db"
#else
#define QCRIL_DATABASE_NAME "/data/misc/radio/qcril.db"
#endif


/* QCRIL DB handle */
sqlite3* qcril_db_handle = NULL;

typedef enum
{
    QCRIL_DB_TABLE_FIRST = 0,
    QCRIL_DB_TABLE_OPERATOR_TYPE = QCRIL_DB_TABLE_FIRST,
    QCRIL_DB_SIG_CONFIG_TYPE,
    QCRIL_DB_TABLE_MAX
} qcril_db_table_type;

typedef struct qcril_db_table_info {
    char *table_name;
    char *create_stmt;
} qcril_db_table_info;

typedef struct qcril_db_escv_in_out {
    char *mnc;
    int escv_type;
} qcril_db_escv_in_out;


#define QCRIL_PROPERTIES_TABLE_NAME "qcril_properties_table"

/* Statement to create qcril db tables */
#define QCRIL_CREATE_EMRGENCY_TABLE    \
            "create table if not exists %s" \
            "(MCC TEXT, NUMBER TEXT, IMS_ADDRESS TEXT, SERVICE TEXT, PRIMARY KEY(MCC,NUMBER))"

/* Statement to create qcril db escv iin table */
#define QCRIL_DB_CREATE_ESCV_IIN_TABLE  \
            "create table if not exists %s" \
            "(IIN TEXT, NUMBER TEXT, ESCV INTEGER, ROAM TEXT, PRIMARY KEY(IIN,NUMBER,ROAM))"

/* Statement to create qcril db escv nw table */
#define QCRIL_DB_CREATE_ESCV_NW_TABLE  \
            "create table if not exists %s"\
            "(MCC TEXT, MNC TEXT, NUMBER TEXT, ESCV INTEGER, PRIMARY KEY(MCC,NUMBER, ESCV))"

/* Statement to create qcril db escv nw table */
#define QCRIL_DB_CREATE_PROPERTIES_TABLE  \
            "create table if not exists %s"\
            "(PROPERTY TEXT,VALUE TEXT, PRIMARY KEY(PROPERTY))"

/* Statement to create qcril db operator type table */
#define QCRIL_DB_CREATE_OPERATOR_TYPE_TABLE  \
            "create table if not exists %s"\
            "(MCC TEXT, MNC TEXT, TYPE TEXT, PRIMARY KEY(MCC,MNC))"

/* Statement to create qcril db sig config table */
#define QCRIL_DB_CREATE_SIG_CONFIG_TABLE  \
            "create table if not exists %s "\
            "(SIG_CONFIG_TYPE TEXT, DELTA TEXT, PRIMARY KEY(SIG_CONFIG_TYPE))"

/* Table containing qcril db emergency table names */
qcril_db_table_info qcril_db_emergency_number_tables[QMI_RIL_CUSTOM_EMERGENCY_NUMBERS_SOURCE_MAX] =
{
    [QMI_RIL_CUSTOM_EMERGENCY_NUMBERS_SOURCE_MCC]       =
                 {  "qcril_emergency_source_mcc_table", QCRIL_CREATE_EMRGENCY_TABLE },
    [QMI_RIL_CUSTOM_EMERGENCY_NUMBERS_SOURCE_VOICE]    =
                 { "qcril_emergency_source_voice_table", QCRIL_CREATE_EMRGENCY_TABLE },
    [QMI_RIL_CUSTOM_EMERGENCY_NUMBERS_SOURCE_HARD_MCC] =
                 {"qcril_emergency_source_hard_mcc_table", QCRIL_CREATE_EMRGENCY_TABLE },
    [QMI_RIL_CUSTOM_EMERGENCY_NUMBERS_SOURCE_NW]       =
                 { "qcril_emergency_source_nw_table", QCRIL_CREATE_EMRGENCY_TABLE },
    [QMI_RIL_CUSTOM_EMERGENCY_NUMBERS_SOURCE_ESCV_IIN] =
                 { "qcril_emergency_source_escv_iin_table", QCRIL_DB_CREATE_ESCV_IIN_TABLE },
    [QMI_RIL_CUSTOM_EMERGENCY_NUMBERS_SOURCE_ESCV_NW]  =
                 { "qcril_emergency_source_escv_nw_table", QCRIL_DB_CREATE_ESCV_NW_TABLE },
};

/* Table containing qcril db table names */
qcril_db_table_info qcril_db_tables[QCRIL_DB_TABLE_MAX] =
{
    [QCRIL_DB_TABLE_OPERATOR_TYPE]  =
                 { "qcril_operator_type_table", QCRIL_DB_CREATE_OPERATOR_TYPE_TABLE},
    [QCRIL_DB_SIG_CONFIG_TYPE]  =
                 { "qcril_sig_config_table", QCRIL_DB_CREATE_SIG_CONFIG_TABLE},
};

#define QCRIL_DB_MAX_STMT_LEN 300
#define RESERVED_TO_STORE_LENGTH 4

/* Query statement to query emergency number */
static char* qcril_db_query_number_and_mcc_stmt =
                      "select NUMBER from %s where MCC='%s' and NUMBER='%s'";

/* Query statement to query emergency number*/
static char* qcril_db_query_number_and_mcc_and_service_stmt =
                      "select NUMBER from %s where MCC='%s' and NUMBER='%s' and SERVICE='%s'";

/* Query statement to query emergency number and mcc */
static char* qcril_db_query_emergency_number_stmt =
                      "select NUMBER from %s where MCC='%s'";

/* Query statement to query emergency number */
static char* qcril_db_query_number_from_mcc_and_service_stmt =
                      "select NUMBER from %s where MCC='%s' and SERVICE='%s'";

/* Query statement to query ims_address from mcc and emergency number */
static char* qcril_db_query_ims_address_from_mcc_number_stmt =
                      "select IMS_ADDRESS from %s where MCC='%s' and NUMBER='%s'";

/* Emergency numbers retrieved */
static char qcril_db_emergency_numbers[QCRIL_MAX_EMERGENCY_NUMBERS_LEN] = {0};

/* Query statement to query emergency number escv type using iin */
static char* qcril_db_query_escv_iin_stmt =
                      "select ESCV from %s where IIN='%s' and NUMBER='%s' and ROAM='%s'";

/* Query statement to query emergency number escv type using mcc and mnc */
static char* qcril_db_query_escv_nw_stmt   =
                      "select ESCV, MNC from %s where MCC='%s' and NUMBER='%s'";

/* Query statement to query emergency number escv type using iin */
static char* qcril_db_query_properties_stmt =
                      "select VALUE from %s where PROPERTY='%s'";

/* Query statement to query operator type using mcc and mnc */
static char* qcril_db_query_operator_type_stmt =
                      "select TYPE from %s where MCC='%s' and MNC='%s'";

/* Insert statement to insert operator type, mcc and mnc */
static char* qcril_db_insert_operator_type_stmt =
                      "insert into %s values('%s', '%s', '%s')";

/* Query statement from sig config*/
static char* qcril_db_query_sig_config_stmt =
                      "select DELTA from %s where SIG_CONFIG_TYPE='%s'";

/* Emergency numbers retrieved */
static int qcril_db_emergency_numbers_escv_type = 0;
static int qcril_db_query_result = 0;

/*===========================================================================

  FUNCTION  qcril_db_retrieve_emergency_num_callback

===========================================================================*/
/*!
    @brief
    Retireves emergency number from db output.

    @return
    0 if function is successful.
*/
/*=========================================================================*/
static int qcril_db_retrieve_emergency_num_callback
(
    void   *data,
    int     argc,
    char  **argv,
    char  **azColName
)
{
    int     tmp_len = 0;
    char   *ptr;
    uint32_t len;

    QCRIL_NOTUSED(azColName);

    if (data)
    {
        QCRIL_LOG_INFO("argc %d argv[0] %s", argc, argv[0] ? argv[0] : "null");
        len     = *((uint32_t*)data);
        ptr     = (char*)data + (RESERVED_TO_STORE_LENGTH + len);
        if (argc == 1 && (len < QCRIL_MAX_EMERGENCY_NUMBERS_LEN) && argv[0])
        {
            if (len != 0)
            {
                tmp_len = snprintf(ptr,
                            (QCRIL_MAX_EMERGENCY_NUMBERS_LEN - len), "%s", ",");
                len = len + tmp_len;
                ptr = ptr + tmp_len;
            }

            tmp_len = snprintf(ptr,
                         (QCRIL_MAX_EMERGENCY_NUMBERS_LEN - len), "%s", argv[0]);

            len = len + tmp_len;
            *((int*)data) = len;
        }
    }

    return 0;
}

/*===========================================================================

  FUNCTION  qcril_db_check_num_and_mcc_callback

===========================================================================*/
/*!
    @brief
    checks existence of emergency number and mcc in a table.

    @return
    0 if function is successful.
*/
/*=========================================================================*/
static int qcril_db_check_num_and_mcc_callback
(
    void   *data,
    int     argc,
    char  **argv,
    char  **azColName
)
{

    QCRIL_NOTUSED(azColName);

    if ((argc > 0) && strlen(argv[0]) > 0 && data)
    {
        *(int*)data = 1;
    }

    return 0;
}

/*===========================================================================

  FUNCTION  qcril_db_check_escv_callback

===========================================================================*/
/*!
    @brief
    checks existence of escv row in a table.

    @return
    0 if function is successful.
*/
/*=========================================================================*/
static int qcril_db_check_escv_callback
(
    void   *data,
    int     argc,
    char  **argv,
    char  **azColName
)
{
    int ret = -1;
    qcril_db_escv_in_out *result = data;
    int escv = 0;

    QCRIL_NOTUSED(azColName);

    if (result)
    {
        escv = atoi(argv[0]);
        if (argc == 1)
        {
            QCRIL_LOG_INFO("argc %d argv[0] %s", argc, argv[0] ? argv[0] : "null");
            if (escv >= 0)
            {
                result->escv_type  = escv;
                ret = 0;
            }
        }
        else if (argc == 2)
        {
            QCRIL_LOG_INFO("argc %d argv[0] %s argv[1] %s",
                                 argc, argv[0] ? argv[0] : "null",
                                 argv[1] ? argv[1] : "null");
            if (escv >= 0)
            {
                if ((result->mnc) && (argv[1]))
                {
                    if (!strcmp(result->mnc, argv[1]))
                    {
                        result->escv_type  = escv;
                    }
                }
                else
                {
                    result->escv_type  = escv;
                }
                ret = 0;
             }
         }
    }

    return ret;
}

/*===========================================================================

  FUNCTION  qcril_db_retrieve_ims_address_from_mcc_emergency_num_callback

===========================================================================*/
/*!
    @brief
    Retireves ims_address from mcc and emergency number from db output.

    @return
    0 if function is successful.
*/
/*=========================================================================*/
static int qcril_db_retrieve_ims_address_from_mcc_emergency_num_callback
(
    void   *data,
    int     argc,
    char  **argv,
    char  **azColName
)
{
    int     tmp_len = 0;
    char   *ptr;
    int     len;

    QCRIL_NOTUSED(azColName);

    if (data)
    {
        QCRIL_LOG_INFO("argc %d argv[0] %s", argc, argv[0] ? argv[0] : "null");
        ptr = (char*)data;
        *ptr = 0;

        if (argc == 1 && argv[0])
        {
            len = strlen(argv[0]);
            if (len > 0 && len < QCRIL_MAX_IMS_ADDRESS_LEN)
            {
                snprintf(ptr,
                         QCRIL_MAX_IMS_ADDRESS_LEN,
                         "%s",
                         argv[0]);
            }
        }
    }

    return 0;
}

/*===========================================================================

  FUNCTION  qcril_db_query_property_callback

===========================================================================*/
/*!
    @brief
    update property.

    @return
    0 if function is successful.
*/
/*=========================================================================*/
static int qcril_db_query_property_callback
(
    void   *data,
    int     argc,
    char  **argv,
    char  **azColName
)
{
    int ret = 0;
    int len = 0;

    if ( data )
    {
        if ( argc == 1 && argv[0] )
        {
            len = strlen(argv[0]);
            strlcpy(data,argv[0],len+1);
        }
    }

    return ret;
}



/*===========================================================================

  FUNCTION  qcril_db_init

===========================================================================*/
/*!
    @brief
    Initialize qcril db and tables.

    @return
    0 if function is successful.
*/
/*=========================================================================*/
int qcril_db_init
(
    void
)
{
    int     res     = SQLITE_OK;
    char   *zErrMsg = NULL;

    char    create_stmt[QCRIL_DB_MAX_STMT_LEN] = {0};
    int     i;

    QCRIL_LOG_FUNC_ENTRY();

    /* initialize sqlite engine */
    if (SQLITE_OK != (res = sqlite3_initialize()))
    {
        QCRIL_LOG_ERROR("Failed to initialize sqlite3: %d\n", res);
    }
    else
    {
        /* open qcril DB */
        if (SQLITE_OK !=
                 (res = sqlite3_open_v2(QCRIL_DATABASE_NAME,
                         &qcril_db_handle,
                          SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL)))
        {
            QCRIL_LOG_ERROR("Failed to open qcril db %d\n", res);
        }
        else
        {
            for (i = QMI_RIL_CUSTOM_EMERGENCY_NUMBERS_SOURCE_MCC;
                 i < (QMI_RIL_CUSTOM_EMERGENCY_NUMBERS_SOURCE_MAX); i++)
            {
                if (qcril_db_emergency_number_tables[i].table_name &&
                     qcril_db_emergency_number_tables[i].create_stmt)
                {
                    snprintf(create_stmt, QCRIL_DB_MAX_STMT_LEN,
                             qcril_db_emergency_number_tables[i].create_stmt,
                             qcril_db_emergency_number_tables[i].table_name);

                    /* create qcril DB tables */
                    if (SQLITE_OK !=
                             (res = sqlite3_exec(qcril_db_handle,
                                     create_stmt, NULL, NULL, &zErrMsg)))
                    {
                        if (zErrMsg)
                        {
                            QCRIL_LOG_ERROR("Could not create table %d %s",
                                             res, zErrMsg);
                            sqlite3_free(zErrMsg);
                        }
                    }
                }
                memset(create_stmt,0,sizeof(create_stmt));
            }

            for (i = QCRIL_DB_TABLE_FIRST;
                 i < QCRIL_DB_TABLE_MAX; i++)
            {
                if (qcril_db_tables[i].table_name &&
                     qcril_db_tables[i].create_stmt)
                {
                    snprintf(create_stmt, QCRIL_DB_MAX_STMT_LEN,
                             qcril_db_tables[i].create_stmt,
                             qcril_db_tables[i].table_name);

                    /* create qcril DB tables */
                    if (SQLITE_OK !=
                             (res = sqlite3_exec(qcril_db_handle,
                                     create_stmt, NULL, NULL, &zErrMsg)))
                    {
                        if (zErrMsg)
                        {
                            QCRIL_LOG_ERROR("Could not create table %d %s",
                                             res, zErrMsg);
                            sqlite3_free(zErrMsg);
                        }
                    }
                }

                memset(create_stmt,0,sizeof(create_stmt));
            }

            // create rat tlv table.
            snprintf(create_stmt,
                     QCRIL_DB_MAX_STMT_LEN,
                     QCRIL_DB_CREATE_PROPERTIES_TABLE,
                     QCRIL_PROPERTIES_TABLE_NAME);

            if (SQLITE_OK != (res = sqlite3_exec(qcril_db_handle,
                                                 create_stmt,
                                                 NULL,
                                                 NULL,
                                                 &zErrMsg
                                                 )
                             )
               )
            {
                if (zErrMsg)
                {
                    QCRIL_LOG_ERROR("Could not create table %d %s",
                                     res, zErrMsg);
                    sqlite3_free(zErrMsg);
                }
            }
        }
    }

    QCRIL_LOG_FUNC_RETURN();
    return res;
}

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
)
{
    char    query[QCRIL_DB_MAX_STMT_LEN] = {0};
    int     res     = FALSE;
    int     ret     = SQLITE_OK;
    char   *zErrMsg = NULL;
    char    emergency_numbers[QCRIL_MAX_EMERGENCY_NUMBERS_LEN + RESERVED_TO_STORE_LENGTH] = {0};

    QCRIL_LOG_FUNC_ENTRY();
    QCRIL_LOG_INFO("Source %d MCC %s", source, mcc? mcc: "null");

    if ((source < QMI_RIL_CUSTOM_EMERGENCY_NUMBERS_SOURCE_MAX) &&
         mcc && qcril_db_emergency_number_tables[source].table_name
         && emergency_num)
    {

        snprintf(query, QCRIL_DB_MAX_STMT_LEN,
                 qcril_db_query_emergency_number_stmt,
                 qcril_db_emergency_number_tables[source].table_name, mcc);

        if (SQLITE_OK != (ret = sqlite3_exec(qcril_db_handle, query,
                                  qcril_db_retrieve_emergency_num_callback,
                                  emergency_numbers, &zErrMsg)))
        {
            if (zErrMsg)
            {
                QCRIL_LOG_ERROR("Could not query %d %s", ret, zErrMsg);
                sqlite3_free(zErrMsg);
            }
        }
        else
        {
            if ( *((int*)emergency_numbers) > 0 )
            {
                res = TRUE;
                strlcpy(emergency_num, emergency_numbers + RESERVED_TO_STORE_LENGTH,
                         QCRIL_MAX_EMERGENCY_NUMBERS_LEN);
                QCRIL_LOG_INFO("Emergency numbers %s", emergency_num);
            }
        }
    }

    QCRIL_LOG_FUNC_RETURN_WITH_RET(res);
    return res;
}

/*===========================================================================

  FUNCTION  qcril_db_is_mcc_part_of_emergency_numbers_table_with_service_state

===========================================================================*/
/*!
    @brief
    Checks for mcc & service state existence in db and retireves
    emergency number from db.

    @return
    0 if function is successful.
*/
/*=========================================================================*/
int qcril_db_is_mcc_part_of_emergency_numbers_table_with_service_state
(
    qmi_ril_custom_emergency_numbers_source_type source,
    char *mcc,
    char *service,
    char *emergency_num
)
{
    char    query[QCRIL_DB_MAX_STMT_LEN] = {0};
    int     res     = FALSE;
    int     ret     = SQLITE_OK;
    char   *zErrMsg = NULL;
    char    emergency_numbers[QCRIL_MAX_EMERGENCY_NUMBERS_LEN + RESERVED_TO_STORE_LENGTH] = {0};

    QCRIL_LOG_FUNC_ENTRY();
    QCRIL_LOG_INFO("Source %d MCC %s", source, mcc? mcc: "null");

    if ((source < QMI_RIL_CUSTOM_EMERGENCY_NUMBERS_SOURCE_MAX) &&
         mcc && service &&qcril_db_emergency_number_tables[source].table_name
         && emergency_num)
    {

        snprintf(query, QCRIL_DB_MAX_STMT_LEN,
                 qcril_db_query_number_from_mcc_and_service_stmt,
                 qcril_db_emergency_number_tables[source].table_name,
                 mcc, service);

        if (SQLITE_OK != (ret = sqlite3_exec(qcril_db_handle, query,
                                  qcril_db_retrieve_emergency_num_callback,
                                  emergency_numbers, &zErrMsg)))
        {
            if (zErrMsg)
            {
                QCRIL_LOG_ERROR("Could not query %d %s", ret, zErrMsg);
                sqlite3_free(zErrMsg);
            }
        }
        else
        {
            if ( *((int*)emergency_numbers) > 0 )
            {
                res = TRUE;
                strlcpy(emergency_num, emergency_numbers + RESERVED_TO_STORE_LENGTH,
                         QCRIL_MAX_EMERGENCY_NUMBERS_LEN);
                QCRIL_LOG_INFO("Emergency numbers %s", emergency_num);
            }
        }
    }

    QCRIL_LOG_FUNC_RETURN_WITH_RET(res);
    return res;
}

/*===========================================================================

  FUNCTION  qcril_db_query_number_from_emergency_table

===========================================================================*/
/*!
    @brief
    Checks whether number present in table as per query

    @return
    0 if function is successful.
*/
/*=========================================================================*/
int qcril_db_query_number_from_emergency_table
(
    char *query,
    char *is_num_present
)
{
    int     ret     = SQLITE_OK;
    char   *zErrMsg = NULL;

    QCRIL_LOG_FUNC_ENTRY();

    if ( query && is_num_present )
    {
        if (SQLITE_OK != (ret = sqlite3_exec(qcril_db_handle, query,
                                         qcril_db_check_num_and_mcc_callback,
                                         is_num_present, &zErrMsg)))
        {
            if (zErrMsg)
            {
                QCRIL_LOG_ERROR("Could not query %d %s", ret, zErrMsg);
                sqlite3_free(zErrMsg);
            }
        }
    }

    QCRIL_LOG_FUNC_RETURN_WITH_RET(ret);
    return ret;
}

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
)
{
    char    query[QCRIL_DB_MAX_STMT_LEN] = {0};
    int     res     = FALSE;
    int     ret     = SQLITE_OK;
    char   *zErrMsg = NULL;

    QCRIL_LOG_FUNC_ENTRY();

    if ((source < QMI_RIL_CUSTOM_EMERGENCY_NUMBERS_SOURCE_MAX) &&
         mcc && emergency_num &&
         qcril_db_emergency_number_tables[source].table_name)
    {
        QCRIL_LOG_INFO("Source %d MCC %s emergency num %s", source, mcc, emergency_num);

        snprintf(query, QCRIL_DB_MAX_STMT_LEN,
                 qcril_db_query_number_and_mcc_stmt,
                 qcril_db_emergency_number_tables[source].table_name,
                 mcc,
                 emergency_num);

        qcril_db_query_number_from_emergency_table(query,&res);
    }

    QCRIL_LOG_FUNC_RETURN_WITH_RET(res);
    return res;
}

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
)
{
    char    query[QCRIL_DB_MAX_STMT_LEN] = {0};
    int     res     = FALSE;
    int     ret     = SQLITE_OK;
    char   *zErrMsg = NULL;

    QCRIL_LOG_FUNC_ENTRY();

    if ((source < QMI_RIL_CUSTOM_EMERGENCY_NUMBERS_SOURCE_MAX) &&
         mcc && emergency_num &&
         qcril_db_emergency_number_tables[source].table_name)
    {
        QCRIL_LOG_INFO("Source %d MCC %s emergency num %s", source, mcc, emergency_num);

        snprintf(query, QCRIL_DB_MAX_STMT_LEN,
                 qcril_db_query_number_and_mcc_and_service_stmt,
                 qcril_db_emergency_number_tables[source].table_name,
                 mcc,
                 emergency_num,
                 service);

        qcril_db_query_number_from_emergency_table(query,&res);
    }

    QCRIL_LOG_FUNC_RETURN_WITH_RET(res);
    return res;
}



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
)
{
    char    query[QCRIL_DB_MAX_STMT_LEN] = {0};
    int     res     = FALSE;
    int     ret     = SQLITE_OK;
    char   *zErrMsg = NULL;

    QCRIL_LOG_FUNC_ENTRY();
    QCRIL_LOG_INFO("Source %d MCC %s emergency_num %s",
                   source,
                   mcc? mcc: "null",
                   emergency_num? emergency_num: "null");

    if ((source < QMI_RIL_CUSTOM_EMERGENCY_NUMBERS_SOURCE_MAX) &&
         mcc && emergency_num && ims_address && qcril_db_emergency_number_tables[source].table_name)
    {

        snprintf(query, QCRIL_DB_MAX_STMT_LEN,
                 qcril_db_query_ims_address_from_mcc_number_stmt,
                 qcril_db_emergency_number_tables[source].table_name,
                 mcc,
                 emergency_num);

        if (SQLITE_OK != (ret = sqlite3_exec(qcril_db_handle, query,
                                  qcril_db_retrieve_ims_address_from_mcc_emergency_num_callback,
                                  ims_address, &zErrMsg)))
        {
            if (zErrMsg)
            {
                QCRIL_LOG_ERROR("Could not query %d %s", ret, zErrMsg);
                sqlite3_free(zErrMsg);
            }
        }
        else
        {
            QCRIL_LOG_INFO("ims_address %s", ims_address);
            if(strlen(ims_address))
            {
                res = TRUE;
            }
        }
    }

    QCRIL_LOG_FUNC_RETURN_WITH_RET(res);
    return res;
}

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
)
{
    char    query[QCRIL_DB_MAX_STMT_LEN] = {0};
    int     res     = 0;
    int     ret     = SQLITE_OK;
    char   *zErrMsg = NULL;
    sqlite3_callback qcril_db_escv_callback = NULL;
    qcril_db_escv_in_out result = {0};
    qmi_ril_custom_emergency_numbers_source_type source;

    QCRIL_LOG_FUNC_ENTRY();

    QCRIL_LOG_INFO(" emergency_num %s iin %s mcc %s mnc %s roam %s",
                     emergency_num? emergency_num : "null",
                     iin? iin : "null",
                     mcc? mcc : "null",
                     mnc? mnc : "null",
                     roam? roam : "null");
    do {
        if (!emergency_num )
        {
            break;
        }

        if ( iin && roam )
        {
            snprintf(query, QCRIL_DB_MAX_STMT_LEN,
               qcril_db_query_escv_iin_stmt,
               qcril_db_emergency_number_tables[QMI_RIL_CUSTOM_EMERGENCY_NUMBERS_SOURCE_ESCV_IIN].table_name,
               iin,
               emergency_num,
               roam);
            result.escv_type = -1;
        }
        else if ( mcc )
        {
            snprintf(query, QCRIL_DB_MAX_STMT_LEN,
               qcril_db_query_escv_nw_stmt,
               qcril_db_emergency_number_tables[QMI_RIL_CUSTOM_EMERGENCY_NUMBERS_SOURCE_ESCV_NW].table_name,
               mcc,
               emergency_num);

            if (mnc)
            {
                result.mnc = mnc;
            }
        }
        else
        {
            break;
        }

        QCRIL_LOG_INFO(" Query %s", query);
        if (SQLITE_OK != (ret = sqlite3_exec(qcril_db_handle, query,
                                         qcril_db_check_escv_callback,
                                         &result, &zErrMsg)))
        {
            if (zErrMsg)
            {
                QCRIL_LOG_ERROR("Could not query %d %s", ret, zErrMsg);
                sqlite3_free(zErrMsg);
            }
        }
        else
        {
            res = result.escv_type;
        }
    } while (0);

    QCRIL_LOG_FUNC_RETURN_WITH_RET(res);
    return res;
}

/*===========================================================================

  FUNCTION  qcril_db_query_properties_table

===========================================================================*/
/*!
    @brief
    Query property table.
    Caller of this function should pass sufficient buffer (value)
    for storing the information retrieved from database

    @return
    None
*/
/*=========================================================================*/
void qcril_db_query_properties_table
(
    char *property_name,
    char *value
)
{
    int     res     = 0;
    char   *zErrMsg = NULL;
    int     ret     = SQLITE_OK;
    char    query[QCRIL_DB_MAX_STMT_LEN] = {0};

    QCRIL_LOG_FUNC_ENTRY();

    do
    {
        // if null pointer
        if ( !value || !property_name )
        {
            break;
        }

        /* Initialize it to default value */
        *value = '\0';

        snprintf(query,
                 QCRIL_DB_MAX_STMT_LEN,
                 qcril_db_query_properties_stmt,
                 QCRIL_PROPERTIES_TABLE_NAME,
                 property_name
                 );

        QCRIL_LOG_INFO(" Query %s", query);

        if (SQLITE_OK != (ret = sqlite3_exec(qcril_db_handle,
                                             query,
                                             qcril_db_query_property_callback,
                                             value,
                                             &zErrMsg
                                             )
                         )
           )
        {
            if (zErrMsg)
            {
                QCRIL_LOG_ERROR("Could not query %d %s", ret, zErrMsg);
                sqlite3_free(zErrMsg);
                break;
            }
        }
    } while (0);

    QCRIL_LOG_FUNC_RETURN();
}

/*===========================================================================

  FUNCTION  qcril_db_query_operator_type_callback

===========================================================================*/
/*!
    @brief
    retrieve operator type.

    @return
    0 if function is successful.
*/
/*=========================================================================*/
static int qcril_db_query_operator_type_callback
(
    void   *data,
    int     argc,
    char  **argv,
    char  **azColName
)
{
    int ret = 0;
    int len = 0;

    if (data)
    {
        if (argc == 1 && argv[0])
        {
            strlcpy(data, argv[0], QCRIL_DB_MAX_OPERATOR_TYPE_LEN);
        }
    }

    return ret;
}

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
)
{
    char    query[QCRIL_DB_MAX_STMT_LEN] = {0};
    int     res     = 0;
    int     ret     = SQLITE_OK;
    char   *zErrMsg = NULL;

    QCRIL_LOG_FUNC_ENTRY();

    QCRIL_LOG_INFO(" mcc: %s, mnc: %s",
                     mcc? mcc : "null",
                     mnc? mnc : "null");
    do {
        if (!(mcc && mnc && operator_type))
        {
            break;
        }

        snprintf(query, QCRIL_DB_MAX_STMT_LEN,
           qcril_db_query_operator_type_stmt,
           qcril_db_tables[QCRIL_DB_TABLE_OPERATOR_TYPE].table_name,
           mcc,
           mnc);

        QCRIL_LOG_INFO(" Query: %s", query);

        if (SQLITE_OK != (ret = sqlite3_exec(qcril_db_handle,
                                             query,
                                             qcril_db_query_operator_type_callback,
                                             operator_type, &zErrMsg)))
        {
            if (zErrMsg)
            {
                QCRIL_LOG_ERROR("Could not query %d %s", ret, zErrMsg);
                sqlite3_free(zErrMsg);
            }
        }

    } while (0);

    return;
}

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
)
{
    char    insert_stmt[QCRIL_DB_MAX_STMT_LEN] = {0};
    int     res     = E_FAILURE;
    int     ret     = SQLITE_OK;
    char   *zErrMsg = NULL;

    QCRIL_LOG_FUNC_ENTRY();

    QCRIL_LOG_INFO(" mcc: %s, mnc: %s, operator type %s",
                     mcc? mcc : "null",
                     mnc? mnc : "null",
                     operator_type? operator_type: "null");
    do {

        if (!(mcc && mnc && operator_type))
        {
            break;
        }

        snprintf(insert_stmt, QCRIL_DB_MAX_STMT_LEN,
                 qcril_db_insert_operator_type_stmt,
                 qcril_db_tables[QCRIL_DB_TABLE_OPERATOR_TYPE].table_name,
                 mcc, mnc, operator_type);

        QCRIL_LOG_INFO(" insert stmt: %s", insert_stmt);

        if (SQLITE_OK != (ret = sqlite3_exec(qcril_db_handle,
                                             insert_stmt,
                                             NULL,
                                             operator_type, &zErrMsg)))
        {
            if (zErrMsg)
            {
                QCRIL_LOG_ERROR("Could not query %d %s", ret, zErrMsg);
                sqlite3_free(zErrMsg);
            }

            break;
        }

        res = E_SUCCESS;

    } while (0);

    return res;
}

/*===========================================================================

  FUNCTION  qcril_db_query_sig_config_callback

===========================================================================*/
/*!
    @brief
    Call back for querying sig config

    @return
    0 if function is successful.
*/
/*=========================================================================*/
static int qcril_db_query_sig_config_callback
(
    void   *data,
    int     argc,
    char  **argv,
    char  **azColName
)
{
    int ret = 0;
    int len = 0;

    QCRIL_NOTUSED(azColName);
    if (data)
    {
        if (argc == 1 && argv[0])
        {
            *(uint16_t*)data = atoi(argv[0]);
            QCRIL_LOG_DEBUG(" data %d", *(uint16_t*)data);
        }
        else
        {
            QCRIL_LOG_DEBUG(" argc: %d, argv[0]: %p", argc, argv[0]);
        }
    }

    return ret;
}

/*===========================================================================

  FUNCTION  qcril_db_query_sig_config

===========================================================================*/
/*!
    @brief
    Query sig config

    @return
    E_SUCCESS or E_FAILURE
*/
/*=========================================================================*/
int qcril_db_query_sig_config
(
    char        *sig_config_type,
    uint16_t    *delta
)
{
    int     res     = 0;
    int     ret     = SQLITE_OK;
    char   *zErrMsg = NULL;
    char    query_stmt[QCRIL_DB_MAX_STMT_LEN] = {0};

    QCRIL_LOG_FUNC_ENTRY();

    do
    {
        if (!sig_config_type || !delta)
        {
            res = -1;
            break;
        }

        snprintf(query_stmt, QCRIL_DB_MAX_STMT_LEN,
                 qcril_db_query_sig_config_stmt,
                 qcril_db_tables[QCRIL_DB_SIG_CONFIG_TYPE].table_name,
                 sig_config_type);

        QCRIL_LOG_INFO(" Query: %s", query_stmt);

        if (SQLITE_OK != (res = sqlite3_exec(qcril_db_handle,
                                             query_stmt,
                                             qcril_db_query_sig_config_callback,
                                             delta, &zErrMsg)))
        {
            if (zErrMsg)
            {
                QCRIL_LOG_ERROR("Could not query %d %s", res, zErrMsg);
                sqlite3_free(zErrMsg);
            }
        }

    } while (0);


    QCRIL_LOG_FUNC_RETURN_WITH_RET(res);
    return res;
}


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

int qcril_db_reset_cleanup()
{
  int ret = sqlite3_close(qcril_db_handle);
  sqlite3_shutdown();
  qcril_db_emergency_numbers_escv_type = 0;
  qcril_db_query_result = 0;


  if (ret != SQLITE_OK)
  {
    return -1;
  }

  return 0;

}

