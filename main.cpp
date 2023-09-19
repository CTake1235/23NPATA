#include    "mbed.h"
#include    <ctime>


// インタラプタいろいろ
DigitalIn   PhIn_FR (A0);
DigitalIn   PhIn_FL (A1);
DigitalIn   PhIn_CR (A2);
DigitalIn   PhIn_CL (A3);
DigitalIn   PhIn_BR (A4);
DigitalIn   PhIn_BL (A5);

// 各サーボ
PwmOut      fr_lock (D1);
PwmOut      front   (D2);
PwmOut      center  (D3);
PwmOut      back    (D4);
PwmOut      bc_lock (D5);

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
const int   LOCK    = 850;
const int   FREE    = 1700;

int main(){

    // 全閉じ&通電
    sig.write(1);
    fr_lock     .pulsewidth_us(LOCK);
    front       .pulsewidth_us(TOUCH);
    center      .pulsewidth_us(TOUCH);
    back        .pulsewidth_us(TOUCH);
    bc_lock     .pulsewidth_us(LOCK);

    while (true) {
        reader();

        // 通過チェック
        for(int i = 0; i < 6; i++){
            if(InterrupterData[i] && (passcount[i] == false)) passcount[i] = true;
        }
        if(passcount[0] || passcount[1]){   // 前のセンサどっちかで読んだとき
            sig     .write(0); // 送電停止
            fr_lock .pulsewidth_us(FREE);
            front   .pulsewidth_us(LOSS);
        }
        if(passcount[2] || passcount[3]){   // 中のセンサどっちかで読んだとき
            if(passcount[0] && passcount[1]){
                fr_lock .pulsewidth_us(LOCK);
                front   .pulsewidth_us(TOUCH);
                ThisThread::sleep_for(500ms);    // fr_lockが閉じ切らないうちにfrontが開くのを防ぐ
                center  .pulsewidth_us(LOSS);
            }
        }
        if(passcount[4] || passcount[5]){   // 後ろのセンサどっちかで読んだとき
            if(passcount[2] && passcount[3]){
                center  .pulsewidth_us(TOUCH);
                ThisThread::sleep_for(500ms);   // frontが閉じ切らないうちにcenterが開くのを防ぐ
                back    .pulsewidth_us(LOSS);
                bc_lock .pulsewidth_us(FREE);
            }
        }

        Timer tm;   // 後ろが開いてから閉じるまでの時間を測る

        while(passcount[4] && passcount[5]){
            if(tm.elapsed_time() >= 5s){
                back    .pulsewidth_us(TOUCH);
                bc_lock .pulsewidth_us(LOCK);
                sig.write(1);
                for(int i = 0; i < 6; i++)  passcount[i] = false;
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