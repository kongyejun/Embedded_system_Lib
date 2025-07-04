#include "ICM2060.h"
#include "I2C_ICM\I2C0_CTL.h"
#include "sleep.h"
#include "LOG_SYSTEM/LOG.h"
#include "math.h"

#define G_LC(L)  16.4f * (2000 / (L))  //角速度精度

#define PI 3.1415926f

typedef struct {
    float ax, ay, az;
}ICM_ACCEL;

typedef struct {
    float gx, gy, gz;
}ICM_GYRO;

static float Kp=0.85,Ki=0.005,Kd = 0;//比例系数，积分系数

volatile static ICM_ACCEL accel;  //原始加速度数据
volatile static ICM_GYRO   gyro;  //原始角速度数据
volatile static float q0 = 1,q1 = 0,q2 = 0,q3 = 0;   //四元数
static float exInt = 0,eyInt = 0,ezInt = 0; //叉积计算误差的累计积分
static float GyroOffset[3] = {0};        //陀螺仪零漂
static uint8_t val[2];

uint8_t   ICM2060_ID = 0;  //ICM2060芯片ID
volatile FLINAL_DATA ICM_EndData;   //最终欧拉角

uint16_t ICM_Write(uint8_t reg, uint8_t data){
    return I2C_Write_Byte(ICM_ADDRESS, reg, data);
}

uint16_t ICM_Read(uint8_t reg, uint8_t *data, uint8_t len){
    return I2C_Read(ICM_ADDRESS, reg, data, len);
}

/* 判断是否是icm20607芯片 */
ICM2060_STATE ICM_WHO_AM_I(){
    uint8_t val = 0,cont = 0;
    do{
        ICM_Read(WHO_AM_I, &val, 1);  // 读ICM20607的ID
        if (cont == 0 && val != ICM20607_ID){  // 当ID不对时，只报一次。
            EMLOG(LOG_ERROR,"WHO_AM_I=0x%02x\n", val);
        }else if (cont > 99){
            return ICM2060_READ_ERROR;//获取100次都不对，则认为读取失败
        }
        cont++;
    } while(ICM20607_ID != val);
    EMLOG(LOG_INFO,"WHO_AM_I=0x%02x\n", val);
    ICM2060_ID = val;//保存ID
    return ICM2060_OK;
}
//初始化ICM2060芯片(测试级)
ICM2060_STATE ICM2060_Quick_Init(void){
    uint8_t val = 0x00, res = 0;
    I2C_HardWare_Init(ICM_ADDRESS);//I2C初始化
    msleep(10);
    res = ICM_Write(ICM_PWR_MGMT_1, 0x80);//复位设备
    msleep(100);
    res = ICM_WHO_AM_I(0);//判断是否是icm20607芯片
    //如果是icm20607芯片
    if(res == ICM2060_OK){
        do{
            ICM_Read(ICM_PWR_MGMT_1, &val, 1);//等待复位成功
        }while(val != 0x41);

        res = ICM_Write(ICM_PWR_MGMT_1, 0x01);  //时钟设置
        res = ICM_Write(ICM_PWR_MGMT_2, 0x00);  //开启陀螺仪和加速度计
        res = ICM_Write(CONFIG, 0x01);          //176HZ 1KHZ
        res = ICM_Write(SMPLRT_DIV, 0x07);      //采样速率 SAMPLE_RATE = INTERNAL_SAMPLE_RATE / (1 + SMPLRT_DIV)
        res = ICM_Write(GYRO_CONFIG, 0x18);     //±2000 dps
        res = ICM_Write(ACCEL_CONFIG, 0x10);    //±8g
        res = ICM_Write(ACCEL_CONFIG_2, 0x23);  //Average 8 samples   44.8HZ

        EMLOG(LOG_INFO,"ICM2060_Init OK\n");
    }else{
        EMLOG(LOG_ERROR,"ICM2060 Get ID ERROR\n");
    }
    return res;
}

