#ifndef COMMON_H
#define COMMON_H

#define min(a, b) (((a) < (b)) ? (a) : (b))
#define max(a, b) (((a) > (b)) ? (a) : (b))

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

#define __always_inline inline __attribute__((always_inline))
#define __printf(a, b) __attribute__((format(printf, a, b)))
#define __aligned(x) __attribute__((aligned(x)))
#define __maybe_unused __attribute__((unused))
#define __packed __attribute__((packed))
#define __naked __attribute__((naked))

#endif
