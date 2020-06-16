#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/string.h>
#include <linux/vmalloc.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Kupry");

static struct proc_dir_entry *proc_entry;
static struct proc_dir_entry *proc_dir;
static struct proc_dir_entry *proc_link;

static char *buffer;
static int buf_index, next_fortune;

static ssize_t fortune_write(struct file *filp, const char __user *buf, size_t len, loff_t *offp);
static ssize_t fortune_read(struct file *filp, char __user *buf, size_t count, loff_t *offp);

static int fortune_init(void)
{
    static struct file_operations fileops =
    {
        .owner = THIS_MODULE,
        .read = fortune_read,
        .write = fortune_write
    };

    buffer = (char *)vmalloc(PAGE_SIZE);
    if (!buffer)
    {
        printk(KERN_ERR "fortune: no memory for create buffer\n");
        return -ENOMEM;
    }

    memset(buffer, 0, PAGE_SIZE);

    proc_entry = proc_create("fortune", 0644, NULL, &fileops);
    if (proc_entry == NULL)
    {
        vfree(buffer);
        printk(KERN_ERR "fortune: couldn't create proc entry\n");
        return -ENOMEM;
    }

    proc_dir = proc_mkdir("fortune_dir", NULL);
    if (!proc_dir)
    {
        vfree(buffer);
        printk(KERN_ERR "fortune: Couldn't create proc dir, symlink\n");
        return -ENOMEM;
    }

    proc_link = proc_symlink("fortune_symlink", NULL, "/proc/fortune");
    if (!proc_link)
    {
        vfree(buffer);
        printk(KERN_ERR "fortune: Couldn't create proc dir, symlink\n");
        return -ENOMEM;
    }

    buf_index = 0;
    next_fortune = 0;

    printk(KERN_INFO "fortune: module loaded.\n");
    return 0;
}

static ssize_t fortune_read(struct file *filp, char __user *buf, size_t count, loff_t *offp)
{
    if (*offp > 0)
    {
        return 0;
    }

    if (next_fortune >= buf_index)
    {
        next_fortune = 0;
    }

    count = copy_to_user(buf, &buffer[next_fortune], count);
    next_fortune += count;
    *offp += count;

    return count;
}

static ssize_t fortune_write(struct file *filp, const char __user *buf, size_t len, loff_t *offp)
{
    int available_space = PAGE_SIZE - buf_index + 1;

    if (len > (size_t)available_space)
    {
        printk(KERN_NOTICE "fortune: there isn't enough memory\n");
        return -ENOSPC;
    }

    if (copy_from_user(&buffer[buf_index], buf, len))
    {
        printk(KERN_NOTICE "fortune: copy_to_user failed\n");
        return -EFAULT;
    }

    buf_index += len;
    buffer[buf_index - 1] = 0;

    return len;
}

static void fortune_exit(void)
{
    proc_remove(proc_entry);
    proc_remove(proc_dir);
    proc_remove(proc_link);

    vfree(buffer);

    printk(KERN_INFO "fortune: module unloaded.\n");
}

module_init(fortune_init);
module_exit(fortune_exit);
