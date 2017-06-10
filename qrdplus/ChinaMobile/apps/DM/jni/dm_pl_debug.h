#ifndef __DM_PL_DEBUG_H__
#define __DM_PL_DEBUG_H__

#include "vdm_pl_types.h"

//#define DM_LOG_SWITCH

#ifdef DM_LOG_SWITCH
#define dm_debug_macro(x,y,z) dm_debug_print(x,y,z)
#define dm_debug_macro_ex     dm_debug_print_ex
#else
#define dm_debug_macro        dm_debug_trace
#define dm_debug_macro_ex
#endif

#define DM_DEBUG_TREE_MASK               0x10000000
#define DM_DEBUG_SOCKET_MASK             0x20000000
#define DM_DEBUG_WRAPPER_MASK            0x30000000

#ifdef  DM_LOG_SWITCH

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     *  @brief   This function outputs message for debug purpose.
     *
     *  @param   mask  [I]Refer to Mask value to use.
     *  @param   str   [I]Message to print.
     *  @param   num   [I]Print param.
     *
     *  @return  none.
     *
     *  @note    none.
     */
    void dm_debug_trace(IU32 mask, const IS8 *str, ...);

    /**
     *  @brief   This function outputs message for debug purpose.
     *
     *  @param   mask  [I]Refer to Mask value to use.
     *  @param   str   [I]Message to print.
     *  @param   num   [I]Print param.
     *
     *  @return  none.
     *
     *  @note    none.
     */
    void dm_debug_print(IU32 mask, IS8 * str, IS32 num);

    /**
     * this function outputs message for debug purpose
     *
     * \param mask mask type
     * \param file the filename which this function is called
     * \param line the line which this function is called
     * \param str  the message to print
     * \param num  print parameter
     *
     * \return none
     */
    void dm_debug_print_ex(IU32 mask, const IS8 *file, IU32 line, const IS8 *str, IU32 num);

    /**
     *  @brief   This function sets the print mask flags.
     *
     *  @param   mask   [I]The value to set.
     *
     *  @return  none.
     *
     *  @note    none.
     */
    void dmDebugSetMask(IU32 mask);

    /**
     *  @brief   This function gets the print mask flags.
     *
     *  @param   none.
     *
     *  @return  Debug Print Mask value.
     *
     *  @note    none.
     */
    IU32 dmDebugGetMask(void);

#ifdef __cplusplus
}
#endif

#endif // DM_LOG_SWITCH
#endif // __HISENSE_DM_PL_DEBUG_H__
