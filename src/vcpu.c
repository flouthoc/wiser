#include <asm/bootparam.h>
#include <asm/processor-flags.h>
#include <err.h>
#include <fcntl.h>
#include <linux/kvm.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "devices.h"
#include "vcpu.h"

int cpu_create(struct vm *vm) {
  int ret;
  vm->fd_vcpu = ioctl(vm->fd_vm, KVM_CREATE_VCPU, (unsigned long)0);
  if (vm->fd_vcpu < 0)
    err(1, "KVM_CREATE_VCPU");
  ret = cpu_setup_cpuid(vm);
  if (ret < 0)
    err(1, "KVM_SET_CPUID");
  ret = cpu_mmap(vm);
  if (ret < 0)
    err(1, "CPU_MMAP");

  return 0;
}

int cpu_setup_cpuid(struct vm *vm) {
  struct {
    struct kvm_cpuid a;
    struct kvm_cpuid_entry b[4];
  } id;
  id.a.nent = 4;
  id.a.entries[0].function = 0;
  id.a.entries[0].eax = 1;
  id.a.entries[0].ebx = 0;
  id.a.entries[0].ecx = 0;
  id.a.entries[0].edx = 0;
  id.a.entries[1].function = 1;
  id.a.entries[1].eax = 0x400;
  id.a.entries[1].ebx = 0;
  id.a.entries[1].ecx = 0;
  id.a.entries[1].edx = 0x701b179;
  id.a.entries[2].function = 0x80000000;
  id.a.entries[2].eax = 0x80000001;
  id.a.entries[2].ebx = 0;
  id.a.entries[2].ecx = 0;
  id.a.entries[2].edx = 0;
  id.a.entries[3].function = 0x80000001;
  id.a.entries[3].eax = 0;
  id.a.entries[3].ebx = 0;
  id.a.entries[3].ecx = 0;
  id.a.entries[3].edx = 0x20100800;
  if (ioctl(vm->fd_vcpu, KVM_SET_CPUID, &id.a) < 0)
    err(1, "KVM_SET_CPUID failed");

  return 0;
}

int cpu_mmap(struct vm *vm) {
  if (vm->mmap_size < sizeof(*vm->run))
    errx(1, "KVM_GET_VCPU_MMAP_SIZE too small");

  vm->run = mmap(NULL, vm->mmap_size, PROT_READ | PROT_WRITE, MAP_SHARED,
                 vm->fd_vcpu, 0);
  if (!vm->run)
    err(1, "mmap vcpu");

  return 0;
}

int cpu_setup_protected_mode(struct vm *vm) {
  int ret;
  ret = ioctl(vm->fd_vcpu, KVM_GET_SREGS, &vm->sregs);
  if (ret < 0)
    err(1, "KVM_GET_SREGS");

  struct kvm_segment segments = {
      .base = 0,
      .limit = 0xffffffff,
      .selector = 0x8,
      .present = 1,
      .type = 11,
      .dpl = 0,
      .db = 1,
      .s = 1,
      .l = 0,
      .g = 1,
  };

  vm->sregs.cs = segments;

  segments.type = 3;
  segments.selector = 0x10;
  vm->sregs.ds = vm->sregs.es = vm->sregs.ss = segments;
  vm->sregs.cr0 |= X86_CR0_PE;
  vm->sregs.cr4 &= ~(1U << 5);

  ret = ioctl(vm->fd_vcpu, KVM_SET_SREGS, &vm->sregs);
  if (ret < 0)
    err(1, "KVM_SET_SREGS");

  return 0;
}

int cpu_init_kernel(struct vm *vm) {
  int ret;
  struct kvm_regs regs = {
      .rip = 0x100000,
      .rsp = 0x400000,
      .rsi = 0x6000,
      .rbp = 0,
      .rdi = 0,
      .rbx = 0,
  };
  ret = ioctl(vm->fd_vcpu, KVM_SET_REGS, &regs);
  if (ret < 0)
    err(1, "Kernel: KVM_SET_REGS");

  // single-step
  struct kvm_guest_debug dbg;
  memset(&dbg, 0, sizeof(struct kvm_guest_debug));
  dbg.control |= KVM_GUESTDBG_ENABLE | KVM_GUESTDBG_SINGLESTEP;
  dbg.control |= KVM_GUESTDBG_ENABLE | KVM_GUESTDBG_USE_SW_BP;

  ret = ioctl(vm->fd_vcpu, KVM_SET_GUEST_DEBUG, &dbg);
  if (ret == -1)
    err(1, "KVM_CAP_SET_GUEST_DEBUG");
  return 0;
}

int cpu_main(struct vm *vm, char *begin_addr) {
  struct kvm_regs regs;
  struct kvm_sregs sregs;
  uint16_t port;
  (void)begin_addr;

  ioctl(vm->fd_vcpu, KVM_GET_REGS, &regs);

  while (1) {
    int ret = ioctl(vm->fd_vcpu, KVM_RUN, NULL);
    ioctl(vm->fd_vcpu, KVM_GET_REGS, &regs);
    ioctl(vm->fd_vcpu, KVM_GET_SREGS, &sregs);
    if (ret < 0)
      err(1, "KVM_RUN");
    switch (vm->run->exit_reason) {
    case KVM_EXIT_HLT:
      puts("KVM_EXIT_HLT");
      return 1;
    case KVM_EXIT_IO:
      port = vm->run->io.port;
      if (port == COM1_DATA_PORT || port == COM1_IRQ_ENABLE_PORT ||
          port == COM1_IIR_PORT || port == COM1_LINE_CTRL_PORT ||
          port == COM1_LINE_STATUS_PORT || port == COM1_MODEM_CTRL_PORT ||
          port == COM1_MODEM_STATUS_PORT)
        devices_handle_serial_io(vm->run);
      else if (port == 0x61 || port == 0x43 || port == 0x42 || port == 0xcf8 ||
               port == 0xcfc || port == 0xcfe || port == 0xa1 || port == 0x21 ||
               port == 0x70 || port == 0x71 || port == 0x80 || port == 0x40)
        (void)port;
      else {
        // TODO
        // errx(1, "unexpected KVM_EXIT_IO");
      }
      break;
    case KVM_EXIT_MMIO:

      printf("MMIO: = 0x%llx\n", vm->run->mmio.phys_addr);
      break;
    case KVM_EXIT_FAIL_ENTRY:
      errx(1, "KVM_EXIT_FAIL_ENTRY: hardware_entry_failure_reason = 0x%llx",
           (unsigned long long)
               vm->run->fail_entry.hardware_entry_failure_reason);
    case KVM_EXIT_INTERNAL_ERROR:
      errx(1, "KVM_EXIT_INTERNAL_ERROR: 0x%x", vm->run->internal.suberror);
    case KVM_EXIT_DEBUG:
      break;
    default:
      puts("OTHER\n");
      errx(1, "exit_reason = 0x%x", vm->run->exit_reason);
    }
  }
  return 0;
}
