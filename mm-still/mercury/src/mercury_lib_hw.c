/* Copyright (c) 2012, Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#include <string.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <media/msm_mercury.h>

#include "mercury_dbg.h"
#include "mercury_lib_hw.h"
#include "mercury_lib_hw_reg_ctrl.h"

static int mercury_fd = 0;

void mercury_lib_hw_setfd(int mcrfd)
{
    mercury_fd = mcrfd;
}

static struct msm_mercury_hw_cmds *mercury_lib_hw_cmd_malloc_and_copy (uint16_t size,
    struct msm_mercury_hw_cmd *p_hw_cmd) {
    struct msm_mercury_hw_cmds *p_hw_cmds;

    p_hw_cmds = malloc (sizeof (struct msm_mercury_hw_cmds) -
        sizeof (struct msm_mercury_hw_cmd) + size);

    if (p_hw_cmds) {
        p_hw_cmds->m = size / sizeof (struct msm_mercury_hw_cmd);
        if (p_hw_cmd) {
            memcpy (p_hw_cmds->hw_cmd, p_hw_cmd, size);
        }
    }
    return p_hw_cmds;
}

int mercury_lib_hw_jpegd_core_reset(void)
{
    struct msm_mercury_hw_cmd hw_cmd;
    int rc;

    mercury_write(RTDMA_JPEG_WR_STA_ACK, 0);
    rc = ioctl(mercury_fd, MSM_MCR_IOCTL_RESET, &hw_cmd);
    if (rc) {
        MCR_PR_ERR("(%d)%s()  (rc=%d) ioctl fails\n\n", __LINE__,
            __func__, rc);
        return -1;
    }

    return 0;
}
int mercury_lib_hw_jpeg_dqt( mercury_cmd_quant_cfg_t dqt_cfg)
{
    int i;
    int cnt;
    int rc;

    uint16_t jpeg_dqt_qk;
    uint8_t jpeg_dqt_tq;
    uint32_t val;

    struct msm_mercury_hw_cmds *p_hw_cmds;
    struct msm_mercury_hw_cmd *p_hw_cmd;

    uint8_t qtable_present_flag =  dqt_cfg.qtable_present_flag;

    for (cnt=0; qtable_present_flag>0; qtable_present_flag >>= 1) {
        cnt++;
    }

    MCR_PR_ERR("%s:%d\n", __func__,__LINE__);


    MCR_DBG("\n Number of Quantization Tables = %d\n", cnt);

    p_hw_cmds =
        mercury_lib_hw_cmd_malloc_and_copy (sizeof (struct msm_mercury_hw_cmd) *
        (64*cnt + 1), NULL);

    if (!p_hw_cmds) {
        return NULL;
    }

    p_hw_cmd = &p_hw_cmds->hw_cmd[0];

    val = 0;
    MEM_OUTF(&val, JPEG_QT_IDX, JPEG_QT_IDX_TABLE_1, 0);
    MEM_OUTF(&val, JPEG_QT_IDX, JPEG_QT_IDX_TABLE_0, 0);
    p_mercury_writes(JPEG_QT_IDX, val);
    MCR_PR_ERR("%s:%d, dqt_cfg.qtable_present_flag 0x%x\n", __func__,
        __LINE__,dqt_cfg.qtable_present_flag);

    for (jpeg_dqt_tq=0; jpeg_dqt_tq<4; jpeg_dqt_tq++) {

        if (!(dqt_cfg.qtable_present_flag & 1<<jpeg_dqt_tq)) {
            continue;
        }
        for (i=1; i<=64; i++) {
            jpeg_dqt_qk = dqt_cfg.p_qtables[jpeg_dqt_tq][i-1];

            val = 0;
            MEM_OUTF2(&val, JPEG_DQT, JPEG_DQT_TQ, JPEG_DQT_QK,
                jpeg_dqt_tq, jpeg_dqt_qk);
            p_mercury_writes(JPEG_DQT, val);
        }
        MCR_PR_ERR("%s:%d: jpeg_dqt_tq=%d\n",__func__,__LINE__,jpeg_dqt_tq);
    }

    rc = ioctl (mercury_fd, MSM_MCR_IOCTL_HW_CMDS, p_hw_cmds);
    if (rc  < 0) {
        MCR_PR_ERR ("(%d)%s  %s ioctl-%d error...\n",
            __LINE__, __func__, MSM_MERCURY_NAME,
            _IOC_NR(MSM_MCR_IOCTL_HW_CMDS));
        free(p_hw_cmds);
        return 1;
    }
    MCR_PR_ERR("%s:%d:\n",__func__,__LINE__);
    free(p_hw_cmds);

    return 0;
}


int mercury_lib_hw_jpeg_sof(mercury_cmd_sof_cfg_t sof_cfg)
{
    uint16_t imageWidth;
    uint16_t imageHeight;
    uint8_t numOfComponents;
    uint8_t samplingPrecision;
    uint8_t Ci;
    uint8_t Hi;
    uint8_t Vi;
    uint8_t Tqi;
    int rc;

    int i;

    uint8_t temp_byte;
    uint32_t val;

    mer_comp_info_t *p_info = sof_cfg.p_comp_infos;

    struct msm_mercury_hw_cmds *p_hw_cmds;
    struct msm_mercury_hw_cmd *p_hw_cmd;

    imageWidth = sof_cfg.width;
    imageHeight = sof_cfg.height;
    samplingPrecision = sof_cfg.precision;
    numOfComponents = sof_cfg.num_comps;

    MCR_DBG("\n(%d)%s()  SOF(Start Of Frame) Infromation\n", __LINE__,
        __func__);
    MCR_DBG("   Image Width:  %d \n", sof_cfg.width);
    MCR_DBG("   Image Heigh:  %d \n", sof_cfg.height);

    p_hw_cmds =
        mercury_lib_hw_cmd_malloc_and_copy (sizeof(struct msm_mercury_hw_cmd) *
        (2+numOfComponents), NULL);
    if (!p_hw_cmds) {
        return NULL;
    }

    p_hw_cmd = &p_hw_cmds->hw_cmd[0];

    val = 0;
    MEM_OUTF(&val, JPEG_SOF_REG_2 , JPEG_SOF_REG_2_Y, imageHeight);
    MEM_OUTF(&val, JPEG_SOF_REG_2 , JPEG_SOF_REG_2_X, imageWidth);
    p_mercury_writes(JPEG_SOF_REG_2 , val);

    val = 0;
    MEM_OUTF(&val, JPEG_SOF_REG_0, JPEG_SOF_REG_0_NF, numOfComponents);
    p_mercury_writes(JPEG_SOF_REG_0, val);

    Hi = p_info[0].sampling_h;
    Vi = p_info[0].sampling_v;

    MCR_DBG("   Input Image detected as H%dV%d\n", Hi, Vi);

    for (i=0; i<numOfComponents; i++) {
        Ci = p_info[i].comp_id;
        Hi = p_info[i].sampling_h;
        Vi = p_info[i].sampling_v;
        Tqi = p_info[i].qtable_sel;

        val = 0;
        MEM_OUTF(&val, JPEG_SOF_REG_1, JPEG_SOF_REG_1_C, Ci);
        MEM_OUTF(&val, JPEG_SOF_REG_1, JPEG_SOF_REG_1_H, Hi);
        MEM_OUTF(&val, JPEG_SOF_REG_1, JPEG_SOF_REG_1_V, Vi);
        MEM_OUTF(&val, JPEG_SOF_REG_1, JPEG_SOF_REG_1_TQ, Tqi);
        p_mercury_writes(JPEG_SOF_REG_1, val);

    }

    rc = ioctl (mercury_fd, MSM_MCR_IOCTL_HW_CMDS, p_hw_cmds);
    if (rc  < 0) {
        MCR_PR_ERR ("(%d)%s  %s ioctl-%d error...\n",
            __LINE__, __func__, MSM_MERCURY_NAME,
            _IOC_NR(MSM_MCR_IOCTL_HW_CMDS));
        free(p_hw_cmds);
        return 1;
    }

    free(p_hw_cmds);

    return 0;
}




int mercury_lib_hw_jpeg_dht(mercury_cmd_huff_cfg_t huff_cfg)
{
    uint32_t retVal;
    uint32_t loop;
    uint32_t CCC = 0;
    uint8_t Vij_3;
    uint8_t Vij_2;
    uint8_t Vij_1;
    uint8_t Vij_0;
    uint8_t Remainder;
    uint8_t i, j;

    uint32_t Th;
    uint32_t Tc;
    uint32_t val;

    uint8_t index;
    uint8_t ac_cnt;
    uint8_t dc_cnt;
    uint8_t bytevalue;
    uint8_t ht_bit;
    uint8_t ht_value;
    int rc;
    int cmd_cnt;

    struct msm_mercury_hw_cmds *p_hw_cmds;
    struct msm_mercury_hw_cmd *p_hw_cmd;
    MCR_PR_ERR("%s:%d\n", __func__,__LINE__);
    bytevalue = huff_cfg.htable_present_flag;
    i = (bytevalue>>4);
    j = (bytevalue&0xF);

    for (ac_cnt=0; i>0; i >>=1) {
        ac_cnt++;
    }

    for (dc_cnt=0; j>0; j >>=1) {
        dc_cnt++;
    }

    cmd_cnt = 0;
    /* HT Destination ID*/
    for (Th=0; Th<dc_cnt; Th++) {
        /* HT Table class: 0=DC, 1=AC*/
        for (Tc=0; Tc<ac_cnt; Tc++) {
            index = (Tc<<2)|(Th);
            cmd_cnt++;

            for (CCC = 0, j=0; j<16; j++) {
                bytevalue = huff_cfg.p_htables[index].bits[j+1];
                CCC += bytevalue;
                cmd_cnt+=2;
            }

            Remainder = (CCC % 4);
            loop = (CCC/4);

            for (i=0, j=0; j<loop; j++) {
                i += 4;
                cmd_cnt+=2;
            }

            if (Remainder) {
                switch (Remainder) {
                case 1:
                    i++;
                    break;
                case 2:
                    i+=2;
                    break;
                case 3:
                    i+=3;
                default: break;
                }

                cmd_cnt+=2;
            }
        }
    }

    p_hw_cmds =
        mercury_lib_hw_cmd_malloc_and_copy ( sizeof (struct msm_mercury_hw_cmd) *
        cmd_cnt, NULL);
    if (!p_hw_cmds) {
        return -1;
    }

    MCR_DBG("\n(%d)%s()  HT cmd_cnt = %d\n", __LINE__, __func__, cmd_cnt);
    MCR_PR_ERR("%s:%d\n", __func__,__LINE__);
    p_hw_cmd = &p_hw_cmds->hw_cmd[0];

    for (Th=0; Th<dc_cnt; Th++) {
        /*HT Table class: 0=DC, 1=AC*/
        for (Tc=0; Tc<ac_cnt; Tc++) {
            index = (Tc<<2)|(Th);

            val = 0;
            MEM_OUTF(&val, JPEG_DHT_REG_0, JPEG_DHT_REG_0_TH, Th);
            MEM_OUTF(&val, JPEG_DHT_REG_0, JPEG_DHT_REG_0_TC, Tc);
            p_mercury_writes(JPEG_DHT_REG_0, val);

            for (CCC = 0, j=0; j<16; j++) {
                bytevalue = huff_cfg.p_htables[index].bits[j+1];
                CCC += bytevalue;

                val = 0;
                MEM_OUTF(&val, JPEG_DHT_IDX, JPEG_DHT_IDX_CCC_MAX, j);
                p_mercury_writes(JPEG_DHT_IDX, val);

                val = 0;
                MEM_OUTF(&val, JPEG_DHT_CCC_MAX, JPEG_DHT_LI, bytevalue);
                p_mercury_writes(JPEG_DHT_CCC_MAX, val);

            }

            Remainder = (CCC % 4);
            loop = (CCC/4);

            for (i=0, j=0; j<loop; j++) {
                Vij_3 = huff_cfg.p_htables[index].values[i++];
                Vij_2 = huff_cfg.p_htables[index].values[i++];
                Vij_1 = huff_cfg.p_htables[index].values[i++];
                Vij_0 = huff_cfg.p_htables[index].values[i++];

                val = 0;
                MEM_OUTF(&val, JPEG_DHT_IDX, JPEG_DHT_IDX_VIJ, (j*4) );
                p_mercury_writes(JPEG_DHT_IDX, val);

                val = 0;
                MEM_OUTF(&val, JPEG_DHT_REG_1, JPEG_DHT_REG_1_VIJ_0, Vij_0);
                MEM_OUTF(&val, JPEG_DHT_REG_1, JPEG_DHT_REG_1_VIJ_1, Vij_1);
                MEM_OUTF(&val, JPEG_DHT_REG_1, JPEG_DHT_REG_1_VIJ_2, Vij_2);
                MEM_OUTF(&val, JPEG_DHT_REG_1, JPEG_DHT_REG_1_VIJ_3, Vij_3);
                p_mercury_writes(JPEG_DHT_REG_1, val);
            }

            if (Remainder) {
                switch (Remainder) {
                case 1:
                    Vij_3 = huff_cfg.p_htables[index].values[i++];
                    Vij_2 = Vij_1 = Vij_0 = 0x0;
                    break;
                case 2:
                    Vij_3 = huff_cfg.p_htables[index].values[i++];
                    Vij_2 = huff_cfg.p_htables[index].values[i++];
                    Vij_1 = Vij_0 = 0x0;
                    break;
                case 3:
                    Vij_3 = huff_cfg.p_htables[index].values[i++];
                    Vij_2 = huff_cfg.p_htables[index].values[i++];
                    Vij_1 = huff_cfg.p_htables[index].values[i++];
                    Vij_0 = 0x0;
                default: break;
                }

                val = 0;
                MEM_OUTF(&val, JPEG_DHT_IDX, JPEG_DHT_IDX_VIJ, (j*4));
                p_mercury_writes(JPEG_DHT_IDX, val);

                val = 0;
                MEM_OUTF(&val, JPEG_DHT_REG_1, JPEG_DHT_REG_1_VIJ_0, Vij_0);
                MEM_OUTF(&val, JPEG_DHT_REG_1, JPEG_DHT_REG_1_VIJ_1, Vij_1);
                MEM_OUTF(&val, JPEG_DHT_REG_1, JPEG_DHT_REG_1_VIJ_2, Vij_2);
                MEM_OUTF(&val, JPEG_DHT_REG_1, JPEG_DHT_REG_1_VIJ_3, Vij_3);
                p_mercury_writes(JPEG_DHT_REG_1, val);

            }
        }
    }
    MCR_PR_ERR("%s:%d\n", __func__,__LINE__);
    rc = ioctl (mercury_fd, MSM_MCR_IOCTL_HW_CMDS, p_hw_cmds);
    if (rc  < 0) {
        MCR_PR_ERR ("(%d)%s  %s ioctl-%d error...\n",
            __LINE__, __func__, MSM_MERCURY_NAME,
            _IOC_NR(MSM_MCR_IOCTL_HW_CMDS));

        free(p_hw_cmds);
        return 1;
    }

    free(p_hw_cmds);
    MCR_PR_ERR("%s:%d\n", __func__,__LINE__);
    return 0;
}

