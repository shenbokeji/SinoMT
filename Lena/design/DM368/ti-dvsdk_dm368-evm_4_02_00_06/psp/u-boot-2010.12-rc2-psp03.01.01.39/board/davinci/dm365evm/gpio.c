/*
 * GPIO driver for TI DaVinci DA8xx SOCs.
 *
 * (C) Copyright 2011 Guralp Systems Ltd.
 * Laurence Withers <lwithers@guralp.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/gpio_defs.h>




int gpio_direction_input(unsigned gpio)
{
	struct davinci_gpio *bank;

	bank = GPIO_BANK(gpio);
	setbits_le32(&bank->dir, 1U << GPIO_BIT(gpio));
	return 0;
}

int gpio_direction_output(unsigned gpio, int value)
{
	struct davinci_gpio *bank;

	bank = GPIO_BANK(gpio);
	clrbits_le32(&bank->dir, 1U << GPIO_BIT(gpio));
	gpio_set_value(gpio, value);
	return 0;
}

int gpio_get_value(unsigned gpio)
{
	struct davinci_gpio *bank;
	unsigned int ip;

	bank = GPIO_BANK(gpio);
	ip = in_le32(&bank->in_data) & (1U << GPIO_BIT(gpio));
	return ip ? 1 : 0;
}

int gpio_set_value(unsigned gpio, int value)
{
	struct davinci_gpio *bank;

	bank = GPIO_BANK(gpio);

	if (value)
		bank->set_data = 1U << GPIO_BIT(gpio);
	else
		bank->clr_data = 1U << GPIO_BIT(gpio);

	return 0;
}


