#include <linux/types.h>

#define FIELD_SIZE 32

struct phone_book_entry {
    char name[FIELD_SIZE];
    char surname[FIELD_SIZE];
    uint8_t age;
    char phone[FIELD_SIZE];
    char email[FIELD_SIZE];
    struct phone_book_entry* next;
};

struct phone_book_entry* get_entry (char *surname);
void add_entry (char *name, char *surname, uint8_t age, char* phone, char* email);
int remove_entry (char *surname);
void clear(void);