int mercury_lib_hw_jpeg_sos(mercury_cmd_sos_cfg_t sos_cfg)
{
    uint8_t numOfComponents;
    uint8_t spectralSelectStart;
    uint8_t spectralSelectEnd;
    uint8_t SuccessiveApprox;
    uint32_t i;
    uint32_t Ta;
    uint32_t Td;
    uint32_t Cs;
    uint32_t val;

    uint16_t temp_word;
    int rc;


    mecury_scan_info_t  *p_scan_info  = sos_cfg.pp_scan_infos[0];

    struct msm_mercury_hw_cmds *p_hw_cmds;
    struct msm_mercury_hw_cmd *p_hw_cmd;

    MCR_DBG("\n(%d)%s() SOS(Start Of Scan) Information\n", __LINE__, __func__);
    numOfComponents = p_scan_info->num_selectors;
    MCR_DBG("    Number of image components in scan Ns = %d\n", numOfComponents);

    if (numOfComponents != 1 && numOfComponents != 3) {
        MCR_PR_ERR("%s unexpected number of compt sel: %d\n",__func__,
            __LINE__, numOfComponents);
        return JPEGERR_EFAILED;
    }

    p_hw_cmds =
        mercury_lib_hw_cmd_malloc_and_copy (sizeof (struct msm_mercury_hw_cmd) *
        (1+numOfComponents), NULL);
    if (!p_hw_cmds) {
        return 1;
    }
    p_hw_cmd = &p_hw_cmds->hw_cmd[0];

    val = 0;
    MEM_OUTF(&val, JPEG_SOS_REG_0, JPEG_SOS_REG_0_NS, numOfComponents);
    p_mercury_writes(JPEG_SOS_REG_0, val);

    for (i=0; i<numOfComponents; i++) {
        Cs = p_scan_info->p_selectors[i].comp_id+1;
        Td = p_scan_info->p_selectors[i].dc_selector;
        Ta = p_scan_info->p_selectors[i].ac_selector;

        val = 0;
        MEM_OUTF(&val, JPEG_SOS_REG_1, JPEG_SOS_REG_1_CS, Cs);
        MEM_OUTF(&val, JPEG_SOS_REG_1, JPEG_SOS_REG_1_TD, Td);
        MEM_OUTF(&val, JPEG_SOS_REG_1, JPEG_SOS_REG_1_TA, Ta);
        p_mercury_writes(JPEG_SOS_REG_1, val);
    }

    spectralSelectStart = p_scan_info->spec_start;
    spectralSelectEnd = p_scan_info->spec_end;
    SuccessiveApprox = (p_scan_info->succ_approx_h<<4)|
        p_scan_info->succ_approx_l;

    rc = ioctl (mercury_fd, MSM_MCR_IOCTL_HW_CMDS, p_hw_cmds);
    if (rc  < 0) {
        MCR_PR_ERR ("(%d)%s  %s ioctl-%d error...\n",
            __LINE__, __func__, MSM_MERCURY_NAME, _IOC_NR(MSM_MCR_IOCTL_HW_CMDS));
        free(p_hw_cmds);
        return 1;
    }

    free(p_hw_cmds);

    return 0;

}

