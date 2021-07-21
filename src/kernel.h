#ifndef KERNEL_H
#define KERNEL_H

#include "vcpu.h"
#include "vm.h"

#define MEMORY_SIZE 1 << 30
#define KERNEL_OPTS                                                            \
  "init=/bin/init console=ttyS0  apm=off vsdo=0 reboot=1 "                     \
  "clocksource=kvm-clock noapic initcall_debug=1"

void *kernel_load(struct vm *v, char *path);
uint64_t kernel_get_offset(struct setup_header *setup_header);
void *kernel_setup_memory(struct vm *v);
int kernel_init_boot(struct setup_header *setup_header, void *end_setup_header,
                     char *ram_addr);
void kernel_set_e820_entry(struct boot_e820_entry *entry, uint64_t addr,
                           uint64_t size, uint32_t type);

#endif
