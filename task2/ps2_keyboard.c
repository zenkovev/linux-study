#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/input.h>
#include <linux/timer.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("zenkov_ev");
MODULE_DESCRIPTION("MIPT Linux PS2 Keyboard");
MODULE_VERSION("0.01");

/* char xcb_to_ascii(unsigned char xcb_keycode) {
  static const char xcb_to_ascii_table[] = {
    '.', '.', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '.', '.', '.', '.',
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '.', '.', '.', '.', 'a', 's',
    'd', 'f', 'g', 'h', 'j', 'k', 'l', '.', '.', '.', '.', '.', 'z', 'x', 'c', 'v',
    'b', 'n', 'm', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.',
    '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.',
    '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.',
    '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.',
    '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.'
  };

  if (xcb_keycode < sizeof(xcb_to_ascii_table)) {
    return xcb_to_ascii_table[xcb_keycode];
  }
  return 0;
} */

static atomic64_t symbols_counter;

irqreturn_t ps2_keyboard_event(int irq, void* dev_id) {
  /* int ps2_keyboard_register = 0x60;
  int ps2_keyboard_mask = 0x7f;
  int ps2_keyboard_pressed = 0x80;

  unsigned char symbol = inb(ps2_keyboard_register);
  if ((symbol & ps2_keyboard_pressed) == 0) {
    printk(KERN_INFO "Key is pressed\n");
  } else {
    printk(KERN_INFO "Key is released\n");
  }
  char key = xcb_to_ascii(symbol & ps2_keyboard_mask);
  printk(KERN_INFO "Symbol ASCII: %c\n", key); */

  int ps2_keyboard_register = 0x60;
  int ps2_keyboard_pressed = 0x80;

  unsigned char symbol = inb(ps2_keyboard_register);
  if ((symbol & ps2_keyboard_pressed) == 0) {
    atomic64_add(1, &symbols_counter);
  }

  return IRQ_HANDLED;
}

static struct timer_list timer;

static void timer_handler(struct timer_list* obj) {
  int count = atomic64_xchg(&symbols_counter, 0);
  printk(KERN_INFO "Symbols typed in last ten seconds: %d\n", count);
  mod_timer(&timer, jiffies + 10 * 1000);
}

static int __init mipt_keyboard_init(void) {
  int ps2_keyboard_irq = 1;
  int err = request_irq(ps2_keyboard_irq, ps2_keyboard_event, IRQF_SHARED, "ps2_keyboard", (void*)(ps2_keyboard_event));
  if (err != 0) {
    return 1;
  }

  timer_setup(&timer, timer_handler, 0);
  atomic64_set(&symbols_counter, 0);
  mod_timer(&timer, jiffies + 5 * 1000);

  printk(KERN_INFO "PS/2 Keyboard Counter started!\n");
  return 0;
}

static void __exit mipt_keyboard_exit(void) {
  int ps2_keyboard_irq = 1;
  free_irq(ps2_keyboard_irq, (void*)(ps2_keyboard_event));

  timer_shutdown_sync(&timer);

  printk(KERN_INFO "PS/2 Keyboard Counter finished!\n");
}

module_init(mipt_keyboard_init);
module_exit(mipt_keyboard_exit);