int mercury_lib_hw_bus_read_config(mercury_cmd_readbus_cfg_t bus_read_cfg)
{
    uint32_t bufferPitch;
    uint32_t bufferLength;
    uint32_t bufferHeight;
    uint16_t planeHsize;
    uint16_t planeVsize;
    uint16_t blockHsize;
    uint16_t blockVsize;
    uint32_t image_buffer;

    uint32_t val;
    uint8_t encode_format = 0x6;                /*byte format*/
    int rc;

    struct msm_mercury_hw_cmds *p_hw_cmds;
    struct msm_mercury_hw_cmd *p_hw_cmd;

    bufferLength = bus_read_cfg.bitstream_length;
    image_buffer = (uint32_t)bus_read_cfg.bitstream_buf;
    bufferPitch  = bufferLength/16;

    if (bufferPitch & 0x7) {
        /*ensure buffer pitch is 8 byte aligned due to hardware restriction*/
        bufferPitch = (bufferPitch&0xfffffff8) + 0x8;
    }

    if (bufferPitch > 0x1000) {
        bufferPitch  = 0x1000;
        bufferHeight = (bufferLength+0xfff)/bufferPitch;
    } else if (bufferPitch <= 0x0) {
        bufferPitch  = 0x20;
        bufferHeight = (bufferLength+0x1f)/bufferPitch;
    } else {
        if (bufferPitch % 0x20) {
            /* round to nearest multiple*/
            bufferPitch = ((bufferPitch/0x20)+1)*0x20;
        }
        bufferHeight = (bufferLength+bufferPitch-1)/bufferPitch;
    }

    blockHsize = 0x20;
    blockVsize = 0x1;
    planeHsize = bufferPitch;
    planeVsize = 0x2000;/*max plane v-size*/

    p_hw_cmds =
        mercury_lib_hw_cmd_malloc_and_copy ( sizeof (struct msm_mercury_hw_cmd)*8,
        NULL);
    if (!p_hw_cmds) {
        return 1;
    }

    p_hw_cmd = &p_hw_cmds->hw_cmd[0];

    val = 0;
    MEM_OUTF(&val, RTDMA_JPEG_RD_BUF_CONFIG, BUF_FORMAT, encode_format);
    MEM_OUTF(&val, RTDMA_JPEG_RD_BUF_CONFIG, NUM_OF_PLANES, 0);
    p_mercury_writes(RTDMA_JPEG_RD_BUF_CONFIG, val);

    val = 0;
    MEM_OUTF(&val, RTDMA_JPEG_RD_BUF_Y_PNTR, PNTR, (image_buffer>>3));
    p_mercury_writes(RTDMA_JPEG_RD_BUF_Y_PNTR, val);

    val = 0;
    MEM_OUTF(&val, RTDMA_JPEG_RD_BUF_U_PNTR, PNTR, 0x0);
    p_mercury_writes(RTDMA_JPEG_RD_BUF_U_PNTR, val);

    val = 0;
    MEM_OUTF(&val, RTDMA_JPEG_RD_BUF_V_PNTR, PNTR, 0x0);
    p_mercury_writes(RTDMA_JPEG_RD_BUF_V_PNTR, val);

    val = 0;
    MEM_OUTF(&val, RTDMA_JPEG_RD_BUF_PITCH, PITCH, (bufferPitch>>3));
    p_mercury_writes(RTDMA_JPEG_RD_BUF_PITCH, val);

    val = 0;
    MEM_OUTF(&val, RTDMA_JPEG_RD_PLANE_SIZE, PLANE_VSIZE, (planeVsize-1));
    MEM_OUTF(&val, RTDMA_JPEG_RD_PLANE_SIZE, PLANE_HSIZE, (planeHsize-1));
    p_mercury_writes(RTDMA_JPEG_RD_PLANE_SIZE, val);

    val = 0;
    MEM_OUTF(&val, RTDMA_JPEG_RD_BLOCK_SIZE, BLOCK_VSIZE, (blockVsize-1));
    MEM_OUTF(&val, RTDMA_JPEG_RD_BLOCK_SIZE, BLOCK_HSIZE, (blockHsize-1));
    p_mercury_writes(RTDMA_JPEG_RD_BLOCK_SIZE, val);

    val = 0;
    MEM_OUTF(&val, RTDMA_JPEG_RD_BUFFER_SIZE, BUFFER_VSIZE, (bufferHeight-1));
    p_mercury_writes(RTDMA_JPEG_RD_BUFFER_SIZE, val);

    rc = ioctl (mercury_fd, MSM_MCR_IOCTL_HW_CMDS, p_hw_cmds);
    if (rc  < 0) {
        MCR_PR_ERR ("(%d)%s  %s ioctl-%d error...\n",
            __LINE__, __func__, MSM_MERCURY_NAME,
            _IOC_NR(MSM_MCR_IOCTL_HW_CMDS));
        free(p_hw_cmds);
        return 1;
    }

    free(p_hw_cmds);
    return 0;
}


