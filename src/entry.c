#include "GPIO/gpio.h"
#include "TIME/timer.h"
#include "TIME/decimaltime.h"
#include "mydef.h"
#include "interrupt.h"
#include "mailbox.h"




struct system_data
{
    word_t firmware_version;
    word_t board_model;
    word_t board_revision;
    word_t arm_mem_addr_base;
    word_t arm_mem_size;
    word_t gpu_mem_addr_base;
    word_t gpu_mem_size;
};


void system_init(struct system_data* data)
{

    word_t buffer[32];
    //mailbox_process(buffer,5,MAILBOX_GET_MAX_CLOCK_RATE,8,8,3,0);
    //mailbox_process(buffer,5,MAILBOX_SET_CLOCK_RATE,8,8,3,buffer[1]);
    mailbox_process(buffer,4*5+2,MAILBOX_GET_FIRMWARE_VER,4,0,0,
                               MAILBOX_GET_BOARD_MODEL,4,0,0,
                               MAILBOX_GET_BOARD_REVISION,4,0,0,
                               MAILBOX_GET_ARM_MEM,4,0,0,0,
                               MAILBOX_GET_VC_MEM,4,0,0,0);
    data -> firmware_version = buffer[0];
    //mailbox_process(buffer,4,MAILBOX_GET_BOARD_MODEL,4,0,0);
    data -> board_model      = buffer[1];
    //mailbox_process(buffer,4,MAILBOX_GET_BOARD_REVISION,4,0,0);
    data -> board_revision   = buffer[2];
    //mailbox_process(buffer,4,MAILBOX_GET_ARM_MEM,4,0,0);
    data -> arm_mem_addr_base= buffer[3];
    data -> arm_mem_size     = buffer[4];
    //mailbox_process(buffer,4,MAILBOX_GET_VC_MEM,4,0,0);
    data -> gpu_mem_addr_base= buffer[5];
    data -> gpu_mem_size     = buffer[6];
}




struct gpu_driver
{
    uint8_t* frame_buffer;
    word_t  frame_buffer_size;
    word_t height;
    word_t width;
    word_t depth;
    uint8_t* clock_draw_start;
    word_t  line_step;
};

void read_edid(uint32_t* height, uint32_t* width, uint32_t* depth)
{
   *height = 1080;
   *width = 1920;
   *depth = 32;

}

void gpu_driver_init(struct gpu_driver* gd)
{
    //read_edid(&gd->height,&gd->width,&gd->depth);
    word_t buffer[32];

    mailbox_process(buffer,5+4,MAILBOX_GET_PHYSICAL_WH,8,0,0,0,
                               MAILBOX_GET_DEPTH,4,4,0);
    gd->height = buffer[1];
    gd->width  = buffer[0];
    gd->depth  = buffer[2];

    mailbox_process(buffer,5*3+4,MAILBOX_SET_PHYSICAL_WH, 8, 8, gd -> width, gd -> height,
                                   MAILBOX_SET_VIRTUAL_WH,8, 8, gd -> width, gd -> height,
                                   MAILBOX_SET_DEPTH,4,4,gd->depth,
                                   MAILBOX_ALLOCATE_BUFFER,8,4,16,0);
    gd -> frame_buffer = (uint8_t*)buffer[5];
    gd -> frame_buffer_size = buffer[6];
    word_t start_height = (gd -> height - 32)/2;
    word_t start_width  = (gd -> width - (16 * 7))/2;
    //gd -> clock_draw_start = gd -> frame_buffer +  (gd -> width*(gd -> depth/8)*start_height + (gd->depth/8)*start_width);
   
    gd -> line_step = gd -> width*(gd -> depth / 8);
    gd -> clock_draw_start = gd -> frame_buffer + (gd -> line_step*start_height) + start_width*gd->depth/8;
}

void *cpy (void *restrict __dest, const void *restrict __src, size_t __n)
{
    char *d = __dest;
    const char *s = __src;
    for(;__n;__n--)
    {
        *d++ = *s++;
    }
    return __dest;
}
void *set (void *__s, int __c, size_t __n)
{
    uint32_t *s = __s;
    //__c;
    for(;__n>4;__n-=4)
    {
        *s++ =__c;
    }
    return __s;
}