/* 读取陀螺仪X轴原始数据 */
int16_t GetRawGyroscopeX(void) {
    ICM_Read(GYRO_XOUT_H, val, 2);
    return ((int16_t)val[0] << 8) + val[1];
}
/* 读取陀螺仪Y轴原始数据 */
int16_t GetRawGyroscopeY(void) {
    ICM_Read(GYRO_YOUT_H, val, 2);
    return ((int16_t)val[0] << 8) + val[1];
}
/* 读取陀螺仪Z轴原始数据 */
int16_t GetRawGyroscopeZ(void) {
    ICM_Read(GYRO_ZOUT_H, val, 2);
    return ((int16_t)val[0] << 8) + val[1];
}
/* 读取加速度计X轴原始数据 */
int16_t GetRawAccelerationX(void) {
    ICM_Read(ACCEL_XOUT_H, val, 2);
    return ((int16_t)val[0] << 8) + val[1];
}
/* 读取加速度计Y轴原始数据 */
int16_t GetRawAccelerationY(void) {
    ICM_Read(ACCEL_YOUT_H, val, 2);
    return ((int16_t)val[0] << 8) + val[1];
}
/* 读取加速度计Z轴原始数据 */
int16_t GetRawAccelerationZ(void) {
    ICM_Read(ACCEL_ZOUT_H, val, 2);
    return ((int16_t)val[0] << 8) + val[1];
}
//读取MPU6050的温度数据，转化成摄氏度
float GetICMTemp(void){
    ICM_Read(TEMP_OUT_H, val, 2);
    return ((double) ((((int16_t)val[0] << 8) + val[1] )/340.0))+36.53;
}

//获取陀螺仪XYZ轴数据
void GetRawGyroscopeXYZ(volatile ICM_GYRO *GYRO) {
    GYRO->gx = GetRawGyroscopeX();
    GYRO->gy = GetRawGyroscopeY();
    GYRO->gz = GetRawGyroscopeZ();
}
//获取加速度计XYZ轴数据
void GetRawAccelerationXYZ(volatile ICM_ACCEL *ACCEL) {
    ACCEL->ax = GetRawAccelerationX();
    ACCEL->ay = GetRawAccelerationY();
    ACCEL->az = GetRawAccelerationZ();
}

//获取ICM的6轴原始数据
ICM2060_STATE ICM2060_GeOriginalData(void){
    GetRawGyroscopeXYZ(&gyro);
    GetRawAccelerationXYZ(&accel);
    ICM_EndData.temp = GetICMTemp();
    return ICM2060_OK;
}
/**
 * @brief 陀螺仪零漂初始化
 * 通过采集一定数据求均值计算陀螺仪零点偏移值。
 * 后续 陀螺仪读取的数据 - 零飘值，即可去除零点偏移量。
 */
void GYROFFSET_Init(void){
    ICM_GYRO GYROFFSET = {0};
    int16_t GYRO_X = 0, GYRO_Y = 0, GYRO_Z = 0;
    for (uint16_t i = 0; i < 100; ++i) {
        GetRawGyroscopeXYZ(&GYROFFSET);    // 获取陀螺仪角速度
        GYRO_X += GYROFFSET.gx;
        GYRO_Y += GYROFFSET.gy;
        GYRO_Z += GYROFFSET.gz;
        msleep(20);    // 50hz的更新频率
    }
    GyroOffset[0] = (GYRO_X / 100);
    GyroOffset[1] = (GYRO_Y / 100);
    GyroOffset[2] = (GYRO_Z / 100);
    printf("GyroOffset: %f, %f, %f\n", GyroOffset[0], GyroOffset[1], GyroOffset[2]);
} 

