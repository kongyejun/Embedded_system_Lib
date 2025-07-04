#include "pin_config.h"
#include "uarths.h"
#include "sysctl.h"
#include "sdcard.h"
#include "stdio.h"
#include "ff.h"
#include "bsp.h"
#include "LOG.h"

FATFS fs;                         /* FatFs文件系统对象 */
FIL fnew;                         /* 文件对象 */
UINT fnum;                        /* 文件成功读写数量 */
BYTE ReadBuffer[1024]= {0};       /* 读缓冲区 */
BYTE WriteBuffer[] = "AbCDEFG";   /* 写缓冲区*/

void Hardware_Init(void){
    fpioa_set_function(PIN_TF_MISO,FUNC_TF_SPI_MISO);
    fpioa_set_function(PIN_TF_CLK,FUNC_TF_SPI_CLK);
    fpioa_set_function(PIN_TF_MOSI,FUNC_TF_SPI_MOSI);
    fpioa_set_function(PIN_TF_CS,FUNC_TF_SPI_CS);

    /* 设置PLL频率（系统时钟频率） */
    sysctl_pll_set_freq(SYSCTL_PLL0, 800000000UL);
    sysctl_pll_set_freq(SYSCTL_PLL1, 300000000UL);
    sysctl_pll_set_freq(SYSCTL_PLL2, 45158400UL);

    /* 由于PLL频率被改，需要重新初始化以下uarths */        
    uarths_init();
}

FRESULT Check_SDCARD_have_FASFT(void){
    FRESULT res; /* 文件操作结果 */
    res = f_mount(&fs,"0:",1);//会自动调用初始化SD卡函数的
    if(res == FR_NO_FILESYSTEM){
        printf("SD_CARD NO FileSystem, next will init FatfsSystem\n");
        res = f_mkfs("0:",NULL,ReadBuffer,1024);
        if(res == FR_OK){
            printf("SD_CARD Make Files System is OK\n");
            res = f_mount(NULL,"0:",1);// 卸载文件系统
            res = f_mount(&fs,"0:",1);// 重新挂载文件系统
        }else{
            printf("SD_CARD Make Files System ERRROR\n");
            return res;
        }
    }else if(res != FR_OK){
        printf("SD_CARD mount ERROR\n");
        return res;
    }
    return FR_OK;
}

int main(void){
    FRESULT res;
    //软硬件映射
    Hardware_Init();
    //检查是否装载有FATFS系统
    if(Check_SDCARD_have_FASFT() == FR_OK){
        printf("SD_CARD FilesSystem mount OK\n");
        printf("next Can read or write files test\n");
                                /* 写文件测试 */
        res=f_open(&fnew,"0:FatFs读写测试文件.txt",FA_CREATE_ALWAYS|FA_WRITE);
        if ( res == FR_OK ) {
            printf_green("open files succeed, can write data to files\n");
            /* 将指定存储区内容写入到文件内 */
            res=f_write(&fnew,WriteBuffer,sizeof(WriteBuffer),&fnum);
            /* 实测SPI_SD驱动下写入大于512字节的数据在SD卡里打开会显示乱码，如
            需写入大量数据使用f_write_co替代上面f_write即可 */
            //res=f_write_co(&fnew,WriteBuffer,sizeof(WriteBuffer),&fnum);
            if (res==FR_OK) {
                printf("files write succeed, write byte szie: %d\n",fnum);
                printf("data is: %s\n",WriteBuffer);
            } else {
                printf("! ! files write fail, ERROR ID:(%d)\n",res);
            }
            /* 不再读写，关闭文件 */
            f_close(&fnew);
        } else {
            printf("! ! open files fail\n");
        }
                            /* 读文件测试 */
        res=f_open(&fnew,"0:FatFs读写测试文件.txt",FA_OPEN_EXISTING|FA_READ);
        if ( res == FR_OK ) {
            printf_green("open files succeed, can read data to files\n");
            /* 将指定存储区内容写入到文件内 */
            res=f_read(&fnew,ReadBuffer,sizeof(ReadBuffer),&fnum);
            /* 实测SPI_SD驱动下写入大于512字节的数据在SD卡里打开会显示乱码，如
            需写入大量数据使用f_write_co替代上面f_write即可 */
            //res=f_write_co(&fnew,WriteBuffer,sizeof(WriteBuffer),&fnum);
            if (res==FR_OK) {
                printf("files write succeed, read byte szie: %d\n",fnum);
                printf("data is: %s\n",ReadBuffer);
            } else {
                printf("! ! files read fail, ERROR ID:(%d)\n",res);
            }
            /* 不再读写，关闭文件 */
            f_close(&fnew);
        } else {
            printf("! ! open files fail\n");
        }
    }
    /* 不再使用文件系统，取消挂载文件系统 */
    f_mount(NULL,"0:",1);

    while(1){
        ;
    }
    return 0;
}