obj-m += spying_module.o
# spying_module-objs+= phonebook_storage.o my_phonebook_module.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
