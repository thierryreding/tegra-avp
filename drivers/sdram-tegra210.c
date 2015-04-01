#include <avp/bct.h>
#include <avp/i2c.h>
#include <avp/io.h>
#include <avp/iomap.h>
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

#define PMC_WEAK_BIAS 0x2c8
#define PMC_REG_SHORT 0x2cc

#define PMC_IO_DPD3_REQ 0x45c
#define PMC_IO_DPD4_REQ 0x464

#define PMC_DDR_CNTRL 0x4e4

/*
 * MC
 */
#define MC_EMEM_CFG 0x050
#define MC_EMEM_ADR_CFG 0x054
#define MC_EMEM_ADR_CFG_DEV0 0x058
#define MC_EMEM_ADR_CFG_DEV1 0x05c
#define MC_EMEM_ADR_CFG_CHANNEL_MASK 0x060
#define MC_EMEM_ADR_CFG_BANK_MASK_0 0x064
#define MC_EMEM_ADR_CFG_BANK_MASK_1 0x068
#define MC_EMEM_ADR_CFG_BANK_MASK_2 0x06c

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
#define MC_EMEM_ARB_MISC2 0x0c8
#define MC_EMEM_ARB_DA_TURNS 0x0d0
#define MC_EMEM_ARB_DA_COVERS 0x0d4
#define MC_EMEM_ARB_MISC0 0x0d8
#define MC_EMEM_ARB_MISC1 0x0dc
#define MC_EMEM_ARB_RING1_THROTTLE 0x0e0
#define MC_EMEM_ARB_OVERRIDE_0 0x0e8
#define MC_EMEM_ARB_RSV 0x0ec
#define MC_CLKEN_OVERRIDE 0x0f4
#define MC_TIMING_CONTROL 0x0fc
#define  MC_TIMING_CONTROL_UPDATE (1 << 0)
#define MC_STAT_CONTROL 0x100

#define MC_VIDEO_PROTECT_VPR_OVERRIDE0 0x418

#define MC_VIDEO_PROTECT_VPR_OVERRIDE1 0x590

#define MC_VIDEO_PROTECT_BOM 0x648
#define MC_VIDEO_PROTECT_SIZE_MB 0x64c
#define MC_VIDEO_PROTECT_CTRL 0x650

#define MC_EMEM_CFG_ACCESS_CTRL 0x664
#define  MC_EMEM_CFG_ACCESS_CTRL_DISABLE (1 << 0)

#define MC_SEC_CARVEOUT_BOM 0x670
#define MC_SEC_CARVEOUT_SIZE_MB 0x674
#define MC_SEC_CARVEOUT_CTRL 0x678

#define MC_EMEM_ARB_TIMING_RFCPB 0x6c0
#define MC_EMEM_ARB_TIMING_CCDMW 0x6c4

#define MC_EMEM_ARB_REFPB_HP_CTRL 0x6f0
#define MC_EMEM_ARB_REFPB_BANK_CTRL 0x6f4

#define MC_EMEM_ARB_OVERRIDE_1 0x968

#define MC_VIDEO_PROTECT_BOM_ADR_HI 0x978
#define MC_VIDEO_PROTECT_GPU_OVERRIDE0 0x984
#define MC_VIDEO_PROTECT_GPU_OVERRIDE1 0x988

#define MC_MTS_CARVEOUT_BOM 0x9a0
#define MC_MTS_CARVEOUT_SIZE_MB 0x9a4
#define MC_MTS_CARVEOUT_ADR_HI 0x9a8
#define MC_MTS_CARVEOUT_CTRL 0x9ac

#define MC_SEC_CARVEOUT_ADR_HI 0x9d4

#define MC_DA_CONFIG0 0x9dc

#define MC_SECURITY_CARVEOUT_CFG(x) (0xc08 + 0x50 * (x))
#define MC_SECURITY_CARVEOUT_BOM(x) (0xc0c + 0x50 * (x))
#define MC_SECURITY_CARVEOUT_BOM_HI(x) (0xc10 + 0x50 * (x))
#define MC_SECURITY_CARVEOUT_SIZE(x) (0xc14 + 0x50 * (x))
#define MC_SECURITY_CARVEOUT_CLIENT_ACCESS0(x) (0xc18 + 0x50 * (x))
#define MC_SECURITY_CARVEOUT_CLIENT_ACCESS1(x) (0xc1c + 0x50 * (x))
#define MC_SECURITY_CARVEOUT_CLIENT_ACCESS2(x) (0xc20 + 0x50 * (x))
#define MC_SECURITY_CARVEOUT_CLIENT_ACCESS3(x) (0xc24 + 0x50 * (x))
#define MC_SECURITY_CARVEOUT_CLIENT_ACCESS4(x) (0xc28 + 0x50 * (x))
#define MC_SECURITY_CARVEOUT_CLIENT_FORCE_INTERNAL_ACCESS0(x) (0xc2c + 0x50 * (x))
#define MC_SECURITY_CARVEOUT_CLIENT_FORCE_INTERNAL_ACCESS1(x) (0xc30 + 0x50 * (x))
#define MC_SECURITY_CARVEOUT_CLIENT_FORCE_INTERNAL_ACCESS2(x) (0xc34 + 0x50 * (x))
#define MC_SECURITY_CARVEOUT_CLIENT_FORCE_INTERNAL_ACCESS3(x) (0xc38 + 0x50 * (x))
#define MC_SECURITY_CARVEOUT_CLIENT_FORCE_INTERNAL_ACCESS4(x) (0xc3c + 0x50 * (x))

/*
 * EMC
 */
#define EMC_DBG 0x008
#define  EMC_DBG_WRITE_MUX (1 << 1)
#define  EMC_DBG_WRITE_MUX_SHIFT 1
#define EMC_CFG 0x00c
#define EMC_ADR_CFG 0x010

#define EMC_REFCTRL 0x020
#define EMC_PIN 0x024
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
#define EMC_TPPD 0x0ac

#define EMC_ODT_WRITE 0x0b0
#define EMC_PDEX2MRR 0x0b4
#define EMC_WEXT 0x0b8
#define EMC_RFC_SLR 0x0c0
#define EMC_MRS_WAIT_CNT2 0x0c4
#define EMC_MRS_WAIT_CNT 0x0c8
#define EMC_REF 0x0d4
#define EMC_NOP 0x0dc
#define EMC_SELF_REF 0x0e0
#define EMC_MRW 0x0e8

#define EMC_CMDQ 0x0f0
#define EMC_MC2EMCQ 0x0f4

#define EMC_FBIO_SPARE 0x100
#define EMC_FBIO_CFG5 0x104
#define EMC_PDEX2CKE 0x118
#define EMC_CKE2PDEN 0x11c
#define EMC_CFG_RSV 0x120
#define EMC_ACPD_CONTROL 0x124

#define EMC_MRW2 0x134
#define EMC_MRW3 0x138
#define EMC_MRW4 0x13c

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
#define EMC_CFG_2 0x2b8
#define EMC_CFG_DIG_DLL 0x2bc
#define EMC_CFG_DIG_DLL_PERIOD 0x2c0
#define EMC_CFG_DIG_DLL_1 0x2c8
#define EMC_RDV_MASK 0x2cc
#define EMC_WDV_MASK 0x2d0
#define EMC_RDV_EARLY_MASK 0x2d4
#define EMC_RDV_EARLY 0x2d8
#define EMC_AUTO_CAL_CONFIG8 0x2dc
#define EMC_ZCAL_INTERVAL 0x2e0
#define EMC_ZCAL_WAIT_CNT 0x2e4
#define EMC_ZCAL_MRW_CMD 0x2e8
#define EMC_ZQ_CAL 0x2ec
#define EMC_XM2COMPPADCTRL3 0x2f4
#define EMC_AUTO_CAL_VREF_SEL_0 0x2f8
#define EMC_AUTO_CAL_VREF_SEL_1 0x300
#define EMC_XM2COMPPADCTRL 0x30c
#define EMC_FDPD_CTRL_DQ 0x310
#define EMC_FDPD_CTRL_CMD 0x314

#define EMC_PMACRO_CMD_BRICK_CTRL_FDPD 0x318
#define EMC_PMACRO_DATA_BRICK_CTRL_FDPD 0x31c

#define EMC_PMACRO_BRICK_CTRL_RFU1 0x330
#define EMC_PMACRO_BRICK_CTRL_RFU2 0x334

