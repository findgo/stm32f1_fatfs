

#include "debug.h"

#ifdef DEBUG_MSG
//#include "console.h"

bool dbg_verbos = true;
bool dbg_warn = true;
bool dbg_err = true;

void dbg_set_dbg_level(uint8_t level)
{
    dbg_verbos = level & DEBUG_LEVEL_VERBOS;
    dbg_warn = level & DEBUG_LEVEL_WARN;
    dbg_err = level & DEBUG_LEVEL_ERROR;
}
void dbg_print_dbg_level(void)
{
    printf("dbg_verbos: %s\r\n",dbg_verbos ? "enable":"disable");
    printf("dbg_warn: %s\r\n",dbg_warn ? "enable":"disable");
    printf("dbg_err: %s\r\n",dbg_err ? "enable":"disable");
}

/* 重定向fputc 到输出，单片机一般为串口*/ 
int fputc(int ch, FILE *f)
{
    /* e.g. write a character to the USART */
    REDIR_FPUT(ch);
    
    //(void)Serial_WriteByte(ch);
    //USART_SendData(USART_USING1,ch);
    /* Loop until the end of transmission */
    //while(USART_GetFlagStatus(USART_USING1, USART_FLAG_TC) == RESET);
    
    return ch;
}
/*
void cmd_debug_control(uint8_t argc, char *argv[], void *pcmd)
{
    if(argc != 2){
        console_writestring("error parameter\r\n");
        return;
    }
    // todo check a string is all digit
    dbg_set_dbg_level((uint8_t)atoi(argv[1]));  
    dbg_print_dbg_level();  
}
*/
#else
void dbg_set_dbg_level(uint8_t level)
{
    (void) level;
}

void dbg_print_dbg_level(void)
{
}
#endif

void __attribute__((weak)) funcheckinit(void)
{
    dbg_set_dbg_level(7);
    dbg_print_dbg_level();
    return;
}

void __attribute__((weak)) funchecktask(void)
{
    return;
}

