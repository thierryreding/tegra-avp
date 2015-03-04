#include <avp/flow.h>
#include <avp/io.h>
#include <avp/iomap.h>

#define FLOW_CTLR_HALT_COP_EVENTS 0x004
#define  FLOW_CTLR_HALT_COP_EVENTS_MODE_STOP (2 << 29)
#define  FLOW_CTLR_HALT_COP_EVENTS_JTAG (1 << 28)

#define FLOW_CTLR_CLUSTER_CONTROL 0x02c
#define  FLOW_CTLR_CLUSTER_CONTROL_ACTIVE(x) ((x) & 0x1)
#define  FLOW_CTLR_CLUSTER_CONTROL_ACTIVE_MASK 0x1

#define FLOW_CTLR_RAM_REPAIR 0x040
#define  FLOW_CTLR_RAM_REPAIR_STS (1 << 1)
#define  FLOW_CTLR_RAM_REPAIR_REQ (1 << 0)

void flow_set_active_cluster(struct flow *flow, enum flow_cluster_id id)
{
	uint32_t value;

	value = readl(flow->base + FLOW_CTLR_CLUSTER_CONTROL);
	value &= ~FLOW_CTLR_CLUSTER_CONTROL_ACTIVE_MASK;
	value |= FLOW_CTLR_CLUSTER_CONTROL_ACTIVE(id);
	writel(value, flow->base + FLOW_CTLR_CLUSTER_CONTROL);
}

void flow_repair_ram(struct flow *flow)
{
	uint32_t value;

	value = readl(flow->base + FLOW_CTLR_RAM_REPAIR);
	value |= FLOW_CTLR_RAM_REPAIR_REQ;
	writel(value, flow->base + FLOW_CTLR_RAM_REPAIR);

	while (true) {
		value = readl(flow->base + FLOW_CTLR_RAM_REPAIR);
		if (value & FLOW_CTLR_RAM_REPAIR_STS)
			break;
	}
}

void flow_halt_avp(struct flow *flow)
{
	uint32_t value = FLOW_CTLR_HALT_COP_EVENTS_MODE_STOP |
			 FLOW_CTLR_HALT_COP_EVENTS_JTAG;

	while (true)
		writel(value, flow->base + FLOW_CTLR_HALT_COP_EVENTS);
}

struct flow flow = {
	.base = TEGRA_FLOW_CTLR_BASE,
};
