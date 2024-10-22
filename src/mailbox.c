#include "mailbox.h"
#include "mydef.h"
#include "interrupt.h"


/*const word_t MAILBOX_GET_FIRMWARE_VER       = 0x00000001;
const word_t MAILBOX_GET_BOARD_MODEL        = 0x00010001;
const word_t MAILBOX_GET_BOARD_REVISION     = 0x00010002;
const word_t MAILBOX_GET_MAC_ADDR           = 0x00010003;
const word_t MAILBOX_GET_BOARD_SERIAL       = 0x00010004;
const word_t MAILBOX_GET_ARM_MEM            = 0x00010005;
const word_t MAILBOX_GET_VC_MEM             = 0x00010006;
const word_t MAILBOX_GET_CLOCKS             = 0x00010007;
const word_t MAILBOX_GET_COMMAND_LINE       = 0x00050001;
const word_t MAILBOX_GET_DMA_CHANNEL        = 0x00060001;
const word_t MAILBOX_GET_POWER_STATE        = 0x00020001;
const word_t MAILBOX_GET_TIMING             = 0x00020002;
const word_t MAILBOX_SET_POWER_STATE        = 0x00028001;
const word_t MAILBOX_GET_CLOCK_STATE        = 0x00030001;
const word_t MAILBOX_SET_CLOCK_STATE        = 0x00038001;
const word_t MAILBOX_GET_CLOCK_RATE         = 0x00030002;
const word_t MAILBOX_GET_ONBOARD_LED_STATUS = 0x00030041;
const word_t MAILBOX_TEST_ONBOARD_LED_STATUS= 0x00034041;
const word_t MAILBOX_SET_ONBOARD_LED_STATUS = 0x00038041;
const word_t MAILBOX_GET_CLOCK_RATE_MEASURED= 0x00030047;
const word_t MAILBOX_SET_CLOCK_RATE         = 0x00038002;
const word_t MAILBOX_GET_MAX_CLOCK_RATE     = 0x00030004;
const word_t MAILBOX_GET_MIN_CLOCK_RATE     = 0x00030007;
const word_t MAILBOX_GET_TURBO              = 0x00030009;
const word_t MAILBOX_SET_TURBO              = 0x00038009;
const word_t MAILBOX_GET_VOLTAGE            = 0x00030003;
const word_t MAILBOX_SET_VOLTAGE            = 0x00038003;
const word_t MAILBOX_GET_MAX_VOLTAGE        = 0x00030005;
const word_t MAILBOX_GET_MIN_VOLTAGE        = 0x00030008;
const word_t MAILBOX_GET_TEMP               = 0x00030006;
const word_t MAILBOX_GET_MAX_TEMP           = 0x0003000a;
const word_t MAILBOX_ALLOCATE_MEM           = 0x0003000c;
const word_t MAILBOX_LOCK_MEMORY            = 0x0003000d;
const word_t MAILBOX_UNLOCK_MEMORY          = 0x0003000e;
const word_t MAILBOX_RELEASE_MEMORY         = 0x0003000f;
const word_t MAILBOX_EXECUTE_CODE           = 0x00030010;
const word_t MAILBOX_ALLOCATE_BUFFER        = 0x00040001;
const word_t MAILBOX_RELEASE_BUFFER         = 0x00048001;
const word_t MAILBOX_BLANK_SCREEN           = 0x00040002;
const word_t MAILBOX_GET_PHYSICAL_WH        = 0x00040003;
const word_t MAILBOX_TEST_PHYSICAL_WH       = 0x00044003;
const word_t MAILBOX_SET_PHYSICAL_WH        = 0x00048003;
const word_t MAILBOX_GET_VIRTUAL_WH         = 0x00040004;
const word_t MAILBOX_TEST_VIRTUAL_WH        = 0x00044004;
const word_t MAILBOX_SET_VIRTUAL_WH         = 0x00048004;
const word_t MAILBOX_GET_DEPTH              = 0x00040005;
const word_t MAILBOX_TEST_DEPTH             = 0x00044005;
const word_t MAILBOX_SET_DEPTH              = 0x00040006;
const word_t MAILBOX_TEST_PIXEL_ORDER       = 0x00044006;
const word_t MAILBOX_SET_PIXEL_ORDER        = 0x00048006;
const word_t MAILBOX_GET_ALPHA_MODE         = 0x00040007;
const word_t MAILBOX_TEST_ALPHA_MODE        = 0x00048007;
const word_t MAILBOX_GET_PITCH              = 0x00040008;
const word_t MAILBOX_GET_VIRTUAL_OFFSET     = 0x00040009;
const word_t MAILBOX_TEST_VIRTUAL_OFFSET    = 0x00044009;
const word_t MAILBOX_SET_VIRTUAL_OFFSET     = 0x00048009;
const word_t MAILBOX_GET_OVERSCAN           = 0x0004000a;
const word_t MAILBOX_TEST_OVERSCAN          = 0x0004400a;
const word_t MAILBOX_SET_OVERSCAN           = 0x0004800a;
const word_t MAILBOX_GET_PALETTE            = 0x0004000b;
const word_t MAILBOX_TEST_PALETTE           = 0x0004400b;
const word_t MAILBOX_SET_PALETTE            = 0x0004800b;
const word_t MAILBOX_SET_CURSOR_INFO        = 0x00008010;
const word_t MAILBOX_SET_CURSOR_STATE       = 0x00008011;
const word_t MAILBOX_SET_SCREEN_GAMMA       = 0x00008012;
const word_t MAILBOX_GET_EDID_BLOCK         = 0x00030020;*/




