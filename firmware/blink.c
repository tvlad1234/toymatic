#include <stdint.h>

#include "pins.h"

#include "blink.h"

void blink_start(struct blink_ctx *ctx, int blinks, uint64_t now_us)
{
    ctx->count = 0;
    ctx->target = blinks;
    ctx->phase = 0;
    ctx->state = 1;
    ctx->next_time = now_us;
}

void blink_update(struct blink_ctx *ctx, uint64_t now_us)
{
    if (ctx->target == 0)
    {
        gpio_clear(ctx->gpio, ctx->pin); // LED off
        return;
    }

    if (ctx->state == 0)
        return; // idle

    if (now_us < ctx->next_time)
        return; // not time yet

    if (ctx->state == 1)
    {
        // Blinking phase
        if (ctx->phase == 0)
        {
            gpio_toggle(ctx->gpio, ctx->pin); // LED on
            ctx->phase = 1;
            ctx->next_time = now_us + 100000; // 100 ms
        }
        else
        {
            gpio_clear(ctx->gpio, ctx->pin); // LED off
            ctx->phase = 0;
            ctx->count++;
            if (ctx->count >= ctx->target)
            {
                // blinked enough, now pause
                ctx->state = 2;
                ctx->next_time = now_us + 2500000; // 2.5 s pause
            }
            else                                  // keep blinkimg
                ctx->next_time = now_us + 100000; // 100 ms
        }
    }
    else if (ctx->state == 2)
    {
        // Gap before repeating
        ctx->count = 0;
        ctx->state = 1;
    }
}