/**
 * @brief 将采集的数值转化为实际物理值, 并对陀螺仪进行去零漂处理
    ICM20602_GYRO_CONFIG寄存器
    设置为:0x18 陀螺仪量程为:±2000dps     获取到的陀螺仪数据除以16.4    单位：°/s
    ICM20602_ACCEL_CONFIG寄存器
    设置为:0x10 加速度计量程为:±8g        获取到的加速度计数据 除以4096  单位：g(m/s^2)
 * @tips: gyro = (gyro_val / 16.4) °/s = ((gyro_val / 16.4) * PI / 180) rad/s
 */
void ICM_GetValues(volatile ICM_ACCEL *Accel,volatile ICM_GYRO *Gyro,uint8_t LC_A, uint16_t LC_G) {
    static float alpha = 0.7; // 一阶滤波系数
    static float Ax=0, Ay=0, Az=0; // 加速度计数据旧数据
    //不可以认为 装置在背面 对Z轴取相反数据就可以,这是错误的,会干扰其他角度的计算!!!!!!
    //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    if(Ax==0){
        Ax=Accel->ax;
        Ay=Accel->ay;
        Az=Accel->az;
    }
    //一阶线性滤波
    Accel->ax = alpha * Ax + (1 - alpha) * Accel->ax;
    Accel->ay = alpha * Ay + (1 - alpha) * Accel->ay;
    Accel->az = alpha * Az + (1 - alpha) * Accel->az;
    //保存数据
    Ax=(Ax+Accel->ax)/2;
    Ay=(Ay+Accel->ay)/2;
    Az=(Az+Accel->az)/2;

    // // 陀螺仪角速度必须转换为弧度制角速率: deg/s -> rad/s
    Gyro->gx = ((Gyro->gx - GyroOffset[0])) * PI / 180.0f / G_LC(LC_G); 
    Gyro->gy = ((Gyro->gy - GyroOffset[1])) * PI / 180.0f / G_LC(LC_G);
    Gyro->gz = ((Gyro->gz - GyroOffset[2])) * PI / 180.0f / G_LC(LC_G);
}

// 快速计算倒数平方根的函数     快速计算 1/Sqrt(x)
float myRsqrt(float x){
	float halfx = 0.5f * x;
	float y = x;
	long i = *(long*)&y;
	i = 0x5f3759df - (i>>1);
	y = *(float*)&i;
	y = y * (1.5f - (halfx * y * y));
	return y;
}

/**
 * @brief 用互补滤波算法解算陀螺仪姿态(即利用加速度计修正陀螺仪的积分误差)
 * 加速度计对振动之类的噪声比较敏感，长期数据计算出的姿态可信；陀螺仪对振动噪声不敏感，短期数据可信，但长期使用积分误差严重(内部积分算法放大静态误差)。
 * 因此使用姿态互补滤波，短期相信陀螺仪，长期相信加速度计。
 * @tips: n - 导航坐标系； b - 载体坐标系
 */
