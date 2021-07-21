#include <err.h>
#include <linux/kvm.h>
#include <stdint.h>
#include <stdio.h>

#include "devices.h"

static uint8_t line_config = DLAB;
static uint8_t interrupt_enable = 0;
static uint8_t interrupt_identification = UART_IIR_NO_INT;
static uint8_t modem_control = 0;
static uint8_t modem_status = 0;
static uint8_t line_status = 1 << 5 | 1 << 6;
static uint8_t dll = 0;
static uint8_t dlm = 0;
static uint8_t scr = 0;

void devices_log_io(struct kvm_run *run) {
  char *direction;
  if (run->io.direction == KVM_EXIT_IO_IN)
    direction = "IN";
  else
    direction = "OUT";
  printf("io: dir: %s port: 0x%x size: %u off: %llu count: %u\n", direction,
         run->io.port, run->io.size, run->io.data_offset, run->io.count);
}

void device_handle_out(struct kvm_run *run_handle) {
  if (run_handle->io.port == COM1_DATA_PORT) {
    if (line_config & DLAB) {
      uint8_t enable_bit =
          *((uint8_t *)run_handle + run_handle->io.data_offset);
      dll = enable_bit;
    } else
      putchar(*(((char *)run_handle) + run_handle->io.data_offset));
  }
  // Interrupt Enable Register
  else if (run_handle->io.port == COM1_IRQ_ENABLE_PORT) {

    if (line_config & DLAB) {
      uint8_t enable_bit =
          *((uint8_t *)run_handle + run_handle->io.data_offset);
      dlm = enable_bit & 0x0f;

    } else {
      uint8_t enable_bit =
          *((uint8_t *)run_handle + run_handle->io.data_offset);
      interrupt_enable = enable_bit;
    }
  } else if (run_handle->io.port == COM1_IIR_PORT) {
    uint8_t enable_bit = *((uint8_t *)run_handle + run_handle->io.data_offset);
    interrupt_identification = enable_bit;
  }
  // Line Configure Register
  else if (run_handle->io.port == COM1_LINE_CTRL_PORT) {
    uint8_t enable_bit = *((uint8_t *)run_handle + run_handle->io.data_offset);
    line_config = enable_bit;
  } else if (run_handle->io.port == COM1_MODEM_CTRL_PORT) {
    uint8_t enable_bit = *((uint8_t *)run_handle + run_handle->io.data_offset);
    modem_control = enable_bit;
  } else if (run_handle->io.port == COM1_LINE_STATUS_PORT) {
    uint8_t enable_bit = *((uint8_t *)run_handle + run_handle->io.data_offset);
    line_status = enable_bit;
  } else if (run_handle->io.port == COM1_MODEM_STATUS_PORT) {
    uint8_t enable_bit = *((uint8_t *)run_handle + run_handle->io.data_offset);
    modem_status = enable_bit;
  } else if (run_handle->io.port == COM1_SCRATCH_PORT) {
    uint8_t enable_bit = *((uint8_t *)run_handle + run_handle->io.data_offset);
    scr = enable_bit;
  } else {
    devices_log_io(run_handle);
  }
}

void device_handle_in(struct kvm_run *run_handle) {
  if (run_handle->io.port == COM1_DATA_PORT) {
    if (line_config & 0x80) {
      uint8_t *reg = (uint8_t *)run_handle + run_handle->io.data_offset;
      *reg = dll;
      return;
    }
    if (line_status & BI) {
      line_status &= ~BI;
      uint8_t *reg = (uint8_t *)run_handle + run_handle->io.data_offset;
      *reg = 0;
      return;
    }
  } else if (run_handle->io.port == COM1_IRQ_ENABLE_PORT) {
    if (line_config & 0x80) {
      uint8_t *reg = (uint8_t *)run_handle + run_handle->io.data_offset;
      *reg = dlm;
    } else {
      uint8_t *reg = (uint8_t *)run_handle + run_handle->io.data_offset;
      *reg = interrupt_enable;
    }
  } else if (run_handle->io.port == COM1_IIR_PORT) {
    uint8_t *reg = (uint8_t *)run_handle + run_handle->io.data_offset;
    *reg = 0; // interrupt_identification | UART_IIR_TYPE_BITS;
  } else if (run_handle->io.port == COM1_LINE_CTRL_PORT) {
    uint8_t *reg = (uint8_t *)run_handle + run_handle->io.data_offset;
    *reg = line_config;
  } else if (run_handle->io.port == COM1_MODEM_CTRL_PORT) {
    uint8_t *reg = (uint8_t *)run_handle + run_handle->io.data_offset;
    *reg = modem_control;
  } else if (run_handle->io.port == COM1_LINE_STATUS_PORT) {
    uint8_t *reg = (uint8_t *)run_handle + run_handle->io.data_offset;
    *reg = line_status;
  } else if (run_handle->io.port == COM1_MODEM_STATUS_PORT) {
    uint8_t *reg = (uint8_t *)run_handle + run_handle->io.data_offset;
    *reg = modem_status;
  } else if (run_handle->io.port == COM1_SCRATCH_PORT) {
    uint8_t *reg = (uint8_t *)run_handle + run_handle->io.data_offset;
    *reg = scr;
  }
}

void devices_handle_serial_io(struct kvm_run *run) {
  if (run->io.direction == KVM_EXIT_IO_OUT)
    device_handle_out(run);
  else {
    device_handle_in(run);
  }
}
