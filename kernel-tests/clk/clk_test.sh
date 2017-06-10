#-----------------------------------------------------------------------------
# Copyright (c) 2008-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
# Qualcomm Technologies Proprietary and Confidential.
#-----------------------------------------------------------------------------

###############################
#                             #
# Global lists for all SoCs   #
#                             #
###############################

# Clocks that should not be tested
global_blacklist="
		measure_clk
		dsi_vco_clk
		analog_postdiv_clk
		indirect_path_div2_clk
		pixel_clk_src
		byte_mux
		byte_clk_src
		byte0_clk_src byte1_clk_src
		pclk0_clk_src pclk1_clk_src
		mdss_byte0_clk mdss_byte1_clk
		mdss_pclk0_clk mdss_pclk1_clk
		extpclk_clk_src mdss_extpclk_clk
		krait0_m_clk krait1_m_clk krait2_m_clk krait3_m_clk l2_m_clk
		krait0_clk krait1_clk krait2_clk krait3_clk l2_clk
		krait0_pri_mux_clk
		krait1_pri_mux_clk
		krait2_pri_mux_clk
		krait3_pri_mux_clk
		l2_pri_mux_clk
		krait0_sec_mux_clk
		krait1_sec_mux_clk
		krait2_sec_mux_clk
		krait3_sec_mux_clk
		l2_sec_mux_clk
		hfpll_src_clk
		hfpll0_clk hfpll1_clk hfpll2_clk hfpll3_clk hfpll_l2_clk
		hfpll0_div_clk
		hfpll1_div_clk
		hfpll2_div_clk
		hfpll3_div_clk
		hfpll_l2_div_clk
		apc0_m_clk
		apc1_m_clk
		apc2_m_clk
		apc3_m_clk
		gcc_debug_mux
		rpm_debug_mux
		mmss_debug_mux
		debug_mmss_clk
		kpss_debug_pri_mux
		wcnss_m_clk
		xo xo_a_clk
		cxo_clk_src cxo_a_clk_src
		gcc_xo gcc_xo_a_clk
		qdss_clk qdss_a_clk
		snoc_clk snoc_a_clk
		pnoc_clk pnoc_a_clk
		cnoc_clk cnoc_a_clk
		pcnoc_clk pcnoc_a_clk
		bimc_clk bimc_a_clk
		cxo_otg_clk cxo_pil_lpass_clk cxo_pil_mss_clk cxo_wlan_clk
		cxo_pil_pronto_clk cxo_dwc3_clk cxo_ehci_host_clk cxo_lpm_clk
		snoc_msmbus_clk snoc_msmbus_a_clk
		pnoc_msmbus_clk pnoc_msmbus_a_clk
		cnoc_msmbus_clk cnoc_msmbus_a_clk
		pcnoc_msmbus_clk pcnoc_msmbus_a_clk
		pnoc_sps_clk pcnoc_sps_clk
		bimc_msmbus_clk bimc_msmbus_a_clk
		ocmemgx_clk ocmemgx_a_clk
		mmssnoc_ahb_clk mmssnoc_ahb_a_clk
		mmssnoc_ahb
		gcc_ufs_rx_symbol_0_clk gcc_ufs_rx_symbol_1_clk
		gcc_ufs_tx_symbol_0_clk gcc_ufs_tx_symbol_1_clk
		gcc_lpass_q6_axi_clk
		pcie_0_pipe_clk_src pcie_0_pipe_clk_src
		gcc_pcie_0_pipe_clk gcc_pcie_1_pipe_clk
		acpu_aux_clk debug_cpu_clk
		gcc_pcie_phy_0_reset gcc_pcie_phy_1_reset
		gcc_qusb2_phy_reset gcc_usb3_phy_reset
		gcc_usb3phy_phy_reset
		"

# Clocks that should not be tested on Android
global_android_blacklist="
		sdcc1_apps_clk_src
		gcc_sdcc1_apps_clk
		gcc_sdcc1_ahb_clk
		gcc_sdcc1_cdccal_sleep_clk
		gcc_sdcc1_cdccal_ff_clk
		sdcc2_apps_clk_src
		gcc_sdcc2_apps_clk
		gcc_sdcc2_ahb_clk
		sdcc3_apps_clk_src
		gcc_sdcc3_apps_clk
		gcc_sdcc3_ahb_clk
		sdcc4_apps_clk_src
		gcc_sdcc4_apps_clk
		gcc_sdcc4_ahb_clk
		gpll4 gpll4_out_main
		"

# Clocks that are voteable: they might not turn off when disabled
# This also includes RPM NoCs
global_votable_clocklist="
		gcc_bam_dma_ahb_clk
		gcc_blsp1_ahb_clk
		gcc_blsp2_ahb_clk
		gcc_boot_rom_ahb_clk
		gcc_ce1_ahb_clk
		gcc_ce1_axi_clk
		gcc_ce1_clk
		gcc_ce2_ahb_clk
		gcc_ce2_axi_clk
		gcc_ce2_clk
		gcc_ce3_ahb_clk
		gcc_ce3_axi_clk
		gcc_ce3_clk
		gcc_prng_ahb_clk
		gcc_bam_dma_inactivity_timers_clk
		"

###############################
#                             #
# SoC-Specific Configuration: #
#                             #
###############################

