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
#include <linux/regmap.h>
#include <linux/i2c.h>
#include <linux/kgdb.h>
#include <sound/soc.h>
#include <linux/clk.h>

// Definition of some important registers for our board configuration
#define FPGA_FILTER_TAPS_LSB	        0x30
#define FPGA_FILTER_TAPS_MSB	        0x31

#define FPGA_INS_REG		        0x32

#define FPGA_DATA_BUFF_LSB		0x40
#define FPGA_DATA_BUFF_LMB		0x41
#define FPGA_DATA_BUFF_HMB		0x42
#define FPGA_DATA_BUFF_MSB		0x43

#define FPGA_ADDR_BUFF_LSB		0x48
#define FPGA_ADDR_BUFF_LMB		0x49
#define FPGA_ADDR_BUFF_HMB		0x4A
#define FPGA_ADDR_BUFF_MSB		0x4B

enum gowinxxxx_type {
        GW5A = 0,
        GW2A
};

struct gowinxxxx_priv {
	struct snd_soc_component *component;
	struct regmap *regmap;
        /* struct i2c_client *i2c; */
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
	{FPGA_FILTER_TAPS_LSB, 0xA0},
	{FPGA_FILTER_TAPS_MSB, 0x0F},
	
	{FPGA_INS_REG, 0x00},

	/* Data buffer  */
	{FPGA_DATA_BUFF_LSB, 0x00},
	{FPGA_DATA_BUFF_LMB, 0x00},
	{FPGA_DATA_BUFF_HMB, 0x00},
	{FPGA_DATA_BUFF_MSB, 0x00},

	/* Address buffer */
	{FPGA_ADDR_BUFF_LSB, 0x00},
	{FPGA_ADDR_BUFF_LMB, 0x00},
	{FPGA_ADDR_BUFF_HMB, 0x00},
	{FPGA_ADDR_BUFF_MSB, 0x00},
    };

static const struct regmap_config gowinxxxx_regmap = {
    .reg_bits = 8,
    .val_bits = 8,

    .max_register = 0xFFFF,
    .reg_defaults = gowinxxxx_defaults,
    .num_reg_defaults = ARRAY_SIZE(gowinxxxx_defaults),
    .cache_type = REGCACHE_FLAT,
};

static struct snd_soc_dai_driver fpga_dai = {
    .name = "fpga-hifi",
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
	struct snd_soc_component *component = snd_soc_kcontrol_component(kcontrol);
	unsigned int read_value;

	regmap_write(component->regmap, FPGA_INS_REG, 0x00);

	// This part writes on the address registers
	regmap_write(component->regmap, FPGA_ADDR_BUFF_LSB, (int)ucontrol->value.bytes.data[0]);
	regmap_write(component->regmap, FPGA_ADDR_BUFF_LMB, (int)ucontrol->value.bytes.data[1]);
	regmap_write(component->regmap, FPGA_ADDR_BUFF_HMB, (int)ucontrol->value.bytes.data[2]);
	regmap_write(component->regmap, FPGA_ADDR_BUFF_MSB, (int)ucontrol->value.bytes.data[3]);


	// This part reads on the data registers
	regmap_write(component->regmap, FPGA_INS_REG, 0x00);
	regmap_read(component->regmap, FPGA_DATA_BUFF_LSB, &read_value);
	ucontrol->value.bytes.data[0] = (unsigned char)read_value;
	regmap_read(component->regmap, FPGA_DATA_BUFF_LMB, &read_value);
	ucontrol->value.bytes.data[1] = (unsigned char)read_value;
	regmap_read(component->regmap, FPGA_DATA_BUFF_HMB, &read_value);
	ucontrol->value.bytes.data[2] = (unsigned char)read_value;
	regmap_read(component->regmap, FPGA_DATA_BUFF_MSB, &read_value);
	ucontrol->value.bytes.data[3] = (unsigned char)read_value;

  return 0;
}