int mercury_lib_hw_bus_write_config(mercury_cmd_writebus_cfg_t write_config)
{
    uint32_t hBlockSize;
    uint32_t vBlockSize;
    uint32_t hSize;
    uint32_t vSize;

    uint32_t val;

    uint32_t y_buffer;
    uint32_t u_buffer;
    uint32_t v_buffer;
    int rc;



    uint16_t image_width    = write_config.image_width;
    uint16_t image_height   = write_config.image_height;
    /*mcu_type*/
    uint8_t sampling_factor = write_config.sampling_factor;

    struct msm_mercury_hw_cmds *p_hw_cmds;
    struct msm_mercury_hw_cmd *p_hw_cmd;

    y_buffer = (uint32_t)write_config.y_buf;
    u_buffer = (uint32_t)write_config.u_buf;
    v_buffer = (uint32_t)write_config.v_buf;

    if (write_config.sampling_factor == 0x0) {                      /*h2v2*/
        hBlockSize = (16>>write_config.scale_ratio);
        vBlockSize = (16>>write_config.scale_ratio);
    } else if (write_config.sampling_factor == 0x1) {           /*h2v1*/
        hBlockSize = (16>>write_config.scale_ratio);
        vBlockSize = (8>>write_config.scale_ratio);
    } else if (write_config.sampling_factor == 0x2) {           /*h1v2*/
        hBlockSize = (8>>write_config.scale_ratio);
        vBlockSize = (16>>write_config.scale_ratio);
    }

    if (write_config.image_width % hBlockSize == 0) {
        hSize = write_config.image_width;
    } else {
        hSize = hBlockSize<<((write_config.image_width/hBlockSize)+1);
    }

    if (write_config.image_height % vBlockSize == 0) {
        vSize = write_config.image_height;
    } else {
        vSize = vBlockSize<<((write_config.image_height/vBlockSize)+1);
    }

    p_hw_cmds =
        mercury_lib_hw_cmd_malloc_and_copy ( sizeof (struct msm_mercury_hw_cmd)*9,
        NULL);
    if (!p_hw_cmds) {
        return 1;
    }

    p_hw_cmd = &p_hw_cmds->hw_cmd[0];

    val = 0;
    MEM_OUTF(&val, RTDMA_JPEG_WR_OP, ALIGN, 0);
    MEM_OUTF(&val, RTDMA_JPEG_WR_OP, FLIP, 0);
    MEM_OUTF(&val, RTDMA_JPEG_WR_OP, MIRROR, 0);
    p_mercury_writes(RTDMA_JPEG_WR_OP, val);

    val = 0;
    MEM_OUTF(&val, RTDMA_JPEG_WR_BUF_CONFIG, BUF_FORMAT,
        write_config.wr_buf_format);
    MEM_OUTF(&val, RTDMA_JPEG_WR_BUF_CONFIG, NUM_OF_PLANES,
        write_config.num_of_planes);
    p_mercury_writes(RTDMA_JPEG_WR_BUF_CONFIG, val);

    val = 0;
    MEM_OUTF(&val, RTDMA_JPEG_WR_BUF_Y_PNTR, PNTR, (y_buffer>>3));
    p_mercury_writes(RTDMA_JPEG_WR_BUF_Y_PNTR, val);

    val = 0;
    MEM_OUTF(&val, RTDMA_JPEG_WR_BUF_U_PNTR, PNTR, (u_buffer>>3));
    p_mercury_writes(RTDMA_JPEG_WR_BUF_U_PNTR, val);

    val = 0;
    MEM_OUTF(&val, RTDMA_JPEG_WR_BUF_V_PNTR, PNTR, (v_buffer>>3));
    p_mercury_writes(RTDMA_JPEG_WR_BUF_V_PNTR, val);

    val = 0;
    MEM_OUTF(&val, RTDMA_JPEG_WR_BUF_PITCH, PITCH, (hSize>>3));
    p_mercury_writes(RTDMA_JPEG_WR_BUF_PITCH, val);

    val = 0;
    MEM_OUTF(&val, RTDMA_JPEG_WR_PLANE_SIZE, PLANE_VSIZE, (vSize-1));
    MEM_OUTF(&val, RTDMA_JPEG_WR_PLANE_SIZE, PLANE_HSIZE, (hSize-1));
    p_mercury_writes(RTDMA_JPEG_WR_PLANE_SIZE, val);

    val = 0;
    MEM_OUTF(&val, RTDMA_JPEG_WR_BLOCK_SIZE, BLOCK_VSIZE, (vBlockSize-1));
    MEM_OUTF(&val, RTDMA_JPEG_WR_BLOCK_SIZE, BLOCK_HSIZE, (hBlockSize-1));
    p_mercury_writes(RTDMA_JPEG_WR_BLOCK_SIZE, val);

    val = 0;
    MEM_OUTF(&val, RTDMA_JPEG_WR_BUFFER_SIZE, BUFFER_VSIZE, (vSize-1));
    p_mercury_writes(RTDMA_JPEG_WR_BUFFER_SIZE, val);

    rc = ioctl (mercury_fd, MSM_MCR_IOCTL_HW_CMDS, p_hw_cmds);
    if (rc  < 0) {
        MCR_PR_ERR ("(%d)%s  %s ioctl-%d error... \n",
            __LINE__, __func__, MSM_MERCURY_NAME,
            _IOC_NR(MSM_MCR_IOCTL_HW_CMDS));
        free(p_hw_cmds);
        return 1;
    }

    free(p_hw_cmds);
    return 0;
}


