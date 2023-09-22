#include    "mbed.h"
#include    <ctime>


// インタラプタいろいろ
DigitalIn   PhIn_FR (PA_1);
DigitalIn   PhIn_FL (PA_3);
DigitalIn   PhIn_CR (PA_4);
DigitalIn   PhIn_CL (PA_5);
DigitalIn   PhIn_BR (PA_6);
DigitalIn   PhIn_BL (PA_7);

// 各サーボ
PwmOut      fr_lock (PB_4);
PwmOut      front   (PB_5);
PwmOut      center  (PA_11);
PwmOut      back    (PB_7);
PwmOut      bc_lock (PA_12);

// 送電停止
DigitalOut sig(D2);


void reader(void);

// 0:前右 1:前左 2:中右 3:中左 4:後右 5:後左
bool InterrupterData[6] = {};

// 0:前 1:中 2:後
bool  passcount[6] = {};   

// サーボモーターええ感じの角度
const int   TOUCH   = 850;
const int   LOSS    = 1700;
const int   LOCK    = 1500;
const int   FREE    = 1000;

int main(){

    // 全閉じ&通電
    sig.write(0);
    fr_lock     .pulsewidth_us(LOCK);
    front       .pulsewidth_us(TOUCH);
    center      .pulsewidth_us(TOUCH);
    back        .pulsewidth_us(TOUCH);
    bc_lock     .pulsewidth_us(LOCK);

    // 起動後少ししたら通電開始
    ThisThread::sleep_for(3s);
    sig.write(1);

    while (true) {
        reader();

        // 通過チェック
        for(int i = 0; i < 6; i++){
            if(!InterrupterData[i]) passcount[i] = true;
        }
        if(passcount[0] || passcount[1]){   // 前のセンサどっちかで読んだとき
            sig     .write(0); // 送電停止
            fr_lock .pulsewidth_us(FREE);
            front   .pulsewidth_us(LOSS);
        }
        if(passcount[2] && passcount[3]){   // 中のセンサどっちかで読んだとき
            if(passcount[0] && passcount[1]){
                fr_lock .pulsewidth_us(LOCK);
                front   .pulsewidth_us(TOUCH);
                ThisThread::sleep_for(300ms);    // fr_lockが閉じ切らないうちにfrontが開くのを防ぐ
                center  .pulsewidth_us(LOSS);
            }
        }
        if(passcount[4] && passcount[5]){   // 後ろのセンサどっちかで読んだとき
            if(passcount[2] && passcount[3]){
                center  .pulsewidth_us(TOUCH);
                ThisThread::sleep_for(300ms);   // frontが閉じ切らないうちにcenterが開くのを防ぐ
                back    .pulsewidth_us(LOSS);
                bc_lock .pulsewidth_us(FREE);
            }
        }

        // Timer tm;   // 後ろが開いてから閉じるまでの時間を測る

        // while(passcount[4] && passcount[5]){
        //     if(tm.elapsed_time() >= 5s){
        //         back    .pulsewidth_us(TOUCH);
        //         bc_lock .pulsewidth_us(LOCK);
        //         sig.write(1);
        //         for(int i = 0; i < 6; i++)  passcount[i] = false;
        //     }
        // }

    }
    return 0;
}

void reader(void){
    InterrupterData[0] = PhIn_FR.read();
    InterrupterData[1] = PhIn_FL.read();
    InterrupterData[2] = PhIn_CR.read();
    InterrupterData[3] = PhIn_CL.read();
    InterrupterData[4] = PhIn_BR.read();
    InterrupterData[5] = PhIn_BL.read();
	for(int i = 0; i < 6; i++){
		printf("%d ", InterrupterData[i]);
	}
	printf("\n");
}