# Given a clock, list its children in the $children variable.
get_children ()
{
  parent=$1
  child_list=""
  if [ "x$soc" = "x8660" ]; then
    # Only list children with measurable rates.  If a child is measurable
    # and so is its parent, but they are expected to report different rates
    # then do not list the children AND blacklist them instead.
    if [ "$parent" = "usb_fs1_src_clk" -o "$parent" = "all" ]; then
      child_list="$child_list usb_fs1_xcvr_clk usb_fs1_sys_clk"
    fi
    if [ "$parent" = "usb_fs2_src_clk" -o "$parent" = "all" ]; then
      child_list="$child_list usb_fs2_xcvr_clk usb_fs2_sys_clk"
    fi
    if [ "$parent" = "csi_src_clk" -o "$parent" = "all" ]; then
      child_list="$child_list csi0_clk csi1_clk"
    fi
    if [ "$parent" = "pixel_mdp_clk" -o "$parent" = "all" ]; then
      child_list="$child_list pixel_lcdc_clk"
    fi
    if [ "$parent" = "tv_src_clk" -o "$parent" = "all" ]; then
      child_list="$child_list tv_enc_clk tv_dac_clk mdp_tv_clk hdmi_tv_clk"
    fi
    if [ "$parent" = "vfe_clk" -o "$parent" = "all" ]; then
      child_list="$child_list csi0_vfe_clk csi1_vfe_clk"
    fi
  elif [ "x$soc" = "x7X30" ]; then
    if [ "$parent" = "grp_3d_src_clk" -o "$parent" = "all" ]; then
      child_list="$child_list imem_clk grp_3d_clk"
    fi
    if [ "$parent" = "mdp_lcdc_pclk_clk" -o "$parent" = "all" ]; then
      child_list="$child_list mdp_lcdc_pad_pclk_clk"
    fi
    if [ "$parent" = "mi2s_codec_rx_m_clk" -o "$parent" = "all" ]; then
      child_list="$child_list mi2s_codec_rx_s_clk"
    fi
    if [ "$parent" = "mi2s_codec_tx_m_clk" -o "$parent" = "all" ]; then
      child_list="$child_list mi2s_codec_tx_s_clk"
    fi
    if [ "$parent" = "mi2s_m_clk" -o "$parent" = "all" ]; then
      child_list="$child_list mi2s_s_clk"
    fi
    if [ "$parent" = "sdac_clk" -o "$parent" = "all" ]; then
      child_list="$child_list sdac_m_clk"
    fi
    if [ "$parent" = "tv_clk" -o "$parent" = "all" ]; then
      child_list="$child_list tv_dac_clk tv_enc_clk hdmi_clk"
    fi
    if [ "$parent" = "vfe_clk" -o "$parent" = "all" ]; then
      child_list="$child_list vfe_mdc_clk vfe_camif_clk csi0_vfe_clk"
    fi
  elif [ "x$soc" = "x8960" ]; then
    if [ "$parent" = "usb_fs1_src_clk" -o "$parent" = "all" ]; then
      child_list="$child_list usb_fs1_xcvr_clk usb_fs1_sys_clk"
    fi
    if [ "$parent" = "usb_fs2_src_clk" -o "$parent" = "all" ]; then
      child_list="$child_list usb_fs2_xcvr_clk usb_fs2_sys_clk"
    fi
    if [ "$parent" = "csi0_src_clk" -o "$parent" = "all" ]; then
      child_list="$child_list csi0_clk csi0_phy_clk"
    fi
    if [ "$parent" = "csi1_src_clk" -o "$parent" = "all" ]; then
      child_list="$child_list csi1_clk csi1_phy_clk"
    fi
    if [ "$parent" = "csi2_src_clk" -o "$parent" = "all" ]; then
      child_list="$child_list csi2_clk csi2_phy_clk"
    fi
    if [ "$parent" = "csiphy_timer_src_clk" -o "$parent" = "all" ]; then
      child_list="$child_list csi0phy_timer_clk csi1phy_timer_clk csi2phy_timer_clk"
    fi
    if [ "$parent" = "tv_src_clk" -o "$parent" = "all" ]; then
      child_list="$child_list tv_enc_clk tv_dac_clk mdp_tv_clk hdmi_tv_clk"
    fi
    if [ "$parent" = "vfe_clk" -o "$parent" = "all" ]; then
      child_list="$child_list csi_vfe_clk"
    fi
    if [ "$parent" = "mdp_clk" -o "$parent" = "all" ]; then
      child_list="$child_list lut_mdp_clk"
    fi
  elif [ "x$soc" = "x8064" ]; then
    if [ "$parent" = "usb_fs1_src_clk" -o "$parent" = "all" ]; then
      child_list="$child_list usb_fs1_xcvr_clk usb_fs1_sys_clk"
    fi
    if [ "$parent" = "usb_hsic_xcvr_fs_clk" -o "$parent" = "all" ]; then
      child_list="$child_list usb_hsic_system_clk"
    fi
    if [ "$parent" = "csi0_src_clk" -o "$parent" = "all" ]; then
      child_list="$child_list csi0_clk csi0_phy_clk"
    fi
    if [ "$parent" = "csi1_src_clk" -o "$parent" = "all" ]; then
      child_list="$child_list csi1_clk csi1_phy_clk"
    fi
    if [ "$parent" = "csi2_src_clk" -o "$parent" = "all" ]; then
      child_list="$child_list csi2_clk csi2_phy_clk"
    fi
    if [ "$parent" = "csiphy_timer_src_clk" -o "$parent" = "all" ]; then
      child_list="$child_list csi0phy_timer_clk csi1phy_timer_clk csi2phy_timer_clk"
    fi
    if [ "$parent" = "tv_src_clk" -o "$parent" = "all" ]; then
      child_list="$child_list hdmi_tv_clk rgb_tv_clk"
    fi
    if [ "$parent" = "vfe_clk" -o "$parent" = "all" ]; then
      child_list="$child_list csi_vfe_clk"
    fi
    if [ "$parent" = "mdp_clk" -o "$parent" = "all" ]; then
      child_list="$child_list lut_mdp_clk"
    fi
    if [ "$parent" = "vcap_clk" -o "$parent" = "all" ]; then
      child_list="$child_list vcap_npl_clk"
    fi
    if [ "$parent" = "ce3_src_clk" -o "$parent" = "all" ]; then
      child_list="$child_list ce3_p_clk ce3_core_clk"
    fi
    if [ "$parent" = "sata_src_clk" -o "$parent" = "all" ]; then
      child_list="$child_list sata_rxoob_clk sata_pmalive_clk"
    fi
  elif [ "x$soc" = "x8930" ]; then
    if [ "$parent" = "usb_fs1_src_clk" -o "$parent" = "all" ]; then
      child_list="$child_list usb_fs1_xcvr_clk usb_fs1_sys_clk"
    fi
    if [ "$parent" = "usb_fs2_src_clk" -o "$parent" = "all" ]; then
      child_list="$child_list usb_fs2_xcvr_clk usb_fs2_sys_clk"
    fi
    if [ "$parent" = "csi0_src_clk" -o "$parent" = "all" ]; then
      child_list="$child_list csi0_clk csi0_phy_clk"
    fi
    if [ "$parent" = "csi1_src_clk" -o "$parent" = "all" ]; then
      child_list="$child_list csi1_clk csi1_phy_clk"
    fi
    if [ "$parent" = "csi2_src_clk" -o "$parent" = "all" ]; then
      child_list="$child_list csi2_clk csi2_phy_clk"
    fi
    if [ "$parent" = "csiphy_timer_src_clk" -o "$parent" = "all" ]; then
      child_list="$child_list csi0phy_timer_clk csi1phy_timer_clk csi2phy_timer_clk"
    fi
    if [ "$parent" = "tv_src_clk" -o "$parent" = "all" ]; then
      child_list="$child_list tv_dac_clk mdp_tv_clk hdmi_tv_clk"
    fi
    if [ "$parent" = "vfe_clk" -o "$parent" = "all" ]; then
      child_list="$child_list csi_vfe_clk"
    fi
    if [ "$parent" = "mdp_clk" -o "$parent" = "all" ]; then
      child_list="$child_list lut_mdp_clk"
    fi
  elif [ "x$soc" = "x8974" ]; then
    if [ "$parent" = "csi0_clk_src" -o "$parent" = "all" ]; then
      child_list="$child_list camss_csi0_clk camss_csi0phy_clk camss_csi0pix_clk camss_csi0rdi_clk"
    fi
    if [ "$parent" = "csi1_clk_src" -o "$parent" = "all" ]; then
      child_list="$child_list camss_csi1_clk camss_csi1phy_clk camss_csi1pix_clk camss_csi1rdi_clk"
    fi
    if [ "$parent" = "csi2_clk_src" -o "$parent" = "all" ]; then
      child_list="$child_list camss_csi2_clk camss_csi2phy_clk camss_csi2pix_clk camss_csi2rdi_clk"
    fi
    if [ "$parent" = "csi3_clk_src" -o "$parent" = "all" ]; then
      child_list="$child_list camss_csi3_clk camss_csi3phy_clk camss_csi3pix_clk camss_csi3rdi_clk"
    fi
    if [ "$parent" = "vfe0_clk_src" -o "$parent" = "all" ]; then
      child_list="$child_list camss_csi_vfe0_clk camss_vfe_vfe0_clk"
    fi
    if [ "$parent" = "vfe1_clk_src" -o "$parent" = "all" ]; then
      child_list="$child_list camss_csi_vfe1_clk camss_vfe_vfe1_clk"
    fi
  elif [ "x$soc" = "x8962" ]; then
    if [ "$parent" = "csi0_clk_src" -o "$parent" = "all" ]; then
      child_list="$child_list camss_csi0_clk camss_csi0phy_clk camss_csi0pix_clk camss_csi0rdi_clk"
    fi
    if [ "$parent" = "csi1_clk_src" -o "$parent" = "all" ]; then
      child_list="$child_list camss_csi1_clk camss_csi1phy_clk camss_csi1pix_clk camss_csi1rdi_clk"
    fi
    if [ "$parent" = "csi2_clk_src" -o "$parent" = "all" ]; then
      child_list="$child_list camss_csi2_clk camss_csi2phy_clk camss_csi2pix_clk camss_csi2rdi_clk"
    fi
    if [ "$parent" = "vfe0_clk_src" -o "$parent" = "all" ]; then
      child_list="$child_list camss_csi_vfe0_clk camss_vfe_vfe0_clk"
    fi
    if [ "$parent" = "vfe1_clk_src" -o "$parent" = "all" ]; then
      child_list="$child_list camss_csi_vfe1_clk camss_vfe_vfe1_clk"
    fi
    if [ "$parent" = "ocmemnoc_clk_src" -o "$parent" = "all" ]; then
      child_list="$child_list venus0_ocmemnoc_clk"
    fi
    if [ "$parent" = "axi_clk_src" -o "$parent" = "all" ]; then
      child_list="$child_list camss_jpeg_jpeg_axi_clk camss_vfe_vfe_axi_clk mdss_axi_clk
	      mmss_mmssnoc_axi_clk mmss_s0_axi_clk venus0_axi_clk"
    fi
  elif [ "x$soc" = "x9625" ]; then
    if [ "$parent" = "ipa_clk_src" -o "$parent" = "all" ]; then
      child_list="$child_list gcc_ipa_clk gcc_sys_noc_ipa_axi_clk"
    fi
  elif [ "x$soc" = "x8226" ] || [ "x$soc" = "x8026" ] || [ "x$soc" = "x8926" ]; then
    if [ "$parent" = "csi0_clk_src" -o "$parent" = "all" ]; then
      child_list="$child_list camss_csi0_clk camss_csi0phy_clk camss_csi0pix_clk camss_csi0rdi_clk"
    fi
    if [ "$parent" = "csi1_clk_src" -o "$parent" = "all" ]; then
      child_list="$child_list camss_csi1_clk camss_csi1phy_clk camss_csi1pix_clk camss_csi1rdi_clk"
    fi
    if [ "$parent" = "vfe0_clk_src" -o "$parent" = "all" ]; then
      child_list="$child_list camss_csi_vfe0_clk camss_vfe_vfe0_clk"
    fi
  elif [ "x$soc" = "x8084" ]; then
    if [ "$parent" = "ufs_axi_clk_src" -o "$parent" = "all" ]; then
      child_list="$child_list gcc_sys_noc_ufs_axi_clk gcc_ufs_axi_clk"
    fi
    if [ "$parent" = "usb_hsic_ahb_clk_src" -o "$parent" = "all" ]; then
      child_list="$child_list gcc_periph_noc_usb_hsic_ahb_clk gcc_usb_hsic_ahb_clk"
    fi
    if [ "$parent" = "usb30_master_clk_src" -o "$parent" = "all" ]; then
      child_list="$child_list gcc_sys_noc_usb3_axi_clk gcc_usb30_master_clk"
    fi
    if [ "$parent" = "usb30_sec_master_clk_src" -o "$parent" = "all" ]; then
      child_list="$child_list gcc_sys_noc_usb3_sec_axi_clk gcc_usb30_sec_master_clk"
    fi
    if [ "$parent" = "csi0_clk_src" -o "$parent" = "all" ]; then
      child_list="$child_list camss_csi0_clk camss_csi0phy_clk camss_csi0pix_clk camss_csi0rdi_clk"
    fi
    if [ "$parent" = "csi1_clk_src" -o "$parent" = "all" ]; then
      child_list="$child_list camss_csi1_clk camss_csi1phy_clk camss_csi1pix_clk camss_csi1rdi_clk"
    fi
    if [ "$parent" = "csi2_clk_src" -o "$parent" = "all" ]; then
      child_list="$child_list camss_csi2_clk camss_csi2phy_clk camss_csi2pix_clk camss_csi2rdi_clk"
    fi
    if [ "$parent" = "csi3_clk_src" -o "$parent" = "all" ]; then
      child_list="$child_list camss_csi3_clk camss_csi3phy_clk camss_csi3pix_clk camss_csi3rdi_clk"
    fi
    if [ "$parent" = "vfe0_clk_src" -o "$parent" = "all" ]; then
      child_list="$child_list camss_csi_vfe0_clk camss_vfe_vfe0_clk"
    fi
    if [ "$parent" = "vfe1_clk_src" -o "$parent" = "all" ]; then
      child_list="$child_list camss_csi_vfe1_clk camss_vfe_vfe1_clk"
    fi
    if [ "$parent" = "mdp_clk_src" -o "$parent" = "all" ]; then
      child_list="$child_list mdss_mdp_clk mdss_mdp_lut_clk"
    fi
    if [ "$parent" = "ocmemnoc_clk_src" -o "$parent" = "all" ]; then
      child_list="$child_list venus0_ocmemnoc_clk"
    fi
  elif [ "x$soc" = "x9635" ]; then
    if [ "$parent" = "ipa_clk_src" -o "$parent" = "all" ]; then
      child_list="$child_list gcc_ipa_clk gcc_sys_noc_ipa_axi_clk"
    fi
    if [ "$parent" = "usb30_master_clk_src" -o "$parent" = "all" ]; then
      child_list="$child_list gcc_usb30_master_clk gcc_sys_noc_usb3_axi_clk"
    fi
  elif [ "x$soc" = "x8916" ]  || [ "x$soc" = "x8939" ]; then
    if [ "$parent" = "vfe0_clk_src" -o "$parent" = "all" ]; then
      child_list="$child_list gcc_camss_csi_vfe0_clk gcc_camss_vfe0_clk"
    fi
    if [ "$parent" = "gfx3d_clk_src" -o "$parent" = "all" ]; then
      child_list="$child_list gcc_oxili_gmem_clk gcc_oxili_gfx3d_clk"
    fi
    if [ "$parent" = "csi0_clk_src" -o "$parent" = "all" ]; then
      child_list="$child_list gcc_camss_csi0_clk gcc_camss_csi0phy_clk gcc_camss_csi0pix_clk gcc_camss_csi0rdi_clk"
    fi
    if [ "$parent" = "csi1_clk_src" -o "$parent" = "all" ]; then
      child_list="$child_list gcc_camss_csi1_clk gcc_camss_csi1phy_clk gcc_camss_csi1pix_clk gcc_camss_csi1rdi_clk"
    fi
    if [ "$parent" = "camss_ahb_clk_src" -o "$parent" = "all" ]; then
      child_list="$child_list gcc_camss_cci_ahb_clk gcc_camss_csi0_ahb_clk gcc_camss_csi1_ahb_clk gcc_camss_ispif_ahb_clk
	      gcc_camss_jpeg_ahb_clk gcc_camss_micro_ahb_clk gcc_camss_ahb_clk gcc_camss_cpp_ahb_clk
	      gcc_camss_vfe_ahb_clk"
    fi
  elif [ "x$soc" = "x8994" ]; then
    if [ "$parent" = "ufs_axi_clk_src" -o "$parent" = "all" ]; then
      child_list="$child_list gcc_sys_noc_ufs_axi_clk gcc_ufs_axi_clk"
    fi
    if [ "$parent" = "usb30_master_clk_src" -o "$parent" = "all" ]; then
      child_list="$child_list gcc_sys_noc_usb3_axi_clk gcc_usb30_master_clk"
    fi
    if [ "$parent" = "csi0_clk_src" -o "$parent" = "all" ]; then
      child_list="$child_list camss_csi0_clk camss_csi0phy_clk camss_csi0pix_clk camss_csi0rdi_clk"
    fi
    if [ "$parent" = "csi1_clk_src" -o "$parent" = "all" ]; then
      child_list="$child_list camss_csi1_clk camss_csi1phy_clk camss_csi1pix_clk camss_csi1rdi_clk"
    fi
    if [ "$parent" = "csi2_clk_src" -o "$parent" = "all" ]; then
      child_list="$child_list camss_csi2_clk camss_csi2phy_clk camss_csi2pix_clk camss_csi2rdi_clk"
    fi
    if [ "$parent" = "csi3_clk_src" -o "$parent" = "all" ]; then
      child_list="$child_list camss_csi3_clk camss_csi3phy_clk camss_csi3pix_clk camss_csi3rdi_clk"
    fi
    if [ "$parent" = "vfe0_clk_src" -o "$parent" = "all" ]; then
      child_list="$child_list camss_csi_vfe0_clk camss_vfe_vfe0_clk"
    fi
    if [ "$parent" = "vfe1_clk_src" -o "$parent" = "all" ]; then
      child_list="$child_list camss_csi_vfe1_clk camss_vfe_vfe1_clk"
    fi
    if [ "$parent" = "ocmemnoc_clk_src" -o "$parent" = "all" ]; then
      child_list="$child_list venus0_ocmemnoc_clk"
    fi
    if [ "$parent" = "fd_core_clk_src" -o "$parent" = "all" ]; then
      child_list="$child_list fd_core_clk fd_core_uar_clk"
    fi
    if [ "$parent" = "axi_clk_src" -o "$parent" = "all" ]; then
      child_list="$child_list camss_vfe_cpp_axi_clk camss_jpeg_jpeg_axi_clk camss_vfe_vfe_axi_clk
                  fd_axi_clk mdss_axi_clk mmss_mmssnoc_axi_clk mmss_s0_axi_clk venus0_axi_clk"
    fi
    if [ "$parent" = "vcodec0_clk_src" -o "$parent" = "all" ]; then
      child_list="$child_list venus0_vcodec0_clk venus0_core0_vcodec_clk venus0_core1_vcodec_clk
	          venus0_core2_vcodec_clk"
    fi
    elif [ "x$soc" = "x8909" ]; then
    if [ "$parent" = "vfe0_clk_src" -o "$parent" = "all" ]; then
      child_list="$child_list gcc_camss_csi_vfe0_clk gcc_camss_vfe0_clk"
    fi
    if [ "$parent" = "gfx3d_clk_src" -o "$parent" = "all" ]; then
      child_list="$child_list gcc_oxili_gfx3d_clk"
    fi
    if [ "$parent" = "csi0_clk_src" -o "$parent" = "all" ]; then
      child_list="$child_list gcc_camss_csi0_clk gcc_camss_csi0phy_clk gcc_camss_csi0pix_clk
		gcc_camss_csi0rdi_clk"
    fi
    if [ "$parent" = "csi1_clk_src" -o "$parent" = "all" ]; then
      child_list="$child_list gcc_camss_csi1_clk gcc_camss_csi1phy_clk gcc_camss_csi1pix_clk
		gcc_camss_csi1rdi_clk"
    fi
    if [ "$parent" = "camss_ahb_clk_src" -o "$parent" = "all" ]; then
      child_list="$child_list gcc_camss_csi0_ahb_clk gcc_camss_csi1_ahb_clk gcc_camss_ispif_ahb_clk
	      gcc_camss_ahb_clk gcc_camss_vfe_ahb_clk"
    fi
  fi

  # "all" should only include clocks in the clock_list
  if [ "$parent" = "all" ]; then
    children=""
    for clock in $clock_list; do
      for child in $child_list; do
        if [ $child = $clock ]; then
          children="$children $child"
	fi
      done
    done
    child_list=$children
  fi

  children=""
  # Ignore children that are not in the clock_list
  for child in $child_list; do
    if [ -d $child ] && [ -f $child/enable ] && [ -f $child/is_local ]; then
      children="$children $child"
    else
      print 2 "Child $child of $parent was not found. Ignoring."
    fi
  done
}

