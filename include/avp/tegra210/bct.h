#ifndef AVP_TEGRA124_BCT_H
#define AVP_TEGRA124_BCT_H

#include <common.h>
#include <types.h>

#define BCT_BAD_BLOCK_TABLE_SIZE 4096
#define BCT_BAD_BLOCK_TABLE_PADDING 10

struct bct_bad_block_table {
	uint32_t num_entries;
	uint8_t virtual_block_size;
	uint8_t block_size;
	uint8_t bad_blocks[BCT_BAD_BLOCK_TABLE_SIZE / 8];
	uint8_t reserved[BCT_BAD_BLOCK_TABLE_PADDING];
} __packed;

#define BCT_RSA_KEY_MODULUS_SIZE 2048

struct bct_rsa_key_modulus {
	uint8_t modulus[BCT_RSA_KEY_MODULUS_SIZE / 8];
} __packed;

#define BCT_CMAC_AES_HASH_SIZE 16

struct bct_hash {
	uint8_t hash[BCT_CMAC_AES_HASH_SIZE];
} __packed;

struct bct_rsa_pss_signature {
	uint8_t signature[BCT_RSA_KEY_MODULUS_SIZE / 8];
} __packed;

struct bct_object_signature {
	struct bct_hash hash;
	struct bct_rsa_pss_signature signature;
} __packed;

#define BCT_CUSTOMER_DATA_SIZE 204

struct bct_ecid {
	uint32_t ecid[4];
} __packed;

struct bct_sdmmc_params {
	uint8_t clock_divider;
	uint32_t data_width;
	uint8_t max_power_class;
	uint8_t multi_page_support;
} __packed;

struct bct_snor_params {
	uint32_t clock_source;
	uint32_t clock_divider;
	uint32_t timing_cfg0;
	uint32_t timing_cfg1;
	uint32_t timing_cfg2;
	uint32_t transfer_mode;
	uint32_t busy_timeout;
	uint32_t dma_timeout;
} __packed;

struct bct_spi_params {
	uint32_t clock_source;
	uint8_t clock_divider;
	uint8_t read_type;
	uint8_t page_size;
} __packed;

union bct_dev_params {
	uint8_t data[64];
	struct bct_sdmmc_params sdmmc;
	struct bct_snor_params snor;
	struct bct_spi_params spi;
} __packed;

#define BCT_MEMORY_TYPE_NONE 0
#define BCT_MEMORY_TYPE_LPDDR2 1
#define BCT_MEMORY_TYPE_DDR3 2

