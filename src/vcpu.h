#ifndef VCPU_H
#define VCPU_H

#include "vm.h"

int cpu_create(struct vm *v);
int cpu_setup_cpuid(struct vm *v);
int cpu_setup_protected_mode(struct vm *v);
int cpu_mmap(struct vm *v);
int cpu_init_kernel(struct vm *v);
int cpu_main(struct vm *v, char *addr);

#endif