static int fpga_coefficient_put(struct snd_kcontrol *kcontrol,
				   struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_component *component = snd_soc_kcontrol_component(kcontrol);

	// Making sure that the FPGA is in write mode
	regmap_write(component->regmap, FPGA_INS_REG, 0x01);

	// This part writes on the data registers
	regmap_write(component->regmap, FPGA_DATA_BUFF_LSB, ucontrol->value.bytes.data[4]);
	regmap_write(component->regmap, FPGA_DATA_BUFF_LMB, ucontrol->value.bytes.data[5]);
	regmap_write(component->regmap, FPGA_DATA_BUFF_HMB, ucontrol->value.bytes.data[6]);
	regmap_write(component->regmap, FPGA_DATA_BUFF_MSB, ucontrol->value.bytes.data[7]);

	// This part writes on the address registers
	regmap_write(component->regmap, FPGA_ADDR_BUFF_LSB, ucontrol->value.bytes.data[0]);
	regmap_write(component->regmap, FPGA_ADDR_BUFF_LMB, ucontrol->value.bytes.data[1]);
	regmap_write(component->regmap, FPGA_ADDR_BUFF_HMB, ucontrol->value.bytes.data[2]);
	regmap_write(component->regmap, FPGA_ADDR_BUFF_MSB, ucontrol->value.bytes.data[3]);

	return 0;
}

static int fpga_coefficient_info(struct snd_kcontrol *kcontrol,
                                 struct snd_ctl_elem_info *uinfo)
{
  return 0;
}

#define FPGA_FILTER_COEFFICIENTS_VALUES(xname, xcount, xmin, xmax) \
 {	.iface = SNDRV_CTL_ELEM_IFACE_CARD, \
	.name = xname, \
	.info = fpga_coefficient_info, \
	.get  = fpga_coefficient_get, \
	.put  = fpga_coefficient_put, \
	.access = SNDRV_CTL_ELEM_ACCESS_READWRITE }

/* #define FPGA_FILTER_COEFFICIENTS_NUMBER(xname, xcount, xmin, xmax) \ */
/*  {	.iface = SNDRV_CTL_ELEM_IFACE_CARD, \ */
/* 	.name = xname, \ */
/* 	.info = , \ */
/* 	.get  = , \ */
/* 	.put  = , \ */
/* 	.access = SNDRV_CTL_ELEM_ACCESS_READWRITE } */


static struct snd_kcontrol_new fpga_filter_controls[] = {
        FPGA_FILTER_COEFFICIENTS_VALUES("FPGA FIR Coefficients Values", 0, 20, 5)
};

static struct snd_soc_component_driver soc_component_dev_fpga = {
	.controls		= fpga_filter_controls,
	.num_controls		= ARRAY_SIZE(fpga_filter_controls),
	.idle_bias_on		= 1,
	.use_pmdown_time	= 1,
	.endianness		= 1,
};

static int fpga_i2c_probe(struct i2c_client *i2c)
{
        int ret = 0;
	struct gowinxxxx_priv *gowinxxxx = NULL;

	printk(KERN_INFO "Initializing routine for allocating device specific data");
	gowinxxxx = devm_kzalloc(&i2c->dev, sizeof(struct gowinxxxx_priv), GFP_KERNEL);
	if (!gowinxxxx)
		return -ENOMEM;

	printk(KERN_INFO "Initializing regmap");        
	gowinxxxx->regmap = devm_regmap_init_i2c(i2c, &gowinxxxx_regmap);
	if (IS_ERR(gowinxxxx->regmap)) {
		ret = PTR_ERR(gowinxxxx->regmap);
		dev_err(&i2c->dev, "Failed to allocate register map: %d\n", ret);
		return ret;
	}

	// Configuring the chip on start
	printk(KERN_INFO "Initializing I2C registers");
	regcache_mark_dirty(gowinxxxx->regmap);
	regcache_sync(gowinxxxx->regmap);	

	// Registering the audio component to ALSA
	printk(KERN_INFO "Registering device");
	ret = devm_snd_soc_register_component(&i2c->dev,
					      &soc_component_dev_fpga, &fpga_dai, 1);
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
		.of_match_table = of_match_ptr(gowinxxxx_of_match),
	},
	.probe_new	= fpga_i2c_probe,
	.id_table = fpga_i2c_id,
};

module_i2c_driver(fpga_codec_driver);

MODULE_DESCRIPTION("ASoC FPGA codec driver with I2C interface");
MODULE_AUTHOR("Gabriel Santos <flatmax@>");
MODULE_LICENSE("GPL");
