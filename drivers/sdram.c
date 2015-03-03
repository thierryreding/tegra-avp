#include <avp/bct.h>
#include <avp/clk-rst.h>
#include <avp/io.h>
#include <avp/iomap.h>
#include <avp/sdram.h>
#include <avp/timer.h>
#include <avp/uart.h>

/*
 * PMC
 */
#define PMC_NO_IOPOWER 0x044
#define  PMC_NO_IOPOWER_MEM_COMP (1 << 16)
#define  PMC_NO_IOPOWER_MEM (1 << 7)

#define PMC_DDR_PWR 0x0e8
#define  PMC_DDR_PWR_VAL_MASK (1 << 0)

#define PMC_VDDP_SEL 0x1cc
#define PMC_DDR_CFG 0x1d0
#define  PMC_DDR_CFG_XM0_RESET_DPDIO_MASK (1 << 13)
#define  PMC_DDR_CFG_XM0_RESET_TRI_MASK (1 << 12)
#define  PMC_DDR_CFG_RESET_SWIZZLE_MASK (0xf << 4)
#define  PMC_DDR_CFG_DDR_SPARE_MASK (1 << 3)
#define  PMC_DDR_CFG_CHANNEL_SWIZZLE_MASK (1 << 2)
#define  PMC_DDR_CFG_IF_MASK (1 << 1)
#define  PMC_DDR_CFG_PKG_MASK (1 << 0)

#define PMC_POR_DPD_CTRL 0x264
#define  PMC_POR_DPD_CTRL_MEM0_HOLD_CKE_LOW_OVR (1 << 31)
#define  PMC_POR_DPD_CTRL_MEM0_ADDR1_CLK_SEL_DPD (1 << 1)
#define  PMC_POR_DPD_CTRL_MEM0_ADDR0_CLK_SEL_DPD (1 << 0)

#define PMC_REG_SHORT 0x2cc
#define PMC_IO_DPD3_REQ 0x45c

/*
 * MC
 */
#define MC_EMEM_CFG 0x050
#define MC_EMEM_ADR_CFG 0x054
#define MC_EMEM_ADR_CFG_DEV0 0x058
#define MC_EMEM_ADR_CFG_DEV1 0x05c

#define MC_EMEM_ARB_CFG 0x090
#define MC_EMEM_ARB_OUTSTANDING_REQ 0x094
#define MC_EMEM_ARB_TIMING_RCD 0x098
#define MC_EMEM_ARB_TIMING_RP 0x09c
#define MC_EMEM_ARB_TIMING_RC 0x0a0
#define MC_EMEM_ARB_TIMING_RAS 0x0a4
#define MC_EMEM_ARB_TIMING_FAW 0x0a8
#define MC_EMEM_ARB_TIMING_RRD 0x0ac
#define MC_EMEM_ARB_TIMING_RAP2PRE 0x0b0
#define MC_EMEM_ARB_TIMING_WAP2PRE 0x0b4
#define MC_EMEM_ARB_TIMING_R2R 0x0b8
#define MC_EMEM_ARB_TIMING_W2W 0x0bc
#define MC_EMEM_ARB_TIMING_R2W 0x0c0
#define MC_EMEM_ARB_TIMING_W2R 0x0c4

#define MC_EMEM_ARB_DA_TURNS 0x0d0
#define MC_EMEM_ARB_DA_COVERS 0x0d4
#define MC_EMEM_ARB_MISC0 0x0d8
#define MC_EMEM_ARB_MISC1 0x0dc
#define MC_EMEM_ARB_RING1_THROTTLE 0x0e0
#define MC_EMEM_ARB_OVERRIDE0 0x0e8
#define MC_EMEM_ARB_RSV 0x0ec

#define MC_CLKEN_OVERRIDE 0x0f4

#define MC_TIMING_CONTROL 0x0fc
#define  MC_TIMING_CONTROL_UPDATE (1 << 0)

#define MC_STAT_CONTROL 0x100

#define MC_DIS_EXTRA_SNAP_LEVELS 0x2ac

#define MC_VIDEO_PROTECT_VPR_OVERRIDE0 0x418
#define MC_VIDEO_PROTECT_VPR_OVERRIDE1 0x590
#define MC_DISPLAY_SNAP_RING 0x608
#define MC_VIDEO_PROTECT_BOM 0x648
#define MC_VIDEO_PROTECT_SIZE_MB 0x64c
#define MC_VIDEO_PROTECT_CTRL 0x650
#define MC_EMEM_CFG_ACCESS_CTRL 0x664
#define MC_SEC_CARVEOUT_BOM 0x670
#define MC_SEC_CARVEOUT_SIZE_MB 0x674
#define MC_SEC_CARVEOUT_CTRL 0x678

#define MC_EMEM_ARB_OVERRIDE1 0x968
#define MC_VIDEO_PROTECT_BOM_ADR_HI 0x978

#define MC_VIDEO_PROTECT_GPU_OVERRIDE0 0x984
#define MC_VIDEO_PROTECT_GPU_OVERRIDE1 0x988

#define MC_MTS_CARVEOUT_BOM 0x9a0
#define MC_MTS_CARVEOUT_SIZE_MB 0x9a4
#define MC_MTS_CARVEOUT_ADR_HI 0x9a8
#define MC_MTS_CARVEOUT_CTRL 0x9ac

#define MC_EMEM_BANK_SWIZZLE_CFG0 0x9c0
#define MC_EMEM_BANK_SWIZZLE_CFG1 0x9c4
#define MC_EMEM_BANK_SWIZZLE_CFG2 0x9c8
#define MC_EMEM_BANK_SWIZZLE_CFG3 0x9cc
#define MC_SEC_CARVEOUT_ADR_HI 0x9d4

/*
 * EMC
 */
#define EMC_DBG 0x008
#define EMC_CFG 0x00c
#define   EMC_CFG_EMC2PMACRO_CFG_BYPASS_DATAPIPE2 (1 << 3)
#define   EMC_CFG_EMC2PMACRO_CFG_BYPASS_DATAPIPE1 (1 << 2)
#define   EMC_CFG_EMC2PMACRO_CFG_BYPASS_ADDRPIPE  (1 << 1)
#define EMC_ADR_CFG 0x010

