#ifndef AVP_FLOW_H
#define AVP_FLOW_H

enum flow_cluster_id {
	FLOW_CLUSTER_ID_G,
	FLOW_CLUSTER_ID_LP,
};

struct flow {
	unsigned long base;
};

void flow_set_active_cluster(struct flow *flow, enum flow_cluster_id id);
void flow_repair_ram(struct flow *flow);
void flow_halt_avp(struct flow *flow);

extern struct flow flow;

#endif
