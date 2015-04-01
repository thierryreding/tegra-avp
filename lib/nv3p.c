#include <common.h>
#include <errno.h>
#include <nv3p.h>
#include <types.h>
#include <string.h>
#include <usb.h>

#include <avp/bct.h>
#include <avp/cpu.h>
#include <avp/flow.h>
#include <avp/io.h>
#include <avp/iomap.h>
#include <avp/sdram.h>
#include <avp/uart.h>
#include <avp/usb.h>

static struct bct bct;

static void nv3p_checksum_init(uint32_t *checksum)
{
	*checksum = 0;
}

static void nv3p_checksum_update(uint32_t *checksum, const void *buffer, size_t size)
{
	const uint8_t *ptr = buffer;
	size_t i;

	for (i = 0; i < size; i++)
		*checksum += ptr[i];
}

static uint32_t nv3p_checksum_done(uint32_t *checksum)
{
	*checksum = ~(*checksum) + 1;

	return *checksum;
}

static ssize_t nv3p_recv(struct nv3p *nv3p, void *buffer, size_t size)
{
	return usb_bulk_recv(nv3p->usb, buffer, size);
}

static ssize_t nv3p_send(struct nv3p *nv3p, const void *buffer, size_t size)
{
	return usb_bulk_send(nv3p->usb, buffer, size);
}

static int nv3p_prepare_platform_info(struct nv3p *nv3p,
				      struct nv3p_platform_info *info)
{
	uint32_t value;

	value = readl(TEGRA_APB_MISC_BASE + 0x804);
	info->chip.id = (value >> 8) & 0xff;
	info->chip.minor = (value >> 16) & 0xf;
	info->chip.major = (value >> 4) & 0xf;

	value = readl(TEGRA_FUSE_BASE + 0x110);
	info->sku = value & 0xf;

	info->version = 0x1;
	info->boot_device = 0x3;

#define PMC_STRAPPING_OPT_A 0x64

	value = readl(TEGRA_PMC_BASE + PMC_STRAPPING_OPT_A);
	if (0)
		uart_printf(debug, "strapping options: %08x\n", (value >> 26) & 0xf);
	/* TODO: map stapping options to boot device */
	info->boot_device = 0x2;

	value = readl(TEGRA_FUSE_BASE + 0x1a0);
	if (value) {
		if (0)
			uart_printf(debug, "security mode\n");

		value = readl(TEGRA_FUSE_BASE + 0x268);
		if (value) {
			unsigned int i;

			value = 0;

			for (i = 0; i < 4; i++)
				value |= readl(TEGRA_FUSE_BASE + 0x050 + (i * 4));

			if (value)
				info->op_mode = 0x4;
			else
				info->op_mode = 0x5;
		} else {
			info->op_mode = 0x4;
		}
	} else {
		value = readl(TEGRA_FUSE_BASE + 0x100);
		if (value) {
			info->op_mode = 0x3;
		} else {
			info->op_mode = 0x1;
		}
	}

	return 0;
}

static int nv3p_recv_command(struct nv3p *nv3p, struct nv3p_packet_command *command)
{
	ssize_t num;

	num = nv3p_recv(nv3p, command, sizeof(*command));
	if (num < 0)
		return num;

	return 0;
}

static int nv3p_recv_ack(struct nv3p *nv3p, uint32_t *error)
{
	/*
	 * The packet could be either an ACK or a NAK. Use the latter for
	 * storage since it is bigger.
	 */
	struct nv3p_packet_nak nak;
	ssize_t num;

	num = nv3p_recv(nv3p, &nak, sizeof(nak));
	if (num < 0)
		return num;

	switch (nak.header.type) {
	case NV3P_PACKET_TYPE_ACK:
		break;

	case NV3P_PACKET_TYPE_NAK:
		if (error)
			*error = nak.error;

		return -EIO;

	default:
		return -EINVAL;
	}

	return 0;
}

static int nv3p_send_ack(struct nv3p *nv3p, uint32_t sequence)
{
	struct nv3p_packet_ack ack;
	uint32_t checksum;
	ssize_t num;

	nv3p_checksum_init(&checksum);

	memset(&ack, 0, sizeof(ack));
	ack.header.version = NV3P_VERSION;
	ack.header.type = NV3P_PACKET_TYPE_ACK;
	ack.header.sequence = sequence;

	nv3p_checksum_update(&checksum, &ack, sizeof(ack));
	ack.checksum = nv3p_checksum_done(&checksum);

	num = nv3p_send(nv3p, &ack, sizeof(ack));
	if (num < 0)
		return num;

	return 0;
}

