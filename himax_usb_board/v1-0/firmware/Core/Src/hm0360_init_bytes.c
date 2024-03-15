#include "hm0360_init_bytes.h"

#if 0
const hm0360_reg_write_t hm0360_init_values[] = {
    // enable test patterns
    //{0x0601, (2 << 4) | (1 << 0)},

    // pclk polarity
    {0x3112, (1 << 2)},

    // non-gated pclk
    {0x1014, 0x0b},

    // pclko divider
    {0x0300, 0x08},

    {0x0100, 0x01}
};
#endif

#define         MODEL_ID_H                      0x0000
#define         MODEL_ID_L                      0x0001
#define         SILICON_REV                     0x0002
#define         FRAME_COUNT_H                   0x0005
#define         FRAME_COUNT_L                   0x0006
#define         PIXEL_ORDER                     0x0007
// Sensor mode control
#define         MODE_SELECT                     0x0100
#define         IMG_ORIENTATION                 0x0101
#define         EMBEDDED_LINE_EN                0x0102
#define         SW_RESET                        0x0103
#define         COMMAND_UPDATE                  0x0104
// Sensor exposure gain control
#define         INTEGRATION_H                   0x0202
#define         INTEGRATION_L                   0x0203
#define         ANALOG_GAIN                     0x0205
#define         DIGITAL_GAIN_H                  0x020E
#define         DIGITAL_GAIN_L                  0x020F
// Clock control
#define         PLL1_CONFIG                     0x0300
#define         PLL2_CONFIG                     0x0301
#define         PLL3_CONFIG                     0x0302
// Frame timing control
#define         FRAME_LEN_LINES_H               0x0340
#define         FRAME_LEN_LINES_L               0x0341
#define         LINE_LEN_PCK_H                  0x0342
#define         LINE_LEN_PCK_L                  0x0343
// Monochrome programming
#define         MONO_MODE                       0x0370
#define         MONO_MODE_ISP                   0x0371
#define         MONO_MODE_SEL                   0x0372
// Binning mode control
#define         H_SUBSAMPLE                     0x0380
#define         V_SUBSAMPLE                     0x0381
#define         BINNING_MODE                    0x0382
// Test pattern control
#define         TEST_PATTERN_MODE               0x0601
// Black level control
#define         BLC_TGT                         0x1004
#define         BLC2_TGT                        0x1009
#define         MONO_CTRL                       0x100A
// VSYNC / HSYNC / pixel shift registers
#define         OPFM_CTRL                       0x1014
// Tone mapping registers
#define         CMPRS_CTRL                      0x102F
#define         CMPRS_01                        0x1030
#define         CMPRS_02                        0x1031
#define         CMPRS_03                        0x1032
#define         CMPRS_04                        0x1033
#define         CMPRS_05                        0x1034
#define         CMPRS_06                        0x1035
#define         CMPRS_07                        0x1036
#define         CMPRS_08                        0x1037
#define         CMPRS_09                        0x1038
#define         CMPRS_10                        0x1039
#define         CMPRS_11                        0x103A
#define         CMPRS_12                        0x103B
#define         CMPRS_13                        0x103C
#define         CMPRS_14                        0x103D
#define         CMPRS_15                        0x103E
#define         CMPRS_16                        0x103F
// Automatic exposure control
#define         AE_CTRL                         0x2000
#define         AE_CTRL1                        0x2001
#define         CNT_ORGH_H                      0x2002
#define         CNT_ORGH_L                      0x2003
#define         CNT_ORGV_H                      0x2004
#define         CNT_ORGV_L                      0x2005
#define         CNT_STH_H                       0x2006
#define         CNT_STH_L                       0x2007
#define         CNT_STV_H                       0x2008
#define         CNT_STV_L                       0x2009
#define         CTRL_PG_SKIPCNT                 0x200A
#define         BV_WIN_WEIGHT_EN                0x200D
#define         MAX_INTG_H                      0x2029
#define         MAX_INTG_L                      0x202A
#define         MAX_AGAIN                       0x202B
#define         MAX_DGAIN_H                     0x202C
#define         MAX_DGAIN_L                     0x202D
#define         MIN_INTG                        0x202E
#define         MIN_AGAIN                       0x202F
#define         MIN_DGAIN                       0x2030
#define         T_DAMPING                       0x2031
#define         N_DAMPING                       0x2032
#define         ALC_TH                          0x2033
#define         AE_TARGET_MEAN                  0x2034
#define         AE_MIN_MEAN                     0x2035
#define         AE_TARGET_ZONE                  0x2036
#define         CONVERGE_IN_TH                  0x2037
#define         CONVERGE_OUT_TH                 0x2038
#define         FS_CTRL                         0x203B
#define         FS_60HZ_H                       0x203C
#define         FS_60HZ_L                       0x203D
#define         FS_50HZ_H                       0x203E
#define         FS_50HZ_L                       0x203F
#define         FRAME_CNT_TH                    0x205B
#define         AE_MEAN                         0x205D
#define         AE_CONVERGE                     0x2060
#define         AE_BLI_TGT                      0x2070
// Interrupt control
#define         PULSE_MODE                      0x2061
#define         PULSE_TH_H                      0x2062
#define         PULSE_TH_L                      0x2063
#define         INT_INDIC                       0x2064
#define         INT_CLEAR                       0x2065
// Motion detection control
#define         MD_CTRL                         0x2080
#define         ROI_START_END_V                 0x2081
#define         ROI_START_END_H                 0x2082
#define         MD_TH_MIN                       0x2083
#define         MD_TH_STR_L                     0x2084
#define         MD_TH_STR_H                     0x2085
#define         MD_LIGHT_COEF                   0x2099
#define         MD_BLOCK_NUM_TH                 0x209B
#define         MD_LATENCY                      0x209C
#define         MD_LATENCY_TH                   0x209D
#define         MD_CTRL1                        0x209E
// Context switch control registers
#define         PMU_CFG_3                       0x3024
#define         PMU_CFG_4                       0x3025
// Operation mode control
#define         WIN_MODE                        0x3030
// IO and clock control
#define         PAD_REGISTER_07                 0x3112