soc_setup()
{
  if [ "x$1" = "x8660" ]; then
    # Clock that are voteable: they might not turn off when disabled.
    voteable_clocks="adm0_clk
		     adm0_p_clk
		     adm1_clk
		     adm1_p_clk
		     pll8_clk
		     pmic_arb0_p_clk
		     pmic_arb1_p_clk
		     pmic_ssbi2_clk
		     rpm_msg_ram_p_clk
		    "

    # Clocks that should not be tested
    blacklist="afab_clk afab_a_clk
	       cfpb_clk cfpb_a_clk
	       dfab_clk dfab_a_clk
	       ebi1_clk ebi1_a_clk
	       mmfab_clk mmfab_a_clk
	       mmfpb_clk mmfpb_a_clk
	       sfab_clk sfab_a_clk
	       sfpb_clk sfpb_a_clk
	       smi_clk smi_a_clk
	       ebi1_adm_clk
	       usb_phy0_clk
	       dsi_byte_clk
	       mi2s_bit_clk
	       codec_i2s_mic_bit_clk
	       spare_i2s_mic_bit_clk
	       codec_i2s_spkr_bit_clk
	       spare_i2s_spkr_bit_clk
	       dfab_dsps_clk
	       dfab_usb_hs_clk
	       dfab_sdc1_clk
	       dfab_sdc2_clk
	       dfab_sdc3_clk
	       dfab_sdc4_clk
	       dfab_sdc5_clk
	       modem_ahb1_p_clk
	       modem_ahb2_p_clk
	       ce2_p_clk
	       sc0_m_clk
	       sc1_m_clk
	      "

    # Clocks that should not be tested on Android
    android_blacklist="mdp_clk"
  elif [ "x$1" = "x7X30" ]; then
    voteable_clocks="adm_clk
		     adm_p_clk
		     ce_clk
		     camif_pad_p_clk
		     csi0_p_clk
		     emdh_p_clk
		     grp_2d_p_clk
		     grp_3d_p_clk
		     jpeg_p_clk
		     lpa_p_clk
		     mdp_p_clk
		     mfc_p_clk
		     pll1_clk
		     pll2_clk
		     pll3_clk
		     pll4_clk
		     pmdh_p_clk
		     rotator_imem_clk
		     rotator_p_clk
		     sdc1_p_clk
		     sdc2_p_clk
		     sdc3_p_clk
		     sdc4_p_clk
		     tsif_p_clk
		     uart1dm_p_clk
		     uart2dm_p_clk
		     usb_hs2_p_clk
		     usb_hs3_p_clk
		     usb_hs_p_clk
		     vfe_p_clk
		     axi_li_apps_clk
		     axi_li_adsp_a_clk
		     axi_li_jpeg_clk
		     axi_li_vfe_clk
		     axi_mdp_clk
		     axi_imem_clk
		     axi_li_vg_clk
		     axi_grp_2d_clk
		     axi_li_grp_clk
		     axi_mfc_clk
		     axi_rotator_clk
		     axi_vpe_clk
		    "

    blacklist="cam_m_clk
	       ebi1_clk
	       ebi1_fixed_clk
	       ebi_adm_clk
	       emdh_clk
	       lpa_codec_clk
	       grp_2d_clk
	       grp_3d_src_clk
	       mdc_clk
	       mfc_div2_clk
	       pmdh_clk
	       tsif_ref_clk
	       usb_phy_clk
	       vdc_clk
	      "
  elif [ "x$1" = "x8960" ]; then
    # Clocks that are voteable: they might not turn off when disabled.
    voteable_clocks="adm0_clk
		     adm0_p_clk
		     pll4_clk
		     pll8_clk
		     pmic_arb0_p_clk
		     pmic_arb1_p_clk
		     pmic_ssbi2_clk
		     rpm_msg_ram_p_clk
		     qdss_stm_clk
		     qdss_at_clk
		     qdss_p_clk
		     qdss_pclkdbg_clk
		     qdss_traceclkin_clk
		     qdss_tsctr_clk
		    "

    # Clocks that are known to be broken
    broken="cxo_clk
	    gp0_clk
	    gp1_clk
	    gp2_clk
	    gsbi12_p_clk
	    gsbi12_qup_clk
           "
    # Clocks that should not be tested
    blacklist="afab_clk afab_a_clk
	       cfpb_clk cfpb_a_clk
	       dfab_clk dfab_a_clk
	       ebi1_clk ebi1_a_clk
	       mmfab_clk mmfab_a_clk
	       mmfpb_clk mmfpb_a_clk
	       sfab_clk sfab_a_clk
	       sfpb_clk sfpb_a_clk
	       ebi1_adm_clk
	       usb_phy0_clk
	       dsi1_esc_clk
	       dsi1_byte_clk
	       dsi1_reset_clk
	       dsi2_esc_clk
	       dsi2_byte_clk
	       dsi2_reset_clk
	       mi2s_bit_clk
	       codec_i2s_mic_bit_clk
	       spare_i2s_mic_bit_clk
	       codec_i2s_spkr_bit_clk
	       spare_i2s_spkr_bit_clk
	       dfab_dsps_clk
	       dfab_usb_hs_clk
	       dfab_sdc1_clk
	       dfab_sdc2_clk
	       dfab_sdc3_clk
	       dfab_sdc4_clk
	       dfab_sdc5_clk
	       modem_ahb1_p_clk
	       modem_ahb2_p_clk
	       ce1_p_clk
	       q6fw_clk
	       q6sw_clk
	       q6_func_clk
	       csi_pix_clk
	       csi_pix1_clk
	       csi_rdi_clk
	       csi_rdi1_clk
	       csi_rdi2_clk
	      "
    # Clocks that should not be tested on Android
    android_blacklist="mdp_clk
		       sdc1_clk
		       sdc1_p_clk
		       gsbi7_p_clk
		       gsbi7_qup_clk
		       gsbi7_uart_clk
		      "

  elif [ "x$1" = "x8064" ]; then
    # Clocks that are voteable: they might not turn off when disabled.
    voteable_clocks="adm0_clk
		     adm0_p_clk
		     pll4_clk
		     pll8_clk
		     pmic_arb0_p_clk
		     pmic_arb1_p_clk
		     pmic_ssbi2_clk
		     rpm_msg_ram_p_clk
		    "

    # Clocks that are known to be broken
    broken="cxo_clk
           "

    # Clocks that should not be tested
    blacklist="afab_clk afab_a_clk
	       cfpb_clk cfpb_a_clk
	       dfab_clk dfab_a_clk
	       ebi1_clk ebi1_a_clk
	       mmfab_clk mmfab_a_clk
	       mmfpb_clk mmfpb_a_clk
	       sfab_clk sfab_a_clk
	       sfpb_clk sfpb_a_clk
	       ebi1_adm_clk
	       dsi1_esc_clk
	       dsi1_byte_clk
	       dsi1_reset_clk
	       dsi2_esc_clk
	       dsi2_byte_clk
	       dsi2_reset_clk
	       mi2s_bit_clk
	       codec_i2s_mic_bit_clk
	       spare_i2s_mic_bit_clk
	       codec_i2s_spkr_bit_clk
	       spare_i2s_spkr_bit_clk
	       dfab_dsps_clk
	       dfab_usb_hs_clk
	       dfab_sdc1_clk
	       dfab_sdc2_clk
	       dfab_sdc3_clk
	       dfab_sdc4_clk
	       dfab_sdc5_clk
	       modem_ahb1_p_clk
	       modem_ahb2_p_clk
	       ce1_p_clk
	       q6fw_clk
	       q6sw_clk
	       q6_func_clk
	       csi_pix_clk
	       csi_pix1_clk
	       csi_rdi_clk
	       csi_rdi1_clk
	       csi_rdi2_clk
	       mdp_tv_clk
	       npl_tv_clk
	       pcie_phy_ref_clk
	      "

    # Clocks that should not be tested on Android
    android_blacklist="mdp_clk sdc1_clk sdc1_p_clk"
  elif [ "x$1" = "x8930" ]; then
    # Clocks that are voteable: they might not turn off when disabled.
    voteable_clocks="adm0_clk
		     adm0_p_clk
		     pll4_clk
		     pll8_clk
		     pmic_arb0_p_clk
		     pmic_arb1_p_clk
		     pmic_ssbi2_clk
		     rpm_msg_ram_p_clk
		    "

    # Clocks that are known to be broken
    broken="cxo_clk
	    gp0_clk
	    gp1_clk
	    gp2_clk
	    gsbi12_p_clk
	    gsbi12_qup_clk
           "
    # Clocks that should not be tested
    blacklist="afab_clk afab_a_clk
	       cfpb_clk cfpb_a_clk
	       dfab_clk dfab_a_clk
	       ebi1_clk ebi1_a_clk
	       mmfab_clk mmfab_a_clk
	       mmfpb_clk mmfpb_a_clk
	       sfab_clk sfab_a_clk
	       sfpb_clk sfpb_a_clk
	       ebi1_adm_clk
	       usb_phy0_clk
	       dsi1_esc_clk
	       dsi1_byte_clk
	       dsi1_reset_clk
	       mi2s_bit_clk
	       codec_i2s_mic_bit_clk
	       spare_i2s_mic_bit_clk
	       codec_i2s_spkr_bit_clk
	       spare_i2s_spkr_bit_clk
	       dfab_dsps_clk
	       dfab_usb_hs_clk
	       dfab_sdc1_clk
	       dfab_sdc2_clk
	       dfab_sdc3_clk
	       dfab_sdc4_clk
	       dfab_sdc5_clk
	       modem_ahb1_p_clk
	       modem_ahb2_p_clk
	       ce1_p_clk
	       q6fw_clk
	       q6sw_clk
	       q6_func_clk
	       csi_pix_clk
	       csi_pix1_clk
	       csi_rdi_clk
	       csi_rdi1_clk
	       csi_rdi2_clk
	      "

    # Clocks that should not be tested on Android
    android_blacklist="mdp_clk sdc1_clk sdc1_p_clk"
  elif [ "x$1" = "x9615" ]; then
    # Clocks that are voteable: they might not turn off when disabled.
    voteable_clocks="adm0_clk
		     adm0_p_clk
		     pll0_clk
		     pll4_clk
		     pll8_clk
		     pll9_clk
		     pll14_clk
		     pmic_arb0_p_clk
		     pmic_arb1_p_clk
		     pmic_ssbi2_clk
		     rpm_msg_ram_p_clk
		    "

    # Clocks that are known to be broken
    broken="cxo_clk
           "
    # Clocks that should not be tested
    blacklist="cfpb_clk cfpb_a_clk
	       dfab_clk dfab_a_clk
	       ebi1_clk ebi1_a_clk
	       sfab_clk sfab_a_clk
	       sfpb_clk sfpb_a_clk
	       ebi1_adm_clk
	       usb_phy0_clk
	       mi2s_bit_clk
	       codec_i2s_mic_bit_clk
	       spare_i2s_mic_bit_clk
	       codec_i2s_spkr_bit_clk
	       spare_i2s_spkr_bit_clk
	       dfab_usb_hs_clk
	       dfab_sdc1_clk
	       dfab_sdc2_clk
	       ce1_p_clk
	       q6fw_clk
	       q6sw_clk
	       q6_func_clk
	      "
  elif [ "x$1" = "x8974" ]; then

    # Clocks that are known to be broken
    broken="gcc_usb30_master_clk"

    # Clocks that should not be tested
    blacklist="
	       gcc_blsp1_qup3_i2c_apps_clk
	       bimc_acpu_a_clk
	       ocmemgx_msmbus_clk ocmemgx_msmbus_a_clk
	       ocmemnoc_clk oxili_gfx3d_clk
	       ocmemcx_ocmemnoc_clk
	       mmss_s0_axi_clk
	       audio_core_lpaif_codec_spkr_clk_src
	       audio_core_lpaif_codec_spkr_osr_clk
               audio_core_lpaif_codec_spkr_ebit_clk
               audio_core_lpaif_codec_spkr_ibit_clk
	       audio_core_lpaif_pri_clk_src
	       audio_core_lpaif_pri_osr_clk
	       audio_core_lpaif_pri_ebit_clk
	       audio_core_lpaif_pri_ibit_clk
	       audio_core_lpaif_sec_clk_src
	       audio_core_lpaif_sec_osr_clk
	       audio_core_lpaif_sec_ebit_clk
	       audio_core_lpaif_sec_ibit_clk
	       audio_core_lpaif_ter_clk_src
	       audio_core_lpaif_ter_osr_clk
	       audio_core_lpaif_ter_ebit_clk
	       audio_core_lpaif_ter_ibit_clk
	       audio_core_lpaif_quad_clk_src
	       audio_core_lpaif_quad_osr_clk
	       audio_core_lpaif_quad_ebit_clk
	       audio_core_lpaif_quad_ibit_clk
	       audio_core_lpaif_pcm0_clk_src
	       audio_core_lpaif_pcm0_ebit_clk
	       audio_core_lpaif_pcm0_ibit_clk
	       audio_core_lpaif_pcm1_clk_src
	       audio_core_lpaif_pcm1_ebit_clk
	       audio_core_lpaif_pcm1_ibit_clk
	       audio_core_slimbus_core_clk
	       audio_core_lpaif_pcmoe_clk_src
	       audio_core_lpaif_pcmoe_clk
	       audio_core_slimbus_lfabif_clk
	       audio_wrapper_br_clk
	       q6ss_xo_clk q6ss_ahb_lfabif_clk
	       mss_bus_q6_clk mss_xo_q6_clk
	       mdss_edppixel_clk mdss_edplink_clk
	       krait0_m_clk krait1_m_clk krait2_m_clk krait3_m_clk l2_m_clk
	       q6ss_ahbm_clk
               gcc_usb30_mock_utmi_clk
	      "

	android_blacklist="mdss_mdp_clk mdss_mdp_lut_clk"
  elif [ "x$1" = "x8962" ]; then

    blacklist=" ocmemcx_ocmemnoc_clk oxili_gfx3d_clk
		gcc_lpass_sys_noc_mport_clk
		bimc_gpu
		"
  elif [ "x$1" = "x8610" ] || [ "x$1" = "x8612" ]; then

    # Clocks that should not be tested
    blacklist="
                bimc_acpu_a_clk
                pnoc_iommu_clk
                pnoc_qseecom_clk
                mdp_axi_clk_src
                mmssnoc_axi_clk_src
                mmssnoc_ahb_a_clk
                mmss_mmssnoc_axi_clk
                gcc_xo_a_clk_src
                cxo_d0_a
                cxo_d1_a
                cxo_a0_a
                cxo_a1_a
                cxo_a2_a
                div_a_clk
                diff_a_clk
                cxo_d0_a_pin
                cxo_d1_a_pin
                cxo_a0_a_pin
                cxo_a1_a_pin
                cxo_a2_a_pin
                csi0phy_clk
                csi1phy_clk
                csi0phy_cam_mux_clk
                csi1phy_cam_mux_clk
                csi0pix_cam_mux_clk
                rdi0_cam_mux_clk
                rdi1_cam_mux_clk
                rdi2_cam_mux_clk
                dsi_pclk_clk_src
                dsi_clk_src
                dsi_byte_clk_src
                dsi_esc_clk_src
                dsi_clk
                dsi_ahb_clk
                dsi_byte_clk
                dsi_esc_clk
                dsi_pclk_clk
                mdp_dsi_clk
                gcc_bimc_smmu_clk
                q6ss_ahb_lfabif_clk
                q6ss_ahbm_clk
                q6ss_xo_clk
                a7sspll
	      "

  elif [ "x$1" = "x9625" ]; then

    # Clocks that should not be tested
    blacklist="
	       audio_core_lpaif_pri_clk_src
	       audio_core_lpaif_pri_osr_clk
	       audio_core_lpaif_pri_ebit_clk
	       audio_core_lpaif_pri_ibit_clk
	       audio_core_lpaif_sec_clk_src
	       audio_core_lpaif_sec_osr_clk
	       audio_core_lpaif_sec_ebit_clk
	       audio_core_lpaif_sec_ibit_clk
	       audio_core_lpaif_pcm0_clk_src
	       audio_core_lpaif_pcm0_ebit_clk
	       audio_core_lpaif_pcm0_ibit_clk
	       audio_core_lpaif_pcm1_clk_src
	       audio_core_lpaif_pcm1_ebit_clk
	       audio_core_lpaif_pcm1_ibit_clk
	       audio_core_lpaif_pcmoe_clk_src
	       audio_core_lpaif_pcmoe_clk
	       audio_core_slimbus_core_clk_src
	       audio_core_slimbus_core_clk
	       audio_core_slimbus_lfabif_clk
               audio_core_lpaif_pcmoe_clk_src
               audio_core_lpaif_pcm_data_oe_clk
               a5_m_clk
               apcspll_clk_src
	      "

elif [ "x$1" = "x8226" ] || [ "x$1" = "x8026" ] || [ "x$1" = "x8926" ]; then

    # Clocks that are known to be broken
    broken="a7sspll"

    # Clocks that should not be tested
    blacklist="
		gfx3d_clk_src gfx3d_a_clk_src
		oxili_gfx3d_clk_src
		oxili_gfx3d_clk
		bimc_acpu_a_clk
		ocmemgx_msmbus_clk ocmemgx_msmbus_a_clk
		ocmemgx_core_clk

		mmpll1_pll

		q6ss_xo_clk q6ss_ahb_lfabif_clk
		q6ss_ahbm_clk

	      "

	android_blacklist="empty"
  elif [ "x$1" = "x9635" ]; then

    # Clocks that should not be tested
    blacklist="a7_m_clk
		a7ssmux
		a7sspll
		xo_clk xo_a_clk
		ipa_clk ipa_a_clk
		qpic_clk qpic_a_clk

		gcc_usb3_phy_com_reset
		gcc_usb3_phy_reset
		gcc_pcie_gpio_ldo
		gcc_usb_ss_ldo

		gcc_pcie_pipe_clk
		gcc_usb3_pipe_clk

		q6ss_ahb_lfabif_clk
		q6ss_ahbm_clk
	      "

    gdsc_list="usb_hsic
		pcie_0
		usb30"

elif [ "x$1" = "x8084" ]; then

    # Clocks that are known to be broken
    broken=""

    # Clocks that should not be tested
    blacklist="
	       gcc_bimc_clk
               gfx3d_clk_src gfx3d_a_clk_src
	       bimc_acpu_a_clk
               oxili_gfx3d_clk_src oxili_gfx3d_clk
	       ocmemgx_msmbus_clk ocmemgx_msmbus_a_clk
               ocmemgx_core_clk
	       ocmemcx_ocmemnoc_clk
	       edppixel_clk_src edp_pixel_clk_src edp_vco_clk
	       edplink_clk_src edp_mainlink_clk_src
	       mdss_edppixel_clk mdss_edplink_clk
	       avsync_pclk0_clk avsync_pclk1_clk
               mdss_avsync_edppixel_clk
	       gcc_lpass_sway_clk
	       gcc_sata_asic0_clk gcc_sata_rx_clk
	       hdmipll_clk_src hdmipll_mux_clk
	       hdmipll_div1_clk hdmipll_div2_clk hdmipll_div4_clk hdmipll_div6_clk
	      "
   android_blacklist="mdss_mdp_clk mdss_mdp_lut_clk mdp_clk_src
	      "
elif [ "x$1" = "x8916" ] || [ "x$1" = "x8939" ]; then

    # Clocks that are voteable: they might not turn off when disabled.
    voteable_clocks="
		gcc_crypto_ahb_clk
		gcc_crypto_axi_clk
		gcc_crypto_clk
		gcc_gfx_tbu_clk
		gcc_gfx_tcu_clk
		gcc_gtcu_ahb_clk
		gcc_jpeg_tbu_clk
		gcc_smmu_cfg_clk
		gcc_venus_tbu_clk
		gcc_vfe_tbu_clk
	        gcc_apss_tcu_clk
		gcc_mdp_tbu_clk
		gcc_cpp_tbu_clk
		gcc_mdp_rt_tbu_clk
		"
    # Clocks that should not be tested
    blacklist="
		bimc_acpu_a_clk
		gcc_xo_a_clk_src
		a53sspll
		gcc_mdss_byte0_clk
		gcc_mdss_pclk0_clk
		apss_debug_pri_mux
		apss_debug_sec_mux
		apss_debug_ter_mux
		gcc_mdss_mdp_clk
		sysmmnoc_clk
		"

  elif [ "x$1" = "x8994" ]; then

    # Clock that are voteable: they might not turn off when disabled.
    voteable_clocks=""

    # Clocks that should not be tested
    blacklist="	ufs_rx_cfg_postdiv_clk_src
		ufs_tx_cfg_postdiv_clk_src
		gcc_ufs_rx_cfg_clk
		gcc_ufs_tx_cfg_clk
		gcc_ce1_ahb_m_clk
		gcc_ce1_axi_m_clk
		gcc_ce2_ahb_m_clk
		gcc_ce2_axi_m_clk
		gcc_ce3_ahb_m_clk
		gcc_ce3_axi_m_clk
		gcc_ipa_ahb_m_clk
		gcc_ipa_sleep_m_clk
		debug_rpm_clk
		mmsscc_mmssnoc_ahb
		gcc_usb3_phy_pipe_clk
		ce1_clk
		ce2_clk
		ce3_clk
		ipa_clk
		gcc_mss_q6_bimc_axi_clk

		a53_clk a53_debug_mux a53_div_clk
		a53_hf_mux a53_lf_mux a53_lf_mux_pll0_div
		a53_pll0 a53_pll0_main a53_pll1
		a53_safe_clk a53_safe_parent
		a57_clk a57_debug_mux a57_div_clk
		a57_hf_mux a57_lf_mux
		a57_pll0 a57_pll1 a57_lf_mux_pll0_div
		a53_pll1_main a53_lf_mux_pll1_div
		a57_safe_clk a57_safe_parent
		cpu_debug_mux xo_ao
		sys_apcsaux_clk
		cci_clk cci_pll cci_hf_mux cci_lf_mux

		a57_pll0_main a57_pll1_main
		a53_hf_mux_v2 a57_hf_mux_v2
		a53_lf_mux_v2 a57_lf_mux_v2
		a57_lf_mux_div a53_lf_mux_div

		fixed_hr_oclk2_div_clk_8994
		bypass_lp_div_mux_8994
		hr_oclk3_div_clk_8994
		indirect_path_div2_clk_8994
		ndiv_clk_8994
		dsi_vco_clk_8994
		ext_byte0_clk_src
		ext_byte1_clk_src
		ext_pclk0_clk_src
		ext_pclk1_clk_src
		ext_extpclk_clk_src
		mdss_pixel_clk_mux
		mdss_byte_clk_mux
		shadow_byte_clk_src
		shadow_pixel_clk_src
		shadow_fixed_hr_oclk2_div_clk_8994
		shadow_bypass_lp_div_mux_8994
		shadow_hr_oclk3_div_clk_8994
		shadow_indirect_path_div2_clk_8994
		shadow_ndiv_clk_8994
		shadow_dsi_vco_clk_8994

		gcc_bimc_kpss_axi_m_clk
		gcc_mmss_bimc_gfx_m_clk

		hdmi_20nm_vco_clk
		"
elif [ "x$1" = "x8909" ]; then

    # Clocks that are voteable: they might not turn off when disabled.
    voteable_clocks="
		gcc_crypto_ahb_clk
		gcc_crypto_axi_clk
		gcc_crypto_clk
		gcc_gfx_tbu_clk
		gcc_gfx_tcu_clk
		gcc_smmu_cfg_clk
		gcc_venus_tbu_clk
		gcc_vfe_tbu_clk
	        gcc_apss_tcu_clk
		gcc_mdp_tbu_clk
		"
    # Clocks that should not be tested
    blacklist="
		gcc_xo_a_clk_src
		a7sspll
		gcc_mdss_byte0_clk
		gcc_mdss_pclk0_clk
		gcc_mdss_mdp_clk
		apss_debug_pri_mux
		apss_debug_sec_mux
		apss_debug_ter_mux
		sdcc1_apps_clk_src
		gcc_sdcc1_apps_clk
		"

  else
  # SoC = Other
    # Clock that are voteable: they might not turn off when disabled.
    voteable_clocks="adm_clk"

    # Clocks that should not be tested
    blacklist="acpu_clk
	       cam_m_clk
	       csi0_clk
	       csi0_p_clk
	       csi0_vfe_clk
	       ebi1_clk
	       pmdh_clk
	       uart2_clk
	       uart2dm_clk
	       usb_phy_clk
	       vdc_clk
	       vfe_mdc_clk
	      "
  fi

  voteable_clocks="$global_votable_clocklist $voteable_clocks"
  blacklist="$global_blacklist $blacklist $broken"
  android_blacklist="$global_android_blacklist $android_blacklist"
}

