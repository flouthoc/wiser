#include <asm/bootparam.h>
#include <asm/e820.h>
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

#include "kernel.h"
#include "vcpu.h"
#include "vm.h"

static char *kernel_opts = KERNEL_OPTS;

void *kernel_setup_memory(struct vm *vm) {
  vm->ram_size = MEMORY_SIZE;
  void *mem = mmap(NULL, vm->ram_size, PROT_READ | PROT_WRITE | PROT_EXEC,
                   MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
  if (mem == MAP_FAILED) {
    errx(1, "mmap failed");
  }
  struct kvm_userspace_memory_region region = {
      .slot = 0,
      .guest_phys_addr = 0x0,
      .memory_size = MEMORY_SIZE,
      .userspace_addr = (uint64_t)mem,
  };
  int ret = ioctl(vm->fd_vm, KVM_SET_USER_MEMORY_REGION, &region);
  if (ret == -1)
    err(1, "KVM_SET_USER_MEMORY_REGION");
  return mem;
}
void *kernel_load(struct vm *vm, char *path) {

  size_t kernel_size;

  int fd_image = open(path, O_RDWR);
  if (fd_image < -1)
    err(1, "Unable to open image");

  struct stat statbuf;
  fstat(fd_image, &statbuf);
  size_t image_size = statbuf.st_size;

  uint8_t *mem =
      mmap(NULL, image_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd_image, 0);
  if (mem == MAP_FAILED) {
    errx(1, "Failed while loading image into memory");
  }

  struct setup_header *setup_header =
      (struct setup_header *)((char *)mem + 0x01f1);

  uint64_t offset_kernel = kernel_get_offset(setup_header);

  char *ram_addr = kernel_setup_memory(vm);
  void *end_header = mem + 0x0202 + *((char *)mem + 0x0201);
  kernel_init_boot(setup_header, end_header, ram_addr);
  // write kernel
  kernel_size = image_size - offset_kernel;

  memcpy(ram_addr + 0x100000, mem + offset_kernel, kernel_size);

  // load initramfs
  if (vm->initramfs != NULL) {
    int fd_initram = open(vm->initramfs, O_RDWR);
    if (fd_initram < -1)
      err(1, "open initramfs image failed");

    struct stat statbuf;
    fstat(fd_initram, &statbuf);
    size_t initram_size = statbuf.st_size;

    uint8_t *init_memory = mmap(NULL, initram_size, PROT_READ | PROT_WRITE,
                                MAP_SHARED, fd_initram, 0);
    if (init_memory == MAP_FAILED)
      errx(1, "initramfs: failed loading image");

    memcpy(ram_addr + kernel_size + 0x100000, init_memory, statbuf.st_size);
    struct boot_params *initramfs_boot_param =
        (struct boot_params *)(ram_addr + 0x6000 + 0x10000);
    struct setup_header *setup_header = &(initramfs_boot_param->hdr);

    setup_header->ramdisk_image = (uint64_t)(kernel_size + 0x100000);
    setup_header->ramdisk_size = statbuf.st_size;
  }

  return ram_addr;
}

uint64_t kernel_get_offset(struct setup_header *setup_header) {
  uint8_t setup_sects = setup_header->setup_sects;
  if (setup_sects == 0)
    setup_sects = 4;
  return (setup_sects + 1) * 512;
}

int kernel_init_boot(struct setup_header *setup_header, void *end_setup_header,
                     char *ram_addr) {
  struct boot_params *params = (struct boot_params *)(ram_addr + 0x6000);
  memset(params, 0, sizeof(struct boot_params));

  memcpy(&(params->hdr), setup_header,
         (uintptr_t)end_setup_header - (uintptr_t)setup_header);

  if (params->hdr.setup_sects == 0)
    params->hdr.setup_sects = 4;

  params->hdr.type_of_loader = 0xff;
  params->hdr.loadflags = 0;
  params->hdr.loadflags |= KEEP_SEGMENTS;

  params->hdr.loadflags &= ~QUIET_FLAG;
  params->hdr.loadflags &= ~CAN_USE_HEAP;

  uint8_t idx = 0;
  struct boot_e820_entry *pre_isa = &params->e820_table[idx++];
  struct boot_e820_entry *post_isa = &params->e820_table[idx++];

  kernel_set_e820_entry(pre_isa, 0x0, ISA_START_ADDRESS - 1, E820_RAM);
  kernel_set_e820_entry(post_isa, ISA_END_ADDRESS,
                        (200 << 20) - ISA_END_ADDRESS, E820_RAM);

  params->e820_entries = idx;
  params->hdr.cmd_line_ptr = 0x6000 + 0x10000;
  memcpy(ram_addr + 0x6000 + 0x10000, kernel_opts, strlen(kernel_opts));
  return 0;
}

void kernel_set_e820_entry(struct boot_e820_entry *entry, uint64_t addr,
                           uint64_t size, uint32_t type) {
  entry->addr = addr;
  entry->size = size;
  entry->type = type;
}