int mercury_lib_hw_bus_control_config(mercury_cmd_control_cfg_t  ctrl_cfg)
{

    uint32_t val;
    uint8_t input_format;



    uint8_t mcu_type              = ctrl_cfg.mcu_type;
    uint8_t output_format         = ctrl_cfg.output_format;
    uint8_t CrCb_Order            = ctrl_cfg.crcb_order;
    int rc;

    struct msm_mercury_hw_cmds *p_hw_cmds;
    struct msm_mercury_hw_cmd *p_hw_cmd;

    switch (ctrl_cfg.jpegdFormat) {
    case MCR_JPEG_H2V2:
        input_format = 2;
        break;
    case MCR_JPEG_H2V1:
        input_format = 3;
        break;
    default:
        MCR_PR_ERR(" (%d)%s()  unsupported jpegdFormat (%d)",
            __LINE__, __func__, ctrl_cfg.jpegdFormat);
        return 1;
    }


    p_hw_cmds =
        mercury_lib_hw_cmd_malloc_and_copy ( sizeof (struct msm_mercury_hw_cmd) * 6,
        NULL);
    if (!p_hw_cmds) {
        return 1;
    }

    p_hw_cmd = &p_hw_cmds->hw_cmd[0];

    val = 0;
    MEM_OUTF(&val, JPEG_CTRL_COMMON, JPEG_CTRL_COMMON_ZZ_OVERRIDE_EN, 0x0);
    MEM_OUTF(&val, JPEG_CTRL_COMMON, JPEG_CTRL_COMMON_MODE, 0x0);
    p_mercury_writes(JPEG_CTRL_COMMON, val);

    val = 0;
    MEM_OUTF(&val, JPEG_CTRL_ENCODE, JPEG_CTRL_ENCODE_EOI_MARKER_EN, 0x0);
    p_mercury_writes(JPEG_CTRL_ENCODE, val);

    val = 0;
    MEM_OUTF(&val, JPEG_DEC_SCALE, JPEG_DEC_SCALE_RATIO, ctrl_cfg.scale_ratio);
    p_mercury_writes(JPEG_DEC_SCALE, val);

    val = 0;
    MEM_OUTF(&val, JPEG_CONVERT, JPEG_CONVERT_MONO_CB_VALUE, 0);
    MEM_OUTF(&val, JPEG_CONVERT, JPEG_CONVERT_MONO_CR_VALUE, 0);
    MEM_OUTF(&val, JPEG_CONVERT, JPEG_CONVERT_CLAMP_EN, 0);
    MEM_OUTF(&val, JPEG_CONVERT, JPEG_CONVERT_CBCR_SWITCH, CrCb_Order);
    MEM_OUTF(&val, JPEG_CONVERT, JPEG_CONVERT_MONOCHROME_EN, 0);
    MEM_OUTF(&val, JPEG_CONVERT, JPEG_CONVERT_MEM_ORG, ctrl_cfg.numofplanes);
    MEM_OUTF(&val, JPEG_CONVERT, JPEG_CONVERT_422_MCU_TYPE, mcu_type);
    MEM_OUTF(&val, JPEG_CONVERT, JPEG_CONVERT_OUTPUT_FORMAT, output_format);
    MEM_OUTF(&val, JPEG_CONVERT, JPEG_CONVERT_INPUT_FORMAT, input_format);
    p_mercury_writes(JPEG_CONVERT, val);

    val = 0;
    MEM_OUTF(&val, JPEGD_CLK_CONTROL, JPEG_CLKIDLE, 0);
    MEM_OUTF(&val, JPEGD_CLK_CONTROL, JPEG_CLKON, 1);
    MEM_OUTF(&val, JPEGD_CLK_CONTROL, AXI_CLKIDLE, 0);
    MEM_OUTF(&val, JPEGD_CLK_CONTROL, AXI_CLKON, 1);
    p_mercury_writes(JPEGD_CLK_CONTROL, val);

    val = 0;
    MEM_OUTF(&val, RTDMA_JPEG_AXI_CONFIG, OUT_OF_ORDER_WR, 0);
    MEM_OUTF(&val, RTDMA_JPEG_AXI_CONFIG, OUT_OF_ORDER_RD, 0);
    MEM_OUTF(&val, RTDMA_JPEG_AXI_CONFIG, BOUND_LIMIT, 0);
    MEM_OUTF(&val, RTDMA_JPEG_AXI_CONFIG, PACK_TIMEOUT, 2);
    MEM_OUTF(&val, RTDMA_JPEG_AXI_CONFIG, PACK_MAX_BLEN, 4);
    p_mercury_writes(RTDMA_JPEG_AXI_CONFIG, val);

    rc = ioctl (mercury_fd, MSM_MCR_IOCTL_HW_CMDS, p_hw_cmds);
    if (rc  < 0) {
        MCR_PR_ERR ("(%d)%s  %s ioctl-%d error...\n",
            __LINE__, __func__, MSM_MERCURY_NAME, _IOC_NR(MSM_MCR_IOCTL_HW_CMDS));
        free(p_hw_cmds);
        return 1;
    }

    free(p_hw_cmds);
    return 0;
}



