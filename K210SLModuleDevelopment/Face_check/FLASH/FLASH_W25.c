#include "FLASH_W25.h"
#include "LOG_SYSTEM/LOG.h"
#include "stdio.h"
#include "memory.h"
#include "spi.h"

/* 
        32--- 0010 0000   SPI_FF_STANDARD --0[0000 0000]
        24--- 0001 1000   SPI_FF_QUAD     --2[0000 0010]
*/
static uint8_t W25_SPI_Init_State;  //SPI初始化标志位
struct W25ID W25FLASH_INFO;

/**
 * @brief  接收W25QXX数据
 * @param  cmd_buff  命令缓冲区
 * @param  cmd_len  命令长度
 * @param  rx_buff  接收缓冲区
 * @param  rx_len  接收长度
 * @return w25qxx_status_t 接收状态
 */
static w25qxx_status_t W25_Receive_Data(uint8_t *cmd_buff, uint8_t cmd_len, uint8_t *rx_buff, uint32_t rx_len){
    if((W25_SPI_Init_State & 0x03) != SPI_FF_STANDARD){
        // 初始化SPI总线为标准模式
        spi_init(W25_FLASH_SPIDEVICE, SPI_WORK_MODE_0, SPI_FF_STANDARD, DATALENGTH, 0);
        W25_SPI_Init_State = 0;W25_SPI_Init_State |= SPI_FF_STANDARD;
        // EMLOG(LOG_DEBUG,"SPI Init state SPI_FF_STANDARD %x\n",W25_SPI_Init_State);
    }
    // 接收SPI标准数据
    spi_receive_data_standard(W25_FLASH_SPIDEVICE, W25_FLASH_CS, cmd_buff, cmd_len, rx_buff, rx_len);
    // 返回正常状态
    return W25QXX_OK;
}

/**
 * @brief  向W25QXX发送数据
 * @param  cmd_buff  命令缓冲区
 * @param  cmd_len  命令长度
 * @param  tx_buff  数据缓冲区
 * @param  tx_len  数据长度
 * @return w25qxx_status_t 发送状态
 */
static w25qxx_status_t W25_Send_Data(uint8_t *cmd_buff, uint8_t cmd_len, uint8_t *tx_buff, uint32_t tx_len){
    if((W25_SPI_Init_State & 0x03) != SPI_FF_STANDARD){
        // 初始化SPI总线为标准模式
        spi_init(W25_FLASH_SPIDEVICE, SPI_WORK_MODE_0, SPI_FF_STANDARD, DATALENGTH, 0);
        W25_SPI_Init_State = 0;W25_SPI_Init_State |= SPI_FF_STANDARD;
        //EMLOG(LOG_DEBUG,"SPI Init state SPI_FF_STANDARD %x\n",W25_SPI_Init_State);
    }
    // 发送SPI标准数据
    spi_send_data_standard(W25_FLASH_SPIDEVICE, W25_FLASH_CS, cmd_buff, cmd_len, tx_buff, tx_len);
    // 返回正常状态
    return W25QXX_OK;
}

static w25qxx_status_t W25_write_enable(void){
    uint8_t cmd[1] = {W25_WRITE_ENABLE};
    W25_Send_Data(cmd, 1, 0, 0);
    return W25QXX_OK;
}

static w25qxx_status_t W25_read_status_reg1(uint8_t *reg_data){
    uint8_t cmd[1] = {W25_READ_REG1};
    uint8_t data[1] = {0};
    W25_Receive_Data(cmd, 1, data, 1);
    *reg_data = data[0];
    return W25QXX_OK;
}

//W25工作状态获取
static w25qxx_status_t W25_WaitForBusy(void){
    uint8_t status = 0;
    while(1){
        W25_read_status_reg1(&status);
        if (!(status & W25_REG1_BUSY_MASK)){
            return W25QXX_OK;
        }
    }
}

//扇区擦除
w25qxx_status_t W25_sector_erase(uint32_t addr){
    uint8_t cmd[4] = {W25_SECTOR_ERASE};
    cmd[1] = (uint8_t)(addr >> 16);
    cmd[2] = (uint8_t)(addr >> 8);
    cmd[3] = (uint8_t)(addr);
    W25_write_enable();
    W25_Send_Data(cmd, 4, 0, 0);
    W25_WaitForBusy();
    return W25QXX_OK;
}

