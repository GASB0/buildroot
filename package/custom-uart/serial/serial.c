// SPDX-License-Identifier: GPL-2.0
#include <linux/of.h>
#include <linux/init.h>
#include <linux/module.h>
#include <asm/io.h>
#include <linux/platform_device.h>
#include <linux/pm_runtime.h>

/* Add your code here */
struct serial_dev {
        void __iomem *regs;
};

static u32 reg_read(struct serial_dev *serial, unsigned int reg) {
        return readl(serial->regs+4*reg);
}

static int reg_write(struct serial_dev *serial, u32 val, unsigned int reg) {
        writel(val, serial->regs+4*reg);
        return 0;
}

static int serial_probe(struct platform_device *pdev)
{
        struct serial_dev *serial;
	serial = devm_kzalloc(&pdev->dev, sizeof(*serial), GFP_KERNEL);
	if (!serial)
	        return -ENOMEM;

	serial->regs = devm_platform_ioremap_resource(pdev, 0);
	if (IS_ERR(serial->regs))
	        return PTR_ERR(serial->regs);
	
	pm_runtime_enable(&pdev->dev);
	pm_runtime_get_sync(&pdev->dev);

	
	pr_info("Called %s\n", __func__);
	return 0;
}

static int serial_remove(struct platform_device *pdev)
{
        pm_runtime_disable(&pdev->dev);

	pr_info("Called %s\n", __func__);
        return 0;
}

static const struct of_device_id custom_uart_of_match[] = {
	{ .compatible = "custom,uart", },
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
