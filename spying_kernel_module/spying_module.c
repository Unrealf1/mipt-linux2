#include <linux/init.h>
#include <linux/module.h>
#include <linux/string.h>
#include <linux/interrupt.h>
#include <linux/kthread.h>
#include <linux/delay.h>

#define  CLASS_NAME  "spy"

MODULE_LICENSE("GPL");

static struct task_struct *kthread;

static struct actions { atomic_long_t counter; } total_actions;
static long long actions_last_minute = 0;

static struct tasklet_struct tasklet;

static void do_bottom(unsigned long unused) {
    atomic_long_inc(&(total_actions.counter));
    printk(KERN_INFO "Spy: bottom executed\n");
}

static irqreturn_t do_top(int irq, void* dev_id) {
    tasklet_schedule(&tasklet);
    return IRQ_HANDLED;
}

static int printer(void* data) {
    int i = 0;
    while (!kthread_should_stop()) {
        long long current_actions = atomic_long_read(&(total_actions.counter));
        printk(KERN_INFO "Spy: %d minutes passed. %lld clicks in total, %lld clicks in the last minute\n",
            i, current_actions, current_actions - actions_last_minute);
        ++i;
        actions_last_minute = current_actions;
        msleep(1000 * 60);
    }
    return 0;
}

static int __init spy_init(void) {
    atomic_long_set(&(total_actions.counter), 0);
    actions_last_minute = 0;

    int req = request_irq(1, do_top, IRQF_SHARED, "InterruptSpy", &tasklet);
    if (req < 0) {
        printk(KERN_ALERT "Spy: module load failed\n");
        return req;
    }

    tasklet_init(&tasklet, do_bottom, 0);

    kthread = kthread_create(printer, NULL, "my_printer_thread");
    wake_up_process(kthread);

    printk(KERN_INFO "Spy: module loaded\n");

    return 0;
}

static void __exit spy_exit(void) {
    free_irq(1, &tasklet);
    tasklet_kill(&tasklet);
    kthread_stop(kthread);
}

module_init(spy_init);
module_exit(spy_exit);