#define EMC_CMD_MAPPING_CMD0_0 0x380
#define EMC_CMD_MAPPING_CMD0_1 0x384
#define EMC_CMD_MAPPING_CMD0_2 0x388
#define EMC_CMD_MAPPING_CMD1_0 0x38c
#define EMC_CMD_MAPPING_CMD1_1 0x390
#define EMC_CMD_MAPPING_CMD1_2 0x394
#define EMC_CMD_MAPPING_CMD2_0 0x398
#define EMC_CMD_MAPPING_CMD2_1 0x39c
#define EMC_CMD_MAPPING_CMD2_2 0x3a0
#define EMC_CMD_MAPPING_CMD3_0 0x3a4
#define EMC_CMD_MAPPING_CMD3_1 0x3a8
#define EMC_CMD_MAPPING_CMD3_2 0x3ac
#define EMC_CMD_MAPPING_BYTE 0x3b0
#define EMC_SEL_DPD_CTRL 0x3d8
#define EMC_PRE_REFRESH_REQ_CNT 0x3dc
#define EMC_DYN_SELF_REF_CONTROL 0x3e0
#define EMC_TXSRDLL 0x3e4

#define EMC_SWIZZLE_RANK0_BYTE0 0x404
#define EMC_SWIZZLE_RANK0_BYTE1 0x408
#define EMC_SWIZZLE_RANK0_BYTE2 0x40c
#define EMC_SWIZZLE_RANK0_BYTE3 0x410
#define EMC_SWIZZLE_RANK1_BYTE0 0x418
#define EMC_SWIZZLE_RANK1_BYTE1 0x41c
#define EMC_SWIZZLE_RANK1_BYTE2 0x420
#define EMC_SWIZZLE_RANK1_BYTE3 0x424
#define EMC_ISSUE_QRST 0x428

#define EMC_PMC_SCRATCH1 0x440
#define EMC_PMC_SCRATCH2 0x444
#define EMC_PMC_SCRATCH3 0x448

#define EMC_AUTO_CAL_CONFIG2 0x458
#define EMC_AUTO_CAL_CONFIG3 0x45c

#define EMC_AUTO_CAL_CHANNEL 0x464
#define EMC_IBDLY 0x468
#define EMC_OBDLY 0x46c

#define EMC_TXDSRVTTGEN 0x480
#define EMC_WE_DURATION 0x48c
#define EMC_WS_DURATION 0x490
#define EMC_WEV 0x494
#define EMC_WSV 0x498
#define EMC_CFG_3 0x49c
#define EMC_MRW5 0x4a0
#define EMC_MRW6 0x4a4
#define EMC_MRW8 0x4ac
#define EMC_MRW9 0x4b0
#define EMC_MRW12 0x4bc
#define EMC_MRW13 0x4c0
#define EMC_MRW14 0x4c4

#define EMC_FDPD_CTRL_CMD_NO_RAMP 0x4d8
#define EMC_WDV_CHK 0x4e0

#define EMC_CFG_PIPE_2 0x554
#define EMC_CFG_PIPE_CLK 0x558
#define EMC_CFG_PIPE_1 0x55c
#define EMC_CFG_PIPE 0x560
#define EMC_QPOP 0x564
#define EMC_QUSE_WIDTH 0x568
#define EMC_PUTERM_WIDTH 0x56c

#define EMC_AUTO_CAL_CONFIG7 0x574
#define EMC_XM2COMPPADCTRL2 0x578

#define EMC_REFCTRL2 0x580
#define EMC_FBIO_CFG7 0x584
#define EMC_DATA_BRLSHFT_0 0x588
#define EMC_DATA_BRLSHFT_1 0x58c
#define EMC_RFCPB 0x590
#define EMC_DQS_BRLSHFT_0 0x594
#define EMC_DQS_BRLSHFT_1 0x598
#define EMC_CMD_BRLSHFT_0 0x59c
#define EMC_CMD_BRLSHFT_1 0x5a0
#define EMC_CMD_BRLSHFT_2 0x5a4
#define EMC_CMD_BRLSHFT_3 0x5a8
#define EMC_QUSE_BRLSHFT_0 0x5ac
#define EMC_AUTO_CAL_CONFIG4 0x5b0
#define EMC_AUTO_CAL_CONFIG5 0x5b4
#define EMC_QUSE_BRLSHFT_1 0x5b8
#define EMC_QUSE_BRLSHFT_2 0x5bc
#define EMC_CCDMW 0x5c0
#define EMC_QUSE_BRLSHFT_3 0x5c4
#define EMC_FBIO_CFG8 0x5c8
#define EMC_AUTO_CAL_CONFIG6 0x5cc

#define EMC_DLL_CFG_0 0x5e4
#define EMC_DLL_CFG_1 0x5e8

#define EMC_CONFIG_SAMPLE_DELAY 0x5f0
#define EMC_CFG_UPDATE 0x5f4

#define EMC_PMACRO_QUSE_DDLL_RANK0_0 0x600
#define EMC_PMACRO_QUSE_DDLL_RANK0_1 0x604
#define EMC_PMACRO_QUSE_DDLL_RANK0_2 0x608
#define EMC_PMACRO_QUSE_DDLL_RANK0_3 0x60c
#define EMC_PMACRO_QUSE_DDLL_RANK0_4 0x610
#define EMC_PMACRO_QUSE_DDLL_RANK0_5 0x614
#define EMC_PMACRO_QUSE_DDLL_RANK1_0 0x620
#define EMC_PMACRO_QUSE_DDLL_RANK1_1 0x624
#define EMC_PMACRO_QUSE_DDLL_RANK1_2 0x628
#define EMC_PMACRO_QUSE_DDLL_RANK1_3 0x62c
#define EMC_PMACRO_QUSE_DDLL_RANK1_4 0x630
#define EMC_PMACRO_QUSE_DDLL_RANK1_5 0x634
#define EMC_PMACRO_OB_DDLL_LONG_DQ_RANK0_0 0x640
#define EMC_PMACRO_OB_DDLL_LONG_DQ_RANK0_1 0x644
#define EMC_PMACRO_OB_DDLL_LONG_DQ_RANK0_2 0x648
#define EMC_PMACRO_OB_DDLL_LONG_DQ_RANK0_3 0x64c
#define EMC_PMACRO_OB_DDLL_LONG_DQ_RANK0_4 0x650
#define EMC_PMACRO_OB_DDLL_LONG_DQ_RANK0_5 0x654
#define EMC_PMACRO_OB_DDLL_LONG_DQ_RANK1_0 0x660
#define EMC_PMACRO_OB_DDLL_LONG_DQ_RANK1_1 0x664
#define EMC_PMACRO_OB_DDLL_LONG_DQ_RANK1_2 0x668
#define EMC_PMACRO_OB_DDLL_LONG_DQ_RANK1_3 0x66c
#define EMC_PMACRO_OB_DDLL_LONG_DQ_RANK1_4 0x670
#define EMC_PMACRO_OB_DDLL_LONG_DQ_RANK1_5 0x674
#define EMC_PMACRO_OB_DDLL_LONG_DQS_RANK0_0 0x680
#define EMC_PMACRO_OB_DDLL_LONG_DQS_RANK0_1 0x684
#define EMC_PMACRO_OB_DDLL_LONG_DQS_RANK0_2 0x688
#define EMC_PMACRO_OB_DDLL_LONG_DQS_RANK0_3 0x68c
#define EMC_PMACRO_OB_DDLL_LONG_DQS_RANK0_4 0x690
#define EMC_PMACRO_OB_DDLL_LONG_DQS_RANK0_5 0x694
#define EMC_PMACRO_OB_DDLL_LONG_DQS_RANK1_0 0x6a0
#define EMC_PMACRO_OB_DDLL_LONG_DQS_RANK1_1 0x6a4
#define EMC_PMACRO_OB_DDLL_LONG_DQS_RANK1_2 0x6a8
#define EMC_PMACRO_OB_DDLL_LONG_DQS_RANK1_3 0x6ac
#define EMC_PMACRO_OB_DDLL_LONG_DQS_RANK1_4 0x6b0
#define EMC_PMACRO_OB_DDLL_LONG_DQS_RANK1_5 0x6b4
#define EMC_PMACRO_IB_DDLL_LONG_DQS_RANK0_0 0x6c0
#define EMC_PMACRO_IB_DDLL_LONG_DQS_RANK0_1 0x6c4
#define EMC_PMACRO_IB_DDLL_LONG_DQS_RANK0_2 0x6c8
#define EMC_PMACRO_IB_DDLL_LONG_DQS_RANK0_3 0x6cc
#define EMC_PMACRO_IB_DDLL_LONG_DQS_RANK0_4 0x6d0
#define EMC_PMACRO_IB_DDLL_LONG_DQS_RANK0_5 0x6d4
#define EMC_PMACRO_IB_DDLL_LONG_DQS_RANK1_0 0x6e0
#define EMC_PMACRO_IB_DDLL_LONG_DQS_RANK1_1 0x6e4
#define EMC_PMACRO_IB_DDLL_LONG_DQS_RANK1_2 0x6e8
#define EMC_PMACRO_IB_DDLL_LONG_DQS_RANK1_3 0x6ec
#define EMC_PMACRO_IB_DDLL_LONG_DQS_RANK1_4 0x6f0
#define EMC_PMACRO_IB_DDLL_LONG_DQS_RANK1_5 0x6f4

