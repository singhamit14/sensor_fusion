#include "test_runner.h"
#include "ringbuf.h"

static uint8_t store[16];

static void test_init_rejects(void)
{
  ringbuf_t rb;
  CHECK(!ringbuf_init(&rb, store, 0));
  CHECK(!ringbuf_init(&rb, store, 3));
  CHECK(!ringbuf_init(&rb, store, 12));
  CHECK(!ringbuf_init(&rb, NULL, 16));
  CHECK(!ringbuf_init(NULL, store, 16));
  CHECK(ringbuf_init(&rb, store, 16));
  CHECK(rb.mask == 15);
}

static void test_static_init(void)
{
  ringbuf_t rb = RINGBUF_STATIC_INIT(store, sizeof store);
  CHECK(rb.size == 16 && rb.mask == 15);
  CHECK(ringbuf_used(&rb) == 0);
  CHECK(ringbuf_free(&rb) == 16);
}

static void test_empty(void)
{
  ringbuf_t rb;
  const uint8_t *p = NULL;
  ringbuf_init(&rb, store, 16);
  CHECK(ringbuf_used(&rb) == 0);
  CHECK(ringbuf_free(&rb) == 16);
  CHECK(ringbuf_peek(&rb, &p) == 0);
  CHECK(ringbuf_write(&rb, (const uint8_t *)"x", 0) == 0);
  CHECK(rb.dropped == 0);             /* len 0 is not a drop */
}

static void test_full_and_drop(void)
{
  ringbuf_t rb;
  uint8_t d[16] = {0};
  ringbuf_init(&rb, store, 16);
  CHECK(ringbuf_write(&rb, d, 16) == 16);
  CHECK(ringbuf_used(&rb) == 16);
  CHECK(ringbuf_free(&rb) == 0);
  CHECK(ringbuf_write(&rb, d, 1) == 0);
  CHECK(rb.dropped == 1);
  CHECK(ringbuf_used(&rb) == 16);     /* full ring unchanged */
}

static void test_all_or_nothing(void)
{
  ringbuf_t rb;
  uint8_t d[16] = {0};
  ringbuf_init(&rb, store, 16);
  CHECK(ringbuf_write(&rb, d, 10) == 10);
  CHECK(ringbuf_write(&rb, d, 7) == 0);   /* free is 6 */
  CHECK(rb.dropped == 7);
  CHECK(ringbuf_used(&rb) == 10);         /* no partial write */
  CHECK(ringbuf_write(&rb, d, 6) == 6);
  CHECK(ringbuf_free(&rb) == 0);
}

/* head/tail near 2^32 AND index near buffer end: write must split,
 * peek must stop at buffer end, counters must wrap through 0. */
static void test_wrap_2pow32(void)
{
  ringbuf_t rb;
  const uint8_t *p;
  uint8_t d[10];
  for (int i = 0; i < 10; i++) d[i] = (uint8_t)i;

  ringbuf_init(&rb, store, 16);
  rb.head = rb.tail = 0xFFFFFFFAu;        /* idx = 10, 6 bytes before end */

  CHECK(ringbuf_write(&rb, d, 10) == 10);
  CHECK(rb.head == 4u);                   /* wrapped past 2^32 */
  CHECK(ringbuf_used(&rb) == 10);
  CHECK(ringbuf_free(&rb) == 6);

  CHECK(ringbuf_peek(&rb, &p) == 6);      /* stops at buffer end */
  CHECK(p == &store[10]);
  CHECK(memcmp(p, d, 6) == 0);
  ringbuf_consume(&rb, 6);

  CHECK(ringbuf_peek(&rb, &p) == 4);      /* rest from buffer start */
  CHECK(p == &store[0]);
  CHECK(memcmp(p, d + 6, 4) == 0);
  ringbuf_consume(&rb, 4);

  CHECK(ringbuf_used(&rb) == 0);
  CHECK(rb.tail == 4u);
}

static void test_consume_clamp(void)
{
  ringbuf_t rb;
  uint8_t d[3] = {1, 2, 3};
  ringbuf_init(&rb, store, 16);
  ringbuf_write(&rb, d, 3);
  ringbuf_consume(&rb, 100);
  CHECK(ringbuf_used(&rb) == 0);
  CHECK(rb.tail == rb.head);
}

int main(void)
{
  test_init_rejects();
  test_static_init();
  test_empty();
  test_full_and_drop();
  test_all_or_nothing();
  test_wrap_2pow32();
  test_consume_clamp();
  TEST_SUMMARY();
}