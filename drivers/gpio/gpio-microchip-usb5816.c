// SPDX-License-Identifier: GPL-2.0-only
/*
 * Microchip USB5816 GPIO support
 *
 */

#include <linux/cleanup.h>
#include <linux/gpio/driver.h>
#include <linux/usb.h>

/* According to datasheet, by default we have 3 GPIOs, but 25 are available */
#define USB5816_NGPIO 25

struct usb5816_priv {
	struct gpio_chip gpio;
	struct usb_device *usb_dev;
};

static int gpio_usb5816_get(struct gpio_chip *gc, unsigned int offset)
{
	struct usb5816_priv *priv = gpiochip_get_data(gc);
	int val = 0;

	dev_info(&priv->usb_dev->dev, "Getting GPIO %u\n", offset);

	return val;
}

static int gpio_usb5816_set(struct gpio_chip *gc, unsigned int offset, int val)
{
	struct usb5816_priv *priv = gpiochip_get_data(gc);

	dev_info(&priv->usb_dev->dev, "Setting GPIO %u to %d\n", offset, val);

	return 0;
}

static int gpio_usb5816_get_direction(struct gpio_chip *gc, unsigned int offset)
{
	int ret = GPIO_LINE_DIRECTION_OUT;

	return ret;
}

static int gpio_usb5816_direction_input(struct gpio_chip *gc, unsigned int offset)
{
	// TODO: to set gpio to input
	return 0;
}

static int gpio_usb5816_direction_output(struct gpio_chip *gc,
					 unsigned int offset,
					 int value)
{
	// TODO: to set gpio to output
	return 0;
}

static int gpio_usb5816_probe(struct usb_interface *intf, const struct usb_device_id *id)
{
	struct device *dev = &intf->dev;
	struct usb5816_priv *priv = NULL;

	dev_info(dev, "probe = 0x%x/0x%x\n", id->idVendor, id->idProduct);
	priv = devm_kzalloc(dev, sizeof(*priv), GFP_KERNEL);
	if (!priv) {
		dev_err(dev, "Failed to allocate memory for private data\n");
		return -ENOMEM;
	}

	priv->gpio.label = "UBS5816-HFC";
	priv->gpio.parent = intf->usb_dev;
	priv->gpio.base = -1;
	priv->gpio.get = gpio_usb5816_get;
	priv->gpio.set = gpio_usb5816_set;
	priv->gpio.get_direction = gpio_usb5816_get_direction;
	priv->gpio.direction_input = gpio_usb5816_direction_input;
	priv->gpio.direction_output = gpio_usb5816_direction_output;
	priv->gpio.ngpio = USB5816_NGPIO;

	return devm_gpiochip_add_data(dev, &priv->gpio, priv);
}

static void gpio_usb5816_disconnect(struct usb_interface *intf)
{
	dev_info(&intf->dev, "Hub Feature Controller disconnected\n");
}

static const struct usb_device_id gpio_usb5816_table[] = {
	{ USB_DEVICE(0x0424, 0x2840) },   /* USB5816 Hub Feature Controller */
	{ }                               /* Sentinel */
};

MODULE_DEVICE_TABLE(usb, gpio_usb5816_table);

static struct usb_driver gpio_microchip_driver = {
	.name           = "gpio-usb5816",
	.probe          = gpio_usb5816_probe,
	.disconnect     = gpio_usb5816_disconnect,
	.id_table       = gpio_usb5816_table,
};

module_usb_driver(gpio_microchip_driver);

MODULE_AUTHOR("Andrei Lalaev <andrei.lalaev@gmail.com>");
MODULE_DESCRIPTION("Microchip USB5816 driver");
MODULE_LICENSE("GPL");