######################################
#                                    #
# SoC-independent Support Functions: #
#                                    #
######################################

# Echo a message to the console if the verbosity level is high enough
print ()
{
  if [ $verbose -ge $1 ]; then
    echo -e "$2"
  fi
}

# Count the words in a list
count ()
{
  return $#
}

# Reverse the item in a list, and store them in $reversed
reverse ()
{
  reversed=""
  for item in $@; do
    reversed="$item $reversed"
  done
}

# Return 0 if the difference between $1 and $2 is > $abs_diff_limit
within_limits ()
{
  val=$(($1 - $2))

  if [ $val -lt 0 ]; then
    val=$((0 - $val))
  fi

  if [ $val -gt $abs_diff_limit ]; then
    return 0
  fi

  return 1
}

# Determine what type of test to run for the provided clock
# and return the test's name in $test_type
choose_test ()
{
  clk_name=$1

  # Skip blacklisted clocks
  for c in $blacklist; do
    if [ "$c" = "$clk_name" ]; then
      test_type="blacklisted"
      return 0
    fi
  done

  # Skip child clocks.  They should not be tested directly,
  # but will be tested along with their parents
  for c in $all_children; do
    if [ "$c" = "$clk_name" ]; then
      test_type="child"
      return 0
    fi
  done

  ################################
  # Get clock info from debugfs: #
  ################################

  local=`cat $clk_name/is_local`
  enabled=`cat $clk_name/enable`
  if [ -f $1/has_hw_gating ]; then
    hw_gated=`cat $1/has_hw_gating`
  else
    hw_gated=0
  fi
  if [ -f $1/list_rates ] && [ "x`cat $1/list_rates`" != "x" ]; then
    settable=1
  else
    settable=0
  fi

  ####################################
  # Determine test to run for clock: #
  ####################################

  # Skip clocks which are hardware gated
  if [ $hw_gated -eq 1 ]; then
    test_type="hw_gated"
    return 0
  fi

  # Use 'vote' test for clocks in the $voteable_clocks list
  for c in $voteable_clocks; do
    if [ "$c" = "$clk_name" ]; then
      test_type="vote"
      return 0
    fi
  done

  # Use 'allrate' test for clocks which are Local+Settable+Disabled
  if [ $local -eq 1 -a $settable -eq 1 -a $enabled -eq 0 ]; then
    test_type="allrate"
    return 0
  fi

  # Use 'onerate' test for clocks which are Local+Settable+Enabled
  if [ $local -eq 1 -a $settable -eq 1 -a $enabled -eq 1 ]; then
    test_type="onerate"
    return 0
  fi

  test_type="stateonly"
  return 0
}

