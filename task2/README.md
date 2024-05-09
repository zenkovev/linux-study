# Комментарии к заданию

## Демонстрация

Демонстрация находится в файле `demonstration.mp4`. Для удобства демонстрации сократил
время для сбора логов до 10 секунд, думаю, это не очень принципиально.

## Эмуляция клавиатуры PS/2

Можно посмотреть, кто занимает IRQ линию, и убедиться, что виртуальная машина действительно
умеет эмулировать PS/2 клавиатуру:
```shell
cat /proc/interrupts
# на родной операционной системе:
# 1:          0          0          0          0      13196          0          0          0  IR-IO-APIC    1-edge      i8042
# внутри виртуальной машины:
# 1:          9   IO-APIC   1-edge      i8042, ps2_keyboard
```

Однако символы, которые печатаются в оболочке shell, не относятся к этой клавиатуре. Чтобы
воспользоваться виртуальной клавиатурой, нужно выполнить команды qemu, так как оболочка
shell перехватывает ввод для qemu, нужно создать специальный сокет. Отличнейшее описание
есть здесь: [ссылка](https://unix.stackexchange.com/questions/426652/connect-to-running-qemu-instance-with-qemu-monitor).

Кратко полезные команды из ссылки:
```shell
qemu-system-i386 [..other params..] -monitor unix:qemu-monitor-socket,server,nowait
# запуск qemu

echo "sendkey a" | socat - unix-connect:qemu-monitor-socket && echo
# печать одного символа
```

## Полезные ссылки для реализации

- [Обработка прерываний](https://linux-kernel-labs.github.io/refs/heads/master/labs/interrupts.html)
- `linux-6.7.4/include/linux/timer.h`: методы для таймера
- [Пример использования таймера](https://docs.kernel.org/core-api/local_ops.html#reading-the-counters)