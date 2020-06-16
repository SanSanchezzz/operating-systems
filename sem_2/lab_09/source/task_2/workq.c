#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/time.h>

MODULE_AUTHOR("Alexander Kupry");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("lab_09_task_2");

#define HANDLEDIRQ 1

static int irq = HANDLEDIRQ;
static int irq_call_count = 0;
static int dev_id;
static struct workqueue_struct *workq = NULL;

void work_function(struct work_struct *work)
{
    struct timeval t;
    struct tm brocken;
    do_gettimeofday(&t);
    time_to_tm(t.tv_sec, 0, &brocken);

    printk(KERN_INFO "WORKQUEUE MODULE\n{ data: %ld }, current_time: %d:%d:%d:%ld\n",
        atomic_long_read(&work->data),
        brocken.tm_hour + 3, brocken.tm_min, brocken.tm_sec, t.tv_usec);
}

DECLARE_WORK(work, work_function);

static irqreturn_t interrupt_handler(int irq, void *dev_id)
{
    if (irq == HANDLEDIRQ)
    {
        irq_call_count++;
        queue_work(workq, &work);
        printk(KERN_INFO "WORKQUEUE MODULE\nirq call count = %d\n", irq_call_count);
        return IRQ_HANDLED;
    }
    else
    {
        return IRQ_NONE;
    }
}

static int __init workqueue_module_init(void)
{
    int ret = request_irq(irq, interrupt_handler, IRQF_SHARED, "workqueue_interrupt_handler", &dev_id);

    if (ret)
    {
        printk(KERN_ERR "WORKQUEUE MODULE\nerror while handle irq\n");
        return -1;
    }

    workq = create_workqueue("workqueue");

    if (workq == NULL)
    {
        printk(KERN_ERR "WORKQUEUE MODULE\nerror while create workqueue\n");
        return -1;
    }

    printk(KERN_INFO "WORKQUEUE MODULE\nsuccess load\n");
    return 0;
}

static void __exit workqueue_module_exit(void)
{
    flush_workqueue(workq);
    destroy_workqueue(workq);
    free_irq(irq, &dev_id);
    printk(KERN_INFO "WORKQUEUE MODULE\nunload module\n");
}

module_init(workqueue_module_init);
module_exit(workqueue_module_exit);
