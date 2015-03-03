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

#define BCT_CUSTOMER_DATA_SIZE 656

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
	uint32_t pll_m_pd_lshift_ph45;
	uint32_t pll_m_pd_lshift_ph90;
	uint32_t pll_m_pd_lshift_ph135;
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
	uint32_t emc_clock_source;
	uint32_t emc_auto_cal_interval;
	uint32_t emc_auto_cal_config;
	uint32_t emc_auto_cal_config2;
	uint32_t emc_auto_cal_config3;
	uint32_t emc_auto_cal_wait;
	uint32_t emc_adr_cfg;
	uint32_t emc_pin_program_wait;
	uint32_t emc_pin_extra_wait;
	uint32_t emc_timing_control_wait;
	uint32_t emc_rc;
	uint32_t emc_rfc;
	uint32_t emc_rfc_slr;
	uint32_t emc_ras;
	uint32_t emc_rp;
	uint32_t emc_r2r;
	uint32_t emc_w2w;
	uint32_t emc_r2w;
	uint32_t emc_w2r;
	uint32_t emc_r2p;
	uint32_t emc_w2p;
	uint32_t emc_rd_rcd;
	uint32_t emc_wr_rcd;
	uint32_t emc_rrd;
	uint32_t emc_rext;
	uint32_t emc_wext;
	uint32_t emc_wdv;
	uint32_t emc_wdv_mask;
	uint32_t emc_quse;
	uint32_t emc_quse_width;
	uint32_t emc_ibdly;
	uint32_t emc_einput;
	uint32_t emc_einput_duration;
	uint32_t emc_puterm_extra;
	uint32_t emc_puterm_width;
	uint32_t emc_puterm_adj;
	uint32_t emc_cdb_cntl1;
	uint32_t emc_cdb_cntl2;
	uint32_t emc_cdb_cntl3;
	uint32_t emc_qrst;
	uint32_t emc_qsafe;
	uint32_t emc_rdv;
	uint32_t emc_rdv_mask;
	uint32_t emc_qpop;
	uint32_t emc_ctt;
	uint32_t emc_ctt_duration;
	uint32_t emc_refresh;
	uint32_t emc_burst_refresh_num;
	uint32_t emc_pre_refresh_req_cnt;
	uint32_t emc_pdex2wr;
	uint32_t emc_pdex2rd;
	uint32_t emc_pchg2pden;
	uint32_t emc_act2pden;
	uint32_t emc_ar2pden;
	uint32_t emc_rw2pden;
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
	uint32_t emc_fbio_cfg6;
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
	uint32_t emc_mrw_extra;
	uint32_t emc_warm_boot_mrw_extra;
	uint32_t emc_warm_boot_extra_mode_reg_write_enable;
	uint32_t emc_extra_mode_reg_write_enable;
	uint32_t emc_mrw_reset_command;
	uint32_t emc_mrw_reset_init_wait;
	uint32_t emc_mrs_wait_cnt;
	uint32_t emc_mrs_wait_cnt2;
	uint32_t emc_cfg;
	uint32_t emc_cfg2;
	uint32_t emc_cfg_pipe;
	uint32_t emc_dbg;
	uint32_t emc_cmdq;
	uint32_t emc_mc2emcq;
	uint32_t emc_dyn_self_ref_control;
	uint32_t arb_xbar_ctrl_mem_init_done;
	uint32_t emc_cfg_dig_dll;
	uint32_t emc_cfg_dig_dll_period;
	uint32_t emc_dev_select;
	uint32_t emc_sel_dpd_ctrl;
	uint32_t emc_dll_xform_dqs0;
	uint32_t emc_dll_xform_dqs1;
	uint32_t emc_dll_xform_dqs2;
	uint32_t emc_dll_xform_dqs3;
	uint32_t emc_dll_xform_dqs4;
	uint32_t emc_dll_xform_dqs5;
	uint32_t emc_dll_xform_dqs6;
	uint32_t emc_dll_xform_dqs7;
	uint32_t emc_dll_xform_dqs8;
	uint32_t emc_dll_xform_dqs9;
	uint32_t emc_dll_xform_dqs10;
	uint32_t emc_dll_xform_dqs11;
	uint32_t emc_dll_xform_dqs12;
	uint32_t emc_dll_xform_dqs13;
	uint32_t emc_dll_xform_dqs14;
	uint32_t emc_dll_xform_dqs15;
	uint32_t emc_dll_xform_quse0;
	uint32_t emc_dll_xform_quse1;
	uint32_t emc_dll_xform_quse2;
	uint32_t emc_dll_xform_quse3;
	uint32_t emc_dll_xform_quse4;
	uint32_t emc_dll_xform_quse5;
	uint32_t emc_dll_xform_quse6;
	uint32_t emc_dll_xform_quse7;
	uint32_t emc_dll_xform_addr0;
	uint32_t emc_dll_xform_addr1;
	uint32_t emc_dll_xform_addr2;
	uint32_t emc_dll_xform_addr3;
	uint32_t emc_dll_xform_addr4;
	uint32_t emc_dll_xform_addr5;
	uint32_t emc_dll_xform_quse8;
	uint32_t emc_dll_xform_quse9;
	uint32_t emc_dll_xform_quse10;
	uint32_t emc_dll_xform_quse11;
	uint32_t emc_dll_xform_quse12;
	uint32_t emc_dll_xform_quse13;
	uint32_t emc_dll_xform_quse14;
	uint32_t emc_dll_xform_quse15;
	uint32_t emc_dli_trim_txdqs0;
	uint32_t emc_dli_trim_txdqs1;
	uint32_t emc_dli_trim_txdqs2;
	uint32_t emc_dli_trim_txdqs3;
	uint32_t emc_dli_trim_txdqs4;
	uint32_t emc_dli_trim_txdqs5;
	uint32_t emc_dli_trim_txdqs6;
	uint32_t emc_dli_trim_txdqs7;
	uint32_t emc_dli_trim_txdqs8;
	uint32_t emc_dli_trim_txdqs9;
	uint32_t emc_dli_trim_txdqs10;
	uint32_t emc_dli_trim_txdqs11;
	uint32_t emc_dli_trim_txdqs12;
	uint32_t emc_dli_trim_txdqs13;
	uint32_t emc_dli_trim_txdqs14;
	uint32_t emc_dli_trim_txdqs15;
	uint32_t emc_dll_xform_dq0;
	uint32_t emc_dll_xform_dq1;
	uint32_t emc_dll_xform_dq2;
	uint32_t emc_dll_xform_dq3;
	uint32_t emc_dll_xform_dq4;
	uint32_t emc_dll_xform_dq5;
	uint32_t emc_dll_xform_dq6;
	uint32_t emc_dll_xform_dq7;
	uint32_t emc_warm_boot_wait;
	uint32_t emc_ctt_term_ctrl;
	uint32_t emc_odt_write;
	uint32_t emc_odt_read;
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
	uint32_t mc_dis_extra_snap_levels;
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
	uint32_t pmc_reg_short;
	uint32_t pmc_no_iopower;
	uint32_t pmc_por_dpd_ctrl_wait;
	uint32_t emc_xm2cmdpadctrl;
	uint32_t emc_xm2cmdpadctrl2;
	uint32_t emc_xm2cmdpadctrl3;
	uint32_t emc_xm2cmdpadctrl4;
	uint32_t emc_xm2cmdpadctrl5;
	uint32_t emc_xm2dqspadctrl;
	uint32_t emc_xm2dqspadctrl2;
	uint32_t emc_xm2dqspadctrl3;
	uint32_t emc_xm2dqspadctrl4;
	uint32_t emc_xm2dqspadctrl5;
	uint32_t emc_xm2dqspadctrl6;
	uint32_t emc_xm2dqpadctrl;
	uint32_t emc_xm2dqpadctrl2;
	uint32_t emc_xm2dqpadctrl3;
	uint32_t emc_xm2clkpadctrl;
	uint32_t emc_xm2clkpadctrl2;
	uint32_t emc_xm2comppadctrl;
	uint32_t emc_xm2vttgenpadctrl;
	uint32_t emc_xm2vttgenpadctrl2;
	uint32_t emc_xm2vttgenpadctrl3;
	uint32_t emc_acpd_control;
	uint32_t emc_swizzle_rank0_byte_cfg;
	uint32_t emc_swizzle_rank0_byte0;
	uint32_t emc_swizzle_rank0_byte1;
	uint32_t emc_swizzle_rank0_byte2;
	uint32_t emc_swizzle_rank0_byte3;
	uint32_t emc_swizzle_rank1_byte_cfg;
	uint32_t emc_swizzle_rank1_byte0;
	uint32_t emc_swizzle_rank1_byte1;
	uint32_t emc_swizzle_rank1_byte2;
	uint32_t emc_swizzle_rank1_byte3;
	uint32_t emc_dsr_vttgen_drv;
	uint32_t emc_txdsrvttgen;
	uint32_t emc_bgbias_ctl;
	uint32_t mc_emem_adr_cfg;
	uint32_t mc_emem_adr_cfg_dev0;
	uint32_t mc_emem_adr_cfg_dev1;
	uint32_t mc_emem_bank_swizzle_cfg0;
	uint32_t mc_emem_bank_swizzle_cfg1;
	uint32_t mc_emem_bank_swizzle_cfg2;
	uint32_t mc_emem_bank_swizzle_cfg3;
	uint32_t mc_emem_cfg;
	uint32_t mc_emem_arb_cfg;
	uint32_t mc_emem_arb_outstanding_req;
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
	uint32_t mc_emem_arb_da_turns;
	uint32_t mc_emem_arb_da_covers;
	uint32_t mc_emem_arb_misc0;
	uint32_t mc_emem_arb_misc1;
	uint32_t mc_emem_arb_ring1_throttle;
	uint32_t mc_emem_arb_override0;
	uint32_t mc_emem_arb_override1;
	uint32_t mc_emem_arb_rsv;
	uint32_t mc_clken_override;
	uint32_t mc_stat_control;
	uint32_t mc_display_snap_ring;
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
	uint32_t emc_ca_training_enable;
	uint32_t emc_ca_training_timing_cntl1;
	uint32_t emc_ca_training_timing_cntl2;
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