int mercury_lib_hw_jpegd_rtdma_rd_status_enable(uint8_t sof_val, uint8_t eof_val)
{
    int rc;
    struct msm_mercury_hw_cmd hw_cmd;

    uint32_t val = 0;
    MEM_OUTF2(&val, RTDMA_JPEG_RD_INT_EN, SOF_EN, EOF_EN, sof_val, eof_val);

    mercury_write(RTDMA_JPEG_RD_INT_EN, val);
    rc = ioctl(mercury_fd, MSM_MCR_IOCTL_HW_CMD, &hw_cmd);
    if (rc) {
        MCR_PR_ERR("(%d)%s()  (rc=%d) ioctl fails\n\n", __LINE__, __func__, rc);
        return -1;
    }

    return 0;
}





uint32_t mercury_lib_hw_jpegd_rtdma_wr_status_clear(int mcrfd)
{
    struct msm_mercury_hw_cmd hw_cmd;
    uint32_t val;
    int rc;

    val = 0;
    MEM_OUTF(&val, RTDMA_JPEG_WR_INT_EN, SW_RESET_ABORT_RDY_EN, 0);
    MEM_OUTF(&val, RTDMA_JPEG_WR_INT_EN, ERR_EN, 0);
    MEM_OUTF(&val, RTDMA_JPEG_WR_INT_EN, EOF_EN, 0);
    MEM_OUTF(&val, RTDMA_JPEG_WR_INT_EN, SOF_EN, 0);
    mercury_write(RTDMA_JPEG_WR_INT_EN, val);
    rc = ioctl(mcrfd, MSM_MCR_IOCTL_HW_CMD, &hw_cmd);
    if (rc) {
        MCR_PR_ERR("(%d)%s()  (rc=%d) ioctl fails\n\n", __LINE__, __func__, rc);
        return -1;
    }

    val = 0;
    MEM_OUTF(&val, RTDMA_JPEG_WR_STA_ACK, SW_RESET_ABORT_RDY_ACK, 1);
    MEM_OUTF(&val, RTDMA_JPEG_WR_STA_ACK, ERR_ACK, 1);
    MEM_OUTF(&val, RTDMA_JPEG_WR_STA_ACK, EOF_ACK, 1);
    MEM_OUTF(&val, RTDMA_JPEG_WR_STA_ACK, SOF_ACK, 1);
    mercury_write(RTDMA_JPEG_WR_STA_ACK, val);
    rc = ioctl(mcrfd, MSM_MCR_IOCTL_HW_CMD, &hw_cmd);
    if (rc) {
        MCR_PR_ERR("(%d)%s()  (rc=%d) ioctl fails\n\n", __LINE__, __func__, rc);
        return -1;
    }

    return val;
}