// Register bits/values
#define         HIMAX_RESET                     0x01
#define         HIMAX_MODE_STANDBY              0x00
#define         HIMAX_MODE_STREAMING            0x01     // I2C triggered streaming enable
#define         HIMAX_MODE_STREAMING_NFRAMES    0x03     // Output N frames
#define         HIMAX_MODE_STREAMING_TRIG       0x05     // Hardware Trigger
#define         HIMAX_SET_HMIRROR(r, x)         ((r&0xFE)|((x&1)<<0))
#define         HIMAX_SET_VMIRROR(r, x)         ((r&0xFD)|((x&1)<<1))

#define         PCLK_RISING_EDGE                0x00
#define         PCLK_FALLING_EDGE               0x01
#define         AE_CTRL_ENABLE                  0x00
#define         AE_CTRL_DISABLE                 0x01

/**
 * @}
 */
/* Private variables ---------------------------------------------------------*/

/** @defgroup GAPUINO_HIMAX_Private_Variables I2C Private Variables
 * @{
 */
#define HIMAX_BOOT_RETRY            (10)
#define HIMAX_LINE_LEN_PCK_VGA      0x300
#define HIMAX_FRAME_LENGTH_VGA      0x214

#define HIMAX_LINE_LEN_PCK_QVGA     0x178
#define HIMAX_FRAME_LENGTH_QVGA     0x109

#define HIMAX_LINE_LEN_PCK_QQVGA    0x178
#define HIMAX_FRAME_LENGTH_QQVGA    0x084

#define HIMAX_MD_ROI_VGA_W          40
#define HIMAX_MD_ROI_VGA_H          30

