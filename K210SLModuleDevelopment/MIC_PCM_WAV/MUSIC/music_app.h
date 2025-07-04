#ifndef __MUSIC_APP_H__
#define __MUSIC_APP_H__
#include "SD\FATFS\ff.h"

#define MAX_MUSIC_FILE_NUM 100
#define MAX_MUSIC_FILE_NAME_LEN 50

typedef struct {
    char MusicFiles[MAX_MUSIC_FILE_NUM][MAX_MUSIC_FILE_NAME_LEN];      /*音乐文件目录*/
    uint8_t MusicNum;             /*音乐文件数量*/
} MusicDIR_t;

void Music_Init(TCHAR* DIR);
void Music_DeInit();
void Show_WavFiles(MusicDIR_t *wavdir_Struct,TCHAR* DirPath);

void Music_Recording_Start(TCHAR *FilePath);
void Recording_Restart_Wfiles();
void Recording_Stop_Wfiles();
void Recording_Stop();
void Recording_Restart();
void Music_Recording_Save();

void Music_Play_Malloc(void);
void Music_Play_End(void);
void Music_Play_Stop(void);
void Music_Play_Restart(void);
void Music_Play_Next(TCHAR* NextMusicName);
#endif