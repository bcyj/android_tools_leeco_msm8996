/* Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package org.codeaurora.ims;

public class ServiceStatus {
    public boolean isValid = false;
    public int type; /* Refer "enum CallType" from imsIF.proto file
                      * for possible types
                      */
    public int status; /*
                        * Overall Status (eg. enabled, disabled)
                        * best case for this type across all
                        * access techs
                        * Refer "enum StatusType" from imsIF.proto file
                        * for possible status values
                        */
    public byte[] userdata;
    public StatusForAccessTech[] accessTechStatus;
    public static class StatusForAccessTech {
        public int networkMode; /* Refer "enum RadioTechType" from
                                 * imsIF.proto file for possible
                                 * networkMode values
                                 */
        public int status; /* Refer "enum StatusType" from imsIF.proto
                            * file for possible status values
                            */
        public int restrictCause;
        public int registered; /* Refer "enum RegState" from imsIF.proto
                                * file for possible values
                                */

        /**
         * @return string representation.
         */
        @Override
        public String toString() {
            return " mode = " + networkMode + " Status = " + status + " restrictCause = "
                    + restrictCause + " registered = " + registered;
        }
    }
}
