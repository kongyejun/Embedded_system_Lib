#include "bsp.h"
#include "sleep.h"

uint8_t pubilc_value = 0;
spinlock_t pubilc_value_lock;
/*  实验结论
1.其中一个核心结束会导致其将外设释放掉，导致另一个核心无法使用外设
2.锁有很多种，一种当多个核心或者线程竞争同一个锁时，没抢到的会被阻塞。根据锁的性质执行下一步
*/

int core1_main(void* ctx){
    int cont = 5;
    uint8_t coreid = current_coreid();
    printf("core %d sys: hello!!\n",coreid);
    while(cont--){
        spinlock_lock(&pubilc_value_lock);
        printf("core1 public_value is %d\n",pubilc_value);
        spinlock_unlock(&pubilc_value_lock);
        msleep(2001);
    }
    uint64_t use_time = read_cycle();
    printf("core1 uesed %ld time\n",use_time);
    return 0;
}


int main(void){
    int cont=5;
    uint8_t coreid = current_coreid();
    printf("current core is %d\n",coreid);
    register_core1(core1_main,NULL);
    while(cont--){
        spinlock_lock(&pubilc_value_lock);
        printf("core0 public_value is %d\n",pubilc_value);
        spinlock_unlock(&pubilc_value_lock);
        msleep(1000);
    }
    uint64_t use_time = read_cycle();
    printf("core0 uesed %ld time\n",use_time);
    printf("current free heap is %ld\n",get_free_heap_size());
    return 0;
}