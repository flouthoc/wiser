#ifndef DEVICES_H
#define DEVICES_H

// reference for device macros
// https://gitlab.prognosticlab.org/debashis/palacios/-/blob/ac65c333e0dae71404895e27aa706feb8e8e743d/palacios/src/devices/serial.c

#define COM1_DATA_PORT 0x3F8
#define COM1_IRQ_ENABLE_PORT 0x3F9
#define COM1_DIV_LATCH_LSB_PORT 0x3F8
#define COM1_DIV_LATCH_MSB_PORT 0x3F9
#define COM1_IIR_PORT 0x3FA
#define COM1_FIFO_CTRL_PORT 0x3FA
#define COM1_LINE_CTRL_PORT 0x3FB
#define COM1_MODEM_CTRL_PORT 0x3FC
#define COM1_LINE_STATUS_PORT 0x3FD
#define COM1_MODEM_STATUS_PORT 0x3FE
#define COM1_SCRATCH_PORT 0x3FF

#define COM2_DATA_PORT 0x2f8
#define COM2_IRQ_ENABLE_PORT 0x2f9
#define COM2_DIV_LATCH_LSB_PORT 0x2f8
#define COM2_DIV_LATCH_MSB_PORT 0x2f9
#define COM2_IIR_PORT 0x2fa
#define COM2_FIFO_CTRL_PORT 0x2fa
#define COM2_LINE_CTRL_PORT 0x2fb
#define COM2_MODEM_CTRL_PORT 0x2fc
#define COM2_LINE_STATUS_PORT 0x2fd
#define COM2_MODEM_STATUS_PORT 0x2fe
#define COM2_SCRATCH_PORT 0x2ff

#define COM3_DATA_PORT 0x3e8
#define COM3_IRQ_ENABLE_PORT 0x3e9
#define COM3_DIV_LATCH_LSB_PORT 0x3e8
#define COM3_DIV_LATCH_MSB_PORT 0x3e9
#define COM3_IIR_PORT 0x3ea
#define COM3_FIFO_CTRL_PORT 0x3ea
#define COM3_LINE_CTRL_PORT 0x3eb
#define COM3_MODEM_CTRL_PORT 0x3ec
#define COM3_LINE_STATUS_PORT 0x3ed
#define COM3_MODEM_STATUS_PORT 0x3ee
#define COM3_SCRATCH_PORT 0x3ef

#define COM4_DATA_PORT 0x2e8
#define COM4_IRQ_ENABLE_PORT 0x2e9
#define COM4_DIV_LATCH_LSB_PORT 0x2e8
#define COM4_DIV_LATCH_MSB_PORT 0x2e9
#define COM4_IIR_PORT 0x2ea
#define COM4_FIFO_CTRL_PORT 0x2ea
#define COM4_LINE_CTRL_PORT 0x2eb
#define COM4_MODEM_CTRL_PORT 0x2ec
#define COM4_LINE_STATUS_PORT 0x2ed
#define COM4_MODEM_STATUS_PORT 0x2ee
#define COM4_SCRATCH_PORT 0x2ef

// Interrupt IDs (in priority order, highest is first)
#define STATUS_IRQ_LSR_OE_SET 0x3
#define STATUS_IRQ_LSR_PE_SET 0x3
#define STATUS_IRQ_LSR_FE_SET 0x3
#define STATUS_IRQ_LSR_BI_SET 0x3
#define RX_IRQ_DR 0x2
#define RX_IRQ_TRIGGER_LEVEL 0x2
#define FIFO_IRQ 0x6
#define TX_IRQ_THRE 0x1
#define MODEL_IRQ_DELTA_SET 0x0

// COMs IRQ ID
#define COM1_IRQ 0x4
#define COM2_IRQ 0x3
#define COM3_IRQ 0x4
#define COM4_IRQ 0x3

#define RX_BUFFER 0x1
#define TX_BUFFER 0x2

// initial value for registers

#define IER_INIT_VAL 0x3
// receive data available interrupt and THRE interrupt are enabled
#define IIR_INIT_VAL 0x1
// No Pending Interrupt bit is set.
#define FCR_INIT_VAL 0xc0
// fifo control register is set to 0
#define LCR_INIT_VAL 0x3
#define MCR_INIT_VAL 0x0
#define LSR_INIT_VAL 0x60
#define MSR_INIT_VAL 0x0
#define DLL_INIT_VAL 0x1
#define DLM_INIT_VAL 0x0

#define DLAB 0x80
#define BI (1 << 4)

#define UART_IIR_TYPE_BITS 0xc0
#define UART_IIR_NO_INT 0x01

void devices_handle_serial_io(struct kvm_run *run);
void devices_log_io(struct kvm_run *run);

#endif
