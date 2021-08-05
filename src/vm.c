#include <err.h>
#include <fcntl.h>
#include <linux/kvm.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "vm.h"

#define DEVICE_KVM "/dev/kvm"

int vm_create(struct vm *vm) {
  vm->fd_kvm = open(DEVICE_KVM, O_RDWR | O_CLOEXEC);
  if (vm->fd_kvm < 0)
    err(1, DEVICE_KVM);

  int ret = ioctl(vm->fd_kvm, KVM_GET_VCPU_MMAP_SIZE, NULL);
  if (ret < 0)
    err(1, "KVM_GET_VCPU_MMAP_SIZE");
  vm->mmap_size = ret;

  vm->fd_vm = ioctl(vm->fd_kvm, KVM_CREATE_VM, (unsigned long)0);
  if (vm->fd_vm < 0)
    err(1, "KVM_CREATE_VM");

  return 0;
}

int vm_init(struct vm *vm) {
  int ret;

  ret = ioctl(vm->fd_vm, KVM_SET_TSS_ADDR, 0xfffbd000);
  if (ret < 0)
    err(1, "KVM_SET_TESS_ADDR");

  struct kvm_pit_config pit_conf;
  pit_conf.flags = 0;

  ret = ioctl(vm->fd_vm, KVM_CREATE_PIT2, &pit_conf);
  if (ret < 0)
    err(1, "KVM_CREATE_PIT2");

  ret = ioctl(vm->fd_vm, KVM_CREATE_IRQCHIP, 0);
  if (ret == -1)
    err(1, "KVM_CREATE_IRQCHIP");
  return 0;
}