#define HIMAX_MD_ROI_QVGA_W         20
#define HIMAX_MD_ROI_QVGA_H         15

#define HIMAX_MD_ROI_QQVGA_W        10
#define HIMAX_MD_ROI_QQVGA_H        8

#ifndef MIN
#define MIN(a, b)  (((a) < (b)) ? (a) : (b))
#endif
#ifndef MAX
#define MAX(a, b)  (((a) > (b)) ? (a) : (b))
#endif

enum {
    HIMAX_Standby    = 0x0,
    HIMAX_Streaming  = 0x1,       // I2C triggered streaming enable
    HIMAX_Streaming2 = 0x3,       // Output N frames
    HIMAX_Streaming3 = 0x5        // Hardware Trigger
};

const hm0360_reg_write_t hm0360_init_values[] = {
    {SW_RESET,          0x00},
    {MONO_MODE,         0x00},
    {MONO_MODE_ISP,     0x01},
    {MONO_MODE_SEL,     0x01},

    // BLC control
    {0x1000,            0x01},
    {0x1003,            0x04},
    {BLC_TGT,           0x04},
    {0x1007,            0x01},
    {0x1008,            0x04},
    {BLC2_TGT,          0x04},
    {MONO_CTRL,         0x03},

    // Output format control
    {OPFM_CTRL,         0x0C},

    // Reserved regs
    {0x101D,            0x00},
    {0x101E,            0x01},
    {0x101F,            0x00},
    {0x1020,            0x01},
    {0x1021,            0x00},

    {CMPRS_CTRL,        0x00},
    {CMPRS_01,          0x09},
    {CMPRS_02,          0x12},
    {CMPRS_03,          0x23},
    {CMPRS_04,          0x31},
    {CMPRS_05,          0x3E},
    {CMPRS_06,          0x4B},
    {CMPRS_07,          0x56},
    {CMPRS_08,          0x5E},
    {CMPRS_09,          0x65},
    {CMPRS_10,          0x72},
    {CMPRS_11,          0x7F},
    {CMPRS_12,          0x8C},
    {CMPRS_13,          0x98},
    {CMPRS_14,          0xB2},
    {CMPRS_15,          0xCC},
    {CMPRS_16,          0xE6},

    {0x3112,            0x00},  // PCLKO_polarity falling

    {PLL1_CONFIG,       0x0b},  // Core = 24MHz PCLKO = 24MHz I2C = 12MHz
    {PLL2_CONFIG,       0x0A},  // MIPI pre-dev (default)
    {PLL3_CONFIG,       0x77},  // PMU/MIPI pre-dev (default)

    {PMU_CFG_3,         0x08},  // Disable context switching
    {PAD_REGISTER_07,   0x00},  // PCLKO_polarity falling

    {AE_CTRL,           0x5F},  // Automatic Exposure (NOTE: Auto framerate enabled)
    {AE_CTRL1,          0x00},
    {T_DAMPING,         0x20},  // AE T damping factor
    {N_DAMPING,         0x00},  // AE N damping factor
    {AE_TARGET_MEAN,    0x64},  // AE target
    {AE_MIN_MEAN,       0x0A},  // AE min target mean
    {AE_TARGET_ZONE,    0x23},  // AE target zone
    {CONVERGE_IN_TH,    0x03},  // AE converge in threshold
    {CONVERGE_OUT_TH,   0x05},  // AE converge out threshold
    {MAX_INTG_H,        (HIMAX_FRAME_LENGTH_QVGA-4)>>8},
    {MAX_INTG_L,        (HIMAX_FRAME_LENGTH_QVGA-4)&0xFF},

    {MAX_AGAIN,         0x04},  // Maximum analog gain
    {MAX_DGAIN_H,       0x03},
    {MAX_DGAIN_L,       0x3F},
    {INTEGRATION_H,     0x01},
    {INTEGRATION_L,     0x08},

    {MD_CTRL,           0x6A},
    {MD_TH_MIN,         0x01},
    {MD_BLOCK_NUM_TH,   0x01},
    {MD_CTRL1,          0x06},
    {PULSE_MODE,        0x00},  // Interrupt in level mode.
    {ROI_START_END_V,   0xF0},
    {ROI_START_END_H,   0xF0},

    {FRAME_LEN_LINES_H, HIMAX_FRAME_LENGTH_QVGA>>8},
    {FRAME_LEN_LINES_L, HIMAX_FRAME_LENGTH_QVGA&0xFF},
    {LINE_LEN_PCK_H,    HIMAX_LINE_LEN_PCK_QVGA>>8},
    {LINE_LEN_PCK_L,    HIMAX_LINE_LEN_PCK_QVGA&0xFF},
    {H_SUBSAMPLE,       0x01},
    {V_SUBSAMPLE,       0x01},
    {BINNING_MODE,      0x00},
    {WIN_MODE,          0x00},
    {IMG_ORIENTATION,   0x00},
    {COMMAND_UPDATE,    0x01},

    /// SYNC function config.
    {0x3010,            0x00},
    {0x3013,            0x01},
    {0x3019,            0x00},
    {0x301A,            0x00},
    {0x301B,            0x20},
    {0x301C,            0xFF},

    // PREMETER config.
    {0x3026,            0x03},
    {0x3027,            0x81},
    {0x3028,            0x01},
    {0x3029,            0x00},
    {0x302A,            0x30},
    {0x302E,            0x00},
    {0x302F,            0x00},

    // Magic regs 🪄.
    {0x302B,            0x2A},
    {0x302C,            0x00},
    {0x302D,            0x03},
    {0x3031,            0x01},
    {0x3051,            0x00},
    {0x305C,            0x03},
    {0x3060,            0x00},
    {0x3061,            0xFA},
    {0x3062,            0xFF},
    {0x3063,            0xFF},
    {0x3064,            0xFF},
    {0x3065,            0xFF},
    {0x3066,            0xFF},
    {0x3067,            0xFF},
    {0x3068,            0xFF},
    {0x3069,            0xFF},
    {0x306A,            0xFF},
    {0x306B,            0xFF},
    {0x306C,            0xFF},
    {0x306D,            0xFF},
    {0x306E,            0xFF},
    {0x306F,            0xFF},
    {0x3070,            0xFF},
    {0x3071,            0xFF},
    {0x3072,            0xFF},
    {0x3073,            0xFF},
    {0x3074,            0xFF},
    {0x3075,            0xFF},
    {0x3076,            0xFF},
    {0x3077,            0xFF},
    {0x3078,            0xFF},
    {0x3079,            0xFF},
    {0x307A,            0xFF},
    {0x307B,            0xFF},
    {0x307C,            0xFF},
    {0x307D,            0xFF},
    {0x307E,            0xFF},
    {0x307F,            0xFF},
    {0x3080,            0x01},
    {0x3081,            0x01},
    {0x3082,            0x03},
    {0x3083,            0x20},
    {0x3084,            0x00},
    {0x3085,            0x20},
    {0x3086,            0x00},
    {0x3087,            0x20},
    {0x3088,            0x00},
    {0x3089,            0x04},
    {0x3094,            0x02},
    {0x3095,            0x02},
    {0x3096,            0x00},
    {0x3097,            0x02},
    {0x3098,            0x00},
    {0x3099,            0x02},
    {0x309E,            0x05},
    {0x309F,            0x02},
    {0x30A0,            0x02},
    {0x30A1,            0x00},
    {0x30A2,            0x08},
    {0x30A3,            0x00},
    {0x30A4,            0x20},
    {0x30A5,            0x04},
    {0x30A6,            0x02},
    {0x30A7,            0x02},
    {0x30A8,            0x01},
    {0x30A9,            0x00},
    {0x30AA,            0x02},
    {0x30AB,            0x34},
    {0x30B0,            0x03},
    {0x30C4,            0x10},
    {0x30C5,            0x01},
    {0x30C6,            0xBF},
    {0x30C7,            0x00},
    {0x30C8,            0x00},
    {0x30CB,            0xFF},
    {0x30CC,            0xFF},
    {0x30CD,            0x7F},
    {0x30CE,            0x7F},
    {0x30D3,            0x01},
    {0x30D4,            0xFF},
    {0x30D5,            0x00},
    {0x30D6,            0x40},
    {0x30D7,            0x00},
    {0x30D8,            0xA7},
    {0x30D9,            0x05},
    {0x30DA,            0x01},
    {0x30DB,            0x40},
    {0x30DC,            0x00},
    {0x30DD,            0x27},
    {0x30DE,            0x05},
    {0x30DF,            0x07},
    {0x30E0,            0x40},
    {0x30E1,            0x00},
    {0x30E2,            0x27},
    {0x30E3,            0x05},
    {0x30E4,            0x47},
    {0x30E5,            0x30},
    {0x30E6,            0x00},
    {0x30E7,            0x27},
    {0x30E8,            0x05},
    {0x30E9,            0x87},
    {0x30EA,            0x30},
    {0x30EB,            0x00},
    {0x30EC,            0x27},
    {0x30ED,            0x05},
    {0x30EE,            0x00},
    {0x30EF,            0x40},
    {0x30F0,            0x00},
    {0x30F1,            0xA7},
    {0x30F2,            0x05},
    {0x30F3,            0x01},
    {0x30F4,            0x40},
    {0x30F5,            0x00},
    {0x30F6,            0x27},
    {0x30F7,            0x05},
    {0x30F8,            0x07},
    {0x30F9,            0x40},
    {0x30FA,            0x00},
    {0x30FB,            0x27},
    {0x30FC,            0x05},
    {0x30FD,            0x47},
    {0x30FE,            0x30},
    {0x30FF,            0x00},
    {0x3100,            0x27},
    {0x3101,            0x05},
    {0x3102,            0x87},
    {0x3103,            0x30},
    {0x3104,            0x00},
    {0x3105,            0x27},
    {0x3106,            0x05},
    {0x310B,            0x10},
    {0x3113,            0xA0},
    {0x3114,            0x67},
    {0x3115,            0x42},
    {0x3116,            0x10},
    {0x3117,            0x0A},
    {0x3118,            0x3F},
    {0x311C,            0x10},
    {0x311D,            0x06},
    {0x311E,            0x0F},
    {0x311F,            0x0E},
    {0x3120,            0x0D},
    {0x3121,            0x0F},
    {0x3122,            0x00},
    {0x3123,            0x1D},
    {0x3126,            0x03},
    {0x3128,            0x57},
    {0x312A,            0x11},
    {0x312B,            0x41},
    {0x312E,            0x00},
    {0x312F,            0x00},
    {0x3130,            0x0C},
    {0x3141,            0x2A},
    {0x3142,            0x9F},
    {0x3147,            0x18},
    {0x3149,            0x18},
    {0x314B,            0x01},
    {0x3150,            0x50},
    {0x3152,            0x00},
    {0x3156,            0x2C},
    {0x315A,            0x0A},
    {0x315B,            0x2F},
    {0x315C,            0xE0},
    {0x315F,            0x02},
    {0x3160,            0x1F},
    {0x3163,            0x1F},
    {0x3164,            0x7F},
    {0x3165,            0x7F},
    {0x317B,            0x94},
    {0x317C,            0x00},
    {0x317D,            0x02},
    {0x318C,            0x00},

    // pclk polarity
    {0x3112, (1 << 2)},

    // non-gated pclk
    {0x1014, 0x0b},

    {COMMAND_UPDATE,    0x01},

    {0x0100, 0x01}
};
const int sizeof_hm0360_init_values = sizeof(hm0360_init_values);
