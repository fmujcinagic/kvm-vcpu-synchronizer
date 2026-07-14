#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/proc_fs.h>

static inline long kvm_hypercall1(unsigned int nr, unsigned long p1)
{
    long ret;
    asm volatile("vmmcall"
                 : "=a"(ret)
                 : "a"(nr), "b"(p1)
                 : "memory");
    return ret;
}

// Executed when you run: cat /proc/target
static ssize_t target_read(struct file *file, char __user *buf, size_t count, loff_t *pos)
{
    if (*pos > 0) return 0; // Prevent infinite reading loop
    printk(KERN_INFO "Guest: Starting sensitive timeframe (Freezing)...\n");
    
    kvm_hypercall1(0x1337, 0x12345678); // starting sensitive operation // this 0x1234... acts as a dummy placeholder for the VMSA
    
    printk(KERN_INFO "Guest: VMSA re-validated! Resuming safely.\n");
    return 0; 
}

// Executed when you run: cat /proc/rescue
static ssize_t rescue_read(struct file *file, char __user *buf, size_t count, loff_t *pos)
{
    if (*pos > 0) return 0;
    printk(KERN_INFO "Guest: rescue Core sending re-validation signal...\n");

    kvm_hypercall1(0x1338, 0); // rescuing

    return 0;
}

static const struct proc_ops target_ops = { .proc_read = target_read };
static const struct proc_ops rescue_ops = { .proc_read = rescue_read };

static int __init sim_init(void)
{
    proc_create("target", 0666, NULL, &target_ops);
    proc_create("rescue", 0666, NULL, &rescue_ops);
    printk(KERN_INFO "Guest: Module loaded. Virtual files created in /proc/\n");
    return 0;
}

static void __exit sim_exit(void)
{
    remove_proc_entry("target", NULL);
    remove_proc_entry("rescue", NULL);
    printk(KERN_INFO "Guest: Module unloaded.\n");
}

module_init(sim_init);
module_exit(sim_exit);
MODULE_LICENSE("GPL");