###################
#                 #
# Test functions: #
#                 #
###################

# First arg should be the parent clock, followed by any children.
# Tests should return 0 on success, and 1 on failure. Clocks should
# be left in the same state after the test as they were before.

# 'vote' Test:
# The parent clock is voteable and not settable, so even when it's voted off
# locally, it may stay enabled. Test to make sure, if the clock is voted on,
# the clock really is on.
test_vote ()
{
  # Check feedback for voteable clocks to make sure they are
  # on when enabled.
  clk_name=$1
  fail=0
  print 2 "Testing $clk_name and $(($# - 1)) children with 'vote' test"
  for c in $@; do
    measured_rate=-1
    print 4 "\tenabling $c"
    echo 1 > $c/enable
    if [ -f $c/measure ]; then
      measured_rate=`cat $c/measure`
    fi
    enabled=`cat $c/enable`
    if [ $enabled -eq 0 -o $measured_rate -eq 0 ]; then
      fail=$(($fail + 1))
    fi
    print 4 "\tdisabling $c"
    echo 0 > $c/enable
  done

  if [ $fail -eq 0 ]; then
    return 0;
  else
    return 1;
  fi;
}

# 'allrate' Test:
# The parent clock is rate-settable and disabled.
# Test clocks at all supported rates. On the first run through, just
# enable the clock at the beginning and disable it at the end.  On the
# second run, enable and disable the clock before and after each rate.
test_allrate ()
{
  clk_name=$1
  fail=0
  print 2 "Testing $clk_name and $(($# - 1)) children with 'allrate' test"
  orig_rate=`cat $clk_name/rate`
  for disable_between_rates in 0 1; do
    if [ $quick -eq 1 -a $disable_between_rates -eq 1 ]; then
      continue
    fi
    if [ $disable_between_rates -eq 0 ]; then
      print 3 "<first run for $clk_name 'allrate' test (constantly enabled)>"
    else
      print 3 "<second run for $clk_name 'allrate' test (toggling enable)>"
    fi

    # Test each rate
    if [ -f $clk_name/list_rates ]; then
      all_rates=`cat $clk_name/list_rates`
    fi
    for test_rate in $all_rates; do
      # Skip 0Hz rates, since they will generate warnings in dmesg for
      # failing to enable.
      if [ $test_rate -eq 0 ]; then
        continue
      fi

      # Set the rate and make sure it's accepted
      echo $test_rate > $clk_name/rate
      rate=`cat $clk_name/rate`
      if [ $rate -ne $test_rate ]; then
        print 0 "\t$clk_name: Failed to set rate $test_rate Hz. Rate was not accepted."
        fail=$(($fail + 1))
        continue;
      fi

      if [ $adversarial -eq 1 ]; then
        echo 1 > $clk_name/enable
        if [ -f $clk_name/measure ]; then
          orig_measured_rate=`cat $clk_name/measure`
        fi
        check_rate=`cat $clk_name/rate`
        echo $bogus_rate > $clk_name/rate
        if [ $check_rate -ne $rate ]; then
          print 0 "\t$clk_name: Bogus rate affected set rate."
          fail=$(($fail + 1))
          continue
        fi
        if [ -f $clk_name/measure ]; then
          measured_rate=`cat $clk_name/measure`
          within_limits $measured_rate $orig_measured_rate
          if [ $? -eq 0 ]; then
            print 0 "\t$clk_name: Bogus rate affected measured rate."
            fail=$(($fail + 1))
            continue
	  fi
        fi
        echo 0 > $clk_name/enable
      fi

      # Check the clock and the children at each rate
      for c in $@; do
        print 3 "Testing $c at $rate:"

        # If the clock is disabled, enable it before testing
        enable=`cat $c/enable`
        if [ $enable -eq 0 ]; then
          print 4 "\tenabling $c"
          echo 1 > $c/enable
          enable=`cat $c/enable`
          if [ $enable -ne 1 ]; then
            fail=$(($fail + 1))
            print 0 "$c: Enable failed."
            continue;
          fi
        fi

        # Check measured rate
        if [ -f $c/measure ]; then
          measured_rate=`cat $c/measure`
          if [ $measured_rate -ne -1 ]; then
            within_limits $measured_rate $test_rate
            if [ $? -eq 0 ]; then
              fail=$(($fail + 1))
              if [ $disable_between_rates -ne 1 ]; then
                print 0 "\t$c: Setting rate of enabled clock to $test_rate Hz resulted in $measured_rate Hz"
              else
                print 0 "\t$c: Enabling clock after setting rate to $test_rate Hz resulted in $measured_rate Hz"
              fi
            fi
          fi
        fi

        # Disable the clock after each cycle test (for toggling run)
        if [ $disable_between_rates -eq 1 ]; then
          # Disable the clock
          print 4 "\tdisabling $c"
          echo 0 > $c/enable
          enable=`cat $c/enable`
          if [ $enable -ne 0 ]; then
            print 0 "\t('allrate' toggle test failed) - disable failed for $c"
            fail=$(($fail + 1))
            continue;
          fi
        fi

      done # looping through parent and child clocks
    done # looping through rates

    # Turn off clocks after testing all rates on first run
    if [ $disable_between_rates -eq 0 ]; then
      reverse $@
      for c in $reversed; do
        enable=`cat $c/enable`
        if [ $enable -ne 0 ]; then
          print 4 "\tdisabling $c"
          echo 0 > $c/enable
          enable=`cat $c/enable`
        fi
        if [ $enable -ne 0 ]; then
          print 0 "\t('allrate' constant test failed) - disable failed for $c"
          fail=$(($fail + 1))
          continue;
        fi
      done
    fi
  done # looping through toggle var

  # Return clock to its original rate
  if [ $orig_rate -ne 0 ]; then
    echo $orig_rate > $clk_name/rate
  fi

  if [ $fail -ne 0 ]; then
    return 1
  else
    return 0
  fi
}

