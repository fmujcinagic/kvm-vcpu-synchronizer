# kvm-vcpu-synchronizer
A hypervisor-mediated, cross-vCPU synchronization pipeline using custom KVM wait-queues

This is a simulation of a secure cross-core recovery architecture—similar to the VMSA (Virtual Machine Save Area) re-validation process found in hardware TEEs like **AMD SEV-SNP**—using a software-only approach in standard nested KVM (Linux v7.1.3).

## Overview

In confidential computing environments, if a guest core violates a hardware-enforced memory state, the hypervisor cannot fix it alone. The hypervisor must park the faulting vCPU and delegate the recovery to a secondary, healthy core within the guest's TEE.

Because standard hardware lacks the AMD Secure Processor and the RMP, this project replicates the fundamental OS-level scheduling. It allows a guest OS to dynamically park its own vCPU in the KVM hypervisor and wait for a secondary core to "rescue" it.

## Architecture

* **Host Boundary (KVM):** Intercepts custom `vmmcall` hypercalls. Utilizes KVM `wait_event` and `wake_up` primitives to push the target vCPU thread into a secure `TASK_UNINTERRUPTIBLE` sleep state.
* **Guest Module (`sim.c`):** An out-of-tree kernel module exposing a `/proc` interface to trigger the hypercalls without locking the guest kernel.
* **Payload Routing:** Passes mock physical address payloads (simulating the VMSA pointer) across the isolation boundary via the `RBX` register.

## Execution

Booting a VM is recommended (so the physical host is not used as a hypervisor for this experiment). E.g. boot Ubuntu 26 and load the Linux kernel 7.1.3.

After kernel is loaded, we can hot swap from there quickly, without having to reload the kernel every time. Replace the standard `arch/x86/kvm/x86.c` in your kernel tree with the provided file. Compile and reload the KVM modules:

```Bash
make M=arch/x86/kvm
sudo rmmod kvm_amd kvm
sudo insmod arch/x86/kvm/kvm.ko
sudo insmod arch/x86/kvm/kvm-amd.ko
```

Setup the `initramfs` using the BusyBox (minimum requirements), and boot the guest with the script:

```Bash
./scripts/boot_qemu_guest.sh
```

### Running the Simulation
Inside the guest terminal, load the module and execute the automation script to simulate the cross-core blackout and rescue:

```
/ # insmod /root/sim.ko
/ # ./run_tests.sh
```

## Expected Output from This Demo:

Guest logs: 
```bash
Freezing Core 0...
[   47.941324] Guest: Starting sensitive timeframe (Freezing)...
Rescuing with Core 1...
[   48.943915] Guest: Rescue Core sending re-validation signal...
[   48.947675] Guest: VMSA re-validated! Resuming safely.
```

Hypervisor logs:
```bash
[ 1634.081034] vCPU 0 triggered security fault. Parking indefinitely...
[ 1635.084524] vCPU 1 completed recovery calculation. Waking blocked cores...
[ 1635.084555] vCPU 0 rescued! Resuming execution.
```
