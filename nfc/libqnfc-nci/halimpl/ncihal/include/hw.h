/******************************************************************************
* Copyright (c) 2013, The Linux Foundation. All rights reserved.
* Not a Contribution.
 ******************************************************************************/
/******************************************************************************
 *
 *  Copyright (C) 2012 Broadcom Corporation
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at:
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 ******************************************************************************/

#ifndef _HW_H
#define _HW_H

#define NFCC_MAGIC  0xFA

/*
 * NFCC power control via ioctl
 * NFCC_POWER_CTL(0): power off
 * NFCC_POWER_CTL(1): power on
 * NFCC_WAKE_CTL(0): wake off
 * NFCC_WAKE_CTL(1): wake on
 */
#define NFCC_POWER_CTL              _IO(NFCC_MAGIC, 0x01)
#define NFCC_CHANGE_ADDR            _IO(NFCC_MAGIC, 0x02)
#define NFCC_READ_FULL_PACKET       _IO(NFCC_MAGIC, 0x03)
#define NFCC_SET_WAKE_ACTIVE_STATE  _IO(NFCC_MAGIC, 0x04)
#define NFCC_WAKE_CTL               _IO(NFCC_MAGIC, 0x05)
#define NFCC_READ_MULTI_PACKETS     _IO(NFCC_MAGIC, 0x06)

struct nfcc_platform_data {
    unsigned int irq_gpio;
    unsigned int en_gpio;
    int wake_gpio;
};

#endif