// 页大小写入
static w25qxx_status_t W25_quad_page_program(uint32_t addr, uint8_t *data_buf, uint32_t length)
{
    uint32_t cmd[2]={0};
    cmd[0] = W25_QUAD_PAGE_PROGRAM;
    cmd[1] = addr;
    W25_write_enable();
    if(((W25_SPI_Init_State & 0x18) != 24) || ((W25_SPI_Init_State & 0x03) != SPI_FF_QUAD)){
        spi_init(W25_FLASH_SPIDEVICE, SPI_WORK_MODE_0, SPI_FF_QUAD, 32, 0);
        spi_init_non_standard(W25_FLASH_SPIDEVICE, 8, 24, 0,SPI_AITM_STANDARD);
        W25_SPI_Init_State = 0;W25_SPI_Init_State |= 0x18;W25_SPI_Init_State |= SPI_FF_QUAD;
        //EMLOG(LOG_DEBUG,"SPI Init state SPI_FF_QUAD\n");
    }
    spi_send_data_multiple_dma(W25_FLASH_DMA_SEND, W25_FLASH_SPIDEVICE, W25_FLASH_CS, cmd, 2, data_buf, length);

    W25_WaitForBusy();//等待写入完成
    return W25QXX_OK;
}


// 扇区大小写入
static w25qxx_status_t W25_sector_program(uint32_t addr, uint8_t *data_buf){
    uint8_t index = 0;
    //写入数据都需要 按照页 或者 扇区 或者 簇 为单位进行;最小按照 "页(256字节)"写入
    for (index = 0; index < W25_FLASH_PAGE_NUM_PER_SECTOR; index++){
        //一次最多写一页,如果超过一页数据长度，则分多次完成
        W25_quad_page_program(addr, data_buf, W25_FLASH_PAGE_SIZE);
        addr += W25_FLASH_PAGE_SIZE;
        data_buf += W25_FLASH_PAGE_SIZE;
    }
    return W25QXX_OK;
}
/**
 * @brief  从W25QXX芯片读取数据
 * @param  addr  数据的地址
 * @param  data_buf  数据缓冲区
 * @param  length  读取数据长度
 * @return w25qxx_status_t  写入状态
 */
w25qxx_status_t FLASH_W25_Read(uint32_t addr, uint8_t *data_buf, uint32_t length){
    uint32_t v_remain = length % 4;
    if(v_remain != 0){length = length / 4 * 4;}  // 4字节对齐(4取整)
    uint32_t len    = 0;
    uint32_t cmd[2];
    cmd[0] = W25_FAST_READ_QUAL_IO;
    while (length){
        // GD25LQ128C是一款FLASH芯片，其读取操作理论上是没有限制的。
        len = ((length >= 0x010000) ? 0x010000 : length);// 0x010000 * 32 = 2MB
        //在实际应用中，为了提高读取速度，可能会对每次读取的数据量进行限制
        cmd[1] = addr << 8;
        if(((W25_SPI_Init_State & 0x20) != 32) || ((W25_SPI_Init_State & 0x03) != SPI_FF_QUAD)){
            spi_init(W25_FLASH_SPIDEVICE, SPI_WORK_MODE_0, SPI_FF_QUAD, 32, 0);
            /*  标准的SPI协议下，指令和数据传输的格式如下：
                | CS | 指令 (8-bit) | 地址 (24-bit) | 数据 (8-bit) | 校验/填充 |
            */
            spi_init_non_standard(W25_FLASH_SPIDEVICE, 8, 32, 4,SPI_AITM_ADDR_STANDARD);
            W25_SPI_Init_State = 0;W25_SPI_Init_State |= 0x20 ;W25_SPI_Init_State |= SPI_FF_QUAD;
            //EMLOG(LOG_DEBUG,"SPI Init state SPI_FF_QUAD %x\n",W25_SPI_Init_State);
        }
        //使用收发（2）通道DMA传输
        spi_receive_data_multiple_dma(W25_FLASH_DMA_SEND, W25_FLASH_DMA_RECEIVE, W25_FLASH_SPIDEVICE, W25_FLASH_CS, cmd, 2, data_buf, len);
        addr += len;
        data_buf += len;
        length -= len;
    }
    if(v_remain){//如果还有剩余,则继续读取
    //由于GD25LQ128C的读取操作是4字节对齐的，所以当读取的数据长度不是4的倍数时,需要对读取到的数据进行截取
        cmd[1] = addr << 8;
        uint8_t v_recv_buf[4];
        spi_receive_data_multiple_dma(W25_FLASH_DMA_SEND, W25_FLASH_DMA_RECEIVE, W25_FLASH_SPIDEVICE, W25_FLASH_CS, cmd, 2, v_recv_buf, 4);
        memcpy(data_buf, v_recv_buf, v_remain);
    }
    EMLOG(LOG_DEBUG,"Read Data Success\n");
    return W25QXX_OK;
}

