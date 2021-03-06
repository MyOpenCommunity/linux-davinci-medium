/*
 * ASoC driver for BTICINO AMICO platform
 *
 * Author:      bticino
 * Copyright:   (C) 2010 bticino
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#define DEBUG
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/timer.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/soc.h>
#include <sound/soc-dapm.h>
#include <sound/uda134x.h>

#include <mach/dm365.h>
#include <mach/amico.h>
#include <mach/clock.h>
#include <linux/gpio.h>


#include <asm/dma.h>
#include <asm/mach-types.h>

#include <mach/asp.h>
#include <mach/edma.h>
#include <mach/mux.h>

#include "../codecs/cq93vc.h"
#include "../codecs/zl38005.h"
#include "davinci-pcm.h"
#include "davinci-i2s.h"
#include "davinci-vcif.h"
#include <sound/davinci_amico_asoc.h>
#include <linux/pm_loss.h>
#include <linux/delay.h>

static struct amico_asoc_platform_data amico_i_asoc_priv;

static struct platform_device *amico_snd_device[1];

#define AUDIO_FORMAT (SND_SOC_DAIFMT_DSP_B | \
		SND_SOC_DAIFMT_CBM_CFM | SND_SOC_DAIFMT_IB_NF)
static int amico_hw_params_cq93(struct snd_pcm_substream *substream,
			struct snd_pcm_hw_params *params)
{
	/*
	 * Even if this settings are done in
	 * dm365.c, it assures that they are
	 * correct.
	 */
	davinci_cfg_reg(DM365_EVT2_VC_TX);
	davinci_cfg_reg(DM365_EVT3_VC_RX);

	return 0;
}

static struct snd_soc_ops amico_ops_cq93 = {
	.hw_params = amico_hw_params_cq93,
};

static int amico_line_event(struct snd_soc_dapm_widget *w,
			   struct snd_kcontrol *k, int event)
{
	zl38005_mute_r(SND_SOC_DAPM_EVENT_ON(event) ? 0 : 1);
	return 0;
};

static int amico_mic_event(struct snd_soc_dapm_widget *w,
			   struct snd_kcontrol *k, int event)
{
	zl38005_mute_r(SND_SOC_DAPM_EVENT_ON(event) ? 1 : 0);
	return 0;
};

/* davinci-amico machine dapm widgets */
static const struct snd_soc_dapm_widget cq93_dapm_widgets[] = {
	SND_SOC_DAPM_HP("Speakers out", NULL),
/*	SND_SOC_DAPM_LINE("Line Out", amico_line_event),
	SND_SOC_DAPM_MIC("Microphone", amico_mic_event),
*/
};

/* davinci-amico machine connections to the codec pins */
static const struct snd_soc_dapm_route audio_map[] = {
	/* Speakers connected to SP (actually not connected !) */
	{ "Speakers out", NULL, "SP", },
	/* Line output connected to LO */
	{ "Line Out", NULL, "LINEO", },
	/* Microphone input connected to MIC */
	{ "MICIN", NULL, "Microphone" },
};

static void ext_codec_power_work(struct work_struct *work)
{
	zl38005_init();
}

static int amico_cq93_init(struct snd_soc_codec *codec)
{
	int err;

	pr_debug("amico_cq93_init(%p)\n", codec);
	/* Add davinci-evm specific widgets */
	snd_soc_dapm_new_controls(codec, cq93_dapm_widgets,
				  ARRAY_SIZE(cq93_dapm_widgets));

	INIT_DELAYED_WORK(&amico_asoc_priv.delayed_work, ext_codec_power_work);
	amico_asoc_priv.ext_codec_power(1); /* zl38005 is the ext codec */
	mdelay(100); /* zl38005 startup need at least 83msec */
	zl38005_init();

	err = zl38005_add_controls(codec);
	if (err < 0)
		return err;

	/* Set up davinci-amico specific audio path audio_map */
	snd_soc_dapm_add_routes(codec, audio_map, ARRAY_SIZE(audio_map));

	/* not connected */
	snd_soc_dapm_disable_pin(codec, "SP");

	/* always connected */
	snd_soc_dapm_enable_pin(codec, "LINEO");
	snd_soc_dapm_enable_pin(codec, "MICIN");

	snd_soc_dapm_sync(codec);

	return 0;
}

