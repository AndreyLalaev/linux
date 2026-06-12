// SPDX-License-Identifier: GPL-2.0-only
/*
 * Microchip USB5816 GPIO support
 *
 */

// FIXME: to drop
#define DEBUG

#include <linux/cleanup.h>
#include <linux/gpio/driver.h>
#include <linux/usb.h>

#define GPIO_1_7_PD	0x082F
#define GPIO_8_12_PD	0x082E
#define GPIO_16_23_PD	0x082D
#define GPIO_64_71_PD	0x096F
#define GPIO_72_PD	0x096E

#define GPIO_1_7_DIR	0x0833
#define GPIO_8_12_DIR	0x0832
#define GPIO_16_23_DIR	0x0831
#define GPIO_64_71_DIR	0x0973
#define GPIO_72_DIR	0x0972

#define GPIO_1_7_OUT	0x0837
#define GPIO_8_12_OUT	0x0836
#define GPIO_16_23_OUT	0x0835
#define GPIO_64_71_OUT	0x0977
#define GPIO_72_OUT	0x0976

#define GPIO_1_7_IN	0x083B
#define GPIO_8_12_IN	0x083A
#define GPIO_16_23_IN	0x0839
#define GPIO_64_71_IN	0x097B
#define GPIO_72_IN	0x097A

#define GPIO_1_7_PU	0x083F
#define GPIO_8_12_PU	0x083E
#define GPIO_16_23_PU	0x083D
#define GPIO_64_71_PU	0x097F
#define GPIO_72_PU	0x097E

// FIXME: undocumented
#define GPIO_64_71_INPUT_EN 0x0993

/* According to datasheet, by default we have 3 GPIOs, but 25 are available */
// FIXME should be 25
#define USB5816_NGPIO 72

#define USB58XX_READ_REQ	0x04
#define USB58XX_WRITE_REQ	0x03

struct usb5816_priv {
	struct gpio_chip gpio;
	struct usb_device *usb_dev;
	u8 *in_buf;
};

static int gpio_usb5816_get(struct gpio_chip *gc, unsigned int offset)
{
	struct usb5816_priv *priv = gpiochip_get_data(gc);
	struct device *dev = &priv->usb_dev->dev;
	u16 reg = 0;
	int val = 0;
	int err;

	dev_dbg(dev, "Getting GPIO %u\n", offset);
	// FIXME
	if (offset <= 7) {
		reg = GPIO_1_7_IN;
	} else if (offset >= 8 && offset <= 12) {
		reg = GPIO_8_12_IN;
	} else if (offset >= 16 && offset <= 23) {
		reg = GPIO_16_23_IN;
	} else if (offset >= 64 && offset <= 71) {
		reg = GPIO_64_71_IN;
	} else {
		dev_err(dev, "yet unsupported GPIO %u\n", offset);
		return -1;
	}

	err = usb_control_msg(priv->usb_dev,
			usb_rcvctrlpipe(priv->usb_dev, 0),
			USB58XX_READ_REQ,
			USB_TYPE_VENDOR | USB_DIR_IN | USB_RECIP_INTERFACE,
			reg,
			0x00,
			priv->in_buf, 1,
			USB_CTRL_GET_TIMEOUT);
	dev_dbg(dev, "err: %d; buf = 0x%x\n", err, *priv->in_buf);

	return val;
}

static int gpio_usb5816_set(struct gpio_chip *gc, unsigned int offset, int val)
{
	struct usb5816_priv *priv = gpiochip_get_data(gc);
	struct device *dev = &priv->usb_dev->dev;
	u16 reg = 0;
	u8 buf = 0x00;
	int err = 0;

	dev_dbg(dev, "%s:%d: Setting GPIO %u to %d\n", __func__, __LINE__, offset, val);
	// FIXME
	if (offset <= 7) {
		reg = GPIO_1_7_OUT;
	} else if (offset >= 8 && offset <= 12) {
		reg = GPIO_8_12_OUT;
	} else if (offset >= 16 && offset <= 23) {
		reg = GPIO_16_23_OUT;
	} else if (offset >= 64 && offset <= 71) {
		reg = GPIO_64_71_OUT;
	} else {
		dev_err(dev, "yet unsupported GPIO %u\n", offset);
		return -1;
	}

	buf = val << (offset % 8);
	dev_dbg(dev, "writing 0x%x to 0x%x\n", buf, reg);
	err = usb_control_msg(priv->usb_dev,
			usb_sndctrlpipe(priv->usb_dev, 0),
			USB58XX_WRITE_REQ,
			USB_TYPE_VENDOR | USB_DIR_OUT | USB_RECIP_INTERFACE,
			reg,
			0x00,
			&buf, sizeof(buf),
			USB_CTRL_GET_TIMEOUT);

	return 0;
}

static int gpio_usb5816_get_direction(struct gpio_chip *gc, unsigned int offset)
{
	int ret = GPIO_LINE_DIRECTION_OUT;

	return ret;
}

static int gpio_usb5816_direction_input(struct gpio_chip *gc, unsigned int offset)
{
	struct usb5816_priv *priv = gpiochip_get_data(gc);
	struct device *dev = &priv->usb_dev->dev;
	u8 buf = 0x00;
	int err = 0;

	dev_dbg(dev, "Setting GPIO %u to input\n", offset);
	// FIXME
	if (offset <= 7) {
		buf = ~(1 << offset);
		err = usb_control_msg(priv->usb_dev,
				usb_sndctrlpipe(priv->usb_dev, 0),
				USB58XX_WRITE_REQ,
				USB_TYPE_VENDOR | USB_DIR_OUT | USB_RECIP_INTERFACE,
				GPIO_1_7_DIR,
				0x00,
				&buf, sizeof(buf),
				USB_CTRL_GET_TIMEOUT);
		dev_dbg(dev, "err: %d; buf = 0x%u\n", err, buf);
	}

	return 0;
}

static int gpio_usb5816_direction_output(struct gpio_chip *gc,
					 unsigned int offset,
					 int value)
{
	struct usb5816_priv *priv = gpiochip_get_data(gc);
	struct device *dev = &priv->usb_dev->dev;
	u16 reg = 0;
	u8 buf = 0x00;
	int err = 0;

	dev_dbg(dev, "Setting GPIO %u to output (%d)\n", offset, value);
	if (offset <= 7) {
		reg = GPIO_1_7_DIR;
	} else if (offset >= 8 && offset <= 12) {
		reg = GPIO_8_12_DIR;
	} else if (offset >= 16 && offset <= 23) {
		reg = GPIO_16_23_DIR;
	} else if (offset >= 64 && offset <= 71) {
		reg = GPIO_64_71_DIR;
	} else {
		dev_err(dev, "yet unsupported GPIO %u\n", offset);
		return -1;
	}

	// FIXME
	buf = 1 << (offset % 8);
	dev_dbg(dev, "Writing 0x%x to 0x%x\n", buf, reg);
	err = usb_control_msg(priv->usb_dev,
			usb_sndctrlpipe(priv->usb_dev, 0),
			USB58XX_WRITE_REQ,
			USB_TYPE_VENDOR | USB_DIR_OUT | USB_RECIP_INTERFACE,
			reg,
			0x00,
			&buf, sizeof(buf),
			USB_CTRL_GET_TIMEOUT);

	return gpio_usb5816_set(gc, offset, value);
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
	priv->gpio.can_sleep = 1;

	priv->usb_dev = interface_to_usbdev(intf);

	// FIXME
	priv->in_buf = devm_kzalloc(dev, sizeof(u8), GFP_KERNEL);
	if (!priv->in_buf)
		return -ENOMEM;

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
