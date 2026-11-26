#include "ringbuf.h"
#include <string.h>

#define RB_FENCE()  __atomic_thread_fence(__ATOMIC_SEQ_CST)   /* dmb on Cortex-M7 */

bool ringbuf_init(ringbuf_t *rb, uint8_t *buf, uint32_t size)
{
  if (rb == NULL || buf == NULL) return false;
  if (size == 0 || (size & (size - 1u)) != 0) return false;   /* power of 2 */
  rb->buf     = buf;
  rb->size    = size;
  rb->mask    = size - 1u;
  rb->head    = 0;
  rb->tail    = 0;
  rb->dropped = 0;
  return true;
}

uint32_t ringbuf_used(const ringbuf_t *rb)
{
  return rb->head - rb->tail;
}

uint32_t ringbuf_free(const ringbuf_t *rb)
{
  return rb->size - (rb->head - rb->tail);
}

uint32_t ringbuf_write(ringbuf_t *rb, const uint8_t *data, uint32_t len)
{
  if (len == 0) return 0;
  if (len > ringbuf_free(rb))
  {
    rb->dropped += len;
    return 0;
  }

  uint32_t head  = rb->head;
  uint32_t idx   = head & rb->mask;
  uint32_t first = rb->size - idx;        /* space before wrap */
  if (first > len) first = len;
  memcpy(&rb->buf[idx], data, first);
  memcpy(&rb->buf[0], data + first, len - first);

  RB_FENCE();                             /* data in ring before head moves */
  rb->head = head + len;
  return len;
}

uint32_t ringbuf_peek(const ringbuf_t *rb, const uint8_t **ptr)
{
  uint32_t used = rb->head - rb->tail;
  uint32_t idx  = rb->tail & rb->mask;
  uint32_t n    = rb->size - idx;         /* contiguous up to buffer end */
  if (n > used) n = used;
  RB_FENCE();                             /* see data written before head */
  *ptr = &rb->buf[idx];
  return n;
}

void ringbuf_consume(ringbuf_t *rb, uint32_t n)
{
  uint32_t used = rb->head - rb->tail;
  if (n > used) n = used;
  RB_FENCE();                             /* finish reading before freeing */
  rb->tail += n;
}