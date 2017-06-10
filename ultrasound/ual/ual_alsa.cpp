/*===========================================================================
           ual_alsa.cpp

DESCRIPTION:


INITIALIZATION AND SEQUENCING REQUIREMENTS:


Copyright (c) 2011-2013 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
=============================================================================*/

/*----------------------------------------------------------------------------
   Macros and constants
----------------------------------------------------------------------------*/
#define LOG_TAG  "UAL_ALSA"

/*----------------------------------------------------------------------------
   Include files
----------------------------------------------------------------------------*/
#include "usf_log.h"
#include <sys/ioctl.h>
#include <errno.h>
#include <tinyalsa/asoundlib.h>
#include "ual.h"
#include "ual_alsa_defs.h"
#include <usnd_route.h>

static struct pcm *s_pcm_tx = NULL;
static struct pcm *s_pcm_rx = NULL;
static int s_tx_pcm_ind;
static int s_rx_pcm_ind;
static int s_tx_pcm_card;
static int s_rx_pcm_card;

static const uint16_t TOPOLOGY_NAME_PREFIX_SIZE = 2;
static const uint16_t MAX_TOPOLOGY_NAME_SIZE = TOPOLOGY_NAME_PREFIX_SIZE + 48;
static char s_tx_topology_name[MAX_TOPOLOGY_NAME_SIZE];
static char s_rx_topology_name[MAX_TOPOLOGY_NAME_SIZE];

static struct usnd_route *s_usnd_route = NULL;

/*==============================================================================
   FUNCTION:  ual_alsa_get_topology
==============================================================================*/
/**
   Get topology
*/
static bool ual_alsa_get_topology
(
  // Number of required ports
  uint16_t req_ports_num,
  // Required ports identifications
  uint8_t *req_ports,
  // [out] The ports' data offsets
  uint8_t *offsets,
  // [in,out] Topology name
  char *topology_name,
  // [in] Topology name size
  uint16_t topology_name_size
)
{
  uint8_t ordered_ports[UAL_MAX_PORT_NUM];

  if ((NULL == req_ports) ||
      (NULL == offsets) ||
      (NULL == topology_name) ||
      (UAL_MAX_PORT_NUM < req_ports_num))
  {
    LOGE("%s:  wrong params: ports=%p; amount=%d; offsets=%p; name=%p",
         __FUNCTION__,
         req_ports,
         req_ports_num,
         offsets,
         topology_name
    );
    return false;
  }

  // Calculate offsets
  bool rc = true;
  uint16_t i = 0;

  memset(offsets, 0, req_ports_num);
  for (i = 0; i < req_ports_num; ++i)
  {
    for (uint16_t j = i+1; j < req_ports_num; ++j)
    {
      if (req_ports[i] == req_ports[j])
      {
        LOGE("%s:  duplicated ports: %d,%d; port_id=%d",
             __FUNCTION__,
             i,
             j,
             req_ports[i]);
        return false;
      }

      uint16_t max_ind = (req_ports[i] > req_ports[j])? i: j;
      ++offsets[max_ind];
    }
    uint16_t ordered_ind = offsets[i];
    // ordered_ind < req_ports_num
    ordered_ports[ordered_ind] = req_ports[i];
    LOGD("%s: ind=%d: offset=%d; req_port=%d",
         __FUNCTION__,
         i,
         offsets[i],
         req_ports[i]);
  }

  // Build topology name
  int len = 0;
  for (i = 0; i < req_ports_num; ++i)
  {
    int ret = snprintf(topology_name+len,
                      topology_name_size-len,
                      "_%d",
                      ordered_ports[i]);
    // check truncation
    if (ret >= (topology_name_size - len))
      break;
    len += ret;
  }
  if (i < req_ports_num)
  {
    LOGE("%s:topology name size (%d) is not enough",
          __FUNCTION__,
          topology_name_size);
    rc = false;
  }

  return rc;
} // ual_alsa_get_topology