#define EMC_PMACRO_AUTOCAL_CFG_0 0x700
#define EMC_PMACRO_AUTOCAL_CFG_1 0x704
#define EMC_PMACRO_AUTOCAL_CFG_2 0x708

#define EMC_PMACRO_TX_PWRD_0 0x720
#define EMC_PMACRO_TX_PWRD_1 0x724
#define EMC_PMACRO_TX_PWRD_2 0x728
#define EMC_PMACRO_TX_PWRD_3 0x72c
#define EMC_PMACRO_TX_PWRD_4 0x730
#define EMC_PMACRO_TX_PWRD_5 0x734
#define EMC_PMACRO_TX_SEL_CLK_SRC_0 0x740
#define EMC_PMACRO_TX_SEL_CLK_SRC_1 0x744
#define EMC_PMACRO_TX_SEL_CLK_SRC_2 0x748
#define EMC_PMACRO_TX_SEL_CLK_SRC_3 0x74c
#define EMC_PMACRO_TX_SEL_CLK_SRC_4 0x750
#define EMC_PMACRO_TX_SEL_CLK_SRC_5 0x754
#define EMC_PMACRO_DDLL_BYPASS 0x760
#define EMC_PMACRO_DDLL_PWRD_0 0x770
#define EMC_PMACRO_DDLL_PWRD_1 0x774
#define EMC_PMACRO_DDLL_PWRD_2 0x778
#define EMC_PMACRO_CMD_CTRL_0 0x780
#define EMC_PMACRO_CMD_CTRL_1 0x784
#define EMC_PMACRO_CMD_CTRL_2 0x788

#define EMC_PMACRO_IB_VREF_DQ_0 0xbe0
#define EMC_PMACRO_IB_VREF_DQ_1 0xbe4
#define EMC_PMACRO_IB_VREF_DQS_0 0xbf0
#define EMC_PMACRO_IB_VREF_DQS_1 0xbf4

#define EMC_PMACRO_DDLL_LONG_CMD_0 0xc00
#define EMC_PMACRO_DDLL_LONG_CMD_1 0xc04
#define EMC_PMACRO_DDLL_LONG_CMD_2 0xc08
#define EMC_PMACRO_DDLL_LONG_CMD_3 0xc0c
#define EMC_PMACRO_DDLL_LONG_CMD_4 0xc10
#define EMC_PMACRO_DDLL_LONG_CMD_5 0xc14

#define EMC_PMACRO_DDLL_SHORT_CMD_0 0xc20
#define EMC_PMACRO_DDLL_SHORT_CMD_1 0xc24
#define EMC_PMACRO_DDLL_SHORT_CMD_2 0xc28

#define EMC_PMACRO_VTTGEN_CTRL_0 0xc34
#define EMC_PMACRO_VTTGEN_CTRL_1 0xc38
#define EMC_PMACRO_BG_BIAS_CTRL_0 0xc3c
#define EMC_PMACRO_PAD_CFG_CTRL 0xc40
#define EMC_PMACRO_ZCTRL 0xc44
#define EMC_PMACRO_RX_TERM 0xc48
#define EMC_PMACRO_CMD_TX_DRV 0xc4c

#define EMC_PMACRO_CMD_PAD_RX_CTRL 0xc50
#define EMC_PMACRO_DATA_PAD_RX_CTRL 0xc54
#define EMC_PMACRO_CMD_RX_TERM_MODE 0xc58
#define EMC_PMACRO_DATA_RX_TERM_MODE 0xc5c
#define EMC_PMACRO_CMD_PAD_TX_CTRL 0xc60
#define EMC_PMACRO_DATA_PAD_TX_CTRL 0xc64
#define EMC_PMACRO_COMMON_PAD_TX_CTRL 0xc68

#define EMC_PMACRO_DQ_TX_DRV 0xc70
#define EMC_PMACRO_CA_TX_DRV 0xc74
#define EMC_PMACRO_AUTOCAL_CFG_COMMON 0xc78

#define EMC_PMACRO_BRICK_MAPPING_0 0xc80
#define EMC_PMACRO_BRICK_MAPPING_1 0xc84
#define EMC_PMACRO_BRICK_MAPPING_2 0xc88

#define EMC_PMACRO_VTTGEN_CTRL_2 0xcf0
#define EMC_PMACRO_IB_RXRT 0xcf4

#define EMC_ZCAL_BOOT_ENABLE_WARM (1 << 1)
#define EMC_ZCAL_BOOT_ENABLE_COLD (1 << 0)

/*
 * ARB
 */
#define ARB_XBAR_CTRL 0x0e0
#define  ARB_XBAR_CTRL_MEM_INIT_DONE (1 << 16)

enum {
	LPDDR4 = 3,
};

