#ifndef VM_H
#define VM_H

#include <fcntl.h>
#include <sys/ioctl.h>

struct vm {
  int fd_kvm;
  int fd_vm;
  int fd_vcpu;

  size_t mmap_size;
  size_t ram_size;
  struct kvm_run *run;
  struct kvm_sregs sregs;
  char *initramfs;
};

int vm_create(struct vm *v);
int vm_init(struct vm *v);
int vm_set_memory(struct vm *v);

#endif