#define BCT_MAX_PARAM_SETS 4
#define BCT_MAX_SDRAM_SETS 4
#define BCT_MAX_BOOTLOADERS 4

#ifdef CONFIG_TEGRA132
struct bct_mts_info {
	uint32_t version;
	uint32_t start_block;
	uint32_t start_page;
	uint32_t length;
	uint32_t load;
	uint32_t entry;
	uint32_t attribute;
} __packed;

#define BCT_MAX_MTS_COMPONENTS 6
#endif

#if defined(CONFIG_TEGRA124)
#define BCT_RESERVED_SIZE 2
#endif

#if defined(CONFIG_TEGRA132)
#define BCT_RESERVED_SIZE 341
#endif

struct bct {
	struct bct_bad_block_table bad_blocks;
	struct bct_rsa_key_modulus key;
	struct bct_object_signature signature;
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
#ifdef CONFIG_TEGRA132
	uint32_t num_mts_components;
	struct bct_mts_info mts_components[BCT_MAX_MTS_COMPONENTS];
#endif
	uint8_t enable_fail_back;
	uint8_t secure_jtag_control;
#ifdef CONFIG_TEGRA132
	uint8_t customer_denver_dfd_enable;
#endif
	uint8_t reserved[BCT_RESERVED_SIZE];
} __packed;

#endif
