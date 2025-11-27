#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>

#define QUEUE_TOTAL_SIZE 0x1000                              // 4096 bytes
#define QUEUE_SLOT_SIZE 64                                   // 1 entry = 64 bytes
#define QUEUE_NUM_SLOTS (QUEUE_TOTAL_SIZE / QUEUE_SLOT_SIZE) // 64

uint8_t BigBuffer[0x2000] = {0};    // でかバッファ（この一部がリングバッファとして使われる）

typedef struct
{
    uint8_t *base; // 64スロット
    unsigned int head;                   // 次に読み出す index
    unsigned int tail;                   // 次に書き込む index
    unsigned int count;                  // 入っているエントリ数
} ring_queue_t;

void ring_queue_init(ring_queue_t *q, uint8_t *buf_offset)
{
    q->base = buf_offset;
    q->head = 0;
    q->tail = 0;
    q->count = 0;
}

static inline uint8_t *get_slot_ptr(ring_queue_t *q, unsigned int idx)
{
    return q->base + (idx * QUEUE_SLOT_SIZE);
}

// Enqueueするデータをスロットにコピーする
// 成功: 0, 失敗: -1 (full)
int ring_queue_enqueue(ring_queue_t *q, const void *data)
{
    if (q->count >= QUEUE_NUM_SLOTS) {
        return -1;
    }

    uint8_t *dst = get_slot_ptr(q, q->tail);
    memcpy(dst, data, QUEUE_SLOT_SIZE);

    q->tail = (q->tail + 1) % QUEUE_NUM_SLOTS;
    q->count++;

    return 0;
}

// Dequeueするデータの読み出し先をout_ptrに設定する
// 成功: 0, 失敗: -1 (empty)
int ring_queue_dequeue(ring_queue_t *q, const uint8_t **out_ptr)
{
    if (q->count == 0) {
        return -1;
    }

    if (out_ptr) {
        *out_ptr = get_slot_ptr(q, q->head);
    }

    q->head = (q->head + 1) % QUEUE_NUM_SLOTS;
    q->count--;

    return 0;
}



// テスト（chat gpt謹製）
int main(void)
{
    ring_queue_t q;
    ring_queue_init(&q, &BigBuffer[0]);

    uint8_t tx[QUEUE_SLOT_SIZE];
    const uint8_t *rx_ptr;

    // 1. 単発で enqueue / dequeue テスト
    for (int i = 0; i < QUEUE_SLOT_SIZE; i++) {
        tx[i] = (uint8_t)i;
    }

    if (ring_queue_enqueue(&q, tx) != 0) {
        printf("TEST1: enqueue failed\n");
        return 1;
    }

    if (ring_queue_dequeue(&q, &rx_ptr) != 0) {
        printf("TEST1: dequeue failed\n");
        return 1;
    }

    // 比較
    if (memcmp(tx, rx_ptr, QUEUE_SLOT_SIZE) != 0) {
        printf("TEST1: data mismatch\n");
        return 1;
    }
    printf("TEST1: OK (single enqueue/dequeue)\n");

    // 2. 連続エントリでのテスト（ラップアラウンド含む）
    //    キューは64エントリなので、64個全部埋めてから何個か出すテスト
    ring_queue_init(&q, &BigBuffer[0]);

    // 64エントリ分enqueue
    for (int n = 0; n < QUEUE_NUM_SLOTS; n++) {
        for (int i = 0; i < QUEUE_SLOT_SIZE; i++) {
            tx[i] = (uint8_t)(i + n);  // n に応じて違うパターン
        }
        if (ring_queue_enqueue(&q, tx) != 0) {
            printf("TEST2: enqueue failed at n=%d\n", n);
            return 1;
        }
    }

    // もう1個入れようとして失敗するか（満杯チェック）
    if (ring_queue_enqueue(&q, tx) == 0) {
        printf("TEST2: enqueue should have failed but succeeded (queue not full?)\n");
        return 1;
    }
    printf("TEST2: queue full check OK\n");

    // 64エントリ分 dequeue して、パターン一致確認
    for (int n = 0; n < QUEUE_NUM_SLOTS; n++) {
        if (ring_queue_dequeue(&q, &rx_ptr) != 0) {
            printf("TEST2: dequeue failed at n=%d\n", n);
            return 1;
        }

        // 期待値を再生成して比較
        for (int i = 0; i < QUEUE_SLOT_SIZE; i++) {
            uint8_t expected = (uint8_t)(i + n);
            if (rx_ptr[i] != expected) {
                printf("TEST2: data mismatch at n=%d, i=%d (got=%u, expected=%u)\n",
                       n, i, rx_ptr[i], expected);
                return 1;
            }
        }
    }

    // 空になったので、もう1回dequeueして失敗するか
    if (ring_queue_dequeue(&q, &rx_ptr) == 0) {
        printf("TEST2: dequeue should have failed but succeeded (queue not empty?)\n");
        return 1;
    }
    printf("TEST2: OK (full cycle with wrap-around)\n");

    printf("ALL TESTS PASSED\n");
    return 0;
}