// modified hypervisor kernel patch - /arch/x86/kvm/x86.c
// PoC Linux kernel version - 7.1.3.


// declarations at the top
#include <linux/wait.h>

static DECLARE_WAIT_QUEUE_HEAD(thesis_recovery_wq);
static bool thesis_recovery_done = false;

// existing code ...
// modifying int ____kvm_emulate_hypercall function
int ____kvm_emulate_hypercall(struct kvm_vcpu *vcpu, int cpl,
                              int (*complete_hypercall)(struct kvm_vcpu *))
{


    // existing code ...
      switch (nr) {
        case KVM_HC_VAPIC_POLL_IRQ:
                ret = 0;
                break;
        case KVM_HC_KICK_CPU:
                if (!guest_pv_has(vcpu, KVM_FEATURE_PV_UNHALT))
                        break;

                kvm_pv_kick_cpu_op(vcpu->kvm, a1);
                kvm_sched_yield(vcpu, a1);
                ret = 0;
                break;
#ifdef CONFIG_X86_64
        case KVM_HC_CLOCK_PAIRING:
                ret = kvm_pv_clock_pairing(vcpu, a0, a1);
                break;
#endif
        // existing code ...
        /* --- hypercall --- */
        case 0x1337: {
                printk(KERN_ALERT "vCPU %d triggered security fault. Parking indefinitely...\n", vcpu->vcpu_id);
                thesis_recovery_done = false;
                
                // wait_event (uninterruptible) to ignore QEMU signals
                wait_event(thesis_recovery_wq, thesis_recovery_done);
                
                printk(KERN_ALERT "vCPU %d rescued! Resuming execution.\n", vcpu->vcpu_id);
                ret = 0;
                break;
        }
        // rescue hypercall - wake up blocked cores
        case 0x1338: {
                printk(KERN_ALERT "vCPU %d completed recovery calculation. Waking blocked cores...\n", vcpu->vcpu_id);
                thesis_recovery_done = true;
                
                wake_up(&thesis_recovery_wq);
                
                ret = 0;
                break;
        }
        /* --- */
        default:
                ret = -KVM_ENOSYS;
                break;
        }

out:
        vcpu->run->hypercall.ret = ret;
        return 1;
}

// existing code ...