struct bct_sdram_params {
	uint32_t memory_type;
	uint32_t pll_m_div_m;
	uint32_t pll_m_div_n;
	uint32_t pll_m_stable;
	uint32_t pll_m_setup_control;
	uint32_t pll_m_select_div2;
	uint32_t pll_m_kcp;
	uint32_t pll_m_kvco;
	uint32_t emc_bct_spare0;
	uint32_t emc_bct_spare1;
	uint32_t emc_bct_spare2;
	uint32_t emc_bct_spare3;
	uint32_t emc_bct_spare4;
	uint32_t emc_bct_spare5;
	uint32_t emc_bct_spare6;
	uint32_t emc_bct_spare7;
	uint32_t emc_bct_spare8;
	uint32_t emc_bct_spare9;
	uint32_t emc_bct_spare10;
	uint32_t emc_bct_spare11;
	uint32_t emc_bct_spare12;
	uint32_t emc_bct_spare13;
	uint32_t emc_clock_source;
	uint32_t emc_clock_source_dll;
	uint32_t clk_rst_pll_m_misc2_override;
	uint32_t clk_rst_pll_m_misc2_override_enable;
	uint32_t clear_clk2mc1;
	uint32_t emc_auto_cal_interval;
	uint32_t emc_auto_cal_config;
	uint32_t emc_auto_cal_config2;
	uint32_t emc_auto_cal_config3;
	uint32_t emc_auto_cal_config4;
	uint32_t emc_auto_cal_config5;
	uint32_t emc_auto_cal_config6;
	uint32_t emc_auto_cal_config7;
	uint32_t emc_auto_cal_config8;
	uint32_t emc_auto_cal_vref_sel_0;
	uint32_t emc_auto_cal_vref_sel_1;
	uint32_t emc_auto_cal_channel;
	uint32_t emc_pmacro_autocal_cfg_0;
	uint32_t emc_pmacro_autocal_cfg_1;
	uint32_t emc_pmacro_autocal_cfg_2;
	uint32_t emc_pmacro_rx_term;
	uint32_t emc_pmacro_dq_tx_drv;
	uint32_t emc_pmacro_ca_tx_drv;
	uint32_t emc_pmacro_cmd_tx_drv;
	uint32_t emc_pmacro_autocal_cfg_common;
	uint32_t emc_pmacro_zctrl;
	uint32_t emc_auto_cal_wait;
	uint32_t emc_xm2comppadctrl;
	uint32_t emc_xm2comppadctrl2;
	uint32_t emc_xm2comppadctrl3;
	uint32_t emc_adr_cfg;
	uint32_t emc_pin_program_wait;
	uint32_t emc_pin_extra_wait;
	uint32_t emc_pin_gpio_en;
	uint32_t emc_pin_gpio;
	uint32_t emc_timing_control_wait;
	uint32_t emc_rc;
	uint32_t emc_rfc;
	uint32_t emc_rfcpb;
	uint32_t emc_refctrl2;
	uint32_t emc_rfc_slr;
	uint32_t emc_ras;
	uint32_t emc_rp;
	uint32_t emc_r2r;
	uint32_t emc_w2w;
	uint32_t emc_r2w;
	uint32_t emc_w2r;
	uint32_t emc_r2p;
	uint32_t emc_w2p;
	uint32_t emc_tppd;
	uint32_t emc_ccdmw;
	uint32_t emc_rd_rcd;
	uint32_t emc_wr_rcd;
	uint32_t emc_rrd;
	uint32_t emc_rext;
	uint32_t emc_wext;
	uint32_t emc_wdv;
	uint32_t emc_wdv_chk;
	uint32_t emc_wsv;
	uint32_t emc_wev;
	uint32_t emc_wdv_mask;
	uint32_t emc_ws_duration;
	uint32_t emc_we_duration;
	uint32_t emc_quse;
	uint32_t emc_quse_width;
	uint32_t emc_ibdly;
	uint32_t emc_obdly;
	uint32_t emc_einput;
	uint32_t emc_einput_duration;
	uint32_t emc_puterm_extra;
	uint32_t emc_puterm_width;
	uint32_t emc_qrst;
	uint32_t emc_qsafe;
	uint32_t emc_rdv;
	uint32_t emc_rdv_mask;
	uint32_t emc_rdv_early;
	uint32_t emc_rdv_early_mask;
	uint32_t emc_qpop;
	uint32_t emc_refresh;
	uint32_t emc_burst_refresh_num;
	uint32_t emc_pre_refresh_req_cnt;
	uint32_t emc_pdex2wr;
	uint32_t emc_pdex2rd;
	uint32_t emc_pchg2pden;
	uint32_t emc_act2pden;
	uint32_t emc_ar2pden;
	uint32_t emc_rw2pden;
	uint32_t emc_cke2pden;
	uint32_t emc_pdex2cke;
	uint32_t emc_pdex2mrr;
	uint32_t emc_txsr;
	uint32_t emc_txsrdll;
	uint32_t emc_tcke;
	uint32_t emc_tckesr;
	uint32_t emc_tpd;
	uint32_t emc_tfaw;
	uint32_t emc_trpab;
	uint32_t emc_tclkstable;
	uint32_t emc_tclkstop;
	uint32_t emc_trefbw;
	uint32_t emc_fbio_cfg5;
	uint32_t emc_fbio_cfg7;
	uint32_t emc_fbio_cfg8;
	uint32_t emc_cmd_mapping_cmd0_0;
	uint32_t emc_cmd_mapping_cmd0_1;
	uint32_t emc_cmd_mapping_cmd0_2;
	uint32_t emc_cmd_mapping_cmd1_0;
	uint32_t emc_cmd_mapping_cmd1_1;
	uint32_t emc_cmd_mapping_cmd1_2;
	uint32_t emc_cmd_mapping_cmd2_0;
	uint32_t emc_cmd_mapping_cmd2_1;
	uint32_t emc_cmd_mapping_cmd2_2;
	uint32_t emc_cmd_mapping_cmd3_0;
	uint32_t emc_cmd_mapping_cmd3_1;
	uint32_t emc_cmd_mapping_cmd3_2;
	uint32_t emc_cmd_mapping_byte;
	uint32_t emc_fbio_spare;
	uint32_t emc_cfg_rsv;
	uint32_t emc_mrs;
	uint32_t emc_emrs;
	uint32_t emc_emrs2;
	uint32_t emc_emrs3;
	uint32_t emc_mrw1;
	uint32_t emc_mrw2;
	uint32_t emc_mrw3;
	uint32_t emc_mrw4;
	uint32_t emc_mrw6;
	uint32_t emc_mrw8;
	uint32_t emc_mrw9;
	uint32_t emc_mrw10;
	uint32_t emc_mrw12;
	uint32_t emc_mrw13;
	uint32_t emc_mrw14;
	uint32_t emc_mrw_extra;
	uint32_t emc_warm_boot_mrw_extra;
	uint32_t emc_warm_boot_extra_mode_reg_write_enable;
	uint32_t emc_extra_mode_reg_write_enable;
	uint32_t emc_mrw_reset_command;
	uint32_t emc_mrw_reset_init_wait;
	uint32_t emc_mrs_wait_cnt;
	uint32_t emc_mrs_wait_cnt2;
	uint32_t emc_cfg;
	uint32_t emc_cfg_2;
	uint32_t emc_cfg_pipe;
	uint32_t emc_cfg_pipe_clk;
	uint32_t emc_fdpd_ctrl_cmd_no_ramp;
	uint32_t emc_cfg_update;
	uint32_t emc_dbg;
	uint32_t emc_dbg_write_mux;
	uint32_t emc_cmdq;
	uint32_t emc_mc2emcq;
	uint32_t emc_dyn_self_ref_control;
	uint32_t arb_xbar_ctrl_mem_init_done;
	uint32_t emc_cfg_dig_dll;
	uint32_t emc_cfg_dig_dll_1;
	uint32_t emc_cfg_dig_dll_period;
	uint32_t emc_dev_select;
	uint32_t emc_sel_dpd_ctrl;
	uint32_t emc_fdpd_ctrl_dq;
	uint32_t emc_fdpd_ctrl_cmd;
	uint32_t emc_pmacro_ib_vref_dq_0;
	uint32_t emc_pmacro_ib_vref_dq_1;
	uint32_t emc_pmacro_ib_vref_dqs_0;
	uint32_t emc_pmacro_ib_vref_dqs_1;
	uint32_t emc_pmacro_ib_rxrt;
	uint32_t emc_cfg_pipe_1;
	uint32_t emc_cfg_pipe_2;
	uint32_t emc_pmacro_quse_ddll_rank0_0;
	uint32_t emc_pmacro_quse_ddll_rank0_1;
	uint32_t emc_pmacro_quse_ddll_rank0_2;
	uint32_t emc_pmacro_quse_ddll_rank0_3;
	uint32_t emc_pmacro_quse_ddll_rank0_4;
	uint32_t emc_pmacro_quse_ddll_rank0_5;
	uint32_t emc_pmacro_quse_ddll_rank1_0;
	uint32_t emc_pmacro_quse_ddll_rank1_1;
	uint32_t emc_pmacro_quse_ddll_rank1_2;
	uint32_t emc_pmacro_quse_ddll_rank1_3;
	uint32_t emc_pmacro_quse_ddll_rank1_4;
	uint32_t emc_pmacro_quse_ddll_rank1_5;
	uint32_t emc_pmacro_ob_ddll_long_dq_rank0_0;
	uint32_t emc_pmacro_ob_ddll_long_dq_rank0_1;
	uint32_t emc_pmacro_ob_ddll_long_dq_rank0_2;
	uint32_t emc_pmacro_ob_ddll_long_dq_rank0_3;
	uint32_t emc_pmacro_ob_ddll_long_dq_rank0_4;
	uint32_t emc_pmacro_ob_ddll_long_dq_rank0_5;
	uint32_t emc_pmacro_ob_ddll_long_dq_rank1_0;
	uint32_t emc_pmacro_ob_ddll_long_dq_rank1_1;
	uint32_t emc_pmacro_ob_ddll_long_dq_rank1_2;
	uint32_t emc_pmacro_ob_ddll_long_dq_rank1_3;
	uint32_t emc_pmacro_ob_ddll_long_dq_rank1_4;
	uint32_t emc_pmacro_ob_ddll_long_dq_rank1_5;
	uint32_t emc_pmacro_ob_ddll_long_dqs_rank0_0;
	uint32_t emc_pmacro_ob_ddll_long_dqs_rank0_1;
	uint32_t emc_pmacro_ob_ddll_long_dqs_rank0_2;
	uint32_t emc_pmacro_ob_ddll_long_dqs_rank0_3;
	uint32_t emc_pmacro_ob_ddll_long_dqs_rank0_4;
	uint32_t emc_pmacro_ob_ddll_long_dqs_rank0_5;
	uint32_t emc_pmacro_ob_ddll_long_dqs_rank1_0;
	uint32_t emc_pmacro_ob_ddll_long_dqs_rank1_1;
	uint32_t emc_pmacro_ob_ddll_long_dqs_rank1_2;
	uint32_t emc_pmacro_ob_ddll_long_dqs_rank1_3;
	uint32_t emc_pmacro_ob_ddll_long_dqs_rank1_4;
	uint32_t emc_pmacro_ob_ddll_long_dqs_rank1_5;
	uint32_t emc_pmacro_ib_ddll_long_dqs_rank0_0;
	uint32_t emc_pmacro_ib_ddll_long_dqs_rank0_1;
	uint32_t emc_pmacro_ib_ddll_long_dqs_rank0_2;
	uint32_t emc_pmacro_ib_ddll_long_dqs_rank0_3;
	uint32_t emc_pmacro_ib_ddll_long_dqs_rank1_0;
	uint32_t emc_pmacro_ib_ddll_long_dqs_rank1_1;
	uint32_t emc_pmacro_ib_ddll_long_dqs_rank1_2;
	uint32_t emc_pmacro_ib_ddll_long_dqs_rank1_3;
	uint32_t emc_pmacro_ddll_long_cmd_0;
	uint32_t emc_pmacro_ddll_long_cmd_1;
	uint32_t emc_pmacro_ddll_long_cmd_2;
	uint32_t emc_pmacro_ddll_long_cmd_3;
	uint32_t emc_pmacro_ddll_long_cmd_4;
	uint32_t emc_pmacro_ddll_short_cmd_0;
	uint32_t emc_pmacro_ddll_short_cmd_1;
	uint32_t emc_pmacro_ddll_short_cmd_2;
	uint32_t warm_boot_wait;
	uint32_t emc_odt_write;
	uint32_t emc_zcal_interval;
	uint32_t emc_zcal_wait_cnt;
	uint32_t emc_zcal_mrw_cmd;
	uint32_t emc_mrs_reset_dll;
	uint32_t emc_zcal_init_dev0;
	uint32_t emc_zcal_init_dev1;
	uint32_t emc_zcal_init_wait;
	uint32_t emc_zcal_warm_cold_boot_enables;
	uint32_t emc_mrw_lpddr2_zcal_warm_boot;
	uint32_t emc_zqcal_ddr3_warm_boot;
	uint32_t emc_zqcal_lpddr4_warm_boot;
	uint32_t emc_zcal_warm_boot_wait;
	uint32_t emc_mrs_warm_boot_enable;
	uint32_t emc_mrs_reset_dll_wait;
	uint32_t emc_mrs_extra;
	uint32_t emc_warm_boot_mrs_extra;
	uint32_t emc_emrs_ddr2_dll_enable;
	uint32_t emc_mrs_ddr2_dll_reset;
	uint32_t emc_emrs_ddr2_ocd_calib;
	uint32_t emc_ddr2_wait;
	uint32_t emc_clken_override;
	uint32_t emc_extra_refresh_num;
	uint32_t emc_clken_override_all_warm_boot;
	uint32_t mc_clken_override_all_warm_boot;
	uint32_t emc_cfg_dig_dll_period_warm_boot;
	uint32_t pmc_vddp_sel;
	uint32_t pmc_vddp_sel_wait;
	uint32_t pmc_ddr_pwr;
	uint32_t pmc_ddr_cfg;
	uint32_t pmc_io_dpd3_req;
	uint32_t pmc_io_dpd3_req_wait;
	uint32_t pmc_io_dpd4_req_wait;
	uint32_t pmc_reg_short;
	uint32_t pmc_no_iopower;
	uint32_t pmc_ddr_cntrl_wait;
	uint32_t pmc_ddr_cntrl;
	uint32_t emc_acpd_control;
	uint32_t emc_swizzle_rank0_byte0;
	uint32_t emc_swizzle_rank0_byte1;
	uint32_t emc_swizzle_rank0_byte2;
	uint32_t emc_swizzle_rank0_byte3;
	uint32_t emc_swizzle_rank1_byte0;
	uint32_t emc_swizzle_rank1_byte1;
	uint32_t emc_swizzle_rank1_byte2;
	uint32_t emc_swizzle_rank1_byte3;
	uint32_t emc_txdsrvttgen;
	uint32_t emc_data_brlshft_0;
	uint32_t emc_data_brlshft_1;
	uint32_t emc_dqs_brlshft_0;
	uint32_t emc_dqs_brlshft_1;
	uint32_t emc_cmd_brlshft_0;
	uint32_t emc_cmd_brlshft_1;
	uint32_t emc_cmd_brlshft_2;
	uint32_t emc_cmd_brlshft_3;
	uint32_t emc_quse_brlshft_0;
	uint32_t emc_quse_brlshft_1;
	uint32_t emc_quse_brlshft_2;
	uint32_t emc_quse_brlshft_3;
	uint32_t emc_dll_cfg_0;
	uint32_t emc_dll_cfg_1;
	uint32_t emc_pmc_scratch1;
	uint32_t emc_pmc_scratch2;
	uint32_t emc_pmc_scratch3;
	uint32_t emc_pmacro_pad_cfg_ctrl;
	uint32_t emc_pmacro_vttgen_ctrl_0;
	uint32_t emc_pmacro_vttgen_ctrl_1;
	uint32_t emc_pmacro_vttgen_ctrl_2;
	uint32_t emc_pmacro_brick_ctrl_rfu1;
	uint32_t emc_pmacro_cmd_brick_ctrl_fdpd;
	uint32_t emc_pmacro_brick_ctrl_rfu2;
	uint32_t emc_pmacro_data_brick_ctrl_fdpd;
	uint32_t emc_pmacro_bg_bias_ctrl_0;
	uint32_t emc_pmacro_data_pad_rx_ctrl;
	uint32_t emc_pmacro_cmd_pad_rx_ctrl;
	uint32_t emc_pmacro_data_rx_term_mode;
	uint32_t emc_pmacro_cmd_rx_term_mode;
	uint32_t emc_pmacro_data_pad_tx_ctrl;
	uint32_t emc_pmacro_common_pad_tx_ctrl;
	uint32_t emc_pmacro_cmd_pad_tx_ctrl;
	uint32_t emc_cfg_3;
	uint32_t emc_pmacro_tx_pwrd_0;
	uint32_t emc_pmacro_tx_pwrd_1;
	uint32_t emc_pmacro_tx_pwrd_2;
	uint32_t emc_pmacro_tx_pwrd_3;
	uint32_t emc_pmacro_tx_pwrd_4;
	uint32_t emc_pmacro_tx_pwrd_5;
	uint32_t emc_config_sample_delay;
	uint32_t emc_pmacro_brick_mapping_0;
	uint32_t emc_pmacro_brick_mapping_1;
	uint32_t emc_pmacro_brick_mapping_2;
	uint32_t emc_pmacro_tx_sel_clk_src_0;
	uint32_t emc_pmacro_tx_sel_clk_src_1;
	uint32_t emc_pmacro_tx_sel_clk_src_2;
	uint32_t emc_pmacro_tx_sel_clk_src_3;
	uint32_t emc_pmacro_tx_sel_clk_src_4;
	uint32_t emc_pmacro_tx_sel_clk_src_5;
	uint32_t emc_pmacro_ddll_bypass;
	uint32_t emc_pmacro_ddll_pwrd_0;
	uint32_t emc_pmacro_ddll_pwrd_1;
	uint32_t emc_pmacro_ddll_pwrd_2;
	uint32_t emc_pmacro_cmd_ctrl_0;
	uint32_t emc_pmacro_cmd_ctrl_1;
	uint32_t emc_pmacro_cmd_ctrl_2;
	uint32_t mc_emem_adr_cfg;
	uint32_t mc_emem_adr_cfg_dev0;
	uint32_t mc_emem_adr_cfg_dev1;
	uint32_t mc_emem_adr_cfg_channel_mask;
	uint32_t mc_emem_adr_cfg_bank_mask_0;
	uint32_t mc_emem_adr_cfg_bank_mask_1;
	uint32_t mc_emem_adr_cfg_bank_mask_2;
	uint32_t mc_emem_cfg;
	uint32_t mc_emem_arb_cfg;
	uint32_t mc_emem_arb_outstanding_req;
	uint32_t mc_emem_arb_refpb_hp_ctrl;
	uint32_t mc_emem_arb_refpb_bank_ctrl;
	uint32_t mc_emem_arb_timing_rcd;
	uint32_t mc_emem_arb_timing_rp;
	uint32_t mc_emem_arb_timing_rc;
	uint32_t mc_emem_arb_timing_ras;
	uint32_t mc_emem_arb_timing_faw;
	uint32_t mc_emem_arb_timing_rrd;
	uint32_t mc_emem_arb_timing_rap2pre;
	uint32_t mc_emem_arb_timing_wap2pre;
	uint32_t mc_emem_arb_timing_r2r;
	uint32_t mc_emem_arb_timing_w2w;
	uint32_t mc_emem_arb_timing_r2w;
	uint32_t mc_emem_arb_timing_w2r;
	uint32_t mc_emem_arb_timing_rfcpb;
	uint32_t mc_emem_arb_da_turns;
	uint32_t mc_emem_arb_da_covers;
	uint32_t mc_emem_arb_misc0;
	uint32_t mc_emem_arb_misc1;
	uint32_t mc_emem_arb_misc2;
	uint32_t mc_emem_arb_ring1_throttle;
	uint32_t mc_emem_arb_override_0;
	uint32_t mc_emem_arb_override_1;
	uint32_t mc_emem_arb_rsv;
	uint32_t mc_da_config0;
	uint32_t mc_emem_arb_timing_ccdmw;
	uint32_t mc_clken_override;
	uint32_t mc_stat_control;
	uint32_t mc_video_protect_bom;
	uint32_t mc_video_protect_bom_adr_hi;
	uint32_t mc_video_protect_size_mb;
	uint32_t mc_video_protect_vpr_override0;
	uint32_t mc_video_protect_vpr_override1;
	uint32_t mc_video_protect_gpu_override0;
	uint32_t mc_video_protect_gpu_override1;
	uint32_t mc_sec_carveout_bom;
	uint32_t mc_sec_carveout_adr_hi;
	uint32_t mc_sec_carveout_size_mb;
	uint32_t mc_video_protect_ctrl;
	uint32_t mc_sec_carveout_ctrl;
	struct {
		uint32_t bom;
		uint32_t bom_hi;
		uint32_t size; /* in 128 KiB blocks */
		uint32_t client_access0;
		uint32_t client_access1;
		uint32_t client_access2;
		uint32_t client_access3;
		uint32_t client_access4;
		uint32_t client_force_internal_access0;
		uint32_t client_force_internal_access1;
		uint32_t client_force_internal_access2;
		uint32_t client_force_internal_access3;
		uint32_t client_force_internal_access4;
		uint32_t cfg;
	} mc_security_carveout[5];
	uint32_t emc_ca_training_enable;
	uint32_t swizzle_rank_byte_encode;
	uint32_t boot_rom_patch_control;
	uint32_t boot_rom_patch_data;
	uint32_t mc_mts_carveout_bom;
	uint32_t mc_mts_carveout_adr_hi;
	uint32_t mc_mts_carveout_size_mb;
	uint32_t mc_mts_carveout_ctrl;
} __packed;