static int nv3p_send_nak(struct nv3p *nv3p, uint32_t sequence, uint32_t error)
{
	struct nv3p_packet_nak nak;
	uint32_t checksum;
	ssize_t num;

	nv3p_checksum_init(&checksum);

	memset(&nak, 0, sizeof(nak));
	nak.header.version = NV3P_VERSION;
	nak.header.type = NV3P_PACKET_TYPE_NAK;
	nak.header.sequence = sequence;
	nak.error = error;

	nv3p_checksum_update(&checksum, &nak, sizeof(nak));
	nak.checksum = nv3p_checksum_done(&checksum);

	num = nv3p_send(nv3p, &nak, sizeof(nak));
	if (num < 0)
		return num;

	return 0;
}

static int nv3p_send_status(struct nv3p *nv3p, uint32_t sequence,
			    const char *message, uint32_t code,
			    uint32_t flags)
{
	struct nv3p_packet_status status;
	uint32_t checksum;
	ssize_t num;

	nv3p_checksum_init(&checksum);

	memset(&status, 0, sizeof(status));
	status.header.version = NV3P_VERSION;
	status.header.type = NV3P_PACKET_TYPE_COMMAND;
	status.header.sequence = sequence;
	status.length = sizeof(status.status);
	status.command = NV3P_COMMAND_STATUS;
	strncpy(status.status.message, message, NV3P_STRING_MAX);
	status.status.code = code;
	status.status.flags = flags;

	nv3p_checksum_update(&checksum, &status, sizeof(status));
	status.checksum = nv3p_checksum_done(&checksum);

	num = nv3p_send(nv3p, &status, sizeof(status));
	if (num < 0)
		return num;

	return 0;
}

static int nv3p_process_platform_info(struct nv3p *nv3p)
{
	struct nv3p_packet_platform_info info;
	struct nv3p_packet_command command;
	uint32_t sequence, checksum;
	ssize_t num;
	int err;

	err = nv3p_recv_command(nv3p, &command);
	if (err < 0)
		return err;

	sequence = command.header.sequence;

	if (command.command != NV3P_COMMAND_GET_PLATFORM_INFO) {
		err = nv3p_send_nak(nv3p, sequence, NV3P_NAK_BAD_CMD);
		if (err < 0)
			return err;

		return -EIO;
	}

	err = nv3p_send_ack(nv3p, sequence);
	if (err < 0)
		return err;

	memset(&info, 0, sizeof(info));
	info.header.version = NV3P_VERSION;
	info.header.type = NV3P_PACKET_TYPE_DATA;
	info.header.sequence = sequence;
	info.length = sizeof(info.info);

	err = nv3p_prepare_platform_info(nv3p, &info.info);
	if (err < 0)
		return err;

	nv3p_checksum_init(&checksum);
	nv3p_checksum_update(&checksum, &info, sizeof(info));
	info.checksum = nv3p_checksum_done(&checksum);

	num = nv3p_send(nv3p, &info, sizeof(info));
	if (num < 0)
		return num;

	num = nv3p_recv_ack(nv3p, NULL);
	if (num < 0)
		return num;

	err = nv3p_send_status(nv3p, sequence, "OK", NV3P_STATUS_OK, 0);
	if (err < 0)
		return err;

	num = nv3p_recv_ack(nv3p, NULL);
	if (num < 0)
		return num;

	return 0;
}

static ssize_t nv3p_recv_data_chunk(struct nv3p *nv3p, void *buffer,
				    size_t size)
{
	size_t chunk = min(2048, size), received = 0;
	ssize_t num;

	while (received < size) {
		size_t max = min(chunk, size - received);

		num = nv3p_recv(nv3p, buffer + received, max);
		if (num < 0)
			return num;

		received += num;
	}

	return received;
}

static ssize_t nv3p_recv_data(struct nv3p *nv3p, void *buffer, size_t size)
{
	struct nv3p_packet_data packet;
	ssize_t num, received = 0;

	while (received < size) {
		uint32_t sequence, checksum, cksum;

		num = nv3p_recv(nv3p, &packet, sizeof(packet));
		if (num < 0)
			return num;

		sequence = packet.header.sequence;

		nv3p_checksum_init(&checksum);
		nv3p_checksum_update(&checksum, &packet, sizeof(packet));

		num = nv3p_recv_data_chunk(nv3p, buffer + received,
					   packet.length);
		if (num < 0)
			return num;

		nv3p_checksum_update(&checksum, buffer + received, num);
		nv3p_checksum_done(&checksum);

		received += num;

		num = nv3p_recv(nv3p, &cksum, sizeof(cksum));
		if (num < 0)
			return num;

		if (checksum != cksum) {
			uart_printf(debug, "bad checksum: %08x, expected: %08x\n",
				    checksum, cksum);
			return -EIO;
		}

		num = nv3p_send_ack(nv3p, sequence);
		if (num < 0)
			return num;
	}

	return received;
}