/* amico digital audio interface glue - connects codec <--> CPU */

static struct snd_soc_dai_link dm365_amico_dai[] = {
	{
		.name = "Voice Codec - CQ93VC",
		.stream_name = "CQ93",
		.cpu_dai = &davinci_vcif_dai,
		.codec_dai = &cq93vc_dai,
		.init = amico_cq93_init,
		.ops = &amico_ops_cq93,
	},
};

/* davinci dm365 amico audio machine driver */
static struct snd_soc_card dm365_snd_soc_card_amico[] = {
	{
		.name = "DaVinci AMICO VOICE",
		.platform = &davinci_soc_platform,
		.dai_link = &dm365_amico_dai[0],
		.num_links = 1,
	},
};

/* amico audio subsystem */
static struct snd_soc_device dm365_amico_snd_devdata[] = {
	{
		.card = &dm365_snd_soc_card_amico[0],
		.codec_dev = &soc_codec_dev_cq93vc,
	},
};

static int amico_asoc_probe(struct platform_device *pdev)
{
	int ret;
	int id = pdev->id;
	struct amico_asoc_platform_data *pdata = pdev->dev.platform_data;
	if (!machine_is_amico() || (id > 1))
		return -ENODEV;

	if ((!pdata->ext_codec_power) || (!pdata->ext_circuit_power))
		return -EINVAL;

	memcpy(&amico_asoc_priv, pdata, sizeof(amico_asoc_priv));

	amico_snd_device[id] = platform_device_alloc("soc-audio", id);
	if (!amico_snd_device[id])
		return -ENOMEM;

	platform_set_drvdata(amico_snd_device[id],
			&dm365_amico_snd_devdata[id]);
	dm365_amico_snd_devdata[id].dev = &amico_snd_device[id]->dev;

	ret = platform_device_add(amico_snd_device[id]);
	if (ret)
		platform_device_put(amico_snd_device[id]);

	return ret;
}

static int __devexit amico_asoc_remove(struct platform_device *pdev)
{
	int id = pdev->id;

	platform_device_unregister(amico_snd_device[id]);
	return 0;
}

#ifdef CONFIG_PM_LOSS
static int amico_asoc_power_changed(struct device *dev,
				 enum sys_power_state s)
{
	int ret = -EIO;

	switch (s) {
	case SYS_PWR_GOOD:
		amico_asoc_priv.ext_codec_power(1);
		amico_asoc_priv.ext_circuit_power(1);
		schedule_delayed_work(&amico_asoc_priv.delayed_work,
				      msecs_to_jiffies(100));
		break;
	case SYS_PWR_FAILING:
		amico_asoc_priv.ext_codec_power(0);
		amico_asoc_priv.ext_circuit_power(0);
		ret = cancel_delayed_work(&amico_asoc_priv.delayed_work);
		break;
	default:
		BUG();
		ret = -ENODEV;
	}
	return ret;
}

static struct dev_pm_ops amico_asoc_dev_pm_ops = {
	.power_changed = amico_asoc_power_changed,
};
#endif

static struct platform_driver amico_asoc_driver = {
	.probe = amico_asoc_probe,
	.remove = __devexit_p(amico_asoc_remove),
	.driver = {
		.name = "amico-asoc",
		.owner = THIS_MODULE,
#ifdef CONFIG_PM_LOSS
		.pm = &amico_asoc_dev_pm_ops,
#endif
	},
};

static int __init amico_asoc_init(void)
{
	return platform_driver_register(&amico_asoc_driver);
}

static void __exit amico_asoc_exit(void)
{
	platform_driver_unregister(&amico_asoc_driver);
}

module_init(amico_asoc_init);
module_exit(amico_asoc_exit);

MODULE_AUTHOR("bticino s.p.a.");
MODULE_DESCRIPTION("TI DAVINCI amico ASoC driver");
MODULE_LICENSE("GPL");