void sdram_init(struct bct_sdram_params *params)
{
	unsigned int arb = TEGRA_ARB_BASE;
	unsigned int pmc = TEGRA_PMC_BASE;
	unsigned int emc = TEGRA_EMC_BASE;
	unsigned int mc = TEGRA_MC_BASE;
	bool warmboot = false;
	unsigned int i;
	uint32_t value;

	if (1) {
		uint8_t value;

		reset_assert(&rst_dvfs);
		clock_periph_enable(&clk_dvfs);
		udelay(1);
		reset_deassert(&rst_dvfs);
		udelay(1);

		i2c_init(&dvc, 100000);

		/* disable SD1 remote sense */
		i2c_smbus_read_byte_data(&dvc, 0x3c, 0x22, &value);
		i2c_smbus_write_byte_data(&dvc, 0x3c, 0x22, 0x05);
		i2c_smbus_read_byte_data(&dvc, 0x3c, 0x22, &value);
		/* set DDR voltage to 1125 mV */
		i2c_smbus_write_byte_data(&dvc, 0x3c, 0x17, 0x2a);

		udelay(10);
	}

	/*
	reset_deassert(&rst_emc);
	reset_deassert(&rst_mc);

	udelay(5);
	*/

	writel(params->pmc_vddp_sel, pmc + PMC_VDDP_SEL);
	udelay(params->pmc_vddp_sel_wait);

	value = readl(pmc + PMC_DDR_PWR);
	value &= ~PMC_DDR_PWR_VAL_MASK;
	value |= params->pmc_ddr_pwr & PMC_DDR_PWR_VAL_MASK;
	writel(value, pmc + PMC_DDR_PWR);

	value = readl(pmc + PMC_NO_IOPOWER);
	value &= ~PMC_NO_IOPOWER_MEM;
	value |= params->pmc_no_iopower & PMC_NO_IOPOWER_MEM;
	value &= ~PMC_NO_IOPOWER_MEM_COMP;
	value |= params->pmc_no_iopower & PMC_NO_IOPOWER_MEM_COMP;
	/* XXX */
	value = 0;
	writel(value, pmc + PMC_NO_IOPOWER);

	writel(params->pmc_reg_short, pmc + PMC_REG_SHORT);
	writel(params->pmc_ddr_cntrl, pmc + PMC_DDR_CNTRL);

	if (params->emc_bct_spare0)
		writel(params->emc_bct_spare1, params->emc_bct_spare0);

	if (warmboot) {
		/* XXX */
	} else {
		value = (params->emc_pmc_scratch1 & 0x3fffffff) | 1 << 31;
		value = (value ^ 0x0000ffff) & 0xc000ffff;

		writel(value, pmc + PMC_IO_DPD3_REQ);
		udelay(params->pmc_io_dpd3_req_wait);

		value = (params->emc_pmc_scratch2 & 0x3fffffff) | 1 << 31;
		value = (value ^ 0x3fff0000) & 0xffff0000;

		writel(value, pmc + PMC_IO_DPD4_REQ);
		udelay(params->pmc_io_dpd4_req_wait);

		value = (params->emc_pmc_scratch2 & 0x3fffffff) | 1 << 31;
		value = (value ^ 0x0000ffff) & 0xc000ffff;

		writel(value, pmc + PMC_IO_DPD4_REQ);
		udelay(params->pmc_io_dpd4_req_wait);
	}

	writel(0x00000000, pmc + PMC_WEAK_BIAS);
	udelay(1);

	/* ... */

	clock_pllm_init(&clk_rst, params);

	writel(params->emc_pmacro_vttgen_ctrl_0, emc + EMC_PMACRO_VTTGEN_CTRL_0);
	writel(params->emc_pmacro_vttgen_ctrl_1, emc + EMC_PMACRO_VTTGEN_CTRL_1);
	writel(params->emc_pmacro_vttgen_ctrl_2, emc + EMC_PMACRO_VTTGEN_CTRL_2);
	writel(EMC_TIMING_CONTROL_UPDATE, emc + EMC_TIMING_CONTROL);
	udelay(10);

	value = params->emc_dbg | (params->emc_dbg_write_mux << EMC_DBG_WRITE_MUX_SHIFT);
	writel(value, emc + EMC_DBG);

	if (params->emc_bct_spare2)
		writel(params->emc_bct_spare3, params->emc_bct_spare2);

	writel(params->emc_fbio_cfg7, emc + EMC_FBIO_CFG7);

	writel(params->emc_cmd_mapping_cmd0_0, emc + EMC_CMD_MAPPING_CMD0_0);
	writel(params->emc_cmd_mapping_cmd0_1, emc + EMC_CMD_MAPPING_CMD0_1);
	writel(params->emc_cmd_mapping_cmd0_2, emc + EMC_CMD_MAPPING_CMD0_2);
	writel(params->emc_cmd_mapping_cmd1_0, emc + EMC_CMD_MAPPING_CMD1_0);
	writel(params->emc_cmd_mapping_cmd1_1, emc + EMC_CMD_MAPPING_CMD1_1);
	writel(params->emc_cmd_mapping_cmd1_2, emc + EMC_CMD_MAPPING_CMD1_2);
	writel(params->emc_cmd_mapping_cmd2_0, emc + EMC_CMD_MAPPING_CMD2_0);
	writel(params->emc_cmd_mapping_cmd2_1, emc + EMC_CMD_MAPPING_CMD2_1);
	writel(params->emc_cmd_mapping_cmd2_2, emc + EMC_CMD_MAPPING_CMD2_2);
	writel(params->emc_cmd_mapping_cmd3_0, emc + EMC_CMD_MAPPING_CMD3_0);
	writel(params->emc_cmd_mapping_cmd3_1, emc + EMC_CMD_MAPPING_CMD3_1);
	writel(params->emc_cmd_mapping_cmd3_2, emc + EMC_CMD_MAPPING_CMD3_2);
	writel(params->emc_cmd_mapping_byte, emc + EMC_CMD_MAPPING_BYTE);

	writel(params->emc_pmacro_brick_mapping_0, emc + EMC_PMACRO_BRICK_MAPPING_0);
	writel(params->emc_pmacro_brick_mapping_1, emc + EMC_PMACRO_BRICK_MAPPING_1);
	writel(params->emc_pmacro_brick_mapping_2, emc + EMC_PMACRO_BRICK_MAPPING_2);

	value = (params->emc_pmacro_brick_ctrl_rfu1 | ~0x01120112) & 0x1fff1fff;
	writel(value, emc + EMC_PMACRO_BRICK_CTRL_RFU1);

	writel(params->emc_config_sample_delay, emc + EMC_CONFIG_SAMPLE_DELAY);

	if (warmboot) {
		/* XXX */
	}

	writel(params->emc_fbio_cfg8, emc + EMC_FBIO_CFG8);

	writel(params->emc_swizzle_rank0_byte0, emc + EMC_SWIZZLE_RANK0_BYTE0);
	writel(params->emc_swizzle_rank0_byte1, emc + EMC_SWIZZLE_RANK0_BYTE1);
	writel(params->emc_swizzle_rank0_byte2, emc + EMC_SWIZZLE_RANK0_BYTE2);
	writel(params->emc_swizzle_rank0_byte3, emc + EMC_SWIZZLE_RANK0_BYTE3);
	writel(params->emc_swizzle_rank1_byte0, emc + EMC_SWIZZLE_RANK1_BYTE0);
	writel(params->emc_swizzle_rank1_byte1, emc + EMC_SWIZZLE_RANK1_BYTE1);
	writel(params->emc_swizzle_rank1_byte2, emc + EMC_SWIZZLE_RANK1_BYTE2);
	writel(params->emc_swizzle_rank1_byte3, emc + EMC_SWIZZLE_RANK1_BYTE3);

	if (params->emc_bct_spare6)
		writel(params->emc_bct_spare7, params->emc_bct_spare6);

	writel(params->emc_xm2comppadctrl, emc + EMC_XM2COMPPADCTRL);
	writel(params->emc_xm2comppadctrl2, emc + EMC_XM2COMPPADCTRL2);
	writel(params->emc_xm2comppadctrl3, emc + EMC_XM2COMPPADCTRL3);

	writel(params->emc_auto_cal_config2, emc + EMC_AUTO_CAL_CONFIG2);
	writel(params->emc_auto_cal_config3, emc + EMC_AUTO_CAL_CONFIG3);
	writel(params->emc_auto_cal_config4, emc + EMC_AUTO_CAL_CONFIG4);
	writel(params->emc_auto_cal_config5, emc + EMC_AUTO_CAL_CONFIG5);
	writel(params->emc_auto_cal_config6, emc + EMC_AUTO_CAL_CONFIG6);
	writel(params->emc_auto_cal_config7, emc + EMC_AUTO_CAL_CONFIG7);
	writel(params->emc_auto_cal_config8, emc + EMC_AUTO_CAL_CONFIG8);
	writel(params->emc_pmacro_rx_term, emc + EMC_PMACRO_RX_TERM);
	writel(params->emc_pmacro_dq_tx_drv, emc + EMC_PMACRO_DQ_TX_DRV);
	writel(params->emc_pmacro_ca_tx_drv, emc + EMC_PMACRO_CA_TX_DRV);
	writel(params->emc_pmacro_cmd_tx_drv, emc + EMC_PMACRO_CMD_TX_DRV);
	writel(params->emc_pmacro_autocal_cfg_common, emc + EMC_PMACRO_AUTOCAL_CFG_COMMON);
	writel(params->emc_auto_cal_channel, emc + EMC_AUTO_CAL_CHANNEL);
	writel(params->emc_pmacro_zctrl, emc + EMC_PMACRO_ZCTRL);

	writel(params->emc_dll_cfg_0, emc + EMC_DLL_CFG_0);
	writel(params->emc_dll_cfg_1, emc + EMC_DLL_CFG_1);
	writel(params->emc_cfg_dig_dll_1, emc + EMC_CFG_DIG_DLL_1);

	writel(params->emc_data_brlshft_0, emc + EMC_DATA_BRLSHFT_0);
	writel(params->emc_data_brlshft_1, emc + EMC_DATA_BRLSHFT_1);
	writel(params->emc_dqs_brlshft_0, emc + EMC_DQS_BRLSHFT_0);
	writel(params->emc_dqs_brlshft_1, emc + EMC_DQS_BRLSHFT_1);
	writel(params->emc_cmd_brlshft_0, emc + EMC_CMD_BRLSHFT_0);
	writel(params->emc_cmd_brlshft_1, emc + EMC_CMD_BRLSHFT_1);
	writel(params->emc_cmd_brlshft_2, emc + EMC_CMD_BRLSHFT_2);
	writel(params->emc_cmd_brlshft_3, emc + EMC_CMD_BRLSHFT_3);
	writel(params->emc_quse_brlshft_0, emc + EMC_QUSE_BRLSHFT_0);
	writel(params->emc_quse_brlshft_1, emc + EMC_QUSE_BRLSHFT_1);
	writel(params->emc_quse_brlshft_2, emc + EMC_QUSE_BRLSHFT_2);
	writel(params->emc_quse_brlshft_3, emc + EMC_QUSE_BRLSHFT_3);

	value = (params->emc_pmacro_brick_ctrl_rfu1 | ~0x01bf01bf) & 0x1fff1fff;
	writel(value, emc + EMC_PMACRO_BRICK_CTRL_RFU1);

	writel(params->emc_pmacro_pad_cfg_ctrl, emc + EMC_PMACRO_PAD_CFG_CTRL);
	/*
	writel(params->emc_pmacro_cmd_brick_ctrl, emc + EMC_PMACRO_CMD_BRICK_CTRL);
	*/
	writel(params->emc_pmacro_cmd_brick_ctrl_fdpd, emc + EMC_PMACRO_CMD_BRICK_CTRL_FDPD);
	writel(params->emc_pmacro_brick_ctrl_rfu2 & 0xff7fff7f, emc + EMC_PMACRO_BRICK_CTRL_RFU2);
	writel(params->emc_pmacro_data_brick_ctrl_fdpd, emc + EMC_PMACRO_DATA_BRICK_CTRL_FDPD);
	writel(params->emc_pmacro_bg_bias_ctrl_0, emc + EMC_PMACRO_BG_BIAS_CTRL_0);
	writel(params->emc_pmacro_data_pad_rx_ctrl, emc + EMC_PMACRO_DATA_PAD_RX_CTRL);
	writel(params->emc_pmacro_cmd_pad_rx_ctrl, emc + EMC_PMACRO_CMD_PAD_RX_CTRL);
	writel(params->emc_pmacro_data_pad_tx_ctrl, emc + EMC_PMACRO_DATA_PAD_TX_CTRL);
	/*
	writel(params->emc_pmacro_common_pad_tx_ctrl, emc + EMC_PMACRO_COMMON_PAD_TX_CTRL);
	*/
	writel(params->emc_pmacro_data_rx_term_mode, emc + EMC_PMACRO_DATA_RX_TERM_MODE);
	writel(params->emc_pmacro_cmd_rx_term_mode, emc + EMC_PMACRO_CMD_RX_TERM_MODE);
	writel(params->emc_pmacro_cmd_pad_tx_ctrl, emc + EMC_PMACRO_CMD_PAD_TX_CTRL);

	writel(params->emc_cfg_3, emc + EMC_CFG_3);

	writel(params->emc_pmacro_tx_pwrd_0, emc + EMC_PMACRO_TX_PWRD_0);
	writel(params->emc_pmacro_tx_pwrd_1, emc + EMC_PMACRO_TX_PWRD_1);
	writel(params->emc_pmacro_tx_pwrd_2, emc + EMC_PMACRO_TX_PWRD_2);
	writel(params->emc_pmacro_tx_pwrd_3, emc + EMC_PMACRO_TX_PWRD_3);
	writel(params->emc_pmacro_tx_pwrd_4, emc + EMC_PMACRO_TX_PWRD_4);
	writel(params->emc_pmacro_tx_pwrd_5, emc + EMC_PMACRO_TX_PWRD_5);

	writel(params->emc_pmacro_tx_sel_clk_src_0, emc + EMC_PMACRO_TX_SEL_CLK_SRC_0);
	writel(params->emc_pmacro_tx_sel_clk_src_1, emc + EMC_PMACRO_TX_SEL_CLK_SRC_1);
	writel(params->emc_pmacro_tx_sel_clk_src_2, emc + EMC_PMACRO_TX_SEL_CLK_SRC_2);
	writel(params->emc_pmacro_tx_sel_clk_src_3, emc + EMC_PMACRO_TX_SEL_CLK_SRC_3);
	writel(params->emc_pmacro_tx_sel_clk_src_4, emc + EMC_PMACRO_TX_SEL_CLK_SRC_4);
	writel(params->emc_pmacro_tx_sel_clk_src_5, emc + EMC_PMACRO_TX_SEL_CLK_SRC_5);

	writel(params->emc_pmacro_ddll_bypass, emc + EMC_PMACRO_DDLL_BYPASS);
	writel(params->emc_pmacro_ddll_pwrd_0, emc + EMC_PMACRO_DDLL_PWRD_0);
	writel(params->emc_pmacro_ddll_pwrd_1, emc + EMC_PMACRO_DDLL_PWRD_1);
	writel(params->emc_pmacro_ddll_pwrd_2, emc + EMC_PMACRO_DDLL_PWRD_2);
	writel(params->emc_pmacro_cmd_ctrl_0, emc + EMC_PMACRO_CMD_CTRL_0);
	writel(params->emc_pmacro_cmd_ctrl_1, emc + EMC_PMACRO_CMD_CTRL_1);
	writel(params->emc_pmacro_cmd_ctrl_2, emc + EMC_PMACRO_CMD_CTRL_2);
	writel(params->emc_pmacro_ib_vref_dq_0, emc + EMC_PMACRO_IB_VREF_DQ_0);
	writel(params->emc_pmacro_ib_vref_dq_1, emc + EMC_PMACRO_IB_VREF_DQ_1);
	writel(params->emc_pmacro_ib_vref_dqs_0, emc + EMC_PMACRO_IB_VREF_DQS_0);
	writel(params->emc_pmacro_ib_vref_dqs_1, emc + EMC_PMACRO_IB_VREF_DQS_1);
	writel(params->emc_pmacro_ib_rxrt, emc + EMC_PMACRO_IB_RXRT);

	writel(params->emc_pmacro_quse_ddll_rank0_0, emc + EMC_PMACRO_QUSE_DDLL_RANK0_0);
	writel(params->emc_pmacro_quse_ddll_rank0_1, emc + EMC_PMACRO_QUSE_DDLL_RANK0_1);
	writel(params->emc_pmacro_quse_ddll_rank0_2, emc + EMC_PMACRO_QUSE_DDLL_RANK0_2);
	writel(params->emc_pmacro_quse_ddll_rank0_3, emc + EMC_PMACRO_QUSE_DDLL_RANK0_3);
	writel(params->emc_pmacro_quse_ddll_rank0_4, emc + EMC_PMACRO_QUSE_DDLL_RANK0_4);
	writel(params->emc_pmacro_quse_ddll_rank0_5, emc + EMC_PMACRO_QUSE_DDLL_RANK0_5);
	writel(params->emc_pmacro_quse_ddll_rank1_0, emc + EMC_PMACRO_QUSE_DDLL_RANK1_0);
	writel(params->emc_pmacro_quse_ddll_rank1_1, emc + EMC_PMACRO_QUSE_DDLL_RANK1_1);
	writel(params->emc_pmacro_quse_ddll_rank1_2, emc + EMC_PMACRO_QUSE_DDLL_RANK1_2);
	writel(params->emc_pmacro_quse_ddll_rank1_3, emc + EMC_PMACRO_QUSE_DDLL_RANK1_3);
	writel(params->emc_pmacro_quse_ddll_rank1_4, emc + EMC_PMACRO_QUSE_DDLL_RANK1_4);
	writel(params->emc_pmacro_quse_ddll_rank1_5, emc + EMC_PMACRO_QUSE_DDLL_RANK1_5);
	writel(params->emc_pmacro_brick_ctrl_rfu1, emc + EMC_PMACRO_BRICK_CTRL_RFU1);
	writel(params->emc_pmacro_ob_ddll_long_dq_rank0_0, emc + EMC_PMACRO_OB_DDLL_LONG_DQ_RANK0_0);
	writel(params->emc_pmacro_ob_ddll_long_dq_rank0_1, emc + EMC_PMACRO_OB_DDLL_LONG_DQ_RANK0_1);
	writel(params->emc_pmacro_ob_ddll_long_dq_rank0_2, emc + EMC_PMACRO_OB_DDLL_LONG_DQ_RANK0_2);
	writel(params->emc_pmacro_ob_ddll_long_dq_rank0_3, emc + EMC_PMACRO_OB_DDLL_LONG_DQ_RANK0_3);
	writel(params->emc_pmacro_ob_ddll_long_dq_rank0_4, emc + EMC_PMACRO_OB_DDLL_LONG_DQ_RANK0_4);
	writel(params->emc_pmacro_ob_ddll_long_dq_rank0_5, emc + EMC_PMACRO_OB_DDLL_LONG_DQ_RANK0_5);
	writel(params->emc_pmacro_ob_ddll_long_dq_rank1_0, emc + EMC_PMACRO_OB_DDLL_LONG_DQ_RANK1_0);
	writel(params->emc_pmacro_ob_ddll_long_dq_rank1_1, emc + EMC_PMACRO_OB_DDLL_LONG_DQ_RANK1_1);
	writel(params->emc_pmacro_ob_ddll_long_dq_rank1_2, emc + EMC_PMACRO_OB_DDLL_LONG_DQ_RANK1_2);
	writel(params->emc_pmacro_ob_ddll_long_dq_rank1_3, emc + EMC_PMACRO_OB_DDLL_LONG_DQ_RANK1_3);
	writel(params->emc_pmacro_ob_ddll_long_dq_rank1_4, emc + EMC_PMACRO_OB_DDLL_LONG_DQ_RANK1_4);
	writel(params->emc_pmacro_ob_ddll_long_dq_rank1_5, emc + EMC_PMACRO_OB_DDLL_LONG_DQ_RANK1_5);
	writel(params->emc_pmacro_ob_ddll_long_dqs_rank0_0, emc + EMC_PMACRO_OB_DDLL_LONG_DQS_RANK0_0);
	writel(params->emc_pmacro_ob_ddll_long_dqs_rank0_1, emc + EMC_PMACRO_OB_DDLL_LONG_DQS_RANK0_1);
	writel(params->emc_pmacro_ob_ddll_long_dqs_rank0_2, emc + EMC_PMACRO_OB_DDLL_LONG_DQS_RANK0_2);
	writel(params->emc_pmacro_ob_ddll_long_dqs_rank0_3, emc + EMC_PMACRO_OB_DDLL_LONG_DQS_RANK0_3);
	writel(params->emc_pmacro_ob_ddll_long_dqs_rank0_4, emc + EMC_PMACRO_OB_DDLL_LONG_DQS_RANK0_4);
	writel(params->emc_pmacro_ob_ddll_long_dqs_rank0_5, emc + EMC_PMACRO_OB_DDLL_LONG_DQS_RANK0_5);
	writel(params->emc_pmacro_ob_ddll_long_dqs_rank1_0, emc + EMC_PMACRO_OB_DDLL_LONG_DQS_RANK1_0);
	writel(params->emc_pmacro_ob_ddll_long_dqs_rank1_1, emc + EMC_PMACRO_OB_DDLL_LONG_DQS_RANK1_1);
	writel(params->emc_pmacro_ob_ddll_long_dqs_rank1_2, emc + EMC_PMACRO_OB_DDLL_LONG_DQS_RANK1_2);
	writel(params->emc_pmacro_ob_ddll_long_dqs_rank1_3, emc + EMC_PMACRO_OB_DDLL_LONG_DQS_RANK1_3);
	writel(params->emc_pmacro_ob_ddll_long_dqs_rank1_4, emc + EMC_PMACRO_OB_DDLL_LONG_DQS_RANK1_4);
	writel(params->emc_pmacro_ob_ddll_long_dqs_rank1_5, emc + EMC_PMACRO_OB_DDLL_LONG_DQS_RANK1_5);
	writel(params->emc_pmacro_ib_ddll_long_dqs_rank0_0, emc + EMC_PMACRO_IB_DDLL_LONG_DQS_RANK0_0);
	writel(params->emc_pmacro_ib_ddll_long_dqs_rank0_1, emc + EMC_PMACRO_IB_DDLL_LONG_DQS_RANK0_1);
	writel(params->emc_pmacro_ib_ddll_long_dqs_rank0_2, emc + EMC_PMACRO_IB_DDLL_LONG_DQS_RANK0_2);
	writel(params->emc_pmacro_ib_ddll_long_dqs_rank0_3, emc + EMC_PMACRO_IB_DDLL_LONG_DQS_RANK0_3);
	/*
	writel(params->emc_pmacro_ib_ddll_long_dqs_rank0_4, emc + EMC_PMACRO_IB_DDLL_LONG_DQS_RANK0_4);
	writel(params->emc_pmacro_ib_ddll_long_dqs_rank0_5, emc + EMC_PMACRO_IB_DDLL_LONG_DQS_RANK0_5);
	*/
	writel(params->emc_pmacro_ib_ddll_long_dqs_rank1_0, emc + EMC_PMACRO_IB_DDLL_LONG_DQS_RANK1_0);
	writel(params->emc_pmacro_ib_ddll_long_dqs_rank1_1, emc + EMC_PMACRO_IB_DDLL_LONG_DQS_RANK1_1);
	writel(params->emc_pmacro_ib_ddll_long_dqs_rank1_2, emc + EMC_PMACRO_IB_DDLL_LONG_DQS_RANK1_2);
	writel(params->emc_pmacro_ib_ddll_long_dqs_rank1_3, emc + EMC_PMACRO_IB_DDLL_LONG_DQS_RANK1_3);
	/*
	writel(params->emc_pmacro_ib_ddll_long_dqs_rank1_4, emc + EMC_PMACRO_IB_DDLL_LONG_DQS_RANK1_4);
	writel(params->emc_pmacro_ib_ddll_long_dqs_rank1_5, emc + EMC_PMACRO_IB_DDLL_LONG_DQS_RANK1_5);
	*/
	writel(params->emc_pmacro_ddll_long_cmd_0, emc + EMC_PMACRO_DDLL_LONG_CMD_0);
	writel(params->emc_pmacro_ddll_long_cmd_1, emc + EMC_PMACRO_DDLL_LONG_CMD_1);
	writel(params->emc_pmacro_ddll_long_cmd_2, emc + EMC_PMACRO_DDLL_LONG_CMD_2);
	writel(params->emc_pmacro_ddll_long_cmd_3, emc + EMC_PMACRO_DDLL_LONG_CMD_3);
	writel(params->emc_pmacro_ddll_long_cmd_4, emc + EMC_PMACRO_DDLL_LONG_CMD_4);
	writel(params->emc_pmacro_ddll_short_cmd_0, emc + EMC_PMACRO_DDLL_SHORT_CMD_0);
	writel(params->emc_pmacro_ddll_short_cmd_1, emc + EMC_PMACRO_DDLL_SHORT_CMD_1);
	writel(params->emc_pmacro_ddll_short_cmd_2, emc + EMC_PMACRO_DDLL_SHORT_CMD_2);
#if 0
	writel(params->emc_pmacro_common_pad_tx_ctrl, emc + EMC_PMACRO_COMMON_PAD_TX_CTRL);
#else
	/* XXX */
	value = params->emc_pmacro_common_pad_tx_ctrl;
	value = 0xe;
	writel(value, emc + EMC_PMACRO_COMMON_PAD_TX_CTRL);
#endif

	if (params->emc_bct_spare4)
		writel(params->emc_bct_spare5, params->emc_bct_spare4);

	writel(EMC_TIMING_CONTROL_UPDATE, emc + EMC_TIMING_CONTROL);

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
	writel(params->mc_emem_adr_cfg_channel_mask, mc + MC_EMEM_ADR_CFG_CHANNEL_MASK);
	writel(params->mc_emem_adr_cfg_bank_mask_0, mc + MC_EMEM_ADR_CFG_BANK_MASK_0);
	writel(params->mc_emem_adr_cfg_bank_mask_1, mc + MC_EMEM_ADR_CFG_BANK_MASK_1);
	writel(params->mc_emem_adr_cfg_bank_mask_2, mc + MC_EMEM_ADR_CFG_BANK_MASK_2);
	writel(params->mc_emem_cfg, mc + MC_EMEM_CFG);

	writel(params->mc_sec_carveout_bom, mc + MC_SEC_CARVEOUT_BOM);
	writel(params->mc_sec_carveout_adr_hi, mc + MC_SEC_CARVEOUT_ADR_HI);
	writel(params->mc_sec_carveout_size_mb, mc + MC_SEC_CARVEOUT_SIZE_MB);
	writel(params->mc_mts_carveout_bom, mc + MC_MTS_CARVEOUT_BOM);
	writel(params->mc_mts_carveout_adr_hi, mc + MC_MTS_CARVEOUT_ADR_HI);
	writel(params->mc_mts_carveout_size_mb, mc + MC_MTS_CARVEOUT_SIZE_MB);

	writel(params->mc_emem_arb_cfg, mc + MC_EMEM_ARB_CFG);
	writel(params->mc_emem_arb_outstanding_req, mc + MC_EMEM_ARB_OUTSTANDING_REQ);
	writel(params->mc_emem_arb_refpb_hp_ctrl, mc + MC_EMEM_ARB_REFPB_HP_CTRL);
	writel(params->mc_emem_arb_refpb_bank_ctrl, mc + MC_EMEM_ARB_REFPB_BANK_CTRL);
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
	writel(params->mc_emem_arb_timing_ccdmw, mc + MC_EMEM_ARB_TIMING_CCDMW);
	writel(params->mc_emem_arb_timing_r2w, mc + MC_EMEM_ARB_TIMING_R2W);
	writel(params->mc_emem_arb_timing_w2r, mc + MC_EMEM_ARB_TIMING_W2R);
	writel(params->mc_emem_arb_timing_rfcpb, mc + MC_EMEM_ARB_TIMING_RFCPB);
	writel(params->mc_emem_arb_da_turns, mc + MC_EMEM_ARB_DA_TURNS);
	writel(params->mc_emem_arb_da_covers, mc + MC_EMEM_ARB_DA_COVERS);
	writel(params->mc_emem_arb_misc0, mc + MC_EMEM_ARB_MISC0);
	writel(params->mc_emem_arb_misc1, mc + MC_EMEM_ARB_MISC1);
	writel(params->mc_emem_arb_misc2, mc + MC_EMEM_ARB_MISC2);
	writel(params->mc_emem_arb_ring1_throttle, mc + MC_EMEM_ARB_RING1_THROTTLE);
	writel(params->mc_emem_arb_override_0, mc + MC_EMEM_ARB_OVERRIDE_0);
	writel(params->mc_emem_arb_override_1, mc + MC_EMEM_ARB_OVERRIDE_1);
	writel(params->mc_emem_arb_rsv, mc + MC_EMEM_ARB_RSV);
	writel(params->mc_da_config0, mc + MC_DA_CONFIG0);
	writel(MC_TIMING_CONTROL_UPDATE, mc + MC_TIMING_CONTROL);

	writel(params->mc_clken_override, mc + MC_CLKEN_OVERRIDE);
	writel(params->mc_stat_control, mc + MC_STAT_CONTROL);

	writel(params->emc_adr_cfg, emc + EMC_ADR_CFG);
	writel(params->emc_clken_override, emc + EMC_CLKEN_OVERRIDE);

	writel(params->emc_pmacro_autocal_cfg_0, emc + EMC_PMACRO_AUTOCAL_CFG_0);
	writel(params->emc_pmacro_autocal_cfg_1, emc + EMC_PMACRO_AUTOCAL_CFG_1);
	writel(params->emc_pmacro_autocal_cfg_2, emc + EMC_PMACRO_AUTOCAL_CFG_2);
	writel(params->emc_auto_cal_vref_sel_0, emc + EMC_AUTO_CAL_VREF_SEL_0);
	writel(params->emc_auto_cal_vref_sel_1, emc + EMC_AUTO_CAL_VREF_SEL_1);
	writel(params->emc_auto_cal_interval, emc + EMC_AUTO_CAL_INTERVAL);

	writel(params->emc_auto_cal_config, emc + EMC_AUTO_CAL_CONFIG);
	udelay(params->emc_auto_cal_wait);

	if (params->emc_bct_spare8)
		writel(params->emc_bct_spare9, params->emc_bct_spare8);

	writel(params->emc_cfg_2, emc + EMC_CFG_2);
	writel(params->emc_cfg_pipe, emc + EMC_CFG_PIPE);
	writel(params->emc_cfg_pipe_1, emc + EMC_CFG_PIPE_1);
	writel(params->emc_cfg_pipe_2, emc + EMC_CFG_PIPE_2);
	writel(params->emc_cmdq, emc + EMC_CMDQ);
	writel(params->emc_mc2emcq, emc + EMC_MC2EMCQ);
	writel(params->emc_mrs_wait_cnt, emc + EMC_MRS_WAIT_CNT);
	writel(params->emc_mrs_wait_cnt2, emc + EMC_MRS_WAIT_CNT2);
	writel(params->emc_fbio_cfg5, emc + EMC_FBIO_CFG5);

	writel(params->emc_rc, emc + EMC_RC);
	writel(params->emc_rfc, emc + EMC_RFC);
	writel(params->emc_rfcpb, emc + EMC_RFCPB);
	writel(params->emc_refctrl2, emc + EMC_REFCTRL2);
	writel(params->emc_rfc_slr, emc + EMC_RFC_SLR);
	writel(params->emc_ras, emc + EMC_RAS);
	writel(params->emc_rp, emc + EMC_RP);
	writel(params->emc_tppd, emc + EMC_TPPD);
	writel(params->emc_r2r, emc + EMC_R2R);
	writel(params->emc_w2w, emc + EMC_W2W);
	writel(params->emc_r2w, emc + EMC_R2W);
	writel(params->emc_w2r, emc + EMC_W2R);
	writel(params->emc_r2p, emc + EMC_R2P);
	writel(params->emc_w2p, emc + EMC_W2P);
	writel(params->emc_ccdmw, emc + EMC_CCDMW);
	writel(params->emc_rd_rcd, emc + EMC_RD_RCD);
	writel(params->emc_wr_rcd, emc + EMC_WR_RCD);
	writel(params->emc_rrd, emc + EMC_RRD);
	writel(params->emc_rext, emc + EMC_REXT);
	writel(params->emc_wext, emc + EMC_WEXT);
	writel(params->emc_wdv, emc + EMC_WDV);
	writel(params->emc_wdv_chk, emc + EMC_WDV_CHK);
	writel(params->emc_wsv, emc + EMC_WSV);
	writel(params->emc_wev, emc + EMC_WEV);
	writel(params->emc_wdv_mask, emc + EMC_WDV_MASK);
	writel(params->emc_ws_duration, emc + EMC_WS_DURATION);
	writel(params->emc_we_duration, emc + EMC_WE_DURATION);
	writel(params->emc_quse, emc + EMC_QUSE);
	writel(params->emc_quse_width, emc + EMC_QUSE_WIDTH);
	writel(params->emc_ibdly, emc + EMC_IBDLY);
	writel(params->emc_obdly, emc + EMC_OBDLY);
	writel(params->emc_einput, emc + EMC_EINPUT);
	writel(params->emc_einput_duration, emc + EMC_EINPUT_DURATION);
	writel(params->emc_puterm_extra, emc + EMC_PUTERM_EXTRA);
	writel(params->emc_puterm_width, emc + EMC_PUTERM_WIDTH);

	writel(params->emc_pmacro_common_pad_tx_ctrl, emc + EMC_PMACRO_COMMON_PAD_TX_CTRL);
	writel(params->emc_dbg, emc + EMC_DBG);
	writel(params->emc_qrst, emc + EMC_QRST);
	writel(1, emc + EMC_ISSUE_QRST);
	writel(0, emc + EMC_ISSUE_QRST);
	writel(params->emc_qsafe, emc + EMC_QSAFE);
	writel(params->emc_rdv, emc + EMC_RDV);
	writel(params->emc_rdv_mask, emc + EMC_RDV_MASK);
	writel(params->emc_rdv_early, emc + EMC_RDV_EARLY);
	writel(params->emc_rdv_early_mask, emc + EMC_RDV_EARLY_MASK);
	writel(params->emc_qpop, emc + EMC_QPOP);
	writel(params->emc_refresh, emc + EMC_REFRESH);
	writel(params->emc_burst_refresh_num, emc + EMC_BURST_REFRESH_NUM);
	writel(params->emc_pre_refresh_req_cnt, emc + EMC_PRE_REFRESH_REQ_CNT);
	writel(params->emc_pdex2wr, emc + EMC_PDEX2WR);
	writel(params->emc_pdex2rd, emc + EMC_PDEX2RD);
	writel(params->emc_pchg2pden, emc + EMC_PCHG2PDEN);
	writel(params->emc_act2pden, emc + EMC_ACT2PDEN);
	writel(params->emc_ar2pden, emc + EMC_AR2PDEN);
	writel(params->emc_rw2pden, emc + EMC_RW2PDEN);
	writel(params->emc_cke2pden, emc + EMC_CKE2PDEN);
	writel(params->emc_pdex2cke, emc + EMC_PDEX2CKE);
	writel(params->emc_pdex2mrr, emc + EMC_PDEX2MRR);
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
	writel(params->emc_cfg_dig_dll, emc + EMC_CFG_DIG_DLL);
	writel(params->emc_cfg_dig_dll_period, emc + EMC_CFG_DIG_DLL_PERIOD);
	writel(params->emc_fbio_spare & 0xfffffffd, emc + EMC_FBIO_SPARE);
	writel(params->emc_cfg_rsv, emc + EMC_CFG_RSV);
	writel(params->emc_pmc_scratch1, emc + EMC_PMC_SCRATCH1);
	writel(params->emc_pmc_scratch2, emc + EMC_PMC_SCRATCH2);
	writel(params->emc_pmc_scratch3, emc + EMC_PMC_SCRATCH3);
	writel(params->emc_acpd_control, emc + EMC_ACPD_CONTROL);
	writel(params->emc_txdsrvttgen, emc + EMC_TXDSRVTTGEN);

	value = 0x03c00000 | (params->emc_cfg & 0xe);
	writel(value, emc + EMC_CFG);

	if (params->boot_rom_patch_control & (1 << 31)) {
		value = 0x70000000 + (params->boot_rom_patch_control << 2);
		writel(params->boot_rom_patch_data, value);
		writel(MC_TIMING_CONTROL_UPDATE, mc + MC_TIMING_CONTROL);
	}

	value = ((params->emc_pmc_scratch1 & 0x3fffffff) | 1 << 30) & 0xcfff0000;
	writel(value, pmc + PMC_IO_DPD3_REQ);
	udelay(params->pmc_io_dpd3_req_wait);

	if (params->emc_auto_cal_interval == 0) {
		value = params->emc_auto_cal_config | (1 << 9);
		writel(value, emc + EMC_AUTO_CAL_CONFIG);
	}

	writel(params->emc_pmacro_brick_ctrl_rfu2, emc + EMC_PMACRO_BRICK_CTRL_RFU2);

	if (warmboot) {
		/* XXX */
	} else {
		if (params->emc_zcal_warm_cold_boot_enables & EMC_ZCAL_BOOT_ENABLE_COLD) {
			if (params->memory_type == LPDDR4) {
				writel(params->emc_zcal_wait_cnt, emc + EMC_ZCAL_WAIT_CNT);
				writel(params->emc_zcal_mrw_cmd, emc + EMC_ZCAL_MRW_CMD);
			} else {
				/* XXX */
			}
		}
	}

	writel(EMC_TIMING_CONTROL_UPDATE, emc + EMC_TIMING_CONTROL);
	udelay(params->emc_timing_control_wait);

	value = readl(pmc + PMC_DDR_CNTRL);
	value &= ~0x0007ff80;
	writel(value, pmc + PMC_DDR_CNTRL);
	udelay(params->pmc_ddr_cntrl_wait);

	if (!warmboot) {
		if (params->memory_type == LPDDR4) {
			value = params->emc_pin_gpio_en << 16 |
				params->emc_pin_gpio << 12 |
				0 << 8 | 0 << 4 | 0 << 0;
			writel(value, emc + EMC_PIN);
			(void)readl(emc + EMC_PIN);

			udelay(200 + params->emc_pin_extra_wait);

			value = params->emc_pin_gpio_en << 16 |
				params->emc_pin_gpio << 12 |
				1 << 8 | 0 << 4 | 0 << 0;
			writel(value, emc + EMC_PIN);
			(void)readl(emc + EMC_PIN);

			udelay(2000 + params->emc_pin_extra_wait);
		}
	}

	value = params->emc_pin_gpio_en << 16 |
		params->emc_pin_gpio << 12 |
		1 << 8 | 0 << 4 | 1 << 0;
	writel(value, emc + EMC_PIN);
	(void)readl(emc + EMC_PIN);

	udelay(params->emc_pin_program_wait);

	if (warmboot && params->memory_type == LPDDR4)
		writel(0, emc + EMC_SELF_REF);

	if (params->memory_type != LPDDR4) {
		value = (params->emc_dev_select << 30) | 0x1;
		writel(value, emc + EMC_NOP);
	}

	if (!warmboot) {
		/* XXX wait 200 us for LPDDR, LPDDR2 and LPDDR3 */
		if (0)
			udelay(200 + params->emc_pin_extra_wait);
	}

	if (warmboot) {
		udelay(params->warm_boot_wait);

		/* XXX mode register writes */
	} else {
		switch (params->memory_type) {
		case LPDDR4:
			/* XXX init LPDDR4 */
			if (params->emc_bct_spare10)
				writel(params->emc_bct_spare11, params->emc_bct_spare10);

			writel(params->emc_mrw2, emc + EMC_MRW2);
			writel(params->emc_mrw1, emc + EMC_MRW);
			writel(params->emc_mrw3, emc + EMC_MRW3);
			writel(params->emc_mrw4, emc + EMC_MRW4);
			writel(params->emc_mrw6, emc + EMC_MRW6);
			writel(params->emc_mrw14, emc + EMC_MRW14);

			writel(params->emc_mrw8, emc + EMC_MRW8);
			writel(params->emc_mrw12, emc + EMC_MRW12);
			writel(params->emc_mrw9, emc + EMC_MRW9);
			writel(params->emc_mrw13, emc + EMC_MRW13);

			if (params->emc_zcal_warm_cold_boot_enables & EMC_ZCAL_BOOT_ENABLE_COLD) {
				writel(params->emc_zcal_init_dev0, emc + EMC_ZQ_CAL);
				udelay(params->emc_zcal_init_wait);
				writel(params->emc_zcal_init_dev0 ^ 0x3, emc + EMC_ZQ_CAL);

				if ((params->emc_dev_select & 2) == 0) {
					writel(params->emc_zcal_init_dev1, emc + EMC_ZQ_CAL);
					udelay(params->emc_zcal_init_wait);
					writel(params->emc_zcal_init_dev1 ^ 0x3, emc + EMC_ZQ_CAL);
				}
			}
			break;
		}

		writel(params->pmc_ddr_cfg, pmc + PMC_DDR_CFG);
	}

	/* XXX also for LPDDR2, LPDDR3 and DDR3? */
	if (params->memory_type == LPDDR4) {
		writel(params->emc_zcal_interval, emc + EMC_ZCAL_INTERVAL);
		writel(params->emc_zcal_wait_cnt, emc + EMC_ZCAL_WAIT_CNT);
		writel(params->emc_zcal_mrw_cmd, emc + EMC_ZCAL_MRW_CMD);
	}

	if (params->emc_bct_spare12)
		writel(params->emc_bct_spare13, params->emc_bct_spare12);

	writel(EMC_TIMING_CONTROL_UPDATE, emc + EMC_TIMING_CONTROL);

	if (params->emc_extra_refresh_num > 0) {
		value = params->emc_dev_select << 30 |
			((1 << params->emc_extra_refresh_num) - 1) << 8 |
			1 << 1 | 1 << 0;
		writel(value, emc + EMC_REF);
	}

	writel(1 << 31 | params->emc_dev_select << 0, emc + EMC_REFCTRL);

	writel(params->emc_dyn_self_ref_control, emc + EMC_DYN_SELF_REF_CONTROL);
	writel(params->emc_cfg_update, emc + EMC_CFG_UPDATE);
	writel(params->emc_cfg, emc + EMC_CFG);
	writel(params->emc_fdpd_ctrl_dq, emc + EMC_FDPD_CTRL_DQ);
	writel(params->emc_fdpd_ctrl_cmd, emc + EMC_FDPD_CTRL_CMD);
	writel(params->emc_sel_dpd_ctrl, emc + EMC_SEL_DPD_CTRL);

	writel(params->emc_fbio_spare, emc + EMC_FBIO_SPARE);
	writel(EMC_TIMING_CONTROL_UPDATE, emc + EMC_TIMING_CONTROL);

	writel(params->emc_cfg_pipe_clk, emc + EMC_CFG_PIPE_CLK);
	writel(params->emc_fdpd_ctrl_cmd_no_ramp, emc + EMC_FDPD_CTRL_CMD_NO_RAMP);

	value = readl(arb + ARB_XBAR_CTRL);
	value |= ARB_XBAR_CTRL_MEM_INIT_DONE;
	writel(value, arb + ARB_XBAR_CTRL);

	writel(params->mc_video_protect_ctrl, mc + MC_VIDEO_PROTECT_CTRL);
	writel(params->mc_sec_carveout_ctrl, mc + MC_SEC_CARVEOUT_CTRL);
	writel(params->mc_mts_carveout_ctrl, mc + MC_MTS_CARVEOUT_CTRL);

	for (i = 0; i < 5; i++) {
		writel(params->mc_security_carveout[i].bom, mc + MC_SECURITY_CARVEOUT_BOM(i));
		writel(params->mc_security_carveout[i].bom_hi, mc + MC_SECURITY_CARVEOUT_BOM_HI(i));
		writel(params->mc_security_carveout[i].size, mc + MC_SECURITY_CARVEOUT_SIZE(i));
		writel(params->mc_security_carveout[i].client_access0, mc + MC_SECURITY_CARVEOUT_CLIENT_ACCESS0(i));
		writel(params->mc_security_carveout[i].client_access1, mc + MC_SECURITY_CARVEOUT_CLIENT_ACCESS1(i));
		writel(params->mc_security_carveout[i].client_access2, mc + MC_SECURITY_CARVEOUT_CLIENT_ACCESS2(i));
		writel(params->mc_security_carveout[i].client_access3, mc + MC_SECURITY_CARVEOUT_CLIENT_ACCESS3(i));
		writel(params->mc_security_carveout[i].client_access4, mc + MC_SECURITY_CARVEOUT_CLIENT_ACCESS4(i));
		writel(params->mc_security_carveout[i].client_force_internal_access0, mc + MC_SECURITY_CARVEOUT_CLIENT_FORCE_INTERNAL_ACCESS0(i));
		writel(params->mc_security_carveout[i].client_force_internal_access1, mc + MC_SECURITY_CARVEOUT_CLIENT_FORCE_INTERNAL_ACCESS1(i));
		writel(params->mc_security_carveout[i].client_force_internal_access2, mc + MC_SECURITY_CARVEOUT_CLIENT_FORCE_INTERNAL_ACCESS2(i));
		writel(params->mc_security_carveout[i].client_force_internal_access3, mc + MC_SECURITY_CARVEOUT_CLIENT_FORCE_INTERNAL_ACCESS3(i));
		writel(params->mc_security_carveout[i].client_force_internal_access4, mc + MC_SECURITY_CARVEOUT_CLIENT_FORCE_INTERNAL_ACCESS4(i));
		writel(params->mc_security_carveout[i].cfg, mc + MC_SECURITY_CARVEOUT_CFG(i));
	}

	/* lock EMEM configuration registers */
	writel(MC_EMEM_CFG_ACCESS_CTRL_DISABLE, mc + MC_EMEM_CFG_ACCESS_CTRL);

	/* XXX verify SDRAM */
}