/*==============================================================================
  FUNCTION:   ual_alsa_open
==============================================================================*/
/**
  Open ual_alsa sub-system
*/
bool ual_alsa_open()
{
  const char *MIXER_XML_PATH = "/persist/usf/mixer/mixer_paths.xml";
  const uint32_t MAX_CTLS_AMOUNT = 60;
  int ret = 0;
  bool rc = true;

  if (s_usnd_route)
  {
    LOGW("%s:  ual_alsa was already opened",
         __FUNCTION__);
    return true;
  }

  FILE* pcm_inds_file = fopen(UAL_PCM_INDS_NAME,
                              "r");

  if (NULL == pcm_inds_file)
  {
    LOGE("%s: pcm_port_file(%s) open failed; errno=%d",
         __FUNCTION__,
         UAL_PCM_INDS_NAME,
         errno);
    return false;
  }

  ret = fscanf(pcm_inds_file,
                "%d %d %d %d",
                &s_tx_pcm_ind,
                &s_rx_pcm_ind,
                &s_tx_pcm_card,
                &s_rx_pcm_card);
  (void)fclose(pcm_inds_file);
  pcm_inds_file=NULL;

  if ((PCM_INDS_NUM != ret) ||
      (s_tx_pcm_card != s_rx_pcm_card))
  {
    LOGE("%s: wrong file(%s); rc=%d; tx_card=%d; rx_card=%d;",
         __FUNCTION__,
         UAL_PCM_INDS_NAME,
         ret,
         s_tx_pcm_card,
         s_rx_pcm_card);
     return false;
  }

  s_usnd_route = usnd_route_init_ext(s_tx_pcm_card,
                                       MIXER_XML_PATH,
                                       MAX_CTLS_AMOUNT);
  if (NULL == s_usnd_route)
  {
      LOGE("%s: Failed to init audio route controls, aborting.",
           __FUNCTION__);
      rc = false;
  }
  else
  {
    uint32_t len1 = strlcpy(s_tx_topology_name,
                       "tx",
                       sizeof(s_tx_topology_name));
    uint32_t len2 = strlcpy(s_rx_topology_name,
                       "rx",
                       sizeof(s_rx_topology_name));
    if ((len1 >= sizeof(s_tx_topology_name)) ||
        (len2 >= sizeof(s_rx_topology_name)))
    {
      LOGE("%s: Wrong topolgy names sizes; tx_size=%d; rx_size=%d",
           __FUNCTION__,
           sizeof(s_tx_topology_name),
           sizeof(s_rx_topology_name));
      rc = false;
    }
  }

  return rc;
}

/*==============================================================================
  FUNCTION:  ual_alsa_close
==============================================================================*/
/**
  Close ual_alsa subsystem
*/
void ual_alsa_close()
{
  if (NULL != s_usnd_route)
  {
    usnd_route_free(s_usnd_route);
    s_usnd_route = NULL;
  }
}

/*==============================================================================
   FUNCTION:  ual_alsa_close_device
==============================================================================*/
/**
   Close device (Tx or Rx)
*/
bool ual_alsa_close_device
(
  // Direction (Tx, Rx)
  ual_direction_type direction
)
{
  bool rc = false;
  char *topology_name = s_tx_topology_name;
  struct pcm **pp_pcm = &s_pcm_tx;


  if (UAL_RX_DIRECTION == direction)
  {
    pp_pcm = &s_pcm_rx;
    topology_name = s_rx_topology_name;
  }

  if (NULL != *pp_pcm)
  {
    int err = pcm_close(*pp_pcm);
    if (err)
    {
        LOGE("%s: pcm_close [dir=%d] failed with error %d",
             __FUNCTION__,
             direction,
             err);
    }
    else
    {
      *pp_pcm = NULL;
      rc = true;
    }
  }

  if ((NULL != s_usnd_route) &&
      (0 != topology_name[TOPOLOGY_NAME_PREFIX_SIZE]))
  {
    int ret = usnd_route_reset_path(s_usnd_route,
                                     topology_name);
    ret &= usnd_route_update_mixer(s_usnd_route);
    if (!ret)
    {
      // set the name into its init state
      topology_name[TOPOLOGY_NAME_PREFIX_SIZE] = 0;
    }
    else
      rc = false;
  }

  return  rc;
} // ual_alsa_close_device

