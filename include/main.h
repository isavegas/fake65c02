#include <stddef.h>
#include <stdint.h>
// NOLINTNEXTLINE(llvmlibc-restrict-system-libc-headers)
#include <stdio.h>

#ifdef FAKE6502
#include "fake6502.h"
#endif
#ifdef FAKE65c02
#include "fake65c02.h"
#endif

#if !defined(FAKE6502) && !defined(FAKE65c02)
#error CPU type not provided
#endif


