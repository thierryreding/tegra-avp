#ifndef AVP_BIT_H
#define AVP_BIT_H

#define BIT_BOOT_TYPE_NONE 0
#define BIT_BOOT_TYPE_COLD 1
#define BIT_BOOT_TYPE_RECOVERY 2
#define BIT_BOOT_TYPE_UART 3
#define BIT_BOOT_TYPE_EXIT_RCM 4

#define BIT_DEV_TYPE_NONE 0
#define BIT_DEV_TYPE_NAND 1
#define BIT_DEV_TYPE_SNOR 2
#define BIT_DEV_TYPE_SPI 3
#define BIT_DEV_TYPE_SDMMC 4
#define BIT_DEV_TYPE_IROM 5
#define BIT_DEV_TYPE_UART 6
#define BIT_DEV_TYPE_USB 7
#define BIT_DEV_TYPE_NAND_X16 8
#define BIT_DEV_TYPE_USB3 9

struct bit_boot_time_log {
	uint32_t init;
	uint32_t exit;
	uint32_t bct;
	uint32_t bootloader;
};

#define BIT_MAX_BCT_SEARCH_BLOCKS 64
#define BIT_BCT_STATUS_SIZE ((BIT_MAX_BCT_SEARCH_BLOCKS + 8) / 8)

struct bit_bootloader_state {
	uint32_t status;
	uint32_t first_ecc_block;
	uint32_t first_ecc_page;
	uint32_t first_corrected_ecc_block;
	uint32_t first_corrected_ecc_page;
	uint8_t had_ecc_error;
	uint8_t had_crc_error;
	uint8_t had_corrected_ecc_error;
	uint8_t used_for_ecc_recovery;
};

struct bit_nand_status {
	uint32_t fuse_data_width;
	uint8_t fuse_disable_onfi_support;
	uint32_t fuse_ecc_selection;
	uint8_t fuse_page_block_size_offset;
	uint32_t discovered_data_width;
	uint32_t discovered_ecc_selection;
	uint32_t id_read;
	uint32_t id_read2;
	uint8_t is_part_onfi;
	uint32_t num_pages_read;
	uint32_t num_uncorrectable_error_pages;
	uint32_t num_correctable_error_pages;
	uint32_t max_correctable_errors_encountered;
};

struct bit_usb3_status {
	uint8_t port;
	uint8_t sense_key;
	uint32_t curr_csw_tag;
	uint32_t curr_cmd_csw_status;
	uint32_t curr_ep_bytes_not_transferred;
	uint32_t peripheral_dev_typ;
	uint32_t num_blocks;
	uint32_t last_logical_block_address;
	uint32_t block_length_in_bytes;
	uint32_t usb3_context;
	uint32_t init_return_value;
	uint32_t read_page_return_value;
	uint32_t xusb_driver_status;
	uint32_t device_status;
	uint32_t ep_status;
};

struct bit_sdmmc_status {
	uint8_t fuse_data_width;
	uint8_t fuse_voltage_range;
	uint8_t fuse_disable_boot_mode;
	uint8_t fuse_ddr_mode;
	uint8_t discovered_card_type;
	uint32_t discovered_voltage_range;
	uint8_t data_width_under_use;
	uint8_t power_class_under_use;
	uint8_t auto_cal_status;
	uint32_t cid[4];
	uint32_t num_pages_read;
	uint32_t num_crc_errors;
	uint8_t boot_from_boot_partition;
	uint8_t boot_mode_read_successful;
};

struct bit_snor_status {
	uint32_t clock_source;
	uint32_t clock_divider;
	uint32_t snor_config;
	uint32_t timing_cfg0;
	uint32_t timing_cfg1;
	uint32_t timing_cfg2;
	uint32_t last_block_read;
	uint32_t last_page_read;
	uint32_t snor_driver_status;
};

struct bit_spi_status {
	uint32_t clock_source;
	uint32_t clock_divider;
	uint32_t is_fast_read;
	uint32_t num_pages_read;
	uint32_t last_block_read;
	uint32_t last_page_read;
	uint32_t boot_status;
	uint32_t init_status;
	uint32_t read_status;
	uint32_t params_validated;
};

union bit_secondary_device_status {
	struct bit_nand_status nand;
	struct bit_usb3_status usb3;
	struct bit_sdmmc_status sdmmc;
	struct bit_snor_status snor;
	struct bit_spi_status spi;
};

#define BIT_MAX_BOOTLOADERS 4

struct bit {
	uint32_t boot_rom_version;
	uint32_t data_version;
	uint32_t rcm_version;
	uint32_t boot_type;
	uint32_t primary_device;
	uint32_t secondary_device;
	struct bit_boot_time_log boot_time_log;
	uint32_t osc_frequency;
	uint8_t device_initialized;
	uint8_t sdram_initialized;
	uint8_t cleared_forced_recovery;
	uint8_t cleared_fail_back;
	uint8_t invoked_fail_back;
	uint8_t irom_patch_status;
	uint8_t bct_valid;
	uint8_t bct_status[BIT_BCT_STATUS_SIZE];
	uint32_t bct_last_journal_read;
	uint32_t bct_block;
	uint32_t bct_page;
	uint32_t bct_size;
	uint32_t bct;
	struct bit_bootloader_state bootloader_state[BIT_MAX_BOOTLOADERS];
	union bit_secondary_device_status secondary_dev_status;
	uint32_t usb_charging_status;
	uint32_t safe_start_address;
};

#endif
