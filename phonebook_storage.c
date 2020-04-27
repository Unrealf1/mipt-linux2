#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/slab.h>
#include "phonebook_storage.h"

static struct phone_book_entry* list_head = NULL;
static struct phone_book_entry* list_tail = NULL;

struct phone_book_entry* find_prev (char *surname) {
    // Returns NULL if surname matched head or list is empty, 
    // returns last element, if there isn't such surname in the list.

    struct phone_book_entry* current_entry = list_head;
    struct phone_book_entry* prev = NULL;

    while (current_entry != NULL) {

        if (strncmp(current_entry->surname, surname, FIELD_SIZE) == 0) {
            printk(KERN_INFO "Found entry!");
            return prev;
        }
        prev = current_entry;
        current_entry = current_entry->next;
    }
    printk(KERN_INFO "Failed to find entry");
    return NULL;
}

struct phone_book_entry* get_entry (char *surname) {
    printk(KERN_INFO "Looking for surname %s\n", surname);
    struct phone_book_entry* prev = find_prev(surname);
    if (prev == NULL) {
        return list_head;
    } else {
        return prev->next;
    }
}

void add_entry (char *name, char *surname, uint8_t age, char* phone, char* email) { 
    struct phone_book_entry* new_entry = kmalloc(sizeof(struct phone_book_entry), GFP_KERNEL);

    strncpy(new_entry->name, name, FIELD_SIZE);
    strncpy(new_entry->surname, surname, FIELD_SIZE);
    new_entry->age = age;
    strncpy(new_entry->phone, phone, FIELD_SIZE);
    strncpy(new_entry->email, email, FIELD_SIZE);
    new_entry->next = NULL;

    if (list_tail == NULL) {
        list_head = new_entry;
        list_tail = new_entry;
    } else {
        list_tail->next = new_entry;
        list_tail = new_entry;
    }
}

int remove_entry (char* surname) {
    if (list_head == NULL) {
        return -1;
    }
    struct phone_book_entry* prev = find_prev(surname);
    if (prev == NULL) {
        struct phone_book_entry* old_head = list_head;
        list_head = list_head->next;
        kfree(old_head);
    } else {
        if (prev->next == NULL) {
            return -1;
        }
        struct phone_book_entry* to_remove = prev->next;
        prev->next = to_remove->next;
        kfree(to_remove);
    }
    return 0;
}

void list_clear(void) {
    struct phone_book_entry* current_entry = list_head;
    while (current_entry != NULL) {
        struct phone_book_entry* next = current_entry->next;
        kfree(current_entry);
        current_entry = next;
    }
    list_head = NULL;
    list_tail = NULL;
}
