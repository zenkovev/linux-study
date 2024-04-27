# Linux

## Сборка из исходников и запуск ядра

Сначала с официального сайта [https://www.kernel.org/](https://www.kernel.org/) скачиваем
исходники ядра Linux и распаковываем их:
```shell
wget https://cdn.kernel.org/pub/linux/kernel/v6.x/linux-6.7.4.tar.xz
tar -xvf linux-6.7.4.tar.xz
```

Можно посмотреть всевозможные варианты параметров для сборки:
```shell
make help
```

Для сборки ядра нам потребуется конфиг `.config`. Для сборки конфига есть опции:
- `make defconfig`: собрать дефолтный конфиг от разработчиков
- `make tinyconfig`: собрать самый упрощённый конфиг, с которым можно собрать ядро
- `make menuconfig`: запустить псевдографический интерфейс для настройки конфига

Если будем настраивать конфиг через `menuconfig`, стоит включить следующие параметры:
- `64-bit kernel`: так как работаем на 64-битной машине
- `Virtualization`: так как будем запускать ядро на виртуальной машине
- `Enable loadable module support`: так как будем активно работать с модулями ядра,
в том числе писать свои

Хотим собрать и запустить ядро в виртуальной машине QEMU, KVM и QEMU примерно одно и то же,
так как тестировать ядро лучше в виртуальной среде -- не так страшно, если что-то упадёт.

Другие важные параметры сборки ядра:
- `make all`: собрать всё
- `make vmlinux`: собрать ядро
- `make modules`: собрать все модули

Чтобы ускорить сборку, можно запустить её на нескольких ядрах:
- `make all`: обычная сборка
- `make -j9 all`: сборка с использованием 9 ядер
- `nproc --all`: посмотреть число ядер процессора

Обычно указывают число ядер для сборки на 1 больше числа ядер процессора.
В данном случае процессор содержит 8 ядер.

Создаём директорию, из которой будем запускаться:
```shell
mkdir vroot
mkdir vroot/boot
```

После чего собираем ядро:
```shell
make defconfig
# используем дефолтный конфиг
make -j9 all
# собираем все бинарники
INSTALL_PATH=../vroot/boot make install
# устанавливаем результаты сборки по пути INSTALL_PATH
# здесь ещё и переопределяем INSTALL_PATH как ../vroot/boot
```

В некоторых случаях после запуска ядра может потребоваться вернуть мышку из окна
виртуального экрана. Для этого используется сочетание клавиш `Ctrl` + `Alt` + `G`.

Теперь перейдём в директорию `vroot/boot` и запустим ядро.
Это делается с помощью следующей команды:
```shell
qemu-system-x86_64 -kernel ./vmlinuz-6.7.4 --enable-kvm -cpu host -nographic -append "console=ttyS0"
```

Что означают аргументы этой команды:
- `-kernel ./vmlinuz-6.5.2`: путь до бинарника ядра
- `--enable-kvm`: так как запускаемся из под KVM
- `-cpu host`: эмулируем ту же систему, что и хостовая, на компе
- `-nographic -append "console=ttyS0"`: драйвер для вывода информации, консоль

Ещё есть опция `-m`, которая позволяет установить начальный объём используемой памяти.

После запуска ядра получим следующую ошибку:
```shell
[    1.279163] ---[ end Kernel panic - not syncing: VFS: Unable to mount root fs on unknown-block(0,0) ]---
```

Она означает, что ядро не знает, откуда ему запустить процесс init. init -- это самый первый
процесс, который запускается в системе, в самом базовом варианте он запускает оболочку shell.

Чтобы решить эту проблему, создадим файл `initramfs`. По
[этой ссылке](https://wiki.gentoo.org/wiki/Initramfs_-_make_your_own)
есть хорошее объяснение, для чего он нужен, за что отвечает и как его настроить.
Грубо говоря, этот файл содержит все метапараметры диска, он содержит корневую файловую систему,
которая нужна для загрузки системы, в частности, программу init, в которой уже монтируется
правильная файловая система.

Займёмся его настройкой в следующий раз, а сейчас воспользуемся готовой версией `initramfs.gz`,
которую поместим в директорию `vroot/boot`. Итоговая команда для запуска ядра выглядит следующим
образом:
```shell
qemu-system-x86_64 -kernel ./vmlinuz-6.7.4 -initrd initramfs.gz --enable-kvm -cpu host -nographic -append "console=ttyS0"
```

Видим, что ядро запустилось, и можем выполнять команды в оболочке shell. Чтобы выйти из системы,
нужно выполнить команду `poweroff -f`, параметр `-f` отвечает за принудительное выключение.

## Настройка initramfs

Сначала исправим одну небольшую деталь: мы поместили директорию `boot`, в которой находится
бинарник ядра и другие вспомогательные файлы, в директорию `vroot`, содержимое которой будет
помещено в корень файловой системы запускаемого ядра. В директории `vroot` не очень нужна
директория `boot`, поэтому уберём её оттуда:
```shell
rm -R vroot
mkdir vroot
mkdir boot
```

Вернём результаты сборки ядра в `boot`:
```shell
INSTALL_PATH=../boot make install
```

И проверим, что без `initramfs` ядро по-прежнему запускается и получает `kernel panic`:
```shell
qemu-system-x86_64 -kernel ./boot/vmlinuz-6.7.4 --enable-kvm -cpu host -nographic -append "console=ttyS0"
```

Теперь займёмся тем, что должно лежать в корне файловой системы запускаемого ядра.
В директории `vroot` создадим поддиректории:
```shell
mkdir bin dev lib proc root sys tmp
```

За что будут отвечать эти поддиректории:
- `bin`: для бинарных файлов
- `lib`: для библиотек
- `dev`: в неё монтируются псевдоустройства
- `tmp`: для временной информации
- `proc`: системная виртуальная файловая система, содержит информацию о работе ядра
- `sys`: системная виртуальная файловая система, содержит информацию об оборудовании

Теперь создадим исполняемый файл `init`, который будет самым первым процессом, запускаемым
в системе. В нашем случае он будет скриптом, в общем случае это не обязательно скрипт,
он может быть и бинарным файлом:
```shell
vim init
chmod +x init
```

Содержимое файла `init` с добавленными комментариями:
```shell
#!/bin/sh

mount -t devtmpfs devtmpfs /dev
mount -t tmpfs tmpfs /tmp
mount -t proc proc /proc
mount -t sysfs sysfs /sys
# примонтировали системные виртуальные файловые системы

echo 0 > /proc/sys/kernel/printk
# убрали отладочную печать ядра из текущей консоли

exec setsid sh -c 'exec sh </dev/ttyS0 >/dev/ttyS0 2>&1'
# запустили консоль sh и перенаправили выводы
```

Пока ещё не можем запустить `init`, он получит ошибку, так как ему неоткуда взять `sh`.
Хочется иметь `sh` и некоторый набор базовых утилит.

Возьмём эти утилиты из [проекта busybox](https://www.busybox.net/):
```shell
wget https://www.busybox.net/downloads/busybox-1.36.1.tar.bz2
tar -xvf busybox-1.36.1.tar.bz2
```

В директории `busybox-1.36.1` аналогичным образом нужно сделать конфиг и собрать результаты:
```shell
make defconfig
make menuconfig
```

В `menuconfig` в разделе `Settings` устанавливаем опцию `Build static binary (no shared libs)`,
чтобы вся линковка была статической. Сохраняем через `Exit`, `Exit`, `Save`.

Теперь собираем требуемое:
```shell
make -j9 all
make install
```

Результаты сборки `busybox` появятся в директории `busybox-1.36.1/_install`.

Почистить результаты сборки можно через:
```shell
make uninstall
make clean
```

Теперь устанавливаем нужные нам утилиты в `vroot`:
```shell
cd vroot
../busybox-1.36.1/_install/bin/busybox --install ./bin
```

Видим, что в директории `vroot/bin` появилось огромное количество всяких утилит.
Можем запустить `sh`, чтобы убедиться, что всё работает:
```shell
cd bin
./sh
```

Таким образом, создали в `vroot` структуру каталогов. Теперь хотим при помощи некоторого
контейнера сложить эту структуру в файл, после чего этот файл сжать. Есть контейнер `tar`,
есть контейнер `cpio`, который делает что-то похожее, нам нужен последний, сжатие сделаем
с помощью `gz`. Из директории `vroot` выполним команду:
```shell
find . | cpio -ov --format=newc | gzip -9 > ../initramfs
```

Теперь аналогично запуску ядра выше, сделаем запуск с только что собранным `initramfs`,
убедившись, что всё работает:
```shell
qemu-system-x86_64 -kernel ./boot/vmlinuz-6.7.4 -initrd ./initramfs --enable-kvm -cpu host -nographic -append "console=ttyS0"
```

## Написание и загрузка собственного модуля ядра

Сначала подготовим ядро к тому, что мы будем к нему добавлять модули. Для этого из директории
с исходниками ядра `linux-6.7.4` выполняем команду:
```shell
INSTALL_MOD_PATH=../vroot make modules_install
```

Эта команда в указанной через `INSTALL_MOD_PATH` директории создаст поддиректорию `lib/modules`,
в которую добавит что-то полезное, можно посмотреть, что появилось, в самой директории
`vroot/lib/modules`. После нужно будет пересобрать `initramfs`, но мы это сделаем немного позже.

Теперь создадим какую-то директорию, в которой будем писать модули, например, `awesome_modules`,
в ней поддиректорию для первого рукописного модуля `example`. Там создаём `Makefile` и файл
с кодом модуля.

Содержимое `Makefile`:
```make
obj-m += example.o 

all:
	# make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	make -C /home/evgeniy/main/MIPT_new_sem/linux/vroot/lib/modules/6.7.4/build M=$(PWD) modules
clean:
	# make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	make -C /home/evgeniy/main/MIPT_new_sem/linux/vroot/lib/modules/6.7.4/build M=$(PWD) clean
```

Закомментированные строчки нужны, если в результате хотим добавить модуль к хостовой системе,
раскомментированные, если к виртуальной, которую выше запускали руками. Команда `uname -r`
возвращает текущую версию ядра. 

Сделаем ещё пару комментариев по синтаксису `Makefile`. `make -C /some/path` означает, что мы,
по сути, заинклюдим `Makefile` из директории `/some/path` в наш текущий `Makefile`. При этом
здесь в цели `all` мы указываем, что будем исполнять цель `modules` импортированного `Makefile`,
в цели `clean`, соответственно, `clean`. В запуск того импортированного `Makefile` мы также
передаём переменную `M`, которая ссылается на нашу текущую директорию.

Содержимое файла с кодом модуля `example.c`:
```c
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Melges");
MODULE_DESCRIPTION("MIPT Example");
MODULE_VERSION("0.01");

static int __init mipt_example_init(void) {
  printk(KERN_INFO "Hello, World!\n");
  return 0;
}
static void __exit mipt_example_exit(void) {
  printk(KERN_INFO "Goodbye, World!\n");
}

module_init(mipt_example_init);
module_exit(mipt_example_exit);
```

Теперь выполняем `make all`. Видим, что, в частности, появился скомпилированный код модуля
`example.ko`. Он нам понадобится внутри запущенного ядра, поэтому переместим его в некоторую
директорию внутри `vroot`:
```shell
mkdir -p vroot/home/awesome_modules
cp awesome_modules/example/example.ko vroot/home/awesome_modules/
```

Теперь пересобираем `initramfs` и запускаем ядро:
```shell
cd vroot
find . | cpio -ov --format=newc | gzip -9 > ../initramfs
cd ..
qemu-system-x86_64 -kernel ./boot/vmlinuz-6.7.4 -initrd ./initramfs --enable-kvm -cpu host -nographic -append "console=ttyS0"
```

Внутри запущенного ядра перейдём в директорию `/home/awesome_modules`,
откуда можем установить наш модуль:
```shell
modinfo example.ko
# посмотреть метаинформацию о модуле

insmod example.ko
# установить модуль
dmesg
# посмотреть логи ядра
# должна появиться строчка вида:
# [   56.615270] Hello, World!

lsmod
# посмотреть список установленных модулей
# должно быть что-то вроде:
# example 12288 0 - Live 0xffffffffc0304000 (O)

rmmod example.ko
# удалить модуль
dmesg
# посмотреть логи ядра
# должна появиться строчка вида:
# [   81.748514] Goodbye, World!
```

## Пишем собственный syscall

Для написания своих системных вызовов нет иных вариантов, кроме как вносить изменения в исходники
ядра. Чтобы эти изменения было удобно передавать и сохранять вне исходников ядра, можно
воспользоваться механизмом патчей гита. Патчем является специальный файл, который содержит
информацию об изменениях по сравнению с последним коммитом.

Подготовка, чтобы можно было делать патчи:
```shell
cd linux-6.7.4
git init
git status

git add .
git commit -m "kernel sources"
git status
```

Мы зафиксировали исходное состояние ядра. Теперь, после внесения изменений в код ядра,
можно будет сохранить патч:
```shell
git diff > patch.txt
# сохранить текущие изменения в патч
# не учитывает новые созданные файлы
# не учитывает файлы, добавленные через add
git diff --cached > patch.txt
# сохранить текущие изменения в патч
# учитывает файлы, добавленные через add, в том числе новые
git apply patch.txt
# применить изменения из патча в текущий репозиторий
```

Патч с изменениями кода ядра при реализации собственного системного вызова сохранён в директории
`awesome_modules` как `patch_syscall.txt`. Переименовывать директорию я поленился.

Тут есть ещё один нюанс. Так как мы создали гит-репозиторий, то в случае наличия незакоммиченных
изменений при сборке бинарника ядра к его имени будет добавляться суффикс `dirty`. Это можно
отключить следующим образом:
```shell
# Внесение изменений в конфиг
make menuconfig
# -> General Setup
# -> Automatically append version information to the version string
# -> N

# Сборка бинарника через make
# Нужно очистить переменную окружения LOCALVERSION
LOCALVERSION="" make -j9 all
INSTALL_PATH=../boot make install
```

Теперь приступим непосредственно к написанию сисколла. Полезные ссылки:
- [Шпаргалка по сисколлам](https://www.kernel.org/doc/html/v6.7/process/adding-syscalls.html)
- [Cсылка на нужный раздел](https://www.kernel.org/doc/html/v6.7/process/adding-syscalls.html#generic-system-call-implementation)

Сначала создадим файл для вызываемой при сисколле функции:
```shell
cd linux-6.7.4
mkdir print_to_dmesg
cd print_to_dmesg
vim print_to_dmesg.c
```

И напишем реализацию вызываемой при сисколле функции:
```c
#include <linux/kernel.h>
#include <linux/syscalls.h>

SYSCALL_DEFINE0(print_dmesg) {
  printk("Hello, System World!\n");
  return 0;
}
```

Некоторые комментарии:
- макрос `SYSCALL_DEFINE0` определит функцию следующего вида:
```c
asmlinkage long sys_print_dmesg(void);
```
- здесь `void` нужен, потому что функция не принимает аргументы
- функция `printk` печатает строку в логи ядра, которые можно посмотреть через `dmesg`

В этой же директории создадим `Makefile` следующего содержания:
```
obj-y := print_to_dmesg.o
```

Теперь нужно, чтобы при сборке ядра этот `Makefile` вызывался. Открываем основной `Makefile`:
```shell
cd linux-6.7.4
vim Makefile
```

Находим в этом `Makefile` следующую строчку:
```
core-y    :=
```

Добавляем туда директорию с нашими `syscall` и `Makefile`:
```
core-y    := print_to_dmesg/
```

Теперь нужно добавить только что созданный `syscall` в общую таблицу.
Сначала находим эту самую таблицу:
```shell
cd linux-6.7.4
cd arch
find . -type f -name syscall_64.tbl
# ./x86/entry/syscalls/syscall_64.tbl
vim ./x86/entry/syscalls/syscall_64.tbl
```

После `common` части добавляем свой системный вызов на свободный номер:
```
457 common  print_dmesg   sys_print_dmesg
# слева название функции в userspace
# справа название функции внутри ядра, в точности название, создаваемое макросом
```

Ещё нужно записать объявление сисколла в файле `linux-6.7.4/include/linux/syscalls.h`.
В самом конце файла записываем строку:
```c
asmlinkage long sys_print_dmesg(void);
```

Теперь пересобираем ядро, убеждаясь, что всё компилируется без ошибок:
```shell
LOCALVERSION="" make -j9 all
INSTALL_PATH=../boot make install
INSTALL_MOD_PATH=../vroot make modules_install
```

Таким образом, написание системного вызова прошло в три этапа:
1. Определили функцию и создали `Makefile`
2. Дописали вызов нового `Makefile` в общий `Makefile`
3. Объявили прототип сисколла в общей таблице и общем хедере

Системный вызов написали, теперь поймём, как его вызывать из кода пользователя.

Переходим в директорию `vroot/home`, пишем программу `example.c`:
```c
#include <unistd.h>

int main() {
  syscall(457);
}
```

Так как у нас нет компилятора внутри собранного вручную ядра, скомпилируем программу на текущей
операционной системе. В целом, не важно, каким компилятором собирать прикладную программу,
это имеет значение лишь в том, что ядро и модули ядра нужно собирать одним и тем же компилятором. 
```shell
gcc example.c -o example
```

Теперь, так как линковка программы динамическая, нам нужно положить внутрь запускаемого ядра
требуемые библиотеки. Проще всего их взять с основной операционной системы:
```shell
ldd example
# linux-vdso.so.1 (0x00007fffb2527000)
# libc.so.6 => /lib/x86_64-linux-gnu/libc.so.6 (0x00007fac90000000)
# /lib64/ld-linux-x86-64.so.2 (0x00007fac903dc000)

# посмотрели требуемые shared-библиотеки
# linux-vdso.so.1 это псевдобиблиотека, загрузчик shared-либ, её игнорируем
# остальные добавим

cd vroot
mkdir lib64
cp /lib/x86_64-linux-gnu/libc.so.6 lib/
cp /lib64/ld-linux-x86-64.so.2 lib/
cp /lib/x86_64-linux-gnu/libc.so.6 lib64/
cp /lib64/ld-linux-x86-64.so.2 lib64/
```

После этого пересобираем `initramfs` и запускаем ядро:
```shell
cd vroot
find . | cpio -ov --format=newc | gzip -9 > ../initramfs
qemu-system-x86_64 -kernel ./boot/vmlinuz-6.7.4 -initrd ./initramfs --enable-kvm -cpu host -nographic -append "console=ttyS0"
```

Запускаем внутри ядра программу и видим, что вывод появился в логах ядра:
```shell
./example
dmesg
# [   15.428368] Hello, System World!
# [   15.428581] example (54) used greatest stack depth: 13808 bytes left
```