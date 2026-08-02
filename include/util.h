#ifndef UTIL_H
#define UTIL_H

// Courtesy of Chris. Thanks!
#define MIN(a, b) ((a) < (b)) ? (a) : (b)

#define SET_BIT(reg, flag) ((reg) |= (flag))
#define CLEAR_BIT(reg, flag) ((reg) &= ~(flag))
#define FLIP_BIT(reg, flag) ((reg) ^= (flag))
#define CHECK_BIT(reg, flag) (((reg) & (flag)) != 0)

#endif