#include <stdio.h>
#include "fpioa.h"
#include "sysctl.h"
#include "uarths.h"
#include "ff.h"
#include "FLASH_W25.h"
#include "LOG_SYSTEM/LOG.h"
/***************************************************************************************/
//                              未知原因,无法使用FATFS文件系统在FLASH中
/****************************************************************************************/

#define BUF_LENGTH (40 * 1024 + 5)

uint8_t write_buf[BUF_LENGTH];
uint8_t read_buf[BUF_LENGTH];
#define DATA_ADDRESS  0xB00000  //2816

// FATFS fs;                         /* FatFs文件系统对象 */
// FIL fnew;                         /* 文件对象 */
// UINT fnum;                        /* 文件成功读写数量 */

// FRESULT FATFS_Init_FLASH(){
//     FRESULT res;
//     /* 挂载FATFS系统 */
//     //在外部SPI Flash挂载文件系统，文件系统挂载时会对SPI设备初始化
//     //初始化函数调用流程如下
//     //f_mount()->find_volume()->disk_initialize->SPI_FLASH_Init()
//     res = f_mount(&fs, "1:", 1);
//     if(res == FR_NO_FILESYSTEM){
//         EMLOG(LOG_INFO,"FATFS SYSTEM not found\n");
//         res = f_mkfs("1:",NULL,write_buf, BUF_LENGTH);
//         if(res == FR_OK){
//             EMLOG(LOG_INFO,"FATFS SYSTEM remount\n");
//             res = f_mount(NULL, "1:", 1);
//             res = f_mount(&fs, "1:", 1);
//             if(res == FR_OK){
//                 EMLOG(LOG_INFO,"FATFS SYSTEM Mount Success\n");
//                 return FR_OK;
//             }else{
//                 EMLOG(LOG_ERROR,"FATFS SYSTEM Mount failed\n");
//                 return res;
//             }
//         }else{
//             EMLOG(LOG_ERROR,"FATFS SYSTEM create failed\n");
//             return  res;
//         }
//     }else if(res != FR_OK){
//         EMLOG(LOG_ERROR,"FATFS SYSTEM mount failed,ERROR CODE:%d\n",res);
//         return res;
//     }
//     return FR_OK;
// }


int main(void){
    FRESULT res;
    /* 设置新PLL0频率 */
    sysctl_pll_set_freq(SYSCTL_PLL0, 800000000);
    uarths_init();
    EMLOG(LOG_INFO,"Flash FilesSystem test\n");
    // if(FATFS_Init_FLASH() == FR_OK){
    //     EMLOG(LOG_INFO,"Flash FilesSystem mount OK\n");
    //     EMLOG(LOG_INFO,"next Can read or write files test\n");
    //                             /* 写文件测试 */
    //     res=f_open(&fnew,"1:FatFs11.txt",FA_CREATE_ALWAYS|FA_WRITE);
    //     if ( res == FR_OK ) {
    //         printf_green("open files succeed, can write data to files\n");
    //         /* 将指定存储区内容写入到文件内 */
    //         res=f_write(&fnew,"write_buf",10,&fnum);
    //         if (res==FR_OK) {
    //             printf("files write succeed, write byte szie: %d\n",fnum);
    //             printf("data is: write_buf\n");
    //         } else {
    //             printf("! ! files write fail, ERROR ID:(%d)\n",res);
    //         }
    //         /* 不再读写，关闭文件 */
    //         f_close(&fnew);
    //     } else {
    //         printf("! ! open files fail");
    //         printf(":%d\n",(int)res);
    //     }
    //                         /* 读文件测试 */
    //     res=f_open(&fnew,"1:FatFs11.txt",FA_OPEN_EXISTING|FA_READ);
    //     if ( res == FR_OK ) {
    //         printf_green("open files succeed, can read data to files\n");
    //         /* 将指定存储区内容写入到文件内 */
    //         res=f_read(&fnew,read_buf,sizeof(read_buf),&fnum);
    //         if (res==FR_OK) {
    //             printf("files write succeed, read byte szie: %d\n",fnum);
    //             printf("data is: %s\n",read_buf);
    //         } else {
    //             printf("! ! files read fail, ERROR ID:(%d)\n",res);
    //         }
    //         /* 不再读写，关闭文件 */
    //         f_close(&fnew);
    //     } else {
    //         printf("! ! open files fail\n");
    //     }
    // }
    // /* 不再使用文件系统，取消挂载文件系统 */
    // f_mount(NULL,"1:",1);

    /* 给缓存写入的数据赋值 */
    for (int i = 0; i < BUF_LENGTH; i++)
        write_buf[i] = 'l';
    /* 清空读取的缓存数据 */
    for(int i = 0; i < BUF_LENGTH; i++)
        read_buf[i] = 0;
    FLASH_W25_Init();
    FLASH_W25_Write(DATA_ADDRESS,write_buf,BUF_LENGTH);
    FLASH_W25_Read(DATA_ADDRESS,read_buf,BUF_LENGTH);

    while(1){
        ;
    }
    return 0;
}