struct bct_bootloader_info {
	uint32_t version;
	uint32_t start_block;
	uint32_t start_page;
	uint32_t length;
	uint32_t load;
	uint32_t entry;
	uint32_t attribute;
	struct bct_object_signature signature;
} __packed;

#define BCT_MAX_PARAM_SETS 1
#define BCT_MAX_SDRAM_SETS 4
#define BCT_MAX_BOOTLOADERS 4

#define BCT_RESERVED_SIZE 18

struct bct {
	struct bct_bad_block_table bad_blocks;
	struct bct_rsa_key_modulus key;
	struct bct_object_signature signature;
	uint32_t sec_provisioning_key_num_insecure;
	uint8_t sec_provisioning_key[32];
	uint8_t customer_data[BCT_CUSTOMER_DATA_SIZE];
	struct bct_hash random_aes_block;
	struct bct_ecid unique_chip_id;
	uint32_t boot_data_version;
	uint32_t block_size_log2;
	uint32_t page_size_log2;
	uint32_t partition_size;
	uint32_t num_param_sets;
	uint32_t dev_type[BCT_MAX_PARAM_SETS];
	union bct_dev_params dev_params[BCT_MAX_PARAM_SETS];
	uint32_t num_sdram_sets;
	struct bct_sdram_params sdram_params[BCT_MAX_SDRAM_SETS];
	uint32_t num_bootloaders;
	struct bct_bootloader_info bootloaders[BCT_MAX_BOOTLOADERS];
	uint8_t enable_fail_back;
	uint32_t secure_debug_control;
	uint32_t sec_provisioning_key_num_secure;
	uint8_t reserved[BCT_RESERVED_SIZE];
};

#endif
