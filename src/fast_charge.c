/* FAST_CHARGE_C */
#include "fast_charge.h"
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/mutex.h>
#include <linux/module.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("NikitaiDev");
MODULE_DESCRIPTION("module for /dev/fast_charge");
MODULE_VERSION("0.1");

static int major;
static struct class *fast_charge_class = NULL;
static struct cdev fast_charge_cdev;
static DEFINE_MUTEX(fast_charge_mutex);

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = NULL,
    .release = NULL,
    .read = NULL,
    .write = NULL,
    .unlocked_ioctl = NULL,
};

static int __init fast_charge_init(void)
{
    dev_t dev;
    int ret;

    ret = alloc_chrdev_region(&dev, 0, 1, DEVICE_NAME);
    if (ret < 0) {
        pr_err("fast_charge: failed to allocate major number\n");
        return ret;
    }

    major = MAJOR(dev);
    cdev_init(&fast_charge_cdev, &fops);
    fast_charge_cdev.owner = THIS_MODULE;
    ret = cdev_add(&fast_charge_cdev, dev, 1);
    if (ret < 0) {
        pr_err("fast_charge: unable to add cdev\n");
        unregister_chrdev_region(dev, 1);
        return ret;
    }

    fast_charge_class = class_create(THIS_MODULE, DEVICE_NAME);
    if (IS_ERR(fast_charge_class)) {
        pr_err("fast_charge: failed to create class\n");
        cdev_del(&fast_charge_cdev);
        unregister_chrdev_region(dev, 1);
        return PTR_ERR(fast_charge_class);
    }

    if (IS_ERR(device_create(fast_charge_class, NULL, dev, NULL, DEVICE_NAME))) {
        pr_err("fast_charge: failed to create device node\n");
        class_destroy(fast_charge_class);
        cdev_del(&fast_charge_cdev);
        unregister_chrdev_region(dev, 1);
        return -1;
    }

    mutex_init(&fast_charge_mutex);

    pr_info("fast_charge: module loaded, device /dev/%s created with major %d\n", DEVICE_NAME, major);
    return 0;
}

static void __exit fast_charge_exit(void)
{
    mutex_destroy(&fast_charge_mutex);
    device_destroy(fast_charge_class, MKDEV(major, 0));
    class_destroy(fast_charge_class);
    cdev_del(&fast_charge_cdev);
    unregister_chrdev_region(MKDEV(major, 0), 1);
    pr_info("fast_charge: module unloaded\n");
}

module_init(fast_charge_init);
module_exit(fast_charge_exit);
