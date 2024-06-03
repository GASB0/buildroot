// SPDX-License-Identifier: GPL-2.0-only
/*
 * Driver for the fpga codec based on the pcm5102a and the tlv320adc3xxx drivers
 *
 * Author:	Gabriel Santos <GASB0@>
 *		Copyright 2024
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <linux/kgdb.h>
#include <sound/soc.h>
#include <linux/clk.h>

enum gowinxxxx_type {
        GW5A = 0,
        GW2A
};

struct gowinxxxx_priv {
	struct snd_soc_component *component;
	unsigned int dac_clk;
	struct i2c_client *i2c;
	/* u16 *reg_cache; */
};

static const struct regmap_range_cfg gowinxxxx_ranges[] = {
	{
		.range_min = 0,
		.range_max = 5*128,
		/* .selector_reg = ADC3XXX_PAGE_SELECT, */
		.selector_mask = 0xff,
		.selector_shift = 0,
		.window_start = 0,
		/* .window_len = ADC3XXX_PAGE_SIZE, */
	}
};

static const struct reg_default gowinxxxx_defaults[] =
    {
	/* Configuration and instruction registers */
	{0x30, 0xA0}, {0x31, 0x0F}, {0x32, 0x00},

	/* {0x33, 0x00}, {0x34, 0x00}, {0x35, 0x00}, */
 	/* {0x36, 0x00}, {0x37, 0x00}, {0x38, 0x00}, */
	/* {0x39, 0x00}, {0x3A, 0x00}, {0x3B, 0x00}, */
	/* {0x3C, 0x00}, {0x3D, 0x00}, {0x3E, 0x00}, */
	/* {0x3F, 0x00}, */

	/* Data buffer  */
	{0x40, 0x00}, {0x41, 0x00}, {0x42, 0x00}, {0x43, 0x00},	{0x44, 0x00},

	/* Address buffer */
	{0x48, 0x00}, {0x49, 0x00}, {0x4A, 0x00}, {0x4B, 0x00},
    };

static const struct regmap_config gowinxxxx_regmap = {
    .reg_bits = 8,
    .val_bits = 8,

    /* .writable_reg=, */
    /* .readable_reg=, */
    /* .precious_reg=, */
    /* .volatile_reg = , */

    /* .reg_defaults = gowinxxxx_defaults, */
    /* .num_reg_defaults = ARRAY_SIZE(gowinxxxx_defaults), */
    /* .cache_type = REGCACHE_RBTREE, */
    /* .ranges = gowinxxxx_ranges, */
    /* .num_ranges = ARRAY_SIZE(gowinxxxx_ranges), */
    .max_register = 0xffff,
};

static struct snd_soc_dai_driver fpga_dai = {
    .name = "bare-hifi",
    .playback = {
                 .channels_min = 2,
                 .channels_max = 2,
                 .rates = SNDRV_PCM_RATE_192000,
                 .formats = SNDRV_PCM_FMTBIT_S16_LE |
		            SNDRV_PCM_FMTBIT_S24_LE |
                            SNDRV_PCM_FMTBIT_S32_LE
    },
    .capture = {
                .channels_min = 2,
                .channels_max = 2,
                .rates = SNDRV_PCM_RATE_192000,
                .formats = SNDRV_PCM_FMTBIT_S16_LE |
		           SNDRV_PCM_FMTBIT_S24_LE |
                           SNDRV_PCM_FMTBIT_S32_LE
    },
};

static int fpga_coefficient_get(struct snd_kcontrol *kcontrol,
				   struct snd_ctl_elem_value *ucontrol)
{
	/* struct snd_soc_component *component = snd_soc_kcontrol_component(kcontrol); */
	/* struct gowinxxxx *gowinxxxx = snd_soc_component_get_drvdata(component); */

        /* int regval; */
	/* regmap_read(gowinxxxx->regmap, 0x01, &regval); */

  return 0;
}

static int fpga_coefficient_put(struct snd_kcontrol *kcontrol,
				   struct snd_ctl_elem_value *ucontrol)
{
	/* struct snd_soc_component *component = snd_soc_kcontrol_component(kcontrol); */
	/* struct gowinxxxx *gowinxxxx = snd_soc_component_get_drvdata(component); */
  return 0;
}

static int fpga_coefficient_info(struct snd_kcontrol *kcontrol,
                                 struct snd_ctl_elem_info *uinfo)
{
	/* struct snd_soc_component *component = snd_soc_kcontrol_component(kcontrol); */
	/* struct gowinxxxx *gowinxxxx = snd_soc_component_get_drvdata(component); */
  return 0;
}

#define FPGA_FILTER_COEFFICIENTS_CONTROL(xname, xcount, xmin, xmax) \
 {	.iface = SNDRV_CTL_ELEM_IFACE_CARD, \
	.name = xname, \
	.info = fpga_coefficient_info, \
	.get  = fpga_coefficient_get, \
	.put  = fpga_coefficient_put, \
	.access = SNDRV_CTL_ELEM_ACCESS_READWRITE }

static struct snd_kcontrol_new fpga_filter_controls[] = {
        FPGA_FILTER_COEFFICIENTS_CONTROL("FPGA FIR Coefficients", 0, 20, 5)
};

static struct snd_soc_component_driver soc_component_dev_fpga = {
	.controls		= fpga_filter_controls,
	.num_controls		= ARRAY_SIZE(fpga_filter_controls),
	.idle_bias_on		= 1,
	.use_pmdown_time	= 1,
	.endianness		= 1,
	/* .non_legacy_dai_naming	= 1, */
};

static int fpga_i2c_probe(struct i2c_client *i2c)
{
        int ret;
	printk(KERN_INFO "Registering device");
	/* kgdb_breakpoint(); */
	ret = devm_snd_soc_register_component(&i2c->dev,
					      &soc_component_dev_fpga, &fpga_dai, 1);
	printk(KERN_INFO "Result of the registration %d", ret);
	return ret;
}

static const struct of_device_id gowinxxxx_of_match[] = {
	{ .compatible = "flatmax,bare", },
	{ },
};
MODULE_DEVICE_TABLE(of, gowinxxxx_of_match);

static const struct i2c_device_id fpga_i2c_id[] = {
	{ "bare", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, fpga_i2c_id);

static struct i2c_driver fpga_codec_driver = {
	.driver		= {
		.name	= "fpga-codec",
		.of_match_table = gowinxxxx_of_match,
	},
	.probe_new	= fpga_i2c_probe,
	.id_table = fpga_i2c_id,
};

module_i2c_driver(fpga_codec_driver);

MODULE_DESCRIPTION("ASoC FPGA codec driver with I2C interface");
MODULE_AUTHOR("Gabriel Santos <flatmax@>");
MODULE_LICENSE("GPL");
