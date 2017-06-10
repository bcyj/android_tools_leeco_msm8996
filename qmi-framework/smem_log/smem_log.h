/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
 Copyright (c) 2011, 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
 Qualcomm Technologies Proprietary and Confidential.
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
#ifndef SMEM_LOG_H
#define SMEM_LOG_H

#include <stdint.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define SMEM_LOG_BASE 0x30

#define SMIOC_SETMODE _IOW(SMEM_LOG_BASE, 1, int)
#define SMIOC_SETLOG _IOW(SMEM_LOG_BASE, 2, int)

#define SMIOC_TEXT 0x00000001
#define SMIOC_BINARY 0x00000002
#define SMIOC_LOG 0x00000003
#define SMIOC_STATIC_LOG 0x00000004

/* Event indentifier format:
 * bit  31-28 is processor ID 8 => apps, 4 => Q6, 0 => modem
 * bits 27-16 are subsystem id (event base)
 * bits 15-0  are event id
 */

#define PROC                            0xF0000000
#define SUB                             0x0FFF0000
#define ID                              0x0000FFFF

#define SMEM_LOG_PROC_ID_MODEM          0x00000000
#define SMEM_LOG_PROC_ID_Q6             0x40000000
#define SMEM_LOG_PROC_ID_APPS           0x80000000

