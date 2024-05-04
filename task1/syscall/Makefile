obj-m += phone_book.o

all:
	make -C ${VROOT_PATH}/lib/modules/6.7.4/build M=$(PWD) modules
clean:
	make -C ${VROOT_PATH}/lib/modules/6.7.4/build M=$(PWD) clean