int mercury_lib_hw_jpegd_rtdma_wr_status_enable(uint8_t err_en_val,
    uint8_t eof_en_val,
    uint8_t sof_en_val)
{
    int rc;
    struct msm_mercury_hw_cmd hw_cmd;

    uint32_t val = 0;
    MEM_OUTF(&val, RTDMA_JPEG_WR_INT_EN, ERR_EN, err_en_val);
    MEM_OUTF(&val, RTDMA_JPEG_WR_INT_EN, EOF_EN, eof_en_val);
    MEM_OUTF(&val, RTDMA_JPEG_WR_INT_EN, SOF_EN, sof_en_val);
    mercury_write(RTDMA_JPEG_WR_INT_EN, val);
    rc = ioctl(mercury_fd, MSM_MCR_IOCTL_HW_CMD, &hw_cmd);
    if (rc) {
        MCR_PR_ERR("(%d)%s()  (rc=%d) ioctl fails\n\n", __LINE__, __func__, rc);
        return -1;
    }

    return 0;
}

int mercury_lib_hw_jpegd_rtdma_rd_status_clear(int mcrfd)
{
    struct msm_mercury_hw_cmd hw_cmd;
    uint32_t val;
    int rc;

    val = 0;
    MEM_OUTF(&val, RTDMA_JPEG_RD_INT_EN, EOF_EN, 0);
    MEM_OUTF(&val, RTDMA_JPEG_RD_INT_EN, SOF_EN, 0);
    mercury_write(RTDMA_JPEG_RD_INT_EN, val);
    rc = ioctl(mcrfd, MSM_MCR_IOCTL_HW_CMD, &hw_cmd);
    if (rc) {
        MCR_PR_ERR("(%d)%s()  (rc=%d) ioctl fails\n\n", __LINE__, __func__, rc);
        return -1;
    }

    val = 0;
    MEM_OUTF(&val, RTDMA_JPEG_RD_STA_ACK, EOF_ACK, 1);
    MEM_OUTF(&val, RTDMA_JPEG_RD_STA_ACK, SOF_ACK, 1);
    mercury_write(RTDMA_JPEG_RD_STA_ACK, val);
    rc = ioctl(mcrfd, MSM_MCR_IOCTL_HW_CMD, &hw_cmd);
    if (rc) {
        MCR_PR_ERR("(%d)%s()  (rc=%d) ioctl fails\n\n", __LINE__,
            __func__, rc);
        return -1;
    }

    return 0;
}

int mercury_lib_jpegd_wr_op_cfg(uint8_t align, uint8_t flip,
    uint8_t mirror)
{
    int rc;
    struct msm_mercury_hw_cmd hw_cmd;

    uint32_t val = 0;
    MEM_OUTF3(&val, RTDMA_JPEG_WR_OP, ALIGN, FLIP, MIRROR, align,
        flip, mirror);

    mercury_write(RTDMA_JPEG_WR_OP, val);
    rc = ioctl(mercury_fd, MSM_MCR_IOCTL_HW_CMD, &hw_cmd);
    if (rc) {
        MCR_PR_ERR("(%d)%s()  (rc=%d) ioctl fails\n\n", __LINE__,
            __func__, rc);
        return -1;
    }

    return 0;
}


int mercury_lib_jpeg_dri(uint32_t restartInterval)
{
    uint32_t retVal;
    uint16_t len;
    uint32_t val;
    int rc;

    struct msm_mercury_hw_cmd hw_cmd;


    MCR_DBG("    Restart Interval = 0x%x\n", restartInterval);

    if (restartInterval) {
        val = 0;
        MEM_OUTF(&val, JPEG_DRI, JPEG_DRI_RI, restartInterval);
        mercury_write(JPEG_DRI, val);
        rc = ioctl (mercury_fd, MSM_MCR_IOCTL_HW_CMD, &hw_cmd);
        if (rc  < 0) {
            MCR_PR_ERR ("(%d)%s  %s ioctl-%d error...\n",
                __LINE__, __func__, MSM_MERCURY_NAME,
                MSM_MCR_IOCTL_HW_CMD);
            return 1;
        }

        MCR_DBG("[PROG] JPEG_DRI: 0x%08X\n", val);
    }

    return restartInterval;
}


int mercury_lib_hw_update_bus_write_config(uint8_t *y_buf,
    uint8_t *u_buf, uint8_t *v_buf)
{
    struct msm_mercury_hw_cmds *p_hw_cmds;
    struct msm_mercury_hw_cmd *p_hw_cmd;
    int rc;

    p_hw_cmds =
        mercury_lib_hw_cmd_malloc_and_copy ( sizeof (struct msm_mercury_hw_cmd)*3,
        NULL);
    if (!p_hw_cmds) {
        return 1;
    }

    p_mercury_writes(RTDMA_JPEG_WR_BUF_Y_PNTR, (uint32_t)y_buf);
    p_mercury_writes(RTDMA_JPEG_WR_BUF_U_PNTR, (uint32_t)u_buf);
    p_mercury_writes(RTDMA_JPEG_WR_BUF_V_PNTR, (uint32_t)v_buf);

    rc = ioctl (mercury_fd, MSM_MCR_IOCTL_HW_CMDS, p_hw_cmds);
    if (rc  < 0) {
        MCR_PR_ERR ("(%d)%s()  %s ioctl-%d error...\n",
            __LINE__, __func__, MSM_MERCURY_NAME,
            _IOC_NR(MSM_MCR_IOCTL_HW_CMDS));
        free(p_hw_cmds);
        return 1;
    }

    free(p_hw_cmds);
    return 0;
}

int mercury_lib_hw_decode(void)
{
    uint32_t val=0;
    struct msm_mercury_hw_cmd hw_cmd;
    int rc;
    if (mercury_lib_hw_jpegd_rtdma_rd_status_enable(0, 0) ||
        mercury_lib_hw_jpegd_rtdma_wr_status_enable(1, 1, 0)) {
        MCR_PR_ERR("\n(%d)%s()  Error Configuring JPEGD HW\n",
            __LINE__, __func__);
        return 1;
    }

    MEM_OUTF(&val, RTDMA_JPEG_RD_BUF_MNGR_BUF_ID_FIFO, BUF_APPLY, 1);
    MEM_OUTF(&val, RTDMA_JPEG_RD_BUF_MNGR_BUF_ID_FIFO, BUF_EOF, 1);
    MEM_OUTF(&val, RTDMA_JPEG_RD_BUF_MNGR_BUF_ID_FIFO, BUF_SOF, 1);
    mercury_write(RTDMA_JPEG_RD_BUF_MNGR_BUF_ID_FIFO, val);
    rc = ioctl(mercury_fd, MSM_MCR_IOCTL_HW_CMD, &hw_cmd);
    if (rc) {
        MCR_PR_ERR("(%d)%s()  (rc=%d) ioctl fails\n\n", __LINE__,
            __func__, rc);
        return -1;
    }

    return 0;
}