#define SMEM_LOG_DEBUG_EVENT_BASE       0x00000000
#define SMEM_LOG_ONCRPC_EVENT_BASE      0x00010000
#define SMEM_LOG_SMEM_EVENT_BASE        0x00020000
#define SMEM_LOG_TMC_EVENT_BASE         0x00030000
#define SMEM_LOG_TIMETICK_EVENT_BASE    0x00040000
#define SMEM_LOG_DEM_EVENT_BASE         0x00050000
#define SMEM_LOG_ERROR_EVENT_BASE       0x00060000
#define SMEM_LOG_DCVS_EVENT_BASE        0x00070000
#define SMEM_LOG_SLEEP_EVENT_BASE       0x00080000
#define SMEM_LOG_RPC_ROUTER_EVENT_BASE  0x00090000
#define SMEM_LOG_IPC_ROUTER_EVENT_BASE  0x000D0000
#define SMEM_LOG_QCCI_EVENT_BASE        0x000E0000
#define SMEM_LOG_QCSI_EVENT_BASE        0x000F0000
#define DEM_NO_SLEEP                    (SMEM_LOG_DEM_EVENT_BASE + 1)
#define DEM_INSUF_TIME                  (SMEM_LOG_DEM_EVENT_BASE + 2)
#define DEMAPPS_ENTER_SLEEP             (SMEM_LOG_DEM_EVENT_BASE + 3)
#define DEMAPPS_DETECT_WAKEUP           (SMEM_LOG_DEM_EVENT_BASE + 4)
#define DEMAPPS_END_APPS_TCXO           (SMEM_LOG_DEM_EVENT_BASE + 5)
#define DEMAPPS_ENTER_SLEEPEXIT         (SMEM_LOG_DEM_EVENT_BASE + 6)
#define DEMAPPS_END_APPS_SLEEP          (SMEM_LOG_DEM_EVENT_BASE + 7)
#define DEMAPPS_SETUP_APPS_PWRCLPS      (SMEM_LOG_DEM_EVENT_BASE + 8)
#define DEMAPPS_PWRCLPS_EARLY_EXIT      (SMEM_LOG_DEM_EVENT_BASE + 9)
#define DEMMOD_SEND_WAKEUP              (SMEM_LOG_DEM_EVENT_BASE + 0xA)
#define DEMMOD_NO_APPS_VOTE             (SMEM_LOG_DEM_EVENT_BASE + 0xB)
#define DEMMOD_NO_TCXO_SLEEP            (SMEM_LOG_DEM_EVENT_BASE + 0xC)
#define DEMMOD_BT_CLOCK                 (SMEM_LOG_DEM_EVENT_BASE + 0xD)
#define DEMMOD_UART_CLOCK               (SMEM_LOG_DEM_EVENT_BASE + 0xE)
#define DEMMOD_OKTS                     (SMEM_LOG_DEM_EVENT_BASE + 0xF)
#define DEM_SLEEP_INFO                  (SMEM_LOG_DEM_EVENT_BASE + 0x10)
#define DEMMOD_TCXO_END                 (SMEM_LOG_DEM_EVENT_BASE + 0x11)
#define DEMMOD_END_SLEEP_SIG            (SMEM_LOG_DEM_EVENT_BASE + 0x12)
#define DEMMOD_SETUP_APPSSLEEP          (SMEM_LOG_DEM_EVENT_BASE + 0x13)
#define DEMMOD_ENTER_TCXO               (SMEM_LOG_DEM_EVENT_BASE + 0x14)
#define DEMMOD_WAKE_APPS                (SMEM_LOG_DEM_EVENT_BASE + 0x15)
#define DEMMOD_POWER_COLLAPSE_APPS      (SMEM_LOG_DEM_EVENT_BASE + 0x16)
#define DEMMOD_RESTORE_APPS_PWR         (SMEM_LOG_DEM_EVENT_BASE + 0x17)
#define DEMAPPS_ASSERT_OKTS             (SMEM_LOG_DEM_EVENT_BASE + 0x18)
#define DEMAPPS_RESTART_START_TIMER     (SMEM_LOG_DEM_EVENT_BASE + 0x19)
#define DEMAPPS_ENTER_RUN               (SMEM_LOG_DEM_EVENT_BASE + 0x1A)
#define DEMMOD_MAO_INTS                 (SMEM_LOG_DEM_EVENT_BASE + 0x1B)
#define DEMMOD_POWERUP_APPS_CALLED      (SMEM_LOG_DEM_EVENT_BASE + 0x1C)
#define DEMMOD_PC_TIMER_EXPIRED         (SMEM_LOG_DEM_EVENT_BASE + 0x1D)
#define DEM_DETECT_SLEEPEXIT            (SMEM_LOG_DEM_EVENT_BASE + 0x1E)
#define DEM_DETECT_RUN                  (SMEM_LOG_DEM_EVENT_BASE + 0x1F)
#define DEM_SET_APPS_TIMER              (SMEM_LOG_DEM_EVENT_BASE + 0x20)
#define DEM_NEGATE_OKTS                 (SMEM_LOG_DEM_EVENT_BASE + 0x21)
#define DEMMOD_APPS_WAKEUP_INT          (SMEM_LOG_DEM_EVENT_BASE + 0x22)
#define DEMMOD_APPS_SWFI                (SMEM_LOG_DEM_EVENT_BASE + 0x23)
#define DEM_SEND_BATTERY_INFO           (SMEM_LOG_DEM_EVENT_BASE + 0x24)
#define DEM_SMI_CLK_DISABLED            (SMEM_LOG_DEM_EVENT_BASE + 0x25)
#define DEM_SMI_CLK_ENABLED             (SMEM_LOG_DEM_EVENT_BASE + 0x26)
#define DEMAPPS_SETUP_APPS_SUSPEND      (SMEM_LOG_DEM_EVENT_BASE + 0x27)
#define DEM_RPC_EARLY_EXIT              (SMEM_LOG_DEM_EVENT_BASE + 0x28)
#define DEMAPPS_WAKEUP_REASON           (SMEM_LOG_DEM_EVENT_BASE + 0x29)
#define DEMMOD_UMTS_BASE                (SMEM_LOG_DEM_EVENT_BASE + 0x8000)
#define TIME_REMOTE_LOG_EVENT_START     (SMEM_LOG_TIMETICK_EVENT_BASE + 0)
#define TIME_REMOTE_LOG_EVENT_GOTO_WAIT (SMEM_LOG_TIMETICK_EVENT_BASE + 1)
#define TIME_REMOTE_LOG_EVENT_GOTO_INIT (SMEM_LOG_TIMETICK_EVENT_BASE + 2)
#define ERR_ERROR_FATAL                 (SMEM_LOG_ERROR_EVENT_BASE + 1)
#define ERR_ERROR_FATAL_TASK            (SMEM_LOG_ERROR_EVENT_BASE + 2)
#define DCVSAPPS_LOG_IDLE               (SMEM_LOG_DCVS_EVENT_BASE + 0x0)
#define DCVSAPPS_LOG_ERR                (SMEM_LOG_DCVS_EVENT_BASE + 0x1)
#define DCVSAPPS_LOG_CHG                (SMEM_LOG_DCVS_EVENT_BASE + 0x2)
#define DCVSAPPS_LOG_REG                (SMEM_LOG_DCVS_EVENT_BASE + 0x3)
#define DCVSAPPS_LOG_DEREG              (SMEM_LOG_DCVS_EVENT_BASE + 0x4)
#define SMEM_LOG_EVENT_CB               (SMEM_LOG_SMEM_EVENT_BASE +  0)
#define SMEM_LOG_EVENT_START            (SMEM_LOG_SMEM_EVENT_BASE +  1)
#define SMEM_LOG_EVENT_INIT             (SMEM_LOG_SMEM_EVENT_BASE +  2)
#define SMEM_LOG_EVENT_RUNNING          (SMEM_LOG_SMEM_EVENT_BASE +  3)
#define SMEM_LOG_EVENT_STOP             (SMEM_LOG_SMEM_EVENT_BASE +  4)
#define SMEM_LOG_EVENT_RESTART          (SMEM_LOG_SMEM_EVENT_BASE +  5)
#define SMEM_LOG_EVENT_SS               (SMEM_LOG_SMEM_EVENT_BASE +  6)
#define SMEM_LOG_EVENT_READ             (SMEM_LOG_SMEM_EVENT_BASE +  7)
#define SMEM_LOG_EVENT_WRITE            (SMEM_LOG_SMEM_EVENT_BASE +  8)
#define SMEM_LOG_EVENT_SIGS1            (SMEM_LOG_SMEM_EVENT_BASE +  9)
#define SMEM_LOG_EVENT_SIGS2            (SMEM_LOG_SMEM_EVENT_BASE + 10)
#define SMEM_LOG_EVENT_WRITE_DM         (SMEM_LOG_SMEM_EVENT_BASE + 11)
#define SMEM_LOG_EVENT_READ_DM          (SMEM_LOG_SMEM_EVENT_BASE + 12)
#define SMEM_LOG_EVENT_SKIP_DM          (SMEM_LOG_SMEM_EVENT_BASE + 13)
#define SMEM_LOG_EVENT_STOP_DM          (SMEM_LOG_SMEM_EVENT_BASE + 14)
#define SMEM_LOG_EVENT_ISR              (SMEM_LOG_SMEM_EVENT_BASE + 15)
#define SMEM_LOG_EVENT_TASK             (SMEM_LOG_SMEM_EVENT_BASE + 16)
#define SMEM_LOG_EVENT_RS               (SMEM_LOG_SMEM_EVENT_BASE + 17)
#define ONCRPC_LOG_EVENT_SMD_WAIT       (SMEM_LOG_ONCRPC_EVENT_BASE +  0)
#define ONCRPC_LOG_EVENT_RPC_WAIT       (SMEM_LOG_ONCRPC_EVENT_BASE +  1)
#define ONCRPC_LOG_EVENT_RPC_BOTH_WAIT  (SMEM_LOG_ONCRPC_EVENT_BASE +  2)
#define ONCRPC_LOG_EVENT_RPC_INIT       (SMEM_LOG_ONCRPC_EVENT_BASE +  3)
#define ONCRPC_LOG_EVENT_RUNNING        (SMEM_LOG_ONCRPC_EVENT_BASE +  4)
#define ONCRPC_LOG_EVENT_APIS_INITED    (SMEM_LOG_ONCRPC_EVENT_BASE +  5)
#define ONCRPC_LOG_EVENT_AMSS_RESET     (SMEM_LOG_ONCRPC_EVENT_BASE +  6)
#define ONCRPC_LOG_EVENT_SMD_RESET      (SMEM_LOG_ONCRPC_EVENT_BASE +  7)
#define ONCRPC_LOG_EVENT_ONCRPC_RESET   (SMEM_LOG_ONCRPC_EVENT_BASE +  8)
#define ONCRPC_LOG_EVENT_CB             (SMEM_LOG_ONCRPC_EVENT_BASE +  9)
#define ONCRPC_LOG_EVENT_STD_CALL       (SMEM_LOG_ONCRPC_EVENT_BASE + 10)
#define ONCRPC_LOG_EVENT_STD_REPLY      (SMEM_LOG_ONCRPC_EVENT_BASE + 11)
#define ONCRPC_LOG_EVENT_STD_CALL_ASYNC (SMEM_LOG_ONCRPC_EVENT_BASE + 12)
#define NO_SLEEP_OLD                    (SMEM_LOG_SLEEP_EVENT_BASE + 0x1)
#define INSUF_TIME                      (SMEM_LOG_SLEEP_EVENT_BASE + 0x2)
#define MOD_UART_CLOCK                  (SMEM_LOG_SLEEP_EVENT_BASE + 0x3)
#define SLEEP_INFO                      (SMEM_LOG_SLEEP_EVENT_BASE + 0x4)
#define MOD_TCXO_END                    (SMEM_LOG_SLEEP_EVENT_BASE + 0x5)
#define MOD_ENTER_TCXO                  (SMEM_LOG_SLEEP_EVENT_BASE + 0x6)
#define NO_SLEEP_NEW                    (SMEM_LOG_SLEEP_EVENT_BASE + 0x7)
#define RPC_ROUTER_LOG_EVENT_UNKNOWN    (SMEM_LOG_RPC_ROUTER_EVENT_BASE)
#define RPC_ROUTER_LOG_EVENT_MSG_READ   (SMEM_LOG_RPC_ROUTER_EVENT_BASE + 1)
#define RPC_ROUTER_LOG_EVENT_MSG_WRITTEN (SMEM_LOG_RPC_ROUTER_EVENT_BASE + 2)
#define RPC_ROUTER_LOG_EVENT_MSG_CFM_REQ (SMEM_LOG_RPC_ROUTER_EVENT_BASE + 3)
#define RPC_ROUTER_LOG_EVENT_MSG_CFM_SNT (SMEM_LOG_RPC_ROUTER_EVENT_BASE + 4)
#define RPC_ROUTER_LOG_EVENT_MID_READ    (SMEM_LOG_RPC_ROUTER_EVENT_BASE + 5)
#define RPC_ROUTER_LOG_EVENT_MID_WRITTEN (SMEM_LOG_RPC_ROUTER_EVENT_BASE + 6)
#define RPC_ROUTER_LOG_EVENT_MID_CFM_REQ (SMEM_LOG_RPC_ROUTER_EVENT_BASE + 7)

int smem_log_init(void);
void smem_log_exit(void);
int smem_log_get_fd(void);

#define SMEM_LOG_EVENT( event, data1, data2, data3 ) \
do { \
uint32_t data[] = {SMEM_LOG_PROC_ID_APPS | event, \
		   0, data1, data2, data3}; \
int smem_log_fd = smem_log_get_fd(); \
if(smem_log_fd > 0) \
  write(smem_log_fd, data, sizeof(data)); \
} while(0) \

#define SMEM_LOG_EVENT6(identifier, data1, data2, data3, \
                        data4, data5, data6) \
do { \
uint32_t data[] = {SMEM_LOG_PROC_ID_APPS | identifier, \
                   0, data1, data2, data3, 0, 0, data4, data5, data6};	\
int smem_log_fd = smem_log_get_fd(); \
if(smem_log_fd > 0) \
  write(smem_log_fd, data, sizeof(data)); \
} while(0) \

#endif /* SMEM_LOG_H */