# 'onerate' Test:
# The parent clock has a settable rate, but it's already on. Test to make
# sure the set rate matches the measured rate and that all its children,
# when enabled, match the rate of the parent. This test only supports
# child clocks which run at the same rate as their parent.
test_onerate ()
{
  clk_name=$1
  fail=0
  print 2 "Testing $clk_name and $(($# - 1)) children with 'onerate' test"
  rate=`cat $1/rate`
  for c in $@; do
    if [ ! -f $c/measure ] || [ `cat $c/measure` -eq -1 ]; then
      continue;
    fi

    enabled=`cat $c/enable`
    if [ $enabled -eq 0 ]; then
      print 4 "\tenabling $c"
      echo 1 > $c/enable
      enabled=`cat $c/enable`
      measured_rate=`cat $c/measure`
      within_limits $measured_rate $rate
      if [ $? -eq 0 -o $enabled -eq 0 ]; then
        fail=$(($fail + 1))
      fi

      print 4 "\tdisabling $c"
      echo 0 > $c/enable
      enabled=`cat $c/enable`
      measured_rate=`cat $c/measure`
      if [ $measured_rate -ne 0 -o $enabled -eq 1 ]; then
        fail=$(($fail + 1))
      fi
    else
      echo 1 > $c/enable
      measured_rate=`cat $c/measure`
      within_limits $measured_rate $rate
      if [ $? -eq 0 ]; then
        fail=$(($fail + 1))
      fi
      echo 0 > $c/enable
    fi
  done

  if [ $fail -eq 0 ]; then
    return 0;
  else
    return 1;
  fi;
}

