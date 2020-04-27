#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/string.h>
#include <linux/uaccess.h>

#include "phone_book_storage.h"

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
static char     query[256] = {0};
static char     answer[256] = {0};
static short    size_of_query = 0;
static struct   class*  phoneBookCharClass  = NULL;
static struct   device* phoneBookCharDevice = NULL;
static int      device_open_count = 0;

const static char* ADD = "[add]";
const static char* REMOVE = "[del]";
const static char* GET = "[get]";
const static char* FAIL = "Operation failed";

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

static void prepare_answer(void) {
    const size_t cmd_size = strlen(ADD);
    if (size_of_query < cmd_size) {
        goto fail;
    }
    if (!strncmp(ADD, query, cmd_size)) {
        char name[FIELD_SIZE];
        char surname[FIELD_SIZE];
        uint8_t age;
        char phone[FIELD_SIZE];
        char email[FIELD_SIZE];

        if (sscanf(query+cmd_size, "name: %s\nsurname: %s\nage: %hhu\nphone: %s\nemail: %s\n",
            name, surname, &age, phone, email) == 5) {
            add_entry (name, surname, age, phone, email);
        } else {
            goto fail;
        }

    } else if (!strncmp(REMOVE, query, cmd_size)) {
        char* surname = query + cmd_size;
        if (!remove_entry(surname)) {
            strcpy(answer, "OK");
        } else {
            strcpy(answer, "No such entry");
        }
    } else if (!strncmp(GET, query, cmd_size)) {
        char* surname = query + cmd_size;
        struct phone_book_entry* result = get_entry(surname);
        if (result == NULL) {
            strcpy(answer, "No such entry");
        } else {
            sprintf(answer, "name: %s\nsurname: %s\nage: %hhu\nphone: %s\nemail: %s\n", 
                result->name, result->surname, result->age, result->phone, result->email);
        }

    } else {
        goto fail;
    }


    return;

    fail:
    strcpy(answer, FAIL);
    return;
}

static ssize_t device_read(struct file* flip, char* buffer, size_t len, loff_t* offset) {
    int error_count = 0;

    error_count = copy_to_user(buffer, answer, strlen(answer));

    if (error_count == 0) {
        printk(KERN_INFO "PhoneBook: Sent %lu characters to the user\n", strlen(answer));
        return 0;
    }
    else {
        printk(KERN_INFO "PhoneBook: Failed to send %d characters to the user\n", error_count);
        return -EFAULT;
    }
}

static ssize_t device_write(struct file* flip, const char* buffer, size_t len, loff_t* offset) {
    if (len > 256) {
        printk(KERN_ALERT "PhoneBook: Too many characters from the user\n");
        return -EFAULT;
    }

    printk(KERN_INFO "PhoneBook: Received %zu characters from the user\n", len);
    copy_from_user(query, buffer, len);
    //sprintf(message, "%s(%zu letters)", buffer, len);
    size_of_query = strlen(query);
    prepare_answer();
    return len;
}

static int device_open(struct inode* inode, struct file* file) {
    if (device_open_count) {
        printk(KERN_INFO "PhoneBook: Device is busy\n");
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