/**
 * @brief  将数据写入W25QXX芯片(新版)
 * @param  addr  写入数据的地址
 * @param  data_buf  写入的数据缓冲区
 * @param  length  写入的数据长度
 * @return w25qxx_status_t  写入状态
 * @brief 该函数由于使用了初始化全零的swap_buf，
 *        所以写入数据时不需要考虑不足一页的问题
 */
w25qxx_status_t FLASH_W25_Write(uint32_t addr, uint8_t *data_buf, uint32_t length){
    uint32_t sector_addr   = 0;      // 扇区地址
    uint32_t sector_offset = 0;      // 扇区偏移量
    uint32_t sector_remain = 0;      // 扇区剩余空间
    uint32_t write_len     = 0;      // 本次写入长度(根据剩余空间变化)
    uint8_t* pread         = NULL;
    uint8_t* pwrite        = NULL;
    uint32_t index;
    uint8_t swap_buf[W25_FLASH_SECTOR_SIZE] = {0};  // 交换缓冲区

    while(length){// 当还有数据未写入时
        sector_addr   = addr & (~(W25_FLASH_SECTOR_SIZE - 1));                // 计算扇区地址
        sector_offset = addr & (W25_FLASH_SECTOR_SIZE - 1);                   // 计算扇区偏移量
        sector_remain = W25_FLASH_SECTOR_SIZE - sector_offset;                // 计算扇区剩余空间
        write_len     = ((length < sector_remain) ? length : sector_remain);  // 计算本次写入长度
        /******************************************************************************************/
        /*************************如果写入的数据刚好占用一个扇区**************************************/
        /******************************************************************************************/
        if(write_len == W25_FLASH_SECTOR_SIZE){
            W25_sector_erase(sector_addr);
            W25_sector_program(sector_addr, data_buf);  // 扇区写入
        }else{ // 如果扇区部分写入
            pread  = swap_buf + sector_offset;  // 设置读指针
            pwrite = data_buf;                  // 设置写指针
            FLASH_W25_Read(sector_addr, swap_buf, W25_FLASH_SECTOR_SIZE);  // 读取扇区数据到交换缓冲区
            /**************************************************************************/
            /*********************检查是否需要清除扇区**********************************/
            /**************************************************************************/
            for (index = 0; index < write_len; index++){ // 循环比较数据,看是否需要擦除对应扇区
                if ((*pwrite) != ((*pwrite) & (*pread))){
                    //因为FLASH的特殊性，0可以覆盖1，所以这里可以通过判断 当前数据 直接覆写能否 得到当前数据
                    //来判断是否需要进行扇区擦除
                    W25_sector_erase(sector_addr); // 扇区擦除
                    break; // 跳出循环
                }
                pwrite++; // 写指针后移
                pread++; // 读指针后移
            }
            /**********************************************************************************/
            /*************************整合数据成为一个扇区的大小*********************************/
            /*********************************************************************************/
            pread  = swap_buf + sector_offset;  // 设置读指针
            pwrite = data_buf;                  // 设置写指针
            for (index = 0; index < write_len; index++){ // 循环写入数据
                *pread++ = *pwrite++; // 将扇区原来的数据与待写入的数据进行融合
            }
            W25_sector_program(sector_addr, swap_buf); // 扇区写入
        }
        length   -= write_len;  // 减去已写入长度
        addr     += write_len;  // 增加已写入地址,便于下一轮残余写入
        data_buf += write_len;  // 增加已写入数据缓冲区
    }
    return W25QXX_OK;
}

