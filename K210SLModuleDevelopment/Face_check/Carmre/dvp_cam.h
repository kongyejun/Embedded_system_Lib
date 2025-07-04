#ifndef _DVP_CAM_H_
#define _DVP_CAM_H_

#include <stdint.h>

#define CAM_WIDTH_PIXEL        (320)
#define CAM_HIGHT_PIXEL        (240)

enum OV_type{
    OV_error = 0,
    OV_9655 = 1,
    OV_2640 = 2,
};

extern uint8_t *g_ai_buf_in;
extern uint32_t g_ai_red_buf_addr, g_ai_green_buf_addr, g_ai_blue_buf_addr;

extern uint32_t display_buf_addr1;
extern uint32_t display_buf_addr2;
extern volatile uint8_t g_dvp_finish_flag;
extern uint8_t gindx_i;

void dvp_cam_init(void);
void dvp_camAI_init(void);
void dvp_cam_set_irq(void);
int OVxxxx_read_id(void);

#endif /* _DVP_CAM_H_ */