/*==============================================================================
   FUNCTION:  ual_alsa_open_device
==============================================================================*/
/**
   Open sound device for ultrasound
*/
int ual_alsa_open_device
(
  // Configuration struct
  us_xx_info_type &us_xx_info,
  // Direction (Tx, Rx)
  ual_direction_type direction
)
{
  // The pcm is fake in USND case - we've dedicated data path
  const unsigned int USND_PCM_PERIOD_COUNT = 2;
  const unsigned int USND_PCM_PERIOD_SIZE = 10;
  // aDSP:AFE ports, intended for the Tx, Rx sound devices (hardcoded for now)
  const int AFE_PORT_ID_RX = 0x4004;
  const int AFE_PORT_ID_TX = 0x4005;

  uint8_t offsets[UAL_MAX_PORT_NUM];
  char *topology_name = s_tx_topology_name;
  int afe_port = AFE_PORT_ID_TX;
  int card = s_tx_pcm_card;
  int pcm_ind = s_tx_pcm_ind;
  int pcm_flags = PCM_IN;
  static struct pcm_config config;
  struct pcm **pp_pcm = &s_pcm_tx;

  if (UAL_RX_DIRECTION == direction)
  {
    topology_name = s_rx_topology_name;
    card = s_rx_pcm_card;
    pcm_ind = s_rx_pcm_ind;
    afe_port = AFE_PORT_ID_RX;
    pcm_flags = PCM_OUT;
    pp_pcm = &s_pcm_rx;
  }

  if (NULL != *pp_pcm)
  {
    LOGE("%s:  direction[%d] PCM was opened",
         __FUNCTION__,
         direction);
    return -1;
  }

  bool rc = ual_alsa_get_topology(us_xx_info.port_cnt,
                             us_xx_info.port_id,
                             offsets,
                             topology_name+TOPOLOGY_NAME_PREFIX_SIZE,
                             MAX_TOPOLOGY_NAME_SIZE-TOPOLOGY_NAME_PREFIX_SIZE);
   LOGD("%s:topology name (%s)",
          __FUNCTION__,
          topology_name);
  if (!rc)
  {
    LOGE("%s:  get topology [direction=%d; port_id=%d; port_cnt=%d] failed",
         __FUNCTION__,
         direction,
         us_xx_info.port_id[0],
         us_xx_info.port_cnt);
    return -1;
  }
  // USF driver expects offsets sent in port_id
  memcpy(us_xx_info.port_id,
         offsets,
         UAL_MAX_PORT_NUM);

  // Configure mixer
  int ret = usnd_route_apply_path(s_usnd_route,
                                   topology_name);
  if (ret)
  {
    // set the name into its init state
    topology_name[TOPOLOGY_NAME_PREFIX_SIZE] = 0;
    return -1;
  }
  usnd_route_update_mixer(s_usnd_route);

   // Configure pcm stream
  config.channels = us_xx_info.port_cnt;
  config.rate = us_xx_info.sample_rate;
  config.format = PCM_FORMAT_S16_LE;
  config.period_count = USND_PCM_PERIOD_COUNT;
  config.period_size = USND_PCM_PERIOD_SIZE;

  LOGD("%s: card=%d; pcm_ind=%d; ",
             __FUNCTION__,
             card, pcm_ind);

  // prepare for pcm_open() & pcm_start:
  // default value is failure
  ret = -1;
  *pp_pcm = pcm_open(card,
                     pcm_ind,
                     pcm_flags,
                     &config);
  if ((*pp_pcm) &&
       pcm_is_ready(*pp_pcm))
  {
    ret = pcm_start(*pp_pcm);
  }

  if (ret)
  {
    LOGE("%s:  %s",
          __func__,
          pcm_get_error(*pp_pcm));
    pcm_close(*pp_pcm);
    *pp_pcm = NULL;
    afe_port = -1;
  }

  return afe_port;
} // ual_alsa_open_device
