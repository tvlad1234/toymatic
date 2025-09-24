#ifndef _BLINK_H
#define _BLINK_H

#include "stdint.h"

struct blink_ctx
{
    uint32_t gpio;
    uint32_t pin;

    int count;          // how many blinks so far
    int target;         // how many total blinks for this error
    uint64_t next_time; // when the next toggle/step should happen
    int phase;          // 0=off, 1=on
    int state;          // 0=idle, 1=blinking, 2=gap
};

void blink_start(struct blink_ctx *ctx, int blinks, uint64_t now_us);
void blink_update(struct blink_ctx *ctx, uint64_t now_us);

#endif