# 'stateonly' Test:
# The clock is not rate-settable and not voteable.  If the clock is
# off, test to make sure, when we turn it on, the enable and measure
# nodes (if supported) confirm the state change. When we turn it off
# off again, confirm the change back. If the clock is already on,
# confirm the enable and measure nodes (if supported) agree with this.
test_stateonly ()
{
  clk_name=$1
  fail=0
  print 2 "Testing $clk_name and $(($# - 1)) children with 'stateonly' test"
  for c in $@; do
    enabled=`cat $c/enable`
    measured_rate=-1
    if [ -f $c/measure ]; then
      measured_rate=`cat $c/measure`
    fi

    # Confirm enabled state matches measurement
    if [ $enabled -eq 0 -a $measured_rate -gt 0 ]; then
      fail=$(($fail + 1))
      print 0 "Clock measured as $measured_rate Hz, should be 0 Hz (precheck)"
      continue;
    elif [ $enabled -eq 1 -a $measured_rate -eq 0 ]; then
      fail=$(($fail + 1))
      print 0 "Clock measured as $measured_rate Hz, should be non-zero (precheck)"
      continue;
    fi

    # If clock is off, toggle it and make sure the state changes
    if [ $enabled -eq 0 ]; then
      print 4 "\tenabling $c"
      echo 1 > $c/enable
      enabled=`cat $c/enable`
      measured_rate=-1
      if [ -f $c/measure ]; then
        measured_rate=`cat $c/measure`
      fi
      if [ $enabled -eq 0 ]; then
        fail=$(($fail +1))
	print 0 "Clock still not enabled after turning on"
      elif [ $measured_rate -eq 0 ]; then
        fail=$(($fail + 1))
	print 0 "Clock measured as $measured_rate Hz, should be non-zero (toggle)"
      fi

      print 4 "\tdisabling $c"
      echo 0 > $c/enable
      enabled=`cat $c/enable`
      measured_rate=-1
      if [ -f $c/measure ]; then
        measured_rate=`cat $c/measure`
      fi
      if [ $enabled -eq 1 ]; then
        fail=$(($fail +1))
	print 0 "Clock still enabled after turning off"
      elif [ $measured_rate -gt 0 ]; then
        fail=$(($fail + 1))
	print 0 "Clock measured as $measured_rate Hz, should be 0 Hz (toggle)"
      fi
    fi
  done

  if [ $fail -ne 0 ]; then
    return 1;
  else
    return 0;
  fi;
}