//Each row is 16 bit wide and 32 bits long
const static uint16_t bit_font[] =
{
    0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0FF0,0x1FF8,0x381C,0x300C,0x300C,0x301C,0x303C,0x307C,0x30EC,0x31CC,0x338C,0x370C,0x3E0C,0x3C0C,0x380C,0x300C,0x300C,0x381C,0x1FF8,0x0FF0,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,//0
    0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0380,0x0780,0x0D80,0x1980,0x1180,0x0180,0x0180,0x0180,0x0180,0x0180,0x0180,0x0180,0x0180,0x0180,0x0180,0x0180,0x0180,0x0180,0x1FF8,0x1FF8,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,//1
    0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0FF0,0x1FF8,0x381C,0x300C,0x000C,0x000C,0x000C,0x0018,0x0030,0x0060,0x00C0,0x0180,0x0300,0x0600,0x0C00,0x1800,0x300C,0x300C,0x3FFC,0x3FFC,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,//2
    0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0FF0,0x1FF8,0x381C,0x300C,0x000C,0x000C,0x000C,0x0018,0x07F0,0x07F0,0x0018,0x000C,0x000C,0x000C,0x000C,0x000C,0x300C,0x381C,0x1FF8,0x0FF0,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,//3
    0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x3000,0x3000,0x3000,0x3000,0x3030,0x3030,0x3030,0x3030,0x3030,0x3030,0x3030,0x3030,0x3FFC,0x3FFC,0x0030,0x0030,0x0030,0x0030,0x0030,0x0030,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,//4
    0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x3FFC,0x3FFC,0x300C,0x300C,0x3000,0x3000,0x3000,0x3000,0x3FF0,0x3FF8,0x001C,0x000C,0x000C,0x000C,0x000C,0x000C,0x300C,0x381C,0x1FF8,0x0FF0,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,//5
    0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0FF0,0x1FF8,0x381C,0x300C,0x3000,0x3000,0x3000,0x3000,0x3FF0,0x3FF8,0x301C,0x300C,0x300C,0x300C,0x300C,0x300C,0x300C,0x381C,0x1FF8,0x0FF0,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,//6
    0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x3FFC,0x3FFC,0x300C,0x300C,0x000C,0x000C,0x000C,0x0018,0x0030,0x0060,0x00C0,0x0180,0x0300,0x0300,0x0300,0x0300,0x0300,0x0300,0x0300,0x0300,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,//7
    0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0FF0,0x1FF8,0x381C,0x300C,0x300C,0x300C,0x300C,0x1818,0x0FF0,0x0FF0,0x1818,0x300C,0x300C,0x300C,0x300C,0x300C,0x300C,0x381C,0x1FF8,0x0FF0,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,//8
    0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0FF0,0x1FF8,0x381C,0x300C,0x300C,0x300C,0x300C,0x300C,0x300C,0x380C,0x1FFC,0x0FFC,0x000C,0x000C,0x000C,0x000C,0x300C,0x381C,0x1FF8,0x0FF0,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,//9
    0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0180,0x0180,0x0180,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0180,0x0180,0x0180,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000//:
};
const static uint8_t WHITE[] = {0xFF,0xFF,0xFF,0xFF};
const static uint8_t BLACK[] = {0x00,0x00,0x00,0x00};

struct system_data sd;
struct gpu_driver gd;

void
write_letter(uint8_t* frame_ptr, uint8_t num)
{
    const uint16_t* bit_font_ptr = bit_font + (32*num);
    //uint8_t* font_ptr = font + (3*16*32)*num;
    for(int i = 0; i < 32; i++)
    {
        uint16_t line = *bit_font_ptr++;
        uint16_t mask = 0x1 << 15;
        uint8_t *current_frame_pointer = frame_ptr;
        while(mask != 0)
        {
            if(mask & line)
            {
                cpy(current_frame_pointer,WHITE,gd.depth/8);

            }else
            {
                cpy(current_frame_pointer,BLACK,gd.depth/8);

            }
            mask >>= 1;
            current_frame_pointer += (gd.depth/8); //One bit? 
        }
        //cpy(frame_ptr,font_ptr,3*16);
        frame_ptr+=gd.line_step;
    }
}

uint8_t current_digit[7] = {0,10,0,0,10,0,0};
void
update()
{
    uint8_t digit[7];
    digit[0] = (current_metric_time/(100*100))%10;
    digit[1] = 10;
    digit[2] = ((current_metric_time/100)%100)/10;
    digit[3] = ((current_metric_time/100)%100)%10;
    digit[4] = 10;
    digit[5] = (current_metric_time%(100))/10;
    digit[6] = (current_metric_time%(100))%10;
    uint8_t *ptr = gd.clock_draw_start;
    for(int i = 0; i < 7; i++)
    {
        if(current_digit[i] != digit[i])
            write_letter(ptr,digit[i]);
        ptr += 16*(gd.depth/8);
        current_digit[i] = digit[i];
    }
}

void
entry(void)
{
    gpu_driver_init(&gd);
    system_init(&sd);

    //memset(gd.frame_buffer,0xFF,gd.frame_buffer_size);
    enable_irq(IRQ_TIMER,timer_handler);
    timer_update(tic_per_metric_second);
    enable_irq(IRQ_GPIO0,gpio_handler);
    //enable_irq(IRQ_GPIO2,gpio_handler2);
    //enable_irq(IRQ_GPIO2,gpio_handler2);
    //enable_irq(IRQ_GPIO3,gpio_handler2);
    gpio_function(17,FSEL_IN);
    gpio_function(18,FSEL_IN);
    gpio_function(19,FSEL_IN);
    gpio_enable_rising(17,decimal_add_second);
    gpio_enable_rising(18,decimal_add_minute);
    gpio_enable_rising(19,decimal_add_hour);


    //handlers[IRQ_TIMER] = timer_handler;
    dmp();
    set(gd.frame_buffer,0,gd.frame_buffer_size);
    uint8_t *ptr = gd.clock_draw_start;
    for(int i = 0; i < 7; i++)
    {
        write_letter(ptr,current_digit[i]);
        ptr += 16*(gd.depth/8);
    }
    dmp();
    enable();

    //


    //timer_set_cmp(0,500000*4);


}

//System updates 1 000 000 times an imperial second
//A Day has 86400 imperial seconds
//A Day has 100000 metric seconds
//Thus a 1 000 000 tics / imperial seconds * 86400 imperial seconds/ 100000 metric seconds
//864000 tics/ metric second.



