#!/bin/sh

insmod phone_book.ko
mknod /dev/phone_book c 248 0

cat /dev/phone_book
cat /dev/phone_book
cat /dev/phone_book
cat /dev/phone_book
cat /dev/phone_book

echo "add Ivan_Ivanov 73 +79997779999 ahaha@mail.ru" > /dev/phone_book
echo "add Ivan_Ivanov 73 +79997779999 ahaha@mail.ru" > /dev/phone_book
echo "ahaha" > /dev/phone_book
echo "" > /dev/phone_book

echo "-----"
cat /dev/phone_book
echo "-----"
cat /dev/phone_book
echo "-----"

echo "add Petr_Petrov 57 +79991119999 ura@mail.ru" > /dev/phone_book
echo "add Semyon_Sidorov 52 +79992229999 privet@mail.ru" > /dev/phone_book
echo "add Vasya_Kozlov 64 +79993339999 medved@mail.ru" > /dev/phone_book
echo "add Kozel_Vasilyev 69 +79995559999 baran@mail.ru" > /dev/phone_book

echo "-----"
cat /dev/phone_book
echo "-----"
cat /dev/phone_book | grep "^Ivan_Ivanov$(printf '\t')"
echo "-----"

echo "del Ivan_Ivanov" > /dev/phone_book
echo "del Semyon_Sidorov" > /dev/phone_book
echo "del Baran_Ivanov" > /dev/phone_book

echo "-----"
cat /dev/phone_book
echo "-----"

echo "add Ivan_Ivanov 73 +79997779999 ahaha@mail.ru" > /dev/phone_book
echo "add Ivan_Gornokozlov 95 +79997179999 haha@mail.ru" > /dev/phone_book

echo "-----"
cat /dev/phone_book
echo "-----"

rm /dev/phone_book
rmmod phone_book.ko

dmesg | tail