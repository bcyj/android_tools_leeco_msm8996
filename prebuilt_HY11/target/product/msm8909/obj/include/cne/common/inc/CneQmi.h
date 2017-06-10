
#ifndef CNE_QMI_H
#define CNE_QMI_H
/**
 * @file CneQmi.h
 *
 *
 * ============================================================================
 *             Copyright (c) 2011-2014 Qualcomm Technologies,
 *             Inc. All Rights Reserved.
 *             Qualcomm Technologies Confidential and Proprietary
 * ============================================================================
 */

/*----------------------------------------------------------------------------
 * Include Files
 * -------------------------------------------------------------------------*/
#include <stdlib.h>
#include <CneSrmDefs.h>
#include <CneSrm.h>
#include <qmi.h>
#include <qmi_wds_srvc.h>
#include <string>
#include <map>
#include "CneCom.h"
#include "CneDefs.h"
#ifdef __cplusplus
extern "C" {
#endif
#include "application_traffic_pairing_v01.h"
#include "qmi_client.h"
#ifdef __cplusplus
}
#endif

/*----------------------------------------------------------------------------
 * Preprocessor Definitions and Constants
 * -------------------------------------------------------------------------*/
#ifndef NELEM
#define NELEM(x) (sizeof(x)/sizeof*(x))
#endif

#ifndef IFNAMSIZ
#define IFNAMSIZ 16
#endif

/*----------------------------------------------------------------------------
 * Type Declarations
 * -------------------------------------------------------------------------*/
/**
 * @brief
 * CneQmi events type.
 * TODO: handle event registrations for each QMI port separately and
 * dispatch events as appropriate
 */
typedef enum {
  QMI_EVENT_MIN = 0,
  QMI_DORMANCY_EVENT,
  QMI_EVENT_MAX = QMI_DORMANCY_EVENT
} CneQmiEventType;


/*----------------------------------------------------------------------------
 * Class Definitions
 * -------------------------------------------------------------------------*/

class CneQmi: public EventDispatcher<CneQmiEventType> {
  public:
      /**
       * @brief Constructor
       */
    CneQmi (CneSrm &setSrm, CneCom &setCneCom, CneTimer &setTimer);
      /**
       * @brief Destructor
       */
      ~CneQmi (void);
      /**
       *  @brief Init QMI service for CNE.
       *  @return None
       */
      void init (void);

/**
 * @brief Private class to track each QMI port wds connection
 */

  private:
      /**private copy constructor* - no copies allowed */
      CneQmi (const CneQmi &);

      /**
       * @brief Private class to track each QMI port wds connection
       */
      class WdsTracker {
          public:
              /**
               * @brief Starts QMI WDS
               * @return bool true on success, false otherwise
               */
              bool startQmiWds(qmi_ip_family_pref_type ipFamily);

              /**
               * @brief Stops QMI WDS for the given port
               * @return bool true on success, false otherwise
               */
              bool stopQmiWds(void);

              /** @brief Constructor */
              WdsTracker
              (
                const char  *devId,
                qmi_ip_family_pref_type ipType,
                CneCom &setCneCom
              );

              /** @brief Destructor */
              ~WdsTracker (void);

          private:

              /**private copy constructor -- no copies allowed*/
              WdsTracker(const WdsTracker &);

              static const int DORMANT;

              static const int ACTIVE;

              int eventFd;

              CneCom &wdsCneCom;

              /**qmi port being tracked */
              const char *mQmiPort;

              /** mask of the events set on this WDS port */
              struct {
                  unsigned long mask;
                  unsigned char isSet;
              } mWdsEventConfig;

              /** QMI WDS SVC handle, unique to a QMI port */
              qmi_client_handle_type mQmiWdsSvcHandle;

              /**
               * @brief CNE QMI WDS indication handler wrapper
               * @return None
               */
              static void cneQmiWdsIndHdlr
              (
                int                           user_handle,
                qmi_service_id_type           service_id,
                void                          *user_data,
                qmi_wds_indication_id_type    ind_id,
                qmi_wds_indication_data_type  *ind_data
              );

              /**
               * @brief CNE QMI WDS indication handler
               * @return None
               */
              void processEventReportIndication
              (
                int                           userhandle,
                qmi_wds_indication_data_type  *ind_data
              );

              /**
               * @brief CNE QMI WDS Async callback function pointer type
               * @return None
               */
              static void cneQmiWdsRspCb
              (
                int                          user_handle,
                qmi_service_id_type          service_id,
                int                          sys_err_code,
                int                          qmi_err_code,
                void                         *user_data,
                qmi_wds_async_rsp_id_type    rsp_id,
                qmi_wds_async_rsp_data_type  *rsp_data
              );

              /**
               * @brief Starts Dormancy event reporting via qmi wds for the qmi
               *        port for which this tracker was created
               *
               * @return None
               */
              void setDormancyEventReport (void);
      };


      /**
       * Dev connections container, maps each QMI port with its tracker
       * This is to ensure that each port shall have only one WDS tracker.
       * There is a limit on number of QMI connections per port, hence we need
       * to be conservative and use just one tracker for all CNE needs
       */
      typedef map<std::string, WdsTracker *> WdsTrackersType;
      static WdsTrackersType mWdsTrackers;

      static int lastWwanState;

      /** reference to SRM */
      CneSrm &srm;

      CneCom &com;

      /**QMI handle, only one qmi handle per process*/
      static qmi_client_handle_type mQmiHandle;

      CneTimer &timer;

      /**
       * @brief CNE QMI system event handler wrapper
       *
       * @return None
       */
      static void cneQmiSysEventHdlr
      (
        qmi_sys_event_type        event_id,
        const qmi_sys_event_info_type   *event_info,
        void                      *user_data
      );

      /**
       * @brief QMI sys event handler
       *
       * @return None
       */
      void processSysEvent
      (
        const qmi_sys_event_info_type   *event_info
      );

      /**
       * @brief SRM event handler wrapper
       *
       * @return None
       */
      static void srmEventHandler
      (
        SrmEvent    event,
        const void  *event_data,
        void    *user_data
      );

      /**
       * @brief processes SRM events
       *
       * @return None
       */
      void processSrmEvent (SrmEvent event, const void *event_data);

      /**
       * @brief Establishes a qmi control channel for the given port only once.
       *
       * @return bool true if control channel is established, false otherwise
       */
      bool establishCtrlChannel ( const char *qmiPort );

      /**
       * @brief Creates a qmi control channel for the specified qmi port
       *
       * @return bool True on success, false otherwise
       */
      bool initQmiCtrlChannel (const char *qmiPort );

      /**
       * @brief checks for valid device id to qmi port mapping
       *
       * @return bool true if the device is a valid qmi port, false otherwise
       */
      bool isQmiPort ( const char *devId );

      /**
       * @brief Creates a WDS tracker instance for the specified qmi port
       *
       * @return None
       */
      void createWdsInstance (const char *devId, qmi_ip_family_pref_type ipType);

      /**
       * @brief Destroys the WDS tracker instance for specified qmi port
       *
       * @return None
       */
      void destroyWdsInstance (const char *devId, qmi_ip_family_pref_type ipType);

      /**
       * @brief Deletes all WDS tracker instances and performs
       *        appropriate cleanup
       *
       * @return None
       */
      void deleteWdsTrackers (void);

      /** dump WDS trackers list to log */
      void dumpTrackerInfo(void);

      static void qmiEventCallback(int fd, void *arg);
};

#endif /* CNE_QMI_H */