#define EMC_REFCTRL 0x020
#define  EMC_REFCTRL_REF_VALID (1 << 31)
#define  EMC_REFCTRL_DEVICE_REFRESH_DISABLE(x) ((x) & 0x3)
#define EMC_PIN 0x024
#define  EMC_PIN_RST (1 << 8)
#define  EMC_PIN_DQM (1 << 4)
#define  EMC_PIN_CKE (1 << 0)
#define EMC_TIMING_CONTROL 0x028
#define  EMC_TIMING_CONTROL_UPDATE (1 << 0)
#define EMC_RC 0x02c
#define EMC_RFC 0x030
#define EMC_RAS 0x034
#define EMC_RP 0x038
#define EMC_R2W 0x03c
#define EMC_W2R 0x040
#define EMC_R2P 0x044
#define EMC_W2P 0x048
#define EMC_RD_RCD 0x04c
#define EMC_WR_RCD 0x050
#define EMC_RRD 0x054
#define EMC_REXT 0x058
#define EMC_WDV 0x05c
#define EMC_QUSE 0x060
#define EMC_QRST 0x064
#define EMC_QSAFE 0x068
#define EMC_RDV 0x06c
#define EMC_REFRESH 0x070
#define EMC_BURST_REFRESH_NUM 0x074
#define EMC_PDEX2WR 0x078
#define EMC_PDEX2RD 0x07c
#define EMC_PCHG2PDEN 0x080
#define EMC_ACT2PDEN 0x084
#define EMC_AR2PDEN 0x088
#define EMC_RW2PDEN 0x08c
#define EMC_TXSR 0x090
#define EMC_TCKE 0x094
#define EMC_TFAW 0x098
#define EMC_TRPAB 0x09c
#define EMC_TCLKSTABLE 0x0a0
#define EMC_TCLKSTOP 0x0a4
#define EMC_TREFBW 0x0a8
#define EMC_ODT_WRITE 0x0b0
#define EMC_ODT_READ 0x0b4
#define EMC_WEXT 0x0b8
#define EMC_CTT 0x0bc
#define EMC_RFC_SLR 0x0c0
#define EMC_MRS_WAIT_CNT2 0x0c4
#define EMC_MRS_WAIT_CNT 0x0c8
#define EMC_MRS 0x0cc
#define EMC_EMRS 0x0d0
#define EMC_REF 0x0d4
#define  EMC_REF_DEV_SELECT(x) (((x) & 0x3) << 30)
#define  EMC_REF_NUM(x) (((x) & 0xff) << 8)
#define  EMC_REF_NORMAL (1 << 1)
#define  EMC_REF_CMD (1 << 0)
#define EMC_NOP 0x0dc
#define  EMC_NOP_DEV_SELECT(x) (((x) & 0x3) << 30)
#define  EMC_NOP_CMD (1 << 0)
#define EMC_CMDQ 0x0f0
#define EMC_MC2EMCQ 0x0f4
#define EMC_XM2DQSPADCTRL3 0x0f8

#define EMC_FBIO_SPARE 0x100
#define EMC_FBIO_CFG5 0x104
#define EMC_FBIO_CFG6 0x114
#define EMC_CFG_RSV 0x120
#define EMC_ACPD_CONTROL 0x124
#define EMC_EMRS2 0x12c
#define EMC_EMRS3 0x130

#define EMC_CLKEN_OVERRIDE 0x140
#define EMC_R2R 0x144
#define EMC_W2W 0x148
#define EMC_EINPUT 0x14c
#define EMC_EINPUT_DURATION 0x150
#define EMC_PUTERM_EXTRA 0x154
#define EMC_TCKESR 0x158
#define EMC_TPD 0x15c

#define EMC_AUTO_CAL_CONFIG 0x2a4
#define EMC_AUTO_CAL_INTERVAL 0x2a8

#define EMC_CFG2 0x2b8
#define EMC_CFG_DIG_DLL 0x2bc
#define EMC_CFG_DIG_DLL_PERIOD 0x2c0
#define EMC_RDV_MASK 0x2cc
#define EMC_WDV_MASK 0x2d0
#define EMC_CTT_DURATION 0x2d8
#define EMC_CTT_TERM_CTRL 0x2dc

#define EMC_ZCAL_INTERVAL 0x2e0
#define EMC_ZCAL_WAIT_CNT 0x2e4
#define EMC_ZCAL_MRW_CMD 0x2e8
#define EMC_ZQ_CAL 0x2ec
#define EMC_XM2CMDPADCTRL 0x2f0
#define EMC_XM2CMDPADCTRL2 0x2f4
#define EMC_XM2DQSPADCTRL 0x2f8
#define EMC_XM2DQSPADCTRL2 0x2fc
#define EMC_XM2DQPADCTRL  0x300
#define EMC_XM2DQPADCTRL2 0x304
#define EMC_XM2CLKPADCTRL 0x308
#define EMC_XM2COMPPADCTRL 0x30c
#define EMC_XM2VTTGENPADCTRL 0x310
#define EMC_XM2VTTGENPADCTRL2 0x314
#define EMC_XM2VTTGENPADCTRL3 0x318
#define EMC_XM2DQSPADCTRL4 0x320
#define EMC_DLL_XFORM_DQS0 0x328
#define EMC_DLL_XFORM_DQS1 0x32c
#define EMC_DLL_XFORM_DQS2 0x330
#define EMC_DLL_XFORM_DQS3 0x334
#define EMC_DLL_XFORM_DQS4 0x338
#define EMC_DLL_XFORM_DQS5 0x33c
#define EMC_DLL_XFORM_DQS6 0x340
#define EMC_DLL_XFORM_DQS7 0x344
#define EMC_DLL_XFORM_QUSE0 0x348
#define EMC_DLL_XFORM_QUSE1 0x34c
#define EMC_DLL_XFORM_QUSE2 0x350
#define EMC_DLL_XFORM_QUSE3 0x354
#define EMC_DLL_XFORM_QUSE4 0x358
#define EMC_DLL_XFORM_QUSE5 0x35c
#define EMC_DLL_XFORM_QUSE6 0x360
#define EMC_DLL_XFORM_QUSE7 0x364
#define EMC_DLL_XFORM_DQ0 0x368
#define EMC_DLL_XFORM_DQ1 0x36c
#define EMC_DLL_XFORM_DQ2 0x370
#define EMC_DLL_XFORM_DQ3 0x374

#define EMC_DLI_TRIM_TXDQS0 0x3a8
#define EMC_DLI_TRIM_TXDQS1 0x3ac
#define EMC_DLI_TRIM_TXDQS2 0x3b0
#define EMC_DLI_TRIM_TXDQS3 0x3b4
#define EMC_DLI_TRIM_TXDQS4 0x3b8
#define EMC_DLI_TRIM_TXDQS5 0x3bc
#define EMC_DLI_TRIM_TXDQS6 0x3c0
#define EMC_DLI_TRIM_TXDQS7 0x3c4

