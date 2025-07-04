#ifndef _SDCARD_H
#define _SDCARD_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "stdint.h"

/** 
  * @brief  卡特定数据：CSD寄存器   
  */ 
typedef struct {
	uint8_t  CSDStruct;            /*!< CSD结构 */
	uint8_t  SysSpecVersion;       /*!< 系统规范版本 */
	uint8_t  Reserved1;            /*!< 保留 */
	uint8_t  TAAC;                 /*!< 数据读取访问时间1 */
	uint8_t  NSAC;                 /*!< 数据读取访问时间2（CLK周期数） */
	uint8_t  MaxBusClkFrec;        /*!< 最大总线时钟频率 */
	uint16_t CardComdClasses;      /*!< 卡命令类 */
	uint8_t  RdBlockLen;           /*!< 最大读取数据块长度 */
	uint8_t  PartBlockRead;        /*!< 允许读取部分块 */
	uint8_t  WrBlockMisalign;      /*!< 写块未对齐 */
	uint8_t  RdBlockMisalign;      /*!< 读块未对齐 */
	uint8_t  DSRImpl;              /*!< DSR实现 */
	uint8_t  Reserved2;            /*!< 保留 */
	uint32_t DeviceSize;           /*!< 设备大小 */
	uint8_t  MaxRdCurrentVDDMin;   /*!< 最小VDD下的最大读取电流 */
	uint8_t  MaxRdCurrentVDDMax;   /*!< 最大VDD下的最大读取电流 */
	uint8_t  MaxWrCurrentVDDMin;   /*!< 最小VDD下的最大写入电流 */
	uint8_t  MaxWrCurrentVDDMax;   /*!< 最大VDD下的最大写入电流 */
	uint8_t  DeviceSizeMul;        /*!< 设备大小乘数 */
	uint8_t  EraseGrSize;          /*!< 擦除组大小 */
	uint8_t  EraseGrMul;           /*!< 擦除组大小乘数 */
	uint8_t  WrProtectGrSize;      /*!< 写保护组大小 */
	uint8_t  WrProtectGrEnable;    /*!< 写保护组启用 */
	uint8_t  ManDeflECC;           /*!< 制造商默认ECC */
	uint8_t  WrSpeedFact;          /*!< 写入速度因子 */
	uint8_t  MaxWrBlockLen;        /*!< 最大写入数据块长度 */
	uint8_t  WriteBlockPaPartial;  /*!< 允许写入部分块 */
	uint8_t  Reserved3;            /*!< 保留 */
	uint8_t  ContentProtectAppli;  /*!< 内容保护应用 */
	uint8_t  FileFormatGrouop;     /*!< 文件格式组 */
	uint8_t  CopyFlag;             /*!< 复制标志（OTP） */
	uint8_t  PermWrProtect;        /*!< 永久写保护 */
	uint8_t  TempWrProtect;        /*!< 临时写保护 */
	uint8_t  FileFormat;           /*!< 文件格式 */
	uint8_t  ECC;                  /*!< ECC代码 */
	uint8_t  CSD_CRC;              /*!< CSD CRC */
	uint8_t  Reserved4;            /*!< 始终为1 */
} SD_CSD;

/** 
  * @brief  卡识别数据：CID寄存器   
  */
typedef struct {
	uint8_t  ManufacturerID;       /*!< 制造商ID */
	uint16_t OEM_AppliID;          /*!< OEM/应用ID */
	uint32_t ProdName1;            /*!< 产品名称第1部分 */
	uint8_t  ProdName2;            /*!< 产品名称第2部分 */
	uint8_t  ProdRev;              /*!< 产品版本 */
	uint32_t ProdSN;               /*!< 产品序列号 */
	uint8_t  Reserved1;            /*!< 保留1 */
	uint16_t ManufactDate;         /*!< 生产日期 */
	uint8_t  CID_CRC;              /*!< CID CRC */
	uint8_t  Reserved2;            /*!< 始终为1 */
} SD_CID;


/** 
  * @brief SD Card information 
  */
typedef struct {
	SD_CSD SD_csd;
	SD_CID SD_cid;
	uint64_t CardCapacity;  /*!< Card Capacity */
	uint32_t CardBlockSize; /*!< Card Block Size */
} SD_CardInfo;

extern SD_CardInfo cardinfo;

uint8_t sd_init(void);
uint8_t sd_read_sector(uint8_t *data_buff, uint32_t sector, uint32_t count);
uint8_t sd_write_sector(uint8_t *data_buff, uint32_t sector, uint32_t count);
uint8_t sd_read_sector_dma(uint8_t *data_buff, uint32_t sector, uint32_t count);
uint8_t sd_write_sector_dma(uint8_t *data_buff, uint32_t sector, uint32_t count);

#ifdef __cplusplus
}
#endif

#endif
