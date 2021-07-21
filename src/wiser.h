#ifndef WISER_H
#define WISER_H

#define ERROR(x)                                                               \
  if ((x) < 0)                                                                 \
    return 1;

const char *argp_program_version = "wiser 0.0.1";
const char *argp_program_bug_address =
    "https://github.com/flouthoc/wiser/issues";

/* Program documentation. */
static char doc[] = "wiser - Extremely tiny type-2 hypervisor for linux. Will "
                    "boot your unikernel/linux someday.";

#endif
