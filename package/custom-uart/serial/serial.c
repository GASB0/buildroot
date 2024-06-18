// SPDX-License-Identifier: GPL-2.0
#include <linux/of.h>
#include <linux/init.h>
#include <linux/module.h>
#include <asm/io.h>
#include <linux/platform_device.h>
#include <linux/pm_runtime.h>

// Define offsets for UART registers
#define UART2_DR    0x00
#define UART2_FR    0x18
#define UART2_IBRD  0x24
#define UART2_FBRD  0x28
#define UART2_LCRH  0x2C
#define UART2_CR    0x30
#define UART2_IMSC  0x38
#define UART2_ICR   0x44


/* Add your code here */
struct serial_dev {
        void __iomem *regs;
};

static u32 reg_read(struct serial_dev *serial, unsigned int reg) {
        return readl(serial->regs+reg);
}

static int reg_write(struct serial_dev *serial, u32 val, unsigned int reg) {
        writel(val, serial->regs+reg);
        return 0;
}

static int serial_probe(struct platform_device *pdev)
{
        struct serial_dev *serial;
	u32 uartclk;
	int ret, baud_divisor;
	pr_info("Called %s\n", __func__);

	serial = devm_kzalloc(&pdev->dev, sizeof(*serial), GFP_KERNEL);
	if (!serial)
	        return -ENOMEM;

	serial->regs = devm_platform_ioremap_resource(pdev, 0);
	if (IS_ERR(serial->regs))
	        return PTR_ERR(serial->regs);
	
	pm_runtime_enable(&pdev->dev);
	pm_runtime_get_sync(&pdev->dev);

	ret = of_property_read_u32(pdev->dev.of_node, "clock-frequency", &uartclk);
	if(ret) {
	        dev_err(&pdev->dev, "clock-frequency property not found in Device Tree\n");
	        return ret;
	}

	printk(KERN_INFO"The read uartclk is: %lu\n", (unsigned long)uartclk);
	baud_divisor = (int)uartclk/16/115200;
	
	reg_write(serial, UART2_CR, 0x00); // Disabling UART
	reg_write(serial, UART2_LCRH, (1 << 4) | (1 << 5) | (1 << 6)); // Flushing transmit FIFO and setting parity

	reg_write(serial, UART2_CR, (1 << 8)); // Configuring transmision of data
	reg_write(serial, UART2_DR, 0xAA); // Writing a value into data register

	reg_write(serial, UART2_CR, (1 << 8) | (1 << 0)); // Enabling UART

	return 0;
}

static int serial_remove(struct platform_device *pdev)
{
        pm_runtime_disable(&pdev->dev);

	pr_info("Called %s\n", __func__);
        return 0;
}

static const struct of_device_id custom_uart_of_match[] = {
	{ .compatible = "bootlin,serial", },
	{ },
};
MODULE_DEVICE_TABLE(of, custom_uart_of_match);

static struct platform_driver serial_driver = {
        .driver = {
                .name = "serial",
                .owner = THIS_MODULE,
		.of_match_table = custom_uart_of_match,
        },
        .probe = serial_probe,
        .remove = serial_remove,
};
module_platform_driver(serial_driver);

MODULE_LICENSE("GPL");
