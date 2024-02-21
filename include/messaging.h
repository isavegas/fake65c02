// Ports
#define IO_IN 0x7fff
#define IO_CMD 0x8000
#define IO_OUT 0x8001
#define SERIAL_OUT 0x8002

// Messages
#define IO_HALT 0x01
#define IO_HOOK 0xff
#define IO_HOOK_FUNC 0xfe
#define IO_HOOK_CALL 0xfd
#define IO_IRQ_REQ 0xfc
#define IO_CHAR_REQ 0xfb
#define IO_BANK_SWITCH 0xfa
