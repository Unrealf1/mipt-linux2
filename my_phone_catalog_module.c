#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#define  DEVICE_NAME "phonebook_device"
#define  CLASS_NAME  "phonebook"

MODULE_LICENSE("GPL");

static int     device_open(struct inode *, struct file *);
static int     device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);

static struct file_operations file_ops = {
    .open = device_open,
    .read = device_read,
    .write = device_write,
    .release = device_release,
};

static int      majorNumber;
static char     message[256] = {0};
static short    size_of_message = 0;
static struct   class*  phoneBookCharClass  = NULL;
static struct   device* phoneBookCharDevice = NULL;
static int device_open_count = 0;

static int __init my_phone_catalog_init(void) {
    majorNumber = register_chrdev(0, DEVICE_NAME, &file_ops);

    if (majorNumber < 0) {
        printk(KERN_ALERT "PhoneBook: module load failed\n");
        return majorNumber;
    }

    printk(KERN_INFO "PhoneBook: module has been loaded with major number %d\n", majorNumber);

    // Register the device class
    phoneBookCharClass = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(phoneBookCharClass)){
      unregister_chrdev(majorNumber, DEVICE_NAME);
      printk(KERN_ALERT "PhoneBook: Failed to register device class\n");
      return PTR_ERR(phoneBookCharClass);
    }

    // Register the device driver
    phoneBookCharDevice = device_create(phoneBookCharClass, NULL, MKDEV(majorNumber, 0), NULL, DEVICE_NAME);
    if (IS_ERR(phoneBookCharDevice)){               // Clean up if there is an error
        class_destroy(phoneBookCharClass);           // Repeated code but the alternative is goto statements
        unregister_chrdev(majorNumber, DEVICE_NAME);
        printk(KERN_ALERT "PhoneBook: Failed to create the device\n");
        return PTR_ERR(phoneBookCharDevice);
    }

    return 0;
}

static void __exit my_phone_catalog_exit(void) {
    device_destroy(phoneBookCharClass, MKDEV(majorNumber, 0));
    class_unregister(phoneBookCharClass);
    class_destroy(phoneBookCharClass);
    unregister_chrdev(majorNumber, DEVICE_NAME);

    printk(KERN_INFO "Phone catalog module has been unloaded\n");
}

static ssize_t device_read(struct file* flip, char* buffer, size_t len, loff_t* offset) {
    int error_count = 0;

    error_count = copy_to_user(buffer, message, size_of_message);

    if (error_count == 0) {
        printk(KERN_INFO "PhoneBook: Sent %d characters to the user\n", size_of_message);
        return (size_of_message = 0);
    }
    else {
        printk(KERN_INFO "PhoneBook: Failed to send %d characters to the user\n", error_count);
        return -EFAULT;
    }

}

static ssize_t device_write(struct file* flip, const char* buffer, size_t len, loff_t* offset) {
    printk(KERN_INFO "PhoneBook: Received %zu characters from the user\n", len);
    sprintf(message, "%s(%zu letters)", buffer, len);
    size_of_message = strlen(message);
    return len;
}

static int device_open(struct inode* inode, struct file* file) {
    if (device_open_count) {
        printk(KERN_INFO "PhoneBook: Device is busy\n", size_of_message);
        return -EBUSY;
    }
    printk(KERN_INFO "PhoneBook: Device is opened\n");
    device_open_count++;
    //try_module_get(THIS_MODULE);
    return 0;
}

static int device_release(struct inode* inode, struct file* file) {
    device_open_count--;
    //module_put(THIS_MODULE);
    printk(KERN_INFO "PhoneBook: Device is closed\n");
    return 0;
}

module_init(my_phone_catalog_init);
module_exit(my_phone_catalog_exit);