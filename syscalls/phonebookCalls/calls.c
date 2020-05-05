#include <linux/kernel.h>
#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/string.h>
#include <linux/syscalls.h>
#define FIELD_SIZE 32

#define PHONEBOOK_DEVICE_PATH "/dev/phonebook_device"

static struct user_data {
    char name[FIELD_SIZE];
    char surname[FIELD_SIZE];
    int age;
    char phone[FIELD_SIZE];
    char email[FIELD_SIZE];
};


asmlinkage long sys_get_user(const char* surname_, unsigned int len, struct user_data* output_data) {
    char buffer[FIELD_SIZE];
    if (copy_from_user(buffer, surname_, min(FIELD_SIZE, len))) {
        return -EFAULT;
    }

    mm_segment_t old_fs = get_fs();
    set_fs(KERNEL_DS);

    int fd = ksys_open(PHONEBOOK_DEVICE_PATH, O_RDWR, 0);

    char query[256];
    sprintf(query, "[get]%s", buffer);
    ksys_write(fd, query, strlen(query));

    char name[FIELD_SIZE];
    char surname[FIELD_SIZE];
    uint8_t age;
    char phone[FIELD_SIZE];
    char email[FIELD_SIZE];
    
    char answer[256];
    ksys_read(fd, answer, 256);

    if (sscanf(answer, "name: %s\nsurname: %s\nage: %hhu\nphone: %s\nemail: %s\n",
            name, surname, &age, phone, email) == 5) {

        if (copy_to_user(output_data->name, name, strlen(name)) ||
            copy_to_user(output_data->surname, surname, strlen(surname)) ||
            copy_to_user(output_data->phone, phone, strlen(phone)) ||
            copy_to_user(output_data->email, email, strlen(email)) ||
            copy_to_user(&(output_data->age), &age, sizeof(age))) {
            return -EFAULT;
        }

    } else {
        return -1;
    }
    return 0;   
}

asmlinkage long sys_add_user(struct user_data* input_data) {
    struct user_data input;

    if (copy_from_user(input.name, input_data->name, strlen(input_data->name)) ||
            copy_from_user(input.surname, input_data->surname, strlen(input_data->surname)) ||
            copy_from_user(input.phone, input_data->phone, strlen(input_data->phone)) ||
            copy_from_user(input.email, input_data->email, strlen(input_data->email)) ||
            copy_from_user(&(input.age), &(input_data->age), sizeof(input_data->age))) {
            return -EFAULT;
    }
    char query[256];
    sprintf(query, "[add]name: %s\nsurname: %s\nage: %hhu\nphone: %s\nemail: %s\n",
        input.name, input.surname, input.age, input.phone, input.email);


    mm_segment_t old_fs = get_fs();
    set_fs(KERNEL_DS);

    int fd = ksys_open(PHONEBOOK_DEVICE_PATH, O_RDWR, 0);
    ksys_write(fd, query, strlen(query));
    ksys_read(fd, query, 256);
    return 0;
}

asmlinkage long sys_del_user(const char* surname, unsigned int len) {
    char buffer[FIELD_SIZE];
    if (copy_from_user(buffer, surname, min(FIELD_SIZE, len))) {
        return -EFAULT;
    }

    char query[256];
    sprintf(query, "[del]%s", buffer);

    mm_segment_t old_fs = get_fs();
    set_fs(KERNEL_DS);
    int fd = ksys_open(PHONEBOOK_DEVICE_PATH, O_RDWR, 0);

    ksys_write(fd, query, strlen(query));

    char answer[256];
    ksys_read(fd, answer, 256);

    if (strlen(answer) >= 2 && answer[0] == 'O' && answer[1] == 'K') {
        return 0;
    }    
    return 1;
}