ICM2060_STATE ICM_AHRSupdate(volatile ICM_ACCEL *Data,volatile ICM_GYRO *GYRO) {
    float vx, vy, vz;           // 当前姿态计算得来的重力在三轴上的分量
    float ex, ey, ez;           // 当前加速计测得的重力加速度在三轴上的分量与用当前姿态计算得来的重力在三轴上的分量的误差
    float P0,P1,P2,P3;
    //先相乘，方便后续计算
 	float q0q1 = q0*q1;
	float q0q2 = q0*q2;
	float q1q1 = q1*q1;
 	float q1q3 = q1*q3;
	float q2q2 = q2*q2;
	float q2q3 = q2*q3;
    // // 正常静止状态为-g 反作用力,所以如果正常静止状态,加速度计测得为0,表示错误
    // if(Data->ax * Data->ay * Data->ay == 0){return ICM2060_ERROR;}

    // 对加速度数据进行归一化 得到单位加速度 (a^b -> 载体坐标系下的加速度)
    float norm = myRsqrt(Data->ax * Data->ax + Data->ay * Data->ay + Data->az * Data->az); 
    Data->ax *= norm;
    Data->ay *= norm;
    Data->az *= norm;
    // 载体坐标系下重力在三个轴上的分量
    vx = 2 * (q1q3 - q0q2);
    vy = 2 * (q0q1 + q2q3);
    vz = 1 - 2*(q1q1 + q2q2);
    // g^b 与 a^b 做向量叉乘，得到陀螺仪的校正补偿向量e的系数  (机体坐标系)
    // 转换为平均角速度补偿陀螺仪
    ex = (Data->ay * vz - Data->az * vy)/(halfT*2);
    ey = (Data->az * vx - Data->ax * vz)/(halfT*2);
    ez = (Data->ax * vy - Data->ay * vx)/(halfT*2);
    //// 直接补偿陀螺仪
    // ex = (Data->ay * vz - Data->az * vy);
    // ey = (Data->az * vx - Data->ax * vz);
    // ez = (Data->ax * vy - Data->ay * vx);
    // 误差累加
    exInt += Ki * ex;  
    eyInt += Ki * ey;
    ezInt += Ki * ez;
    // 使用PID控制器消除向量积误差(陀螺仪漂移误差)
    GYRO->gx = GYRO->gx + Kp * ex +  exInt;
    GYRO->gy = GYRO->gy + Kp * ey +  eyInt;
    GYRO->gz = GYRO->gz + Kp * ez +  ezInt;

    // 一阶龙格库塔法求解四元数微分方程，其中halfT为测量周期的1/2，gx gy gz为b系陀螺仪角速度
    // 更新四元数
    P0 = q0 + (-q1 * GYRO->gx - q2 * GYRO->gy - q3 * GYRO->gz) * halfT;
    P1 = q1 + ( q0 * GYRO->gx + q2 * GYRO->gz - q3 * GYRO->gy) * halfT;
    P2 = q2 + ( q0 * GYRO->gy - q1 * GYRO->gz + q3 * GYRO->gx) * halfT;
    P3 = q3 + ( q0 * GYRO->gz + q1 * GYRO->gy - q2 * GYRO->gx) * halfT;

    // 单位化四元数在空间旋转时不会拉伸，仅有旋转角度，下面算法类似线性代数里的正交变换
    norm = myRsqrt(P0*P0 + P1*P1 + P2*P2 + P3*P3);
    q0 = P0 * norm;
    q1 = P1 * norm;
    q2 = P2 * norm;
    q3 = P3 * norm;  //用全局变量记录上一次计算的四元数值
    //printf("%f , %f , %f , %f\n", q0, q1, q2, q3);
    return ICM2060_OK;
}

// 四元数转换为欧拉角的函数
void Quaternion_To_EulerAngles() {
    // atan2返回输入坐标点与坐标原点连线与X轴正方形夹角的弧度值
    ICM_EndData.pitch = asinf(2 * q0 * q2 - 2 * q1 * q3) * 180.0 / PI; 
    ICM_EndData.roll = atan2f(2 * q2 * q3 + 2 * q0 * q1, -2*q1*q1 - 2*q2*q2 + 1) * 180.0 / PI; 
    ICM_EndData.yaw = 0;//由于以上互补滤波并没有对偏航角（Y）进行补偿，所以应当使用其他方法来计算偏航角（Y） 
}

/**
 * @brief 初始化ICM2060芯片(完整级)
 * @param None
 * @return 0:成功 其他:失败
 */
