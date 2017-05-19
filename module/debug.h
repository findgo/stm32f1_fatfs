
/*
 * debug 调试模块，使用宏需要开启宏DEBUG_MSG
 * 配置重定义fput,直接可在头文件 配置REDIR_FPUT(ch)的宏
 * 默认只开启 err, 而warn，verbos不开启。
 * 
 */


#ifndef __DEBUG_H__
#define __DEBUG_H__

#include "app_cfg.h"
#include "mserial.h"

#define DEBUG_LEVEL_VERBOS  0x01
#define DEBUG_LEVEL_WARN    0x02
#define DEBUG_LEVEL_ERROR   0x04

#ifdef DEBUG_MSG

extern bool dbg_verbos;
extern bool dbg_warn;
extern bool dbg_err;

#define DBG_VERBOS(fmt, args...)                    \
do {                                                \
    if (dbg_verbos) {                           \
        printf(fmt, ##args);                        \
    }                                               \
} while (0)

#define DBG_WARN(fmt, args...)                      \
do {                                                \
    if (dbg_warn) {                             \
        printf(fmt, ##args);                        \
    }                                               \
} while (0)

#define DBG_ERR(fmt, args...)                     \
    do {                                                \
        if (dbg_err) {                              \
            printf(fmt, ##args);                        \
        }                                               \
    } while (0)

//重定向fput  用户定义
#define REDIR_FPUT(ch)   (void)Serial_WriteByte(COM0,ch);
#else

#define DBG_VERBOS(fmt, args...)
#define DBG_WARN(fmt, args...)
#define DBG_ERR(fmt, args...)

#endif

void dbg_set_dbg_level(uint8_t level);
void dbg_print_dbg_level(void);
void  funcheckinit(void);
void  funchecktask(void);


#endif