static int nv3p_process_bct(struct nv3p *nv3p)
{
	struct nv3p_packet_download_bct command;
	uint32_t sequence;
	ssize_t num;
	size_t size;

	num = nv3p_recv(nv3p, &command, sizeof(command));
	if (num < 0)
		return num;

	sequence = command.header.sequence;

	if (command.command != NV3P_COMMAND_DOWNLOAD_BCT)
		return -EIO;

	size = command.size;

	uart_printf(debug, "downloading BCT (%zu bytes, expected %zu)...\n",
		    size, sizeof(bct));

	num = nv3p_send_ack(nv3p, sequence);
	if (num < 0)
		return num;

	num = nv3p_recv_data(nv3p, &bct, size);
	if (num < 0)
		return num;

	num = nv3p_send_status(nv3p, sequence, "OK", NV3P_STATUS_OK, 0);
	if (num < 0)
		return num;

	num = nv3p_recv_ack(nv3p, NULL);
	if (num < 0)
		return num;

	if (1) {
#ifdef CONFIG_TEGRA210
#define APB_MISC_PP_STRAPPING_OPT_A 0x00
		uint32_t value = readl(TEGRA_APB_MISC_BASE + APB_MISC_PP_STRAPPING_OPT_A);
		unsigned int index = (value >> 4) & 0x3;
#else
		uint32_t value = readl(TEGRA_PMC_BASE + PMC_STRAPPING_OPT_A);
		unsigned int index = (value >> 4) & 0x3;
#endif

		sdram_init(&bct.sdram_params[index]);

		/*
		sdram_test();
		*/
	}

	return 0;
}

#ifdef CONFIG_TEGRA132
static int nv3p_process_mts(struct nv3p *nv3p)
{
	struct nv3p_packet_download_mts command;
	uint32_t sequence;
	ssize_t num;
	size_t size;
	void *load;

	num = nv3p_recv(nv3p, &command, sizeof(command));
	if (num < 0)
		return num;

	sequence = command.header.sequence;

	if (command.command != NV3P_COMMAND_DOWNLOAD_MTS)
		return -EIO;

	load = (void *)command.load;
	size = command.size;

	uart_printf(debug, "downloading MTS (%zu bytes)...\n", size);

	num = nv3p_send_ack(nv3p, sequence);
	if (num < 0)
		return num;

	num = nv3p_recv_data(nv3p, load, size);
	if (num < 0)
		return num;

	num = nv3p_send_status(nv3p, sequence, "OK", NV3P_STATUS_OK, 0);
	if (num < 0)
		return num;

	num = nv3p_recv_ack(nv3p, NULL);
	if (num < 0)
		return num;

	uart_printf(debug, "MTS downloaded to %p\n", load);

	return 0;
}
#endif

static int nv3p_process_bootloader(struct nv3p *nv3p, uint32_t *entry)
{
	struct nv3p_packet_download_bootloader command;
	uint32_t sequence;
	size_t size;
	ssize_t num;
	void *load;

	num = nv3p_recv(nv3p, &command, sizeof(command));
	if (num < 0)
		return num;

	if (command.command != NV3P_COMMAND_DOWNLOAD_BOOTLOADER) {
		uart_printf(debug, "unexpected command: %08x\n", command.command);
		uart_printf(debug, "type: %08x\n", command.header.type);
		return -EIO;
	}

	sequence = command.header.sequence;
	load = (void *)command.load;
	size = command.size;

	uart_printf(debug, "downloading bootloader (%zu bytes)...\n", size);

	num = nv3p_send_ack(nv3p, sequence);
	if (num < 0)
		return num;

	num = nv3p_send_status(nv3p, sequence, "OK", NV3P_STATUS_OK, 0);
	if (num < 0)
		return num;

	num = nv3p_recv_ack(nv3p, NULL);
	if (num < 0)
		return num;

	num = nv3p_recv_data(nv3p, load, size);
	if (num < 0)
		return num;

	num = nv3p_send_status(nv3p, sequence, "OK", NV3P_STATUS_OK, 0);
	if (num < 0)
		return num;

	num = nv3p_recv_ack(nv3p, NULL);
	if (num < 0)
		return num;

	if (entry)
		*entry = command.entry;

	return 0;
}

int nv3p_init(struct nv3p *nv3p, struct usb *usb)
{
	nv3p->usb = usb;

	return 0;
}

static void __naked __maybe_unused execute_bootloader(uint32_t entry)
{
	asm (
		/* disable interrupts */
		"mrs r1, cpsr\n"
		"orr r1, r1, #0xc0\n"
		"msr cpsr, r1\n"

		"bx r0\n"
	);
}

int nv3p_process(struct nv3p *nv3p)
{
	uint32_t entry;
	int err;

	err = nv3p_process_platform_info(nv3p);
	if (err < 0)
		return err;

	err = nv3p_process_bct(nv3p);
	if (err < 0)
		return err;

#ifdef CONFIG_TEGRA132
	err = nv3p_process_mts(nv3p);
	if (err < 0)
		return err;
#endif

	err = nv3p_process_bootloader(nv3p, &entry);
	if (err < 0)
		return err;

	if (start_cpu(entry))
		flow_halt_avp(&flow);

	uart_printf(debug, "executing bootloader at %08x...\n", entry);
	execute_bootloader(entry);

	return 0;
}