volatile __attribute__((aligned(4))) struct mailbox_registers * const mailbox = (struct mailbox_registers *const) 0x2000B880;


void mailbox_write(MAILBOX_CHANNEL channel, word_t* message)
{
   word_t status = ((0x40000000 | (word_t) message) & ~0xf) | channel;
   dmp();
   while(0 != (mailbox->box[1].status & 0x80000000)) {}
   mailbox -> box[1].read_write = status;
   dmp();
}

word_t* mailbox_read(MAILBOX_CHANNEL channel)
{
    if(channel == MB_CHANNEL_GPU) return NULL;
    //word_t value = 0;
    dmp();
    word_t read_cache = 0;
    do{
        while(0 != (mailbox -> box[0].status & 0x40000000)) {}
        read_cache = mailbox -> box[0].read_write;
    }while((read_cache & 0xf) != channel);
    dmp();
    return (word_t*) (read_cache & ~(0x40000000|0xf));
}

int mailbox_process(word_t* result_buffer, uint8_t result_len, ...)
{
    word_t __attribute__((aligned(16))) message[32];
    message[0] = (result_len+3)*sizeof(word_t); //TOTAL SIZE
    message[result_len + 2] = 0;                //NULL TERM
    message[1] = 0;                             //REQ TAG
    //va_list
    __builtin_va_list arg_list;
    __builtin_va_start (arg_list,result_len);
    /*TAG
     *LEN
     */
    for(uint8_t i = 0; i < result_len; i++)
    {
        message[i + 2] = __builtin_va_arg(arg_list,word_t);
    }
    __builtin_va_end(arg_list);
    mailbox_write(MB_CHANNEL_TAGS,message);
    word_t* raw_mem = mailbox_read(MB_CHANNEL_TAGS); //spin lock
    if(raw_mem[1] == 0x80000000 && result_buffer)
    {
        //need to parse results;
        word_t len_in_bytes_msg = raw_mem[0];
        word_t pos_word_in_raw_mem = 2;
        while(pos_word_in_raw_mem*sizeof(word_t) < len_in_bytes_msg && raw_mem[pos_word_in_raw_mem] != 0)
        {
            if((raw_mem[pos_word_in_raw_mem] &(0x1 << 31)) != 0)
            {
                //RESPONSE
                word_t len_of_response_in_bytes = (raw_mem[pos_word_in_raw_mem] & ~(0x1<<31));

                cpy(result_buffer,raw_mem+pos_word_in_raw_mem+1,len_of_response_in_bytes);
                result_buffer += len_of_response_in_bytes/sizeof(word_t);
                pos_word_in_raw_mem += 1 + len_of_response_in_bytes/sizeof(word_t);
            }
            else
            {
                pos_word_in_raw_mem++;
            }
        }
    }
    return 0;
    
}

