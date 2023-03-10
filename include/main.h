#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

// Vectors are in ROM. If ROM is writable, vectors should be
// as well. The inverse isn't necessarily true.
#ifdef WRITABLE_ROM
#define WRITABLE_VECTORS
#endif

#include "fake65c02.h"
#include "messaging.h"