ICM2060_STATE ICM2060_FULL_Init(){
    uint8_t val = 0x00, res = 0;
    I2C_HardWare_Init(ICM_ADDRESS);//I2C初始化
    msleep(10);
    res = ICM_Write(ICM_PWR_MGMT_1, 0x80);//复位设备
    msleep(100);
    res = ICM_WHO_AM_I();//判断是否是icm20607芯片
    if(res == ICM2060_OK){
        do{
            ICM_Read(ICM_PWR_MGMT_1, &val, 1);//等待复位成功
        }while(val != 0x41);
        res = ICM_Write(ICM_PWR_MGMT_1, 0x01);  //时钟设置
        res = ICM_Write(ICM_PWR_MGMT_2, 0x00);  //开启陀螺仪和加速度计
        res = ICM_Write(CONFIG, 0x01);          //176HZ 1KHZ
        res = ICM_Write(SMPLRT_DIV, 0x07);      //采样速率 SAMPLE_RATE = INTERNAL_SAMPLE_RATE / (1 + SMPLRT_DIV)
        res = ICM_Write(GYRO_CONFIG, 0x18);     //+—2000 dps
        res = ICM_Write(ACCEL_CONFIG, 0x08);    //±8g
        res = ICM_Write(ACCEL_CONFIG_2, 0x23);  //Average 8 samples   420HZ
        // res = ICM_Write(INT_ENABLE,0x00);       //关闭所有中断
        // res = ICM_Write(INT_PIN_CFG,0x80);      //INT引脚低电平有效
        GYROFFSET_Init();    // 陀螺仪零漂计算
        //初始化数据存储空间
        q0 = 1, q1 = 0, q2 = 0, q3 = 0;
        exInt = 0, eyInt = 0, ezInt = 0;
        EMLOG(LOG_INFO,"ICM2060_Init OK\n");
    }else{
        EMLOG(LOG_ERROR,"ICM2060 Get ID ERROR\n");
    }
    return res;
}

// 获取ICM2060原始数据
void Printf_ICM2060_OriginalData(){
    //说明还未获得原始数据,则获取一次原始数据
    ICM2060_GeOriginalData();
    ICM_GetValues(&accel, &gyro , 8, 2000);
    printf("Ax    = %3.2f   g(m/s^2)\n",accel.ax);
    printf("Ay    = %3.2f   g(m/s^2)\n",accel.ay);
    printf("Az    = %3.2f   g(m/s^2)\n",accel.az);
    printf("Gx    = %3.2f\xA1\xE3/s\n",gyro.gx);
    printf("Gy    = %3.2f\xA1\xE3/s\n",gyro.gy);
    printf("Gz    = %3.2f\xA1\xE3/s\n",gyro.gz);
}

// void Printf_ICM2060_MeanData(){
//     Mean_ACCGYRO(&accel,&gyro,2,250);
//     printf("pitch   = %5.2f\xA1\xE3\n",ICM_EndData.pitch );
//     printf("roll    = %5.2f\xA1\xE3\n",ICM_EndData.roll );
// }

void ICM2060_PosSlove(){
    ICM2060_GeOriginalData();
    ICM_GetValues(&accel, &gyro , 4, 2000);//2000bps才可行,瞬时数据有可能大于1000bps
    ICM_AHRSupdate(&accel, &gyro);
    Quaternion_To_EulerAngles();
}

int16_t convertAngle(int16_t angle) {
    if (angle >= 90) {
        return 180 - angle;  // 将大于90度的角度转换
    } else if (angle <= -90) {
        return -180 - angle; // 将小于-90度的角度转换
    }
    return angle; // 在-90到90之间的角度不需要转换
}

//打印ICM2060姿态解算后的数据
void Printf_ICM2060_EndData(float* kp,float* ki,float* kd){
    Kp=*kp;Ki=*ki,Kd=*kd;
    ICM2060_PosSlove();
    ICM_EndData.pitch = convertAngle((int16_t)ICM_EndData.pitch);
    ICM_EndData.roll  = convertAngle((int16_t)ICM_EndData.roll);
    // printf("pitch   = %5.2f\xA1\xE3         ",ICM_EndData.pitch );
    // printf("roll    = %5.2f\xA1\xE3\n\n",ICM_EndData.roll );
    // printf("yaw     = %5.2f\xA1\xE3\n",ICM_EndData.yaw );
    // printf("temp    = %5.2f\xA1\xE3\x43\n",ICM_EndData.temp );
}