###########################
#                         #
# Test Script Entry Point #
#                         #
###########################

if [ "x$TEST_ENV_SETUP" != "x" -a -f $TEST_ENV_SETUP ]; then
  . $TEST_ENV_SETUP 2>/dev/null
fi

# Bogus rate for adversarial testing
bogus_rate=314159265

# Result counts
total=0
passed=0
failed=0
blacklisted=0
hwgated=0

# Test type counts
stateonly=0
onerate=0
allrate=0
vote=0
children_found=0
missed=0

# Parse command-line options
verbose=0 # Lowest verbosity by default
abs_diff_limit=40000 # 40 KHz by default
adversarial=0
quick=0
repeat_times=1
test_debug_fs=${TEST_DEBUG_FS:-/sys/kernel/debug}
platform=$TEST_TARGET
soc=$TARGET_TYPE
args=$@
while [ $# -ge 1 ]; do
  case $1 in
    -v|--verbose)
      if [ $# -gt 1 -a "x`echo $2 | grep -E ^[0-9]+$`" != "x" ]; then
        verbose=$2
      else
        verbose=1
      fi
      ;;
    -t|--tolerance)
      if [ $# -gt 1 -a "x`echo $2 | grep -E ^[0-9]+$`" != "x" ]; then
        abs_diff_limit=$2
      else
        echo "Invalid frequency tolerance specified (in Hz)"
        exit 1
      fi
      ;;
    -d|--debugfs)
      if [ $# -gt 1 ]; then
        test_debug_fs=$2
      else
        echo "No debugfs path specified"
        exit 1
      fi
      ;;
    -p|--platform)
      if [ $# -gt 1 ]; then
        platform=$2
      else
        echo "No platform specified"
        exit 1
      fi
      ;;
    -s|--soc)
      if [ $# -gt 1 ]; then
        soc=$2
      else
        echo "No SoC specified"
        exit 1
      fi
      ;;
    -a|--adversarial)
      adversarial=1
      ;;
    -r|--repeat)
      if [ $# -gt 1 -a "x`echo $2 | grep -E ^[0-9]+$`" != "x" ]; then
        repeat_times=$2
      fi
      ;;
    -q|--quick)
      quick=1
      ;;
    -c|--clocks)
      if [ $# -gt 1 ]; then
        clock_list=$2
      else
        echo "No clocks specified"
        exit 1
      fi
      ;;
    -h|--help)
       echo "Usage: clk_test.sh [-d|--debugfs debugfs_path] [--verbose|-v [level]]
                    [--tolerance|-t hz] [-p|--platform platform] [-s|--soc soc]
                    [-a|--adversarial] [-r|--repeat times] [-q|--quick]
                    [-c|--clocks clock_list]"
      exit 0
      ;;
  esac
  shift
done

# Call script in a loop when running multiple iterations
if [ $repeat_times -gt 1 ]; then
  echo "Attempting to run $repeat_times test iterations"
  i=1
  while [ $i -le $repeat_times ]; do
    $0 $args -c "$clock_list" -r 1
    if [ $? -ne 0 ]; then
      echo "Failed on iteration $i/$repeat_times."
      exit 1
    else
      echo "Passed iteration $i/$repeat_times."
    fi
    i=$(($i + 1))
  done
  exit 0
fi

# Verify presence of debugfs clocks
if [ ! -d "$test_debug_fs/clk" ]; then
  echo "Unable to find $test_debug_fs/clk/"
  echo "Specify path to debugfs with the -d flag"
  exit 1
fi

print 1 "Target SoC: $soc"

soc_setup $soc # Populate blacklist, android_blacklist, and voteable_clocks

# Start testing!
if [ "x$platform" = "xANDROID" ]; then
  print 0 "Stopping Android services"
  stop
  sync
  blacklist="$blacklist $android_blacklist"
  sleep 5
fi
echo "Testing clocks (this may take several minutes):"
echo "==============================================="
cd "$test_debug_fs/clk"
clock_list=${clock_list:-*}
get_children "all" #sets $children
all_children="$children"

for gdsc in $test_debug_fs/regulator/gdsc_*; do
  echo 1 > $gdsc/enable
  if [ $? -eq 0 ]; then
    gdsc_on_list="$gdsc_on_list $gdsc"
  fi
done

for clk in $clock_list; do
  # Make sure $clk really is a clock
  if [ ! -d $clk ] || [ ! -f $clk/enable ] || [ ! -f $clk/is_local ]; then
    continue
  fi
  total=$(($total + 1))
  choose_test $clk  # sets $test_type
  get_children $clk # sets $children
  print 4 "Test type for $clk is $test_type"
  case "$test_type" in
    "blacklisted")
      print 1 "$clk... On blacklist (skipped)"
      blacklisted=$(($blacklisted + 1))
      ;;
    "hw_gated")
      print 1 "$clk... Hardware gated (skipped)"
      hwgated=$(($hwgated + 1))
      ;;
    "vote")
      test_vote $clk $children
      if [ $? -eq 0 ]; then
        print 2 "$clk... Passed (vote test)"
        passed=$(( $passed + 1 ))
      else
        failed=$(( $failed + 1 ))
        print 0 "$clk... Failed (vote test)"
      fi
      vote=$(($vote + 1))
      ;;
    "allrate")
      test_allrate $clk $children
      if [ $? -eq 0 ]; then
        print 2 "$clk... Passed (all-rate test)"
        passed=$(( $passed + 1 ))
      else
        failed=$(( $failed + 1 ))
        print 0 "$clk... Failed (all-rate test)"
      fi
      allrate=$(( $allrate + 1 ))
      ;;
    "onerate")
      test_onerate $clk $children
      if [ $? -eq 0 ]; then
        print 2 "$clk... Passed (one-rate test)"
        passed=$(( $passed + 1 ))
      else
        failed=$(( $failed + 1 ))
        print 0 "$clk... Failed (one-rate test)"
      fi
      onerate=$(( $onerate + 1 ))
      ;;
    "stateonly")
      test_stateonly $clk $children
      if [ $? -eq 0 ]; then
        print 2 "$clk... Passed (state-only test)"
        passed=$(( $passed + 1 ))
      else
        failed=$(( $failed + 1 ))
        print 0 "$clk... Failed (state-only test)"
      fi
      stateonly=$(( $stateonly + 1 ))
      ;;
    "child")
      children_found=$(( $children_found + 1 ))
      print 2 "$clk... Skipping child (tested with parent)"
      ;;
    *)
      missed=$(( $missed + 1 ))
      print 2 "$clk... Missed (no test matches clk)"
      ;;
  esac
done

# 'parent' Test:
# Test to make sure measurable child clocks correctly enable their
# parents. If the parent and children are off and we enable a child,
# the child rate will only measure non-zero if the parent has been
# implicitly enabled.  Note this is only possible to test here,
# at the end of the script, since we now known all rate-settable
# parents have have been set.
parent=0
for c in $all_children; do
  choose_test $c  # sets $test_type
  if [ "x$test_type" = "xblacklisted" ]; then
	continue
  fi
  print 2 "Testing $c with 'parent' test"
  test_stateonly $c
  if [ $? -eq 0 ]; then
    print 2 "$c... Passed (parent test)"
    passed=$(( $passed + 1 ))
  else
    failed=$(( $failed + 1 ))
    print 0 "$c... Failed (parent test)"
  fi
    parent=$(( $parent + 1 ))
done

for gdsc in $gdsc_on_list; do
  echo 0 > $gdsc/enable
done

echo
echo "----------- Test Results -----------"
echo "Passed:                 $passed"
echo "Failed:                 $failed"
echo "Blacklisted (skip)   :  $blacklisted"
echo "Hardware gated (skip):  $hwgated"
echo "Missed:                 $missed"
echo "                        ----"
echo "Total:                  $total = "$(($passed + $failed + $blacklisted + \
                                           $missed + $hwgated))
echo
echo "-------- Test Type Counters --------"
echo "All-rate tests:   $allrate"
echo "One-rate tests:   $onerate"
echo "State-only tests: $stateonly"
echo "Vote tests:       $vote"
echo "Parent tests:     $parent"
echo "Children tests:   $children_found"
echo

if [ "x$platform" = "xANDROID" ]; then
  print 0 "Restarting Android services"
  start
fi

if [ $total -ne $(($passed + $failed + $blacklisted + $missed + $hwgated)) ]; then
  echo "Test error. Discrepancy between number of tested and testable clocks."
  exit 1
fi

if [ $failed -ne 0 -o $passed -eq 0 ]; then
  echo "Test Failed"
  exit 1
else
  echo "Test Passed"
  exit 0
fi
