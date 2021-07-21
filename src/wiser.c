#include <argp.h>
#include <asm/bootparam.h>
#include <asm/processor-flags.h>
#include <err.h>
#include <fcntl.h>
#include <linux/kvm.h>
#include <stdbool.h>
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
#include "kernel.h"
#include "vcpu.h"
#include "vm.h"
#include "wiser.h"

#define KERNEL_OPTS                                                            \
  "init=/bin/sh console=ttyS0  apm=off vsdo=0 reboot=1 "                       \
  "clocksource=kvm-clock noapic initcall_debug=1"

/* A description of the arguments we accept. */
static char args_doc[] = "--image path-to-kernel-image";

/* The options we understand. */
static struct argp_option options[] = {
    {"verbose", 'v', 0, 0, "Produce verbose output"},
    {"memory", 'r', 0, 0, "Ram size for your vm"},
    {"vcpu", 'c', 0, 0, "Number of cpu for your vm"},
    {"image", 'i', "IMAGE", 0, "linux kernel bzimage"},
    {0}};

/* Used by main to communicate with parse_opt. */
struct arguments {
  int verbose, memory, vcpu;
  char *image_file;
};

/* Parse a single option. */
static error_t parse_opt(int key, char *arg, struct argp_state *state) {
  /* Get the input argument from argp_parse, which we
     know is a pointer to our arguments structure. */
  struct arguments *arguments = state->input;

  switch (key) {
  case 'v':
    arguments->verbose = 1;
    break;
  case 'i':
    arguments->image_file = arg;
    break;
  case 'c':
    arguments->vcpu = atoi(arg); /* TODO */
  case 'r':
    arguments->memory = atoi(arg); /* TODO */

  default:
    return ARGP_ERR_UNKNOWN;
  }
  return 0;
}

/* Our argp parser. */
static struct argp argp = {options, parse_opt, args_doc, doc};

int main(int argc, char **argv) {

  struct arguments arguments;

  /* Parse our arguments; every option seen by parse_opt will
     be reflected in arguments. */
  argp_parse(&argp, argc, argv, 0, 0, &arguments);

  int ret;
  struct vm core_vm;

  ret = vm_create(&core_vm);
  ERROR(ret)
  ret = vm_init(&core_vm);
  ERROR(ret)

  ret = cpu_create(&core_vm);
  ERROR(ret)
  ret = cpu_setup_cpuid(&core_vm);
  ERROR(ret)
  ret = cpu_mmap(&core_vm);
  ERROR(ret)

  void *ram_address = kernel_load(&core_vm, arguments.image_file);
  ret = cpu_setup_protected_mode(&core_vm);
  ERROR(ret)
  ret = cpu_init_kernel(&core_vm);
  ERROR(ret)
  ret = cpu_main(&core_vm, ram_address);
  ERROR(ret)

  return 0;
}
