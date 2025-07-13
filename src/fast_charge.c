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

static struct fast_charge_status status = {
    .charging = 0,
    .current_mA = 0,
    .voltage_mV = 0,
    .error_code = 0,
};

static int fast_charge_open(struct inode *inode, struct file *file)
{
    pr_info("fast_charge: device opened\n");
    return 0;
}

static int fast_charge_release(struct inode *inode, struct file *file)
{
    pr_info("fast_charge: device closed\n");
    return 0;
}

static ssize_t fast_charge_read(struct file *file, char __user *buf, size_t len, loff_t *offset)
{
    struct fast_charge_status tmp;
    ssize_t ret;

    mutex_lock(&fast_charge_mutex);
    tmp = status;
    mutex_unlock(&fast_charge_mutex);

    if (*offset >= sizeof(struct fast_charge_status))
        return 0;  // EOF

    if (len < sizeof(struct fast_charge_status))
        return -EINVAL;

    if (copy_to_user(buf, &tmp, sizeof(tmp)))
        return -EFAULT;

    *offset += sizeof(tmp);
    ret = sizeof(tmp);

    pr_info("fast_charge: read status charging=%d current=%dmA voltage=%dmV error=%d\n",
            tmp.charging, tmp.current_mA, tmp.voltage_mV, tmp.error_code);

    return ret;
}

static ssize_t fast_charge_write(struct file *file, const char __user *buf, size_t len, loff_t *offset)
{
    char cmd;
    if (len < 1)
        return -EINVAL;

    if (copy_from_user(&cmd, buf, 1))
        return -EFAULT;

    mutex_lock(&fast_charge_mutex);
    switch (cmd) {
    case 'S':  // START
        status.charging = 1;
        status.current_mA = 50000;
        status.voltage_mV = 400000;
        status.error_code = 0;
        pr_info("fast_charge: START_CHARGE via write\n");
        break;
    case 'P':  // STOP
        status.charging = 0;
        status.current_mA = 0;
        status.voltage_mV = 0;
        status.error_code = 0;
        pr_info("fast_charge: STOP_CHARGE via write\n");
        break;
    default:
        mutex_unlock(&fast_charge_mutex);
        return -EINVAL;
    }
    mutex_unlock(&fast_charge_mutex);

    return len;
}

static long fast_charge_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    mutex_lock(&fast_charge_mutex);

    switch (cmd) {
    case FAST_CHARGE_START:
        status.charging = 1;
        status.current_mA = 50000;
        status.voltage_mV = 400000;
        status.error_code = 0;
        pr_info("fast_charge: ioctl START_CHARGE\n");
        break;
    case FAST_CHARGE_STOP:
        status.charging = 0;
        status.current_mA = 0;
        status.voltage_mV = 0;
        status.error_code = 0;
        pr_info("fast_charge: ioctl STOP_CHARGE\n");
        break;
    case FAST_CHARGE_STATUS:
        if (copy_to_user((struct fast_charge_status __user *)arg, &status, sizeof(status))) {
            mutex_unlock(&fast_charge_mutex);
            return -EFAULT;
        }
        pr_info("fast_charge: ioctl GET_STATUS charging=%d current=%dmA voltage=%dmV error=%d\n",
                status.charging, status.current_mA, status.voltage_mV, status.error_code);
        break;
    default:
        mutex_unlock(&fast_charge_mutex);
        return -EINVAL;
    }

    mutex_unlock(&fast_charge_mutex);
    return 0;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = fast_charge_open,
    .release = fast_charge_release,
    .read = fast_charge_read,
    .write = fast_charge_write,
    .unlocked_ioctl = fast_charge_ioctl,
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
