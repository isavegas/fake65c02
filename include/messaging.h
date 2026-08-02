#ifndef MESSAGING_H
#define MESSAGING_H

// Ports
#define CMD_ADDRESS 0x8000
#define CMD_DATA_ADDRESS 0x8001
#define SERIAL_ADDRESS 0x8002

// Messages
#define CMD_HALT 0x01
#define CMD_HOOK 0xff
#define CMD_HOOK_FUNC 0xfe
#define CMD_HOOK_CALL 0xfd
#define CMD_IRQ_REQ 0xfc
#define CMD_CHAR_REQ 0xfb
#define CMD_BANK_SWITCH 0xfa

#endif