// 读取W25QXX芯片的ID
w25qxx_status_t FLASH_W25_GetID(uint8_t *manuf_id, uint8_t *device_id){
    // 定义读取ID的命令
    uint8_t cmd[4] = {W25_READ_ID, 0x00, 0x00, 0x00};
    // 定义接收数据的变量
    uint8_t data[2] = {0};
    // 发送命令并接收数据
    W25_Receive_Data(cmd, 4, data, 2);//SPI标准模式是收发同时进行的
    // 将接收到的数据赋值给manuf_id和device_id
    W25FLASH_INFO.manuf_id  = data[0];  //制造商 ID 
    W25FLASH_INFO.device_id = data[1];  //和设备 ID 
    EMLOG(LOG_INFO,"FLASH->[manuf_id:0x%02x, device_id:0x%02x]\n",W25FLASH_INFO.manuf_id, W25FLASH_INFO.device_id);
    // 返回状态
    return W25QXX_OK;
}

// 读取W25QXX的状态寄存器2
static w25qxx_status_t W25_read_reg2(uint8_t *reg_data){
    // 定义命令字节
    uint8_t cmd[1]  = {W25_READ_REG2};
    // 定义接收数据字节
    uint8_t data[1] = {0};
    // 发送命令并接收数据
    W25_Receive_Data(cmd, 1, data, 1);
    // 将读取到的数据赋值给reg_data
    *reg_data = data[0];
    // 返回正常状态
    return W25QXX_OK;
}

static w25qxx_status_t W25_write_reg2(uint8_t reg1_data, uint8_t reg2_data){
    uint8_t cmd[3] = {W25_WRITE_REG1, reg1_data, reg2_data};
    W25_write_enable();
    W25_Send_Data(cmd, 3, 0, 0);
    return W25QXX_OK;
}

w25qxx_status_t W25_enable_quad_mode(void){
    uint8_t reg_data = 0;
    W25_read_reg2(&reg_data);
    if (!(reg_data & W25_REG2_QUAL_MASK)){
        reg_data |= W25_REG2_QUAL_MASK;
        W25_write_reg2(0x00, reg_data);
    }
    return W25QXX_OK;
}

w25qxx_status_t FLASH_W25_Init(void){
    //硬件树注册(未来实现)
    /***********************************/
    // 初始化spi总线
    EMLOG(LOG_DEBUG,"W25FLASH SPI INIT\n");
    spi_init(W25_FLASH_SPIDEVICE, SPI_WORK_MODE_0, SPI_FF_STANDARD, DATALENGTH, 0);
    // 设置spi总线时钟频率
    EMLOG(LOG_DEBUG,"W25FLASH SPI CLK SET 25M\n");
    spi_set_clk_rate(W25_FLASH_SPIDEVICE, 25000000);
    // 使能w25qxx四 Quad 模式
    EMLOG(LOG_DEBUG,"W25FLASH ENABLE QUAD MODE\n");
    W25_enable_quad_mode();
    //获取设备商号与设备ID,并判断是否正确
    EMLOG(LOG_DEBUG,"Get FLASH ID\n");
    FLASH_W25_GetID(&W25FLASH_INFO.manuf_id, &W25FLASH_INFO.device_id);
    if((W25FLASH_INFO.manuf_id != 0xEF && W25FLASH_INFO.manuf_id != 0xC8)
        || (W25FLASH_INFO.device_id != 0x17 && W25FLASH_INFO.device_id != 0x16)){
        EMLOG(LOG_ERROR,"w25qxx_read_id error->> \n");
        return W25QXX_ERROR;    
    }else{
        EMLOG(LOG_DEBUG,"W25FLASH Init success\n");
        return W25QXX_OK;
    }
}









// /**
//  * @brief  将数据写入W25QXX芯片(旧版)
//  * @param  addr  写入数据的地址
//  * @param  data_buf  写入的数据缓冲区
//  * @param  length  写入的数据长度
//  * @return w25qxx_status_t  写入状态
//  * @brief 该函数由于使用了初始化全零的swap_buf，
//  *        所以写入数据时不需要考虑不足一页的问题
//  */
// w25qxx_status_t FLASH_W25_Write(uint32_t addr, uint8_t *data_buf, uint32_t length){
//     uint32_t sector_addr                        = 0;     // 扇区地址
//     uint32_t sector_offset                      = 0;     // 扇区偏移量
//     uint32_t sector_remain                      = 0;     // 扇区剩余空间
//     uint32_t write_len                          = 0;     // 本次写入长度(根据剩余空间变化)
//     uint32_t index                              = 0;     // 循环变量
//     uint8_t  *pread                             = NULL;  // 读指针
//     uint8_t  *pwrite                            = NULL;  // 写指针
//     uint8_t  swap_buf[W25_FLASH_SECTOR_SIZE] = {0};   // 交换缓冲区
//     while (length){// 当还有数据未写入时
//         /*
//             GD25LQ128C 的一款存储空间为16MB 的FLASH芯片，总共有4096个扇区，
//             每个扇区有16页，每页是256字节。
//             所以它的最大地址是：0x100 0000 - 1 = 0x00 FF FF FF
//         */
//         sector_addr = addr & (~(W25_FLASH_SECTOR_SIZE - 1)); // 计算扇区地址
//         这里实际操作结果相当于取（ addr & ~0xFFF ）清除低位不影响扇区
//         sector_offset = addr & (W25_FLASH_SECTOR_SIZE - 1); // 计算扇区偏移量
//         获取与扇区内地址的偏移量，相当于取（ addr & 0xFFF ）
//         sector_remain = W25_FLASH_SECTOR_SIZE - sector_offset; // 计算扇区剩余空间
//         write_len = ((length < sector_remain) ? length : sector_remain); // 计算本次写入长度
//         由于FLASH的擦除是按照页来算的,所以需要将当前页中的数据读出来保存在缓冲区中
//         FLASH_W25_Read(sector_addr, swap_buf, W25_FLASH_SECTOR_SIZE); // 读取扇区数据到交换缓冲区
//         pread = swap_buf + sector_offset; // 设置读指针
//         pwrite = data_buf; // 设置写指针
//         EMLOG(LOG_DEBUG,"Check data is different\n");
//         for (index = 0; index < write_len; index++){ // 循环比较数据,看是否需要擦除对应扇区
//             if ((*pwrite) != ((*pwrite) & (*pread))){
//                 因为FLASH的特殊性，0可以覆盖1，所以这里可以通过判断 当前数据 直接覆写能否 得到当前数据
//                 来判断是否需要进行扇区擦除
//                 EMLOG(LOG_DEBUG,"DATA is different, Wait erase sector\n");
//                 W25_sector_erase(sector_addr); // 扇区擦除
//                 break; // 跳出循环
//             }
//             pwrite++; // 写指针后移
//             pread++; // 读指针后移
//         }
//         EMLOG(LOG_INFO,"Write_Len:%d,sector_offset:%x\n",write_len,sector_offset);
//         if (write_len == W25_FLASH_SECTOR_SIZE){//如果可写入数据刚好为一个扇区(write_len = min[扇区可写入空间,待写入数据大小])
//             W25_sector_program(sector_addr, data_buf); // 扇区写入
//         }else{ // 如果扇区部分写入
//             pread = swap_buf + sector_offset;
//             pwrite = data_buf;
//             for (index = 0; index < write_len; index++)
//                 *pread++ = *pwrite++;
//             W25_sector_program(sector_addr, swap_buf); // 扇区写入
//         }
//         length   -= write_len;  // 减去已写入长度
//         addr     += write_len;  // 增加已写入地址,便于下一轮残余写入
//         data_buf += write_len;  // 增加已写入数据缓冲区
//     }
//     EMLOG(LOG_DEBUG,"Write Data Done!!!!!\n");
//     return W25QXX_OK; // 返回写入状态
// }



// static w25qxx_status_t w25qxx_quad_page_program(uint32_t addr, uint8_t *data_buf, uint32_t length)
// {
//     uint32_t cmd[2] = {0};
//     cmd[0] = W25_QUAD_PAGE_PROGRAM;
//     cmd[1] = addr;
//     W25_write_enable();
//     //这三段函数不可分割，否则会导致写入失败，未知原因
//     spi_init(W25_FLASH_SPIDEVICE, SPI_WORK_MODE_0, SPI_FF_QUAD, 32/*DATALENGTH*/, 0);
//     spi_init_non_standard(W25_FLASH_SPIDEVICE, 8/*instrction length*/, 24/*address length*/, 0/*wait cycles*/,
//                           SPI_AITM_STANDARD/*spi address trans mode*/);
//     W25_SPI_Init_State = 0;W25_SPI_Init_State |= 0x18;W25_SPI_Init_State |= SPI_FF_QUAD;
//     EMLOG(LOG_DEBUG,"SPI Init state SPI_FF_QUAD \n");
//     spi_send_data_multiple_dma(W25_FLASH_DMA_SEND, W25_FLASH_SPIDEVICE, W25_FLASH_CS, cmd, 2, data_buf, length);
    
//     W25_WaitForBusy();
//     return W25QXX_OK;
// }

// static w25qxx_status_t w25qxx_sector_program(uint32_t addr, uint8_t *data_buf)
// {
//     uint8_t index = 0;

//     for (index = 0; index < W25_FLASH_PAGE_NUM_PER_SECTOR; index++)
//     {
//         w25qxx_quad_page_program(addr, data_buf, W25_FLASH_PAGE_SIZE);
//         addr += W25_FLASH_PAGE_SIZE;
//         data_buf += W25_FLASH_PAGE_SIZE;
//     }
//     return W25QXX_OK;
// }

// w25qxx_status_t FLASH_W25_Write(uint32_t addr, uint8_t *data_buf, uint32_t length)
// {
//     uint32_t sector_addr                        = 0;     // 扇区地址
//     uint32_t sector_offset                      = 0;     // 扇区偏移量
//     uint32_t sector_remain                      = 0;     // 扇区剩余空间
//     uint32_t write_len                          = 0;     // 本次写入长度(根据剩余空间变化)
//     uint32_t index                              = 0;     // 循环变量
//     uint8_t  *pread                             = NULL;  // 读指针
//     uint8_t  *pwrite                            = NULL;  // 写指针
//     uint8_t  swap_buf[W25_FLASH_SECTOR_SIZE] = {0};   // 交换缓冲区

//     while (length)
//     {
//         sector_addr = addr & (~(W25_FLASH_SECTOR_SIZE - 1)); // 计算扇区地址
//         // 这里实际操作结果相当于取（ addr & ~0xFFF ）清除低位不影响扇区
//         sector_offset = addr & (W25_FLASH_SECTOR_SIZE - 1); // 计算扇区偏移量
//         // 获取与扇区内地址的偏移量，相当于取（ addr & 0xFFF ）
//         sector_remain = W25_FLASH_SECTOR_SIZE - sector_offset; // 计算扇区剩余空间
//         write_len = ((length < sector_remain) ? length : sector_remain); // 计算本次写入长度
//         FLASH_W25_Read(sector_addr, swap_buf, W25_FLASH_SECTOR_SIZE);
//         pread = swap_buf + sector_offset;
//         pwrite = data_buf;
//         for (index = 0; index < write_len; index++)
//         {
//             if ((*pwrite) != ((*pwrite) & (*pread)))
//             {
//                 W25_sector_erase(sector_addr);
//                 break;
//             }
//             pwrite++;
//             pread++;
//         }
//         EMLOG(LOG_INFO,"Write_Len:%d,sector_offset:%x\n",write_len,sector_offset);
//         if (write_len == W25_FLASH_SECTOR_SIZE)
//         {
//             w25qxx_sector_program(sector_addr, data_buf);
//         }
//         else
//         {
//             pread = swap_buf + sector_offset;
//             pwrite = data_buf;
//             for (index = 0; index < write_len; index++)
//                 *pread++ = *pwrite++;
//             w25qxx_sector_program(sector_addr, swap_buf);
//         }
//         length -= write_len;
//         addr += write_len;
//         data_buf += write_len;
//     }
//     return W25QXX_OK;
// }