#define EMC_SEL_DPD_CTRL 0x3d8
#define EMC_PRE_REFRESH_REQ_CNT 0x3dc

#define EMC_DYN_SELF_REF_CONTROL 0x3e0
#define EMC_TXSRDLL 0x3e4

#define EMC_CDB_CNTL1 0x3f4
#define EMC_CDB_CNTL2 0x3f8
#define EMC_XM2CLKPADCTRL2 0x3fc

#define EMC_SWIZZLE_RANK0_BYTE_CFG 0x400
#define EMC_SWIZZLE_RANK0_BYTE0 0x404
#define EMC_SWIZZLE_RANK0_BYTE1 0x408
#define EMC_SWIZZLE_RANK0_BYTE2 0x40c
#define EMC_SWIZZLE_RANK0_BYTE3 0x410
#define EMC_SWIZZLE_RANK1_BYTE_CFG 0x414
#define EMC_SWIZZLE_RANK1_BYTE0 0x418
#define EMC_SWIZZLE_RANK1_BYTE1 0x41c
#define EMC_SWIZZLE_RANK1_BYTE2 0x420
#define EMC_SWIZZLE_RANK1_BYTE3 0x424

#define EMC_CA_TRAINING_TIMING_CNTL1 0x434
#define EMC_CA_TRAINING_TIMING_CNTL2 0x438
#define EMC_AUTO_CAL_CONFIG2 0x458
#define EMC_AUTO_CAL_CONFIG3 0x45c
#define EMC_XM2CMDPADCTRL3 0x464
#define EMC_IBDLY 0x468
#define EMC_DLL_XFORM_ADDR0 0x46c
#define EMC_DLL_XFORM_ADDR1 0x470
#define EMC_DLL_XFORM_ADDR2 0x474
#define EMC_DSR_VTTGEN_DRV 0x47c
#define EMC_TXDSRVTTGEN 0x480
#define EMC_XM2CMDPADCTRL4 0x484
#define EMC_XM2CMDPADCTRL5 0x488

#define EMC_DLL_XFORM_DQS8 0x4a0
#define EMC_DLL_XFORM_DQS9 0x4a4
#define EMC_DLL_XFORM_DQS10 0x4a8
#define EMC_DLL_XFORM_DQS11 0x4ac
#define EMC_DLL_XFORM_DQS12 0x4b0
#define EMC_DLL_XFORM_DQS13 0x4b4
#define EMC_DLL_XFORM_DQS14 0x4b8
#define EMC_DLL_XFORM_DQS15 0x4bc

#define EMC_DLL_XFORM_QUSE8 0x4c0
#define EMC_DLL_XFORM_QUSE9 0x4c4
#define EMC_DLL_XFORM_QUSE10 0x4c8
#define EMC_DLL_XFORM_QUSE11 0x4cc
#define EMC_DLL_XFORM_QUSE12 0x4d0
#define EMC_DLL_XFORM_QUSE13 0x4d4
#define EMC_DLL_XFORM_QUSE14 0x4d8
#define EMC_DLL_XFORM_QUSE15 0x4dc

#define EMC_DLL_XFORM_DQ4 0x4e0
#define EMC_DLL_XFORM_DQ5 0x4e4
#define EMC_DLL_XFORM_DQ6 0x4e8
#define EMC_DLL_XFORM_DQ7 0x4ec

#define EMC_DLI_TRIM_TXDQS8 0x520
#define EMC_DLI_TRIM_TXDQS9 0x524
#define EMC_DLI_TRIM_TXDQS10 0x528
#define EMC_DLI_TRIM_TXDQS11 0x52c
#define EMC_DLI_TRIM_TXDQS12 0x530
#define EMC_DLI_TRIM_TXDQS13 0x534
#define EMC_DLI_TRIM_TXDQS14 0x538
#define EMC_DLI_TRIM_TXDQS15 0x53c
#define EMC_CDB_CNTL3 0x540
#define EMC_XM2DQSPADCTRL5 0x544
#define EMC_XM2DQSPADCTRL6 0x548
#define EMC_XM2DQPADCTRL3 0x54c
#define EMC_DLL_XFORM_ADDR3 0x550
#define EMC_DLL_XFORM_ADDR4 0x554
#define EMC_DLL_XFORM_ADDR5 0x558

#define EMC_CFG_PIPE 0x560
#define EMC_QPOP 0x564
#define EMC_QUSE_WIDTH 0x568

#define EMC_PUTERM_WIDTH 0x56c
#define EMC_BGBIAS_CTL 0x570
#define EMC_PUTERM_ADJ 0x574

/*
 * ARB
 */
#define ARB_XBAR_CTRL 0x0e0
#define  ARB_XBAR_CTRL_MEM_INIT_DONE (1 << 16)

