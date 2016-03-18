#ifndef __KEY_H__
#define __KEY_H__


typedef void (* key_function)(void);
typedef char (* supla_intr_handler)(uint32 gpio_status);

struct single_key_param {
    uint8 key_level;
    uint8 gpio_id;
    os_timer_t key_5s;
    os_timer_t key_50ms;
    key_function short_press;
    key_function long_press;
};

struct keys_param {
    uint8 key_num;
    struct single_key_param **single_key;
    supla_intr_handler handler;
};

struct single_key_param *key_init_single(uint8 gpio_id, key_function long_press, key_function short_press);
void key_init(struct keys_param *key);

#endif
