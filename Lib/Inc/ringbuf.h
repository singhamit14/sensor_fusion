#ifndef RINGBUF_H
#define RINGBUF_H

#include <stdint.h>
#include <stdbool.h>

/* Single-producer / single-consumer byte ring.
 * head: written only by producer. tail: written only by consumer.
 * head/tail free-run (wrap at 2^32); index = value & mask.
 * size must be a power of 2. */
typedef struct
{
  uint8_t          *buf;
  uint32_t          size;
  uint32_t          mask;
  volatile uint32_t head;
  volatile uint32_t tail;
  volatile uint32_t dropped;   /* bytes rejected because ring was full */
} ringbuf_t;

/* Compile-time init, so ring is valid before any init code runs */
#define RINGBUF_STATIC_INIT(storage, sz) \
  { (storage), (sz), (sz) - 1u, 0u, 0u, 0u }

bool     ringbuf_init(ringbuf_t *rb, uint8_t *buf, uint32_t size);
uint32_t ringbuf_used(const ringbuf_t *rb);
uint32_t ringbuf_free(const ringbuf_t *rb);

/* Producer: all-or-nothing. Returns len, or 0 (and adds len to dropped). */
uint32_t ringbuf_write(ringbuf_t *rb, const uint8_t *data, uint32_t len);

/* Consumer: pointer + length of contiguous readable bytes at tail (stops at buffer end). */
uint32_t ringbuf_peek(const ringbuf_t *rb, const uint8_t **ptr);

/* Consumer: free n bytes (clamped to used). */
void     ringbuf_consume(ringbuf_t *rb, uint32_t n);

#endif /* RINGBUF_H */