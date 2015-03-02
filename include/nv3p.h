#ifndef NV3P_H
#define NV3P_H

#include <types.h>

struct usb;

#define NV3P_VERSION 0x01

#define NV3P_PACKET_TYPE_COMMAND 0x01
#define NV3P_PACKET_TYPE_DATA 0x02
#define NV3P_PACKET_TYPE_ACK 0x04
#define NV3P_PACKET_TYPE_NAK 0x05

#define NV3P_COMMAND_GET_PLATFORM_INFO 0x01
#define NV3P_COMMAND_DOWNLOAD_BCT 0x04
#define NV3P_COMMAND_DOWNLOAD_BOOTLOADER 0x06
#define NV3P_COMMAND_STATUS 0x0a

#define NV3P_STATUS_OK 0x00

struct nv3p_header {
	uint32_t version;
	uint32_t type;
	uint32_t sequence;
};

struct nv3p_packet_command {
	struct nv3p_header header;
	uint32_t length;
	uint32_t command;
	uint32_t checksum;
};

struct nv3p_packet_download_bct {
	struct nv3p_header header;
	uint32_t length;
	uint32_t command;
	uint32_t size;
	uint32_t checksum;
};

struct nv3p_packet_download_bootloader {
	struct nv3p_header header;
	uint32_t length;
	uint32_t command;
	uint64_t size;
	uint32_t load;
	uint32_t entry;
	uint32_t checksum;
} __attribute__((packed));

struct nv3p_packet_ack {
	struct nv3p_header header;
	uint32_t checksum;
};

#define NV3P_NAK_SUCCESS  0x00000001
#define NV3P_NAK_BAD_CMD  0x00000002
#define NV3P_NAK_BAD_DATA 0x00000003

struct nv3p_packet_nak {
	struct nv3p_header header;
	uint32_t error;
	uint32_t checksum;
};

#define NV3P_STRING_MAX 32

struct nv3p_command_status {
	char message[NV3P_STRING_MAX];
	uint32_t code;
	uint32_t flags;
};

struct nv3p_packet_status {
	struct nv3p_header header;
	uint32_t length;
	uint32_t command;
	struct nv3p_command_status status;
	uint32_t checksum;
};

struct nv3p_packet_data {
	struct nv3p_header header;
	uint32_t length;
};

struct nv3p_chip_id {
	uint16_t id;
	uint8_t major;
	uint8_t minor;
} __attribute__((packed));

struct nv3p_board_id {
	uint32_t board_no;
	uint32_t fab;
	uint32_t mem_type;
	uint32_t freq;
} __attribute__((packed));

struct nv3p_platform_info {
	uint32_t uid[4];
	struct nv3p_chip_id chip;
	uint32_t sku;
	uint32_t version;
	uint32_t boot_device;
	uint32_t op_mode;
	uint32_t dev_conf_strap;
	uint32_t dev_conf_fuse;
	uint32_t sdram_conf_strap;
	uint32_t reserved[2];
	struct nv3p_board_id board_id;
	/**/
	uint32_t warranty_fuse;
	uint32_t skip_auto_detect;
	/**/
} __attribute__((packed));

struct nv3p_packet_platform_info {
	struct nv3p_header header;
	uint32_t length;
	struct nv3p_platform_info info;
	uint32_t checksum;
} __attribute__((packed));

struct nv3p {
	struct usb *usb;
};

int nv3p_init(struct nv3p *nv3p, struct usb *usb);
int nv3p_process(struct nv3p *nv3p);

#endif
