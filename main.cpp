#include    "mbed.h"
#include    <ctime>

#define     CLOSE   1167
#define     OPEN    1833

// インタラプタいろいろ
DigitalIn   PhIn_FR (A0);
DigitalIn   PhIn_FL (A1);
DigitalIn   PhIn_CR (A2);
DigitalIn   PhIn_CL (A3);
DigitalIn   PhIn_BR (A4);
DigitalIn   PhIn_BL (A5);

PwmOut      fr_lock (D1);
PwmOut      front   (D2);
PwmOut      center  (D3);
PwmOut      back    (D4);
PwmOut      bc_lock (D5);


void reader(void);

// 0:前右 1:前左 2:中右 3:中左 4:後右 5:後左
bool InterrupterData[6] = {};

// 0:前 1:中 2:後
int  passcount[3] = {};   


int main(){
    while (true) {
        reader();

        // 通過チェック
        for(int i = 0; i < 3; i += 2){
            if(InterrupterData[i])      passcount[i] += 1;
            if(InterrupterData[i+1])    passcount[i] += 1;
        }
        if(InterrupterData[0] || InterrupterData[1]){   // 前のセンサどっちかで読んだとき
            fr_lock .pulsewidth_us(OPEN);
            front   .pulsewidth_us(OPEN);
        }
        if(InterrupterData[2] || InterrupterData[3]){   // 中のセンサどっちかで読んだとき
            if(passcount[0] >= 2){
                fr_lock .pulsewidth_us(CLOSE);
                front   .pulsewidth_us(CLOSE);
                ThisThread::sleep_for(500ms);    // fr_lockが閉じ切らないうちにfrontが開くのを防ぐ
                center  .pulsewidth_us(OPEN);
            }
        }
        if(InterrupterData[4] || InterrupterData[5]){   // 後ろのセンサどっちかで読んだとき
            if(passcount[1] >= 2){
                center  .pulsewidth_us(CLOSE);
                ThisThread::sleep_for(500ms);   // frontが閉じ切らないうちにcenterが開くのを防ぐ
                back    .pulsewidth_us(OPEN);
                bc_lock .pulsewidth_us(OPEN);
            }
        }

        Timer tm;   // 後ろが開いてから閉じるまでの時間を測る

        if(passcount[2] >= 2){
            if(tm.elapsed_time() >= 5s){
                back    .pulsewidth_us(CLOSE);
                bc_lock .pulsewidth_us(CLOSE);
                for(int i = 0; i < 3; i++)  passcount[i] = 0;
            }
        }

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
}