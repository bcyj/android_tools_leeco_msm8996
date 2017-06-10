/*=========================================================================
  wbc_psy.h
  DESCRIPTION
  Header file for file access and power supply properties paths

  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  =========================================================================*/

#ifndef WBC_PSY_H
#define WBC_PSY_H

#define BATT_PSY        "/sys/class/power_supply/battery/"
#define USB_PSY         "/sys/class/power_supply/usb/"
#define DC_PSY          "/sys/class/power_supply/dc/"

#define STATUS             "status"
#define HEALTH             "health"
#define TYPE               "type"
#define PRESENT            "present"
#define SYSTEM_TEMP_LEVEL  "system_temp_level"

#define WAKE_LOCK       "/sys/power/wake_lock"
#define WAKE_UNLOCK     "/sys/power/wake_unlock"

#define PSY_NAME               "POWER_SUPPLY_NAME="
#define PSY_TYPE               "POWER_SUPPLY_TYPE="
#define PSY_STATUS             "POWER_SUPPLY_STATUS="
#define PSY_HEALTH             "POWER_SUPPLY_HEALTH="
#define PSY_PRESENT            "POWER_SUPPLY_PRESENT="
#define PSY_CHARGING_ENABLED   "POWER_SUPPLY_CHARGING_ENABLED="
#define PSY_SYSTEM_TEMP_LEVEL  "POWER_SUPPLY_SYSTEM_TEMP_LEVEL="

int directory_exists(const char *prefix, const char *path);
int file_exists(const char *path);
int read_int_from_file(const char *prefix, const char *path, int *ret_val);
int read_from_file(const char *prefix, const char *path, char *buf, size_t count);
int open_file(const char *prefix, const char *path, int flags);
int write_string_to_file(const char *prefix, const char *path, char *string);
int write_int_to_file(const char *prefix, const char *path, int value);
int rename_file(char *oldprefix, char *oldpath, char *newprefix, char *newpath);
int append_string_to_file(char *prefix, char *path, char *string);
int wbc_uevent_open_socket(int, bool);

#endif  /* WBC_PSY_H */
