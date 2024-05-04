#!/bin/sh

insmod phone_book.ko
mknod /dev/phone_book c 248 0

echo "add Ivan_Ivanov 73 +79997779999 ahaha@mail.ru" > /dev/phone_book
echo "add Petr_Petrov 57 +79991119999 ura@mail.ru" > /dev/phone_book
echo "add Semyon_Sidorov 52 +79992229999 privet@mail.ru" > /dev/phone_book
echo "add Vasya_Kozlov 64 +79993339999 medved@mail.ru" > /dev/phone_book
echo "add Kozel_Vasilyev 69 +79995559999 baran@mail.ru" > /dev/phone_book

echo "-----"
cat /dev/phone_book
echo "-----"

./check

echo "-----"
cat /dev/phone_book
echo "-----"

rm /dev/phone_book
rmmod phone_book.ko

dmesg | tail