void sdram_init(struct bct_sdram_params *params)
{
	unsigned int pmc = TEGRA_PMC_BASE;
	unsigned int emc = TEGRA_EMC_BASE;
	unsigned int arb = TEGRA_ARB_BASE;
	unsigned int mc = TEGRA_MC_BASE;
	uint32_t value;

	clock_pllm_init(&clk_rst, params);

	clk_periph_enable(&clk_emc);
	clk_periph_enable(&clk_mc);

	reset_deassert(&rst_emc);
	reset_deassert(&rst_mc);

	udelay(5);

	writel(params->pmc_vddp_sel, pmc + PMC_VDDP_SEL);
	udelay(params->pmc_vddp_sel_wait);

	value = readl(pmc + PMC_DDR_PWR);
	value &= ~PMC_DDR_PWR_VAL_MASK;
	value |= params->pmc_ddr_pwr & PMC_DDR_PWR_VAL_MASK;
	writel(value, pmc + PMC_DDR_PWR);

	value = readl(pmc + PMC_DDR_CFG);
	value &= ~PMC_DDR_CFG_PKG_MASK;
	value |= params->pmc_ddr_cfg & PMC_DDR_CFG_PKG_MASK;
	value &= ~PMC_DDR_CFG_IF_MASK;
	value |= params->pmc_ddr_cfg & PMC_DDR_CFG_IF_MASK;
	/*
	value &= ~PMC_DDR_CFG_CHANNEL_SWIZZLE_MASK;
	value |= params->pmc_ddr_cfg & PMC_DDR_CFG_CHANNEL_SWIZZLE_MASK;
	value &= ~PMC_DDR_CFG_DDR_SPARE_MASK;
	value |= params->pmc_ddr_cfg & PMC_DDR_CFG_DDR_SPARE_MASK;
	value &= ~PMC_DDR_CFG_RESET_SWIZZLE_MASK;
	value |= params->pmc_ddr_cfg & PMC_DDR_CFG_RESET_SWIZZLE_MASK;
	*/
	value &= ~PMC_DDR_CFG_XM0_RESET_TRI_MASK;
	value |= params->pmc_ddr_cfg & PMC_DDR_CFG_XM0_RESET_TRI_MASK;
	value &= ~PMC_DDR_CFG_XM0_RESET_DPDIO_MASK;
	value |= params->pmc_ddr_cfg & PMC_DDR_CFG_XM0_RESET_DPDIO_MASK;
	writel(value, pmc + PMC_DDR_CFG);

	value = readl(pmc + PMC_NO_IOPOWER);
	value &= ~PMC_NO_IOPOWER_MEM;
	value |= params->pmc_no_iopower & PMC_NO_IOPOWER_MEM;
	value &= ~PMC_NO_IOPOWER_MEM_COMP;
	value |= params->pmc_no_iopower & PMC_NO_IOPOWER_MEM_COMP;
	writel(value, pmc + PMC_NO_IOPOWER);

	writel(params->pmc_reg_short, pmc + PMC_REG_SHORT);

	if (params->emc_bct_spare0) {
		uart_printf(debug, "  %08x < %08x\n", params->emc_bct_spare0, params->emc_bct_spare1);
		writel(params->emc_bct_spare1, params->emc_bct_spare0);
	}

	/* XXX set clock source */
	clk_periph_enable(&clk_emc);
	reset_deassert(&rst_emc);

	clk_periph_enable(&clk_mc);
	reset_deassert(&rst_mc);

	udelay(5);

	if (params->emc_bct_spare2) {
		uart_printf(debug, "  %08x < %08x\n", params->emc_bct_spare2, params->emc_bct_spare3);
		writel(params->emc_bct_spare3, params->emc_bct_spare2);
	}

	value = readl(pmc + PMC_POR_DPD_CTRL);
	value &= ~(PMC_POR_DPD_CTRL_MEM0_ADDR1_CLK_SEL_DPD |
		   PMC_POR_DPD_CTRL_MEM0_ADDR0_CLK_SEL_DPD);
	writel(value, pmc + PMC_POR_DPD_CTRL);

	udelay(10);

	writel(params->emc_swizzle_rank0_byte_cfg, emc + EMC_SWIZZLE_RANK0_BYTE_CFG);
	writel(params->emc_swizzle_rank0_byte0, emc + EMC_SWIZZLE_RANK0_BYTE0);
	writel(params->emc_swizzle_rank0_byte1, emc + EMC_SWIZZLE_RANK0_BYTE1);
	writel(params->emc_swizzle_rank0_byte2, emc + EMC_SWIZZLE_RANK0_BYTE2);
	writel(params->emc_swizzle_rank0_byte3, emc + EMC_SWIZZLE_RANK0_BYTE3);

	writel(params->emc_swizzle_rank1_byte_cfg, emc + EMC_SWIZZLE_RANK1_BYTE_CFG);
	writel(params->emc_swizzle_rank1_byte0, emc + EMC_SWIZZLE_RANK1_BYTE0);
	writel(params->emc_swizzle_rank1_byte1, emc + EMC_SWIZZLE_RANK1_BYTE1);
	writel(params->emc_swizzle_rank1_byte2, emc + EMC_SWIZZLE_RANK1_BYTE2);
	writel(params->emc_swizzle_rank1_byte3, emc + EMC_SWIZZLE_RANK1_BYTE3);

	writel(params->emc_xm2cmdpadctrl, emc + EMC_XM2CMDPADCTRL);
	writel(params->emc_xm2cmdpadctrl2, emc + EMC_XM2CMDPADCTRL2);
	writel(params->emc_xm2cmdpadctrl3, emc + EMC_XM2CMDPADCTRL3);
	writel(params->emc_xm2cmdpadctrl4, emc + EMC_XM2CMDPADCTRL4);
	writel(params->emc_xm2cmdpadctrl5, emc + EMC_XM2CMDPADCTRL5);

	writel(params->emc_xm2dqspadctrl, emc + EMC_XM2DQSPADCTRL);
	writel(params->emc_xm2dqspadctrl2, emc + EMC_XM2DQSPADCTRL2);
	writel(params->emc_xm2dqspadctrl3, emc + EMC_XM2DQSPADCTRL3);
	writel(params->emc_xm2dqspadctrl4, emc + EMC_XM2DQSPADCTRL4);
	writel(params->emc_xm2dqspadctrl5, emc + EMC_XM2DQSPADCTRL5);
	writel(params->emc_xm2dqspadctrl6, emc + EMC_XM2DQSPADCTRL6);

	writel(params->emc_xm2dqpadctrl, emc + EMC_XM2DQPADCTRL);
	writel(params->emc_xm2dqpadctrl2, emc + EMC_XM2DQPADCTRL2);
	writel(params->emc_xm2dqpadctrl3, emc + EMC_XM2DQPADCTRL3);

	writel(params->emc_xm2clkpadctrl, emc + EMC_XM2CLKPADCTRL);
	writel(params->emc_xm2clkpadctrl2, emc + EMC_XM2CLKPADCTRL2);

	writel(params->emc_xm2comppadctrl, emc + EMC_XM2COMPPADCTRL);

	writel(params->emc_xm2vttgenpadctrl, emc + EMC_XM2VTTGENPADCTRL);
	writel(params->emc_xm2vttgenpadctrl2, emc + EMC_XM2VTTGENPADCTRL2);
	writel(params->emc_xm2vttgenpadctrl3, emc + EMC_XM2VTTGENPADCTRL3);
	writel(params->emc_ctt_term_ctrl, emc + EMC_CTT_TERM_CTRL);

	if (params->emc_bct_spare4) {
		uart_printf(debug, "  %08x < %08x\n", params->emc_bct_spare4, params->emc_bct_spare5);
		writel(params->emc_bct_spare5, params->emc_bct_spare4);
	}

	writel(EMC_TIMING_CONTROL_UPDATE, emc + EMC_TIMING_CONTROL);

	writel(params->mc_display_snap_ring, mc + MC_DISPLAY_SNAP_RING);
	writel(params->mc_video_protect_bom, mc + MC_VIDEO_PROTECT_BOM);
	writel(params->mc_video_protect_bom_adr_hi, mc + MC_VIDEO_PROTECT_BOM_ADR_HI);
	writel(params->mc_video_protect_size_mb, mc + MC_VIDEO_PROTECT_SIZE_MB);
	writel(params->mc_video_protect_vpr_override0, mc + MC_VIDEO_PROTECT_VPR_OVERRIDE0);
	writel(params->mc_video_protect_vpr_override1, mc + MC_VIDEO_PROTECT_VPR_OVERRIDE1);
	writel(params->mc_video_protect_gpu_override0, mc + MC_VIDEO_PROTECT_GPU_OVERRIDE0);
	writel(params->mc_video_protect_gpu_override1, mc + MC_VIDEO_PROTECT_GPU_OVERRIDE1);

	writel(params->mc_emem_adr_cfg, mc + MC_EMEM_ADR_CFG);
	writel(params->mc_emem_adr_cfg_dev0, mc + MC_EMEM_ADR_CFG_DEV0);
	writel(params->mc_emem_adr_cfg_dev1, mc + MC_EMEM_ADR_CFG_DEV1);

	writel(params->mc_emem_bank_swizzle_cfg0, mc + MC_EMEM_BANK_SWIZZLE_CFG0);
	writel(params->mc_emem_bank_swizzle_cfg1, mc + MC_EMEM_BANK_SWIZZLE_CFG1);
	writel(params->mc_emem_bank_swizzle_cfg2, mc + MC_EMEM_BANK_SWIZZLE_CFG2);
	writel(params->mc_emem_bank_swizzle_cfg3, mc + MC_EMEM_BANK_SWIZZLE_CFG3);
	writel(params->mc_emem_cfg, mc + MC_EMEM_CFG);

	writel(params->mc_sec_carveout_bom, mc + MC_SEC_CARVEOUT_BOM);
	writel(params->mc_sec_carveout_adr_hi, mc + MC_SEC_CARVEOUT_ADR_HI);
	writel(params->mc_sec_carveout_size_mb, mc + MC_SEC_CARVEOUT_SIZE_MB);

	writel(params->mc_mts_carveout_bom, mc + MC_MTS_CARVEOUT_BOM);
	writel(params->mc_mts_carveout_adr_hi, mc + MC_MTS_CARVEOUT_ADR_HI);
	writel(params->mc_mts_carveout_size_mb, mc + MC_MTS_CARVEOUT_SIZE_MB);

	writel(params->mc_emem_arb_cfg, mc + MC_EMEM_ARB_CFG);
	writel(params->mc_emem_arb_outstanding_req, mc + MC_EMEM_ARB_OUTSTANDING_REQ);
	writel(params->mc_emem_arb_timing_rcd, mc + MC_EMEM_ARB_TIMING_RCD);
	writel(params->mc_emem_arb_timing_rp, mc + MC_EMEM_ARB_TIMING_RP);
	writel(params->mc_emem_arb_timing_rc, mc + MC_EMEM_ARB_TIMING_RC);
	writel(params->mc_emem_arb_timing_ras, mc + MC_EMEM_ARB_TIMING_RAS);
	writel(params->mc_emem_arb_timing_faw, mc + MC_EMEM_ARB_TIMING_FAW);
	writel(params->mc_emem_arb_timing_rrd, mc + MC_EMEM_ARB_TIMING_RRD);
	writel(params->mc_emem_arb_timing_rap2pre, mc + MC_EMEM_ARB_TIMING_RAP2PRE);
	writel(params->mc_emem_arb_timing_wap2pre, mc + MC_EMEM_ARB_TIMING_WAP2PRE);
	writel(params->mc_emem_arb_timing_r2r, mc + MC_EMEM_ARB_TIMING_R2R);
	writel(params->mc_emem_arb_timing_w2w, mc + MC_EMEM_ARB_TIMING_W2W);
	writel(params->mc_emem_arb_timing_r2w, mc + MC_EMEM_ARB_TIMING_R2W);
	writel(params->mc_emem_arb_timing_w2r, mc + MC_EMEM_ARB_TIMING_W2R);
	writel(params->mc_emem_arb_da_turns, mc + MC_EMEM_ARB_DA_TURNS);
	writel(params->mc_emem_arb_da_covers, mc + MC_EMEM_ARB_DA_COVERS);
	writel(params->mc_emem_arb_misc0, mc + MC_EMEM_ARB_MISC0);
	writel(params->mc_emem_arb_misc1, mc + MC_EMEM_ARB_MISC1);
	writel(params->mc_emem_arb_ring1_throttle, mc + MC_EMEM_ARB_RING1_THROTTLE);
	writel(params->mc_emem_arb_override0, mc + MC_EMEM_ARB_OVERRIDE0);
	writel(params->mc_emem_arb_override1, mc + MC_EMEM_ARB_OVERRIDE1);
	writel(params->mc_emem_arb_rsv, mc + MC_EMEM_ARB_RSV);
	writel(params->mc_dis_extra_snap_levels, mc + MC_DIS_EXTRA_SNAP_LEVELS);
	writel(MC_TIMING_CONTROL_UPDATE, mc + MC_TIMING_CONTROL);

	writel(params->mc_clken_override, mc + MC_CLKEN_OVERRIDE);
	writel(params->mc_stat_control, mc + MC_STAT_CONTROL);

	writel(params->emc_adr_cfg, emc + EMC_ADR_CFG);
	writel(params->emc_clken_override, emc + EMC_CLKEN_OVERRIDE);

	writel(params->emc_auto_cal_interval, emc + EMC_AUTO_CAL_INTERVAL);
	writel(params->emc_auto_cal_config2, emc + EMC_AUTO_CAL_CONFIG2);
	writel(params->emc_auto_cal_config3, emc + EMC_AUTO_CAL_CONFIG3);
	writel(params->emc_auto_cal_config, emc + EMC_AUTO_CAL_CONFIG);
	udelay(params->emc_auto_cal_wait);

	if (params->emc_bct_spare6) {
		uart_printf(debug, "  %08x < %08x\n", params->emc_bct_spare6, params->emc_bct_spare7);
		writel(params->emc_bct_spare7, params->emc_bct_spare6);
	}

	writel(params->emc_cfg2, emc + EMC_CFG2);
	writel(params->emc_cfg_pipe, emc + EMC_CFG_PIPE);
	writel(params->emc_dbg, emc + EMC_DBG);
	writel(params->emc_cmdq, emc + EMC_CMDQ);
	writel(params->emc_mc2emcq, emc + EMC_MC2EMCQ);
	writel(params->emc_mrs_wait_cnt, emc + EMC_MRS_WAIT_CNT);
	writel(params->emc_mrs_wait_cnt2, emc + EMC_MRS_WAIT_CNT2);
	writel(params->emc_fbio_cfg5, emc + EMC_FBIO_CFG5);
	writel(params->emc_rc, emc + EMC_RC);
	writel(params->emc_rfc, emc + EMC_RFC);
	writel(params->emc_rfc_slr, emc + EMC_RFC_SLR);
	writel(params->emc_ras, emc + EMC_RAS);
	writel(params->emc_rp, emc + EMC_RP);
	writel(params->emc_r2r, emc + EMC_R2R);
	writel(params->emc_w2w, emc + EMC_W2W);
	writel(params->emc_r2w, emc + EMC_R2W);
	writel(params->emc_w2r, emc + EMC_W2R);
	writel(params->emc_r2p, emc + EMC_R2P);
	writel(params->emc_w2p, emc + EMC_W2P);
	writel(params->emc_rd_rcd, emc + EMC_RD_RCD);
	writel(params->emc_wr_rcd, emc + EMC_WR_RCD);
	writel(params->emc_rrd, emc + EMC_RRD);
	writel(params->emc_rext, emc + EMC_REXT);
	writel(params->emc_wext, emc + EMC_WEXT);
	writel(params->emc_wdv, emc + EMC_WDV);
	writel(params->emc_wdv_mask, emc + EMC_WDV_MASK);
	writel(params->emc_quse, emc + EMC_QUSE);
	writel(params->emc_quse_width, emc + EMC_QUSE_WIDTH);
	writel(params->emc_ibdly, emc + EMC_IBDLY);
	writel(params->emc_einput, emc + EMC_EINPUT);
	writel(params->emc_einput_duration, emc + EMC_EINPUT_DURATION);
	writel(params->emc_puterm_extra, emc + EMC_PUTERM_EXTRA);
	writel(params->emc_puterm_width, emc + EMC_PUTERM_WIDTH);
	writel(params->emc_puterm_adj, emc + EMC_PUTERM_ADJ);
	writel(params->emc_cdb_cntl1, emc + EMC_CDB_CNTL1);
	writel(params->emc_cdb_cntl2, emc + EMC_CDB_CNTL2);
	writel(params->emc_cdb_cntl3, emc + EMC_CDB_CNTL3);
	writel(params->emc_qrst, emc + EMC_QRST);
	writel(params->emc_qsafe, emc + EMC_QSAFE);
	writel(params->emc_rdv, emc + EMC_RDV);
	writel(params->emc_rdv_mask, emc + EMC_RDV_MASK);
	writel(params->emc_qpop, emc + EMC_QPOP);
	writel(params->emc_ctt, emc + EMC_CTT);
	writel(params->emc_ctt_duration, emc + EMC_CTT_DURATION);
	writel(params->emc_refresh, emc + EMC_REFRESH);
	writel(params->emc_burst_refresh_num, emc + EMC_BURST_REFRESH_NUM);
	writel(params->emc_pre_refresh_req_cnt, emc + EMC_PRE_REFRESH_REQ_CNT);
	writel(params->emc_pdex2wr, emc + EMC_PDEX2WR);
	writel(params->emc_pdex2rd, emc + EMC_PDEX2RD);
	writel(params->emc_pchg2pden, emc + EMC_PCHG2PDEN);
	writel(params->emc_act2pden, emc + EMC_ACT2PDEN);
	writel(params->emc_ar2pden, emc + EMC_AR2PDEN);
	writel(params->emc_rw2pden, emc + EMC_RW2PDEN);
	writel(params->emc_txsr, emc + EMC_TXSR);
	writel(params->emc_txsrdll, emc + EMC_TXSRDLL);
	writel(params->emc_tcke, emc + EMC_TCKE);
	writel(params->emc_tckesr, emc + EMC_TCKESR);
	writel(params->emc_tpd, emc + EMC_TPD);
	writel(params->emc_tfaw, emc + EMC_TFAW);
	writel(params->emc_trpab, emc + EMC_TRPAB);
	writel(params->emc_tclkstable, emc + EMC_TCLKSTABLE);
	writel(params->emc_tclkstop, emc + EMC_TCLKSTOP);
	writel(params->emc_trefbw, emc + EMC_TREFBW);
	writel(params->emc_odt_write, emc + EMC_ODT_WRITE);
	writel(params->emc_odt_read, emc + EMC_ODT_READ);
	writel(params->emc_fbio_cfg6, emc + EMC_FBIO_CFG6);
	writel(params->emc_cfg_dig_dll, emc + EMC_CFG_DIG_DLL);
	writel(params->emc_cfg_dig_dll_period, emc + EMC_CFG_DIG_DLL_PERIOD);
	writel(params->emc_fbio_spare & 0xfffffffd, emc + EMC_FBIO_SPARE);

	writel(params->emc_cfg_rsv, emc + EMC_CFG_RSV);
	writel(params->emc_dll_xform_dqs0, emc + EMC_DLL_XFORM_DQS0);
	writel(params->emc_dll_xform_dqs1, emc + EMC_DLL_XFORM_DQS1);
	writel(params->emc_dll_xform_dqs2, emc + EMC_DLL_XFORM_DQS2);
	writel(params->emc_dll_xform_dqs3, emc + EMC_DLL_XFORM_DQS3);
	writel(params->emc_dll_xform_dqs4, emc + EMC_DLL_XFORM_DQS4);
	writel(params->emc_dll_xform_dqs5, emc + EMC_DLL_XFORM_DQS5);
	writel(params->emc_dll_xform_dqs6, emc + EMC_DLL_XFORM_DQS6);
	writel(params->emc_dll_xform_dqs7, emc + EMC_DLL_XFORM_DQS7);
	writel(params->emc_dll_xform_dqs8, emc + EMC_DLL_XFORM_DQS8);
	writel(params->emc_dll_xform_dqs9, emc + EMC_DLL_XFORM_DQS9);
	writel(params->emc_dll_xform_dqs10, emc + EMC_DLL_XFORM_DQS10);
	writel(params->emc_dll_xform_dqs11, emc + EMC_DLL_XFORM_DQS11);
	writel(params->emc_dll_xform_dqs12, emc + EMC_DLL_XFORM_DQS12);
	writel(params->emc_dll_xform_dqs13, emc + EMC_DLL_XFORM_DQS13);
	writel(params->emc_dll_xform_dqs14, emc + EMC_DLL_XFORM_DQS14);
	writel(params->emc_dll_xform_dqs15, emc + EMC_DLL_XFORM_DQS15);
	writel(params->emc_dll_xform_quse0, emc + EMC_DLL_XFORM_QUSE0);
	writel(params->emc_dll_xform_quse1, emc + EMC_DLL_XFORM_QUSE1);
	writel(params->emc_dll_xform_quse2, emc + EMC_DLL_XFORM_QUSE2);
	writel(params->emc_dll_xform_quse3, emc + EMC_DLL_XFORM_QUSE3);
	writel(params->emc_dll_xform_quse4, emc + EMC_DLL_XFORM_QUSE4);
	writel(params->emc_dll_xform_quse5, emc + EMC_DLL_XFORM_QUSE5);
	writel(params->emc_dll_xform_quse6, emc + EMC_DLL_XFORM_QUSE6);
	writel(params->emc_dll_xform_quse7, emc + EMC_DLL_XFORM_QUSE7);
	writel(params->emc_dll_xform_quse8, emc + EMC_DLL_XFORM_QUSE8);
	writel(params->emc_dll_xform_quse9, emc + EMC_DLL_XFORM_QUSE9);
	writel(params->emc_dll_xform_quse10, emc + EMC_DLL_XFORM_QUSE10);
	writel(params->emc_dll_xform_quse11, emc + EMC_DLL_XFORM_QUSE11);
	writel(params->emc_dll_xform_quse12, emc + EMC_DLL_XFORM_QUSE12);
	writel(params->emc_dll_xform_quse13, emc + EMC_DLL_XFORM_QUSE13);
	writel(params->emc_dll_xform_quse14, emc + EMC_DLL_XFORM_QUSE14);
	writel(params->emc_dll_xform_quse15, emc + EMC_DLL_XFORM_QUSE15);
	writel(params->emc_dll_xform_dq0, emc + EMC_DLL_XFORM_DQ0);
	writel(params->emc_dll_xform_dq1, emc + EMC_DLL_XFORM_DQ1);
	writel(params->emc_dll_xform_dq2, emc + EMC_DLL_XFORM_DQ2);
	writel(params->emc_dll_xform_dq3, emc + EMC_DLL_XFORM_DQ3);
	writel(params->emc_dll_xform_dq4, emc + EMC_DLL_XFORM_DQ4);
	writel(params->emc_dll_xform_dq5, emc + EMC_DLL_XFORM_DQ5);
	writel(params->emc_dll_xform_dq6, emc + EMC_DLL_XFORM_DQ6);
	writel(params->emc_dll_xform_dq7, emc + EMC_DLL_XFORM_DQ7);
	writel(params->emc_dll_xform_addr0, emc + EMC_DLL_XFORM_ADDR0);
	writel(params->emc_dll_xform_addr1, emc + EMC_DLL_XFORM_ADDR1);
	writel(params->emc_dll_xform_addr2, emc + EMC_DLL_XFORM_ADDR2);
	writel(params->emc_dll_xform_addr3, emc + EMC_DLL_XFORM_ADDR3);
	writel(params->emc_dll_xform_addr4, emc + EMC_DLL_XFORM_ADDR4);
	writel(params->emc_dll_xform_addr5, emc + EMC_DLL_XFORM_ADDR5);
	writel(params->emc_acpd_control, emc + EMC_ACPD_CONTROL);
	writel(params->emc_dsr_vttgen_drv, emc + EMC_DSR_VTTGEN_DRV);
	writel(params->emc_txdsrvttgen, emc + EMC_TXDSRVTTGEN);
	writel(params->emc_bgbias_ctl, emc + EMC_BGBIAS_CTL);

	value = readl(emc + EMC_CFG);
	value &= ~(EMC_CFG_EMC2PMACRO_CFG_BYPASS_ADDRPIPE |
		   EMC_CFG_EMC2PMACRO_CFG_BYPASS_DATAPIPE1 |
		   EMC_CFG_EMC2PMACRO_CFG_BYPASS_DATAPIPE2);
	value |= params->emc_cfg & (EMC_CFG_EMC2PMACRO_CFG_BYPASS_ADDRPIPE |
				    EMC_CFG_EMC2PMACRO_CFG_BYPASS_DATAPIPE1 |
				    EMC_CFG_EMC2PMACRO_CFG_BYPASS_DATAPIPE2);
	writel(value, emc + EMC_CFG);

	if (params->boot_rom_patch_control & (1 << 31)) {
		uart_printf(debug, "  %08x < %08x\n", params->boot_rom_patch_control,
			    params->boot_rom_patch_data);
		value = 0x70000000 + ((params->boot_rom_patch_control & 0x3fffffff) << 2);
		writel(params->boot_rom_patch_data, value);
		writel(MC_TIMING_CONTROL_UPDATE, MC_TIMING_CONTROL);
	}

	writel(params->pmc_io_dpd3_req, pmc + PMC_IO_DPD3_REQ);
	udelay(params->pmc_io_dpd3_req_wait);

	writel(params->emc_dli_trim_txdqs0, emc + EMC_DLI_TRIM_TXDQS0);
	writel(params->emc_dli_trim_txdqs1, emc + EMC_DLI_TRIM_TXDQS1);
	writel(params->emc_dli_trim_txdqs2, emc + EMC_DLI_TRIM_TXDQS2);
	writel(params->emc_dli_trim_txdqs3, emc + EMC_DLI_TRIM_TXDQS3);
	writel(params->emc_dli_trim_txdqs4, emc + EMC_DLI_TRIM_TXDQS4);
	writel(params->emc_dli_trim_txdqs5, emc + EMC_DLI_TRIM_TXDQS5);
	writel(params->emc_dli_trim_txdqs6, emc + EMC_DLI_TRIM_TXDQS6);
	writel(params->emc_dli_trim_txdqs7, emc + EMC_DLI_TRIM_TXDQS7);
	writel(params->emc_dli_trim_txdqs8, emc + EMC_DLI_TRIM_TXDQS8);
	writel(params->emc_dli_trim_txdqs9, emc + EMC_DLI_TRIM_TXDQS9);
	writel(params->emc_dli_trim_txdqs10, emc + EMC_DLI_TRIM_TXDQS10);
	writel(params->emc_dli_trim_txdqs11, emc + EMC_DLI_TRIM_TXDQS11);
	writel(params->emc_dli_trim_txdqs12, emc + EMC_DLI_TRIM_TXDQS12);
	writel(params->emc_dli_trim_txdqs13, emc + EMC_DLI_TRIM_TXDQS13);
	writel(params->emc_dli_trim_txdqs14, emc + EMC_DLI_TRIM_TXDQS14);
	writel(params->emc_dli_trim_txdqs15, emc + EMC_DLI_TRIM_TXDQS15);

	writel(params->emc_ca_training_timing_cntl1, emc + EMC_CA_TRAINING_TIMING_CNTL1);
	writel(params->emc_ca_training_timing_cntl2, emc + EMC_CA_TRAINING_TIMING_CNTL2);

	writel(EMC_TIMING_CONTROL_UPDATE, emc + EMC_TIMING_CONTROL);
	udelay(params->emc_timing_control_wait);

	value = readl(pmc + PMC_POR_DPD_CTRL);
	value &= ~PMC_POR_DPD_CTRL_MEM0_ADDR0_CLK_SEL_DPD;
	value &= ~PMC_POR_DPD_CTRL_MEM0_ADDR1_CLK_SEL_DPD;
	value &= ~PMC_POR_DPD_CTRL_MEM0_HOLD_CKE_LOW_OVR;
	writel(value, pmc + PMC_POR_DPD_CTRL);
	udelay(params->pmc_por_dpd_ctrl_wait);

	if (params->memory_type == BCT_MEMORY_TYPE_DDR3) {
		/* apply reset on DDR3 */
		writel(0, emc + EMC_PIN);
		readl(emc + EMC_PIN);

		udelay(200 + params->emc_pin_extra_wait);

		writel(EMC_PIN_RST, emc + EMC_PIN);
		readl(emc + EMC_PIN);

		udelay(500 + params->emc_pin_extra_wait);
	}

	writel(EMC_PIN_RST | EMC_PIN_CKE, emc + EMC_PIN);
	readl(emc + EMC_PIN);

	udelay(params->emc_pin_program_wait);

	value = EMC_NOP_DEV_SELECT(params->emc_dev_select) | EMC_NOP_CMD;
	writel(value, emc + EMC_NOP);

	if (params->memory_type == BCT_MEMORY_TYPE_DDR3) {
		writel(params->emc_emrs2, emc + EMC_EMRS2);
		writel(params->emc_emrs3, emc + EMC_EMRS3);
		writel(params->emc_emrs, emc + EMC_EMRS);
		writel(params->emc_mrs, emc + EMC_MRS);

		if (params->emc_extra_mode_reg_write_enable)
			writel(params->emc_mrs_extra, emc + EMC_MRS);

		if (params->emc_zcal_warm_cold_boot_enables & 0x1) {
			writel(params->emc_zcal_init_dev0, emc + EMC_ZQ_CAL);
			udelay(params->emc_zcal_init_wait);

			if ((params->emc_dev_select & 0x2) == 0) {
				writel(params->emc_zcal_init_dev1, emc + EMC_ZQ_CAL);
				udelay(params->emc_zcal_init_wait);
			}
		} else {
			udelay(params->emc_zcal_init_wait);
		}
	}

	if (params->emc_bct_spare8) {
		uart_printf(debug, "  %08x < %08x\n", params->emc_bct_spare8,
			    params->emc_bct_spare9);
		writel(params->emc_bct_spare9, params->emc_bct_spare8);
	}

	if (params->memory_type == BCT_MEMORY_TYPE_LPDDR2 ||
	    params->memory_type == BCT_MEMORY_TYPE_DDR3) {
		writel(params->emc_zcal_interval, emc + EMC_ZCAL_INTERVAL);
		writel(params->emc_zcal_wait_cnt, emc + EMC_ZCAL_WAIT_CNT);
		writel(params->emc_zcal_mrw_cmd, emc + EMC_ZCAL_MRW_CMD);
	}

	if (params->emc_bct_spare10) {
		uart_printf(debug, "  %08x < %08x\n", params->emc_bct_spare10,
			    params->emc_bct_spare11);
		writel(params->emc_bct_spare11, params->emc_bct_spare10);
	}

	writel(EMC_TIMING_CONTROL_UPDATE, emc + EMC_TIMING_CONTROL);

	if (params->emc_extra_refresh_num > 0) {
		value = EMC_REF_CMD | EMC_REF_NORMAL |
			EMC_REF_NUM((1 << params->emc_extra_refresh_num) - 1) |
			EMC_REF_DEV_SELECT(params->emc_dev_select);
		writel(value, emc + EMC_REF);
	}

	value = EMC_REFCTRL_DEVICE_REFRESH_DISABLE(params->emc_dev_select) |
		EMC_REFCTRL_REF_VALID;
	writel(value, emc + EMC_REFCTRL);

	writel(params->emc_dyn_self_ref_control, emc + EMC_DYN_SELF_REF_CONTROL);
	writel(params->emc_cfg, emc + EMC_CFG);
	writel(params->emc_sel_dpd_ctrl, emc + EMC_SEL_DPD_CTRL);

	writel(params->emc_fbio_spare, emc + EMC_FBIO_SPARE);

	writel(EMC_TIMING_CONTROL_UPDATE, emc + EMC_TIMING_CONTROL);

	value = readl(arb + ARB_XBAR_CTRL);
	value |= ARB_XBAR_CTRL_MEM_INIT_DONE;
	writel(value, arb + ARB_XBAR_CTRL);

	writel(params->mc_video_protect_ctrl, mc + MC_VIDEO_PROTECT_CTRL);
	writel(1, mc + MC_EMEM_CFG_ACCESS_CTRL);

	writel(params->mc_sec_carveout_ctrl, mc + MC_SEC_CARVEOUT_CTRL);
	writel(params->mc_mts_carveout_ctrl, mc + MC_MTS_CARVEOUT_CTRL);
}

void sdram_test(void)
{
	uint32_t value = readl(TEGRA_MC_BASE + 0x050);
	unsigned int offset = (value & (1 << 31)) ? 0x00000000 : 0x80000000;
	size_t size = (value & 0x7fff) << 20;
	uint32_t *ptr = (uint32_t *)offset;
	unsigned int i;

	uart_printf(debug, "memory: %#x-%#x\n", offset, offset + size - 1);

	size /= 16;

	uart_printf(debug, "writing...");

	for (i = 0; i < size / 4; i++)
		ptr[i] = 0xaa551100;

	uart_printf(debug, "done\n");
	uart_printf(debug, "checking...");

	for (i = 0; i < size / 4; i++)
		if (ptr[i] != 0xaa551100)
			uart_printf(debug, "corruption @%p: %08x\n", &ptr[i], ptr[i]);

	uart_printf(debug, "done\n");
}