uint32_t mercury_lib_hw_check_rd_ack_irq(void)
{
    struct msm_mercury_hw_cmd hw_cmd;
    int rc;

    rc = mercury_read(RTDMA_JPEG_RD_STA_ACK);
    MCR_DBG("-(%d)%s()  (rc=%d) RTDMA_JPEG_RD_STA_ACK = 0x%08X\n",
        __LINE__, __func__, rc, hw_cmd.data);
    if (rc) {
        MCR_PR_ERR("(%d)%s()  (rc=%d) ioctl fails\n\n", __LINE__, __func__, rc);
        return -1;
    }

    return hw_cmd.data;
}

uint32_t mercury_lib_hw_check_wr_ack_irq(void)
{
    struct msm_mercury_hw_cmd hw_cmd;
    int rc;

    rc = mercury_read(RTDMA_JPEG_WR_STA_ACK);
    MCR_DBG("-(%d)%s()  (rc=%d) RTDMA_JPEG_WR_STA_ACK = 0x%08X\n",
        __LINE__, __func__, rc, hw_cmd.data);
    if (rc) {
        MCR_PR_ERR("(%d)%s()  (rc=%d) ioctl fails\n\n",
            __LINE__, __func__, rc);
        return -1;
    }

    return hw_cmd.data;
}

uint32_t mercury_lib_hw_check_jpeg_status(void)
{
    struct msm_mercury_hw_cmd hw_cmd;
    int rc;

    rc = mercury_read(JPEG_STATUS);
    MCR_DBG("-(%d)%s()  (rc=%d) JPEG_STATUS = 0x%08X\n",
        __LINE__, __func__, rc, hw_cmd.data);
    if (rc) {
        MCR_PR_ERR("(%d)%s()  (rc=%d) ioctl fails\n\n",
            __LINE__, __func__, rc);
        return -1;
    }
    return hw_cmd.data;
}

int mercury_lib_hw_check_hw_status(void)
{
    struct msm_mercury_hw_cmd hw_cmd;
    int rc;

    rc = mercury_read(JPEG_STATUS);
    MCR_DBG("  (rc=%d) JPEG_STATUS = 0x%08X\n", rc, hw_cmd.data);
    if (rc) {
        MCR_PR_ERR("(%d)%s()  (rc=%d) ioctl fails\n\n",
            __LINE__, __func__, rc);
        return -1;
    }

    rc = mercury_read(RTDMA_JPEG_WR_STA_ACK);
    MCR_DBG("  (rc=%d) RTDMA_JPEG_WR_STA_ACK = 0x%08X\n", rc, hw_cmd.data);
    if (rc) {
        MCR_PR_ERR("(%d)%s()  (rc=%d) ioctl fails\n\n",
            __LINE__, __func__, rc);
        return -1;
    }

    rc = mercury_read(RTDMA_JPEG_RD_STA_ACK);
    MCR_DBG("  (rc=%d) RTDMA_JPEG_RD_STA_ACK = 0x%08X\n", rc, hw_cmd.data);
    if (rc) {
        MCR_PR_ERR("(%d)%s()  (rc=%d) ioctl fails\n\n",
            __LINE__, __func__, rc);
        return -1;
    }

    return 0;
}


int mercury_lib_hw_check_buf_configs(void)
{
    struct msm_mercury_hw_cmd hw_cmd;
    int rc;

    rc = mercury_read(RTDMA_JPEG_RD_BUF_Y_PNTR);
    MCR_DBG("  (rc=%d) RTDMA_JPEG_RD_BUF_Y_PNTR = 0x%08X\n",
        rc, hw_cmd.data);
    if (rc) {
        MCR_PR_ERR("(%d)%s()  (rc=%d) ioctl fails\n\n",
            __LINE__, __func__, rc);
        return -1;
    }

    rc = mercury_read(RTDMA_JPEG_WR_BUF_Y_PNTR);
    MCR_DBG("  (rc=%d) RTDMA_JPEG_WR_BUF_Y_PNTR = 0x%08X\n",
        rc, hw_cmd.data);
    if (rc) {
        MCR_PR_ERR("(%d)%s()  (rc=%d) ioctl fails\n\n",
            __LINE__, __func__, rc);
        return -1;
    }

    rc = mercury_read(RTDMA_JPEG_WR_BUF_U_PNTR);
    MCR_DBG("  (rc=%d) RTDMA_JPEG_WR_BUF_U_PNTR = 0x%08X\n\n",
        rc, hw_cmd.data);
    if (rc) {
        MCR_PR_ERR("(%d)%s()  (rc=%d) ioctl fails\n\n",
            __LINE__, __func__, rc);
        return -1;
    }

    return 0;
}


int mercury_lib_wait_for_wr_sta_irq(uint32_t timeout)
{
    int cnt = 0;
    uint32_t status;
    struct msm_mercury_hw_cmd hw_cmd;

    uint32_t irq_mask = 0;
    MEM_OUTF(&irq_mask, RTDMA_JPEG_WR_STA_ACK, EOF_ACK, 1);

    mercury_read(RTDMA_JPEG_WR_STA_ACK);
    status = hw_cmd.data;
    while ((status & irq_mask) != irq_mask) {
        cnt++;
        if (cnt > timeout) {
            MCR_PR_ERR("\n[ERROR] RTDMA_JPEG_WR_STA_ACK timeout occurred");
            MCR_PR_ERR("IRQ_STATUS = 0x%08X\n",status);
            break;
        }

        if (cnt%20==0) {
            mercury_read(RTDMA_JPEG_WR_STA_ACK);
            status = hw_cmd.data;
        }
    }

    mercury_read(RTDMA_JPEG_WR_STA_ACK);
    status = hw_cmd.data;
    MCR_DBG("RTDMA_JPEG_WR_STA_ACK = 0x%08X (cnt=%d)\n", status, cnt);

    return cnt;
}
