/*
 * Hybrid engine high pressure N2 test data acquisition system
 *
 * Ayberk Yaraneri Nov 10 2020
 *
 *
 * Teensy hardware configs found here:
 * https://github.com/PaulStoffregen/cores/tree/master/teensy4
 */

#include <SD.h>
#include <ChRt.h>

#include "pins.h"
#include "dataLog.h"

#define PRINT_USB 1
#define PRINT_TLM 0

#define READ_INTERVAL_US    1000
// #define READ_TICKS          (chTimeUS2I(READ_INTERVAL_US))
#define READ_TICKS          TIME_US2I(READ_INTERVAL_US)

#define FIFO_SIZE           10000
#define ADC_RESOLUTION      13

File dataFile;

// static uint32_t counter = 0;
static uint8_t exit_msg = 0;

SEMAPHORE_DECL(fifoData, 0);
SEMAPHORE_DECL(fifoSpace, FIFO_SIZE);

static FifoItem_t fifoArray[FIFO_SIZE];

enum DAQ_State {
    IDLE,
    RECORDING,
    VENTING
};

DAQ_State DAQState;

/* -----------------------------  UART0 ISR  -------------------------------- */
/* -------------------------------------------------------------------------- */

// original handler to be called before anything else
void (*orig_uart0_irqhandler)(void);

// void custom_uart0_irqhandler(void) {
CH_FAST_IRQ_HANDLER(custom_uart0_irqhandler) {
    // CH_IRQ_PROLOGUE();

    // Original handler must be called first as it handles RX/TX buffering
    orig_uart0_irqhandler();

    if (Serial1.available()) {

        uint8_t cmd = Serial1.read();

        switch (cmd) {
            case 0x61:
                DAQState = IDLE;
                break;
            case 0x62:
                DAQState = RECORDING;
                break;
            case 0x63:
                DAQState = VENTING;
                digitalWrite(HYBRID_VENT_PIN, HIGH);
                digitalWrite(LED_BUILTIN, HIGH);
                break;
        }

    }
    // CH_IRQ_EPILOGUE();
}

/* -----------------------------  ADC THREAD  ------------------------------- */
/* -------------------------------------------------------------------------- */

// Declare a stack with 64 bytes beyond task switch and interrupt needs.
static THD_WORKING_AREA(adc_thread_wa, 64);

static THD_FUNCTION(adc_thread, arg) {

    uint16_t fifoHead = 0;  // Index of head data point
    uint16_t errors = 0;    // Number of missed data points

    systime_t log_time = chVTGetSystemTime();   // Get current system time

    uint16_t count = 0;

    FifoItem_t data;

    while (true) {

        data.usec = micros();
        data.value1 = analogRead(HYBRID_PT_1_PIN);
        data.value2 = analogRead(HYBRID_PT_2_PIN);
        data.value3 = analogRead(HYBRID_PT_3_PIN);
        data.errors = errors;

        // Display real time dataressure at ~ 30.03 Hz
        if(count >= 50){
            count = 0;
            Serial1.print(data.usec);
            Serial1.print("\t");
            Serial1.print(data.value1);
            Serial1.print("\t");
            Serial1.print(data.value2);
            Serial1.print("\t");
            Serial1.print(data.value3);
            Serial1.print("\t");
            Serial1.println(data.errors);
        }
        ++count;

        // Sleep until next read time is reached
        log_time = chTimeAddX(log_time, READ_TICKS);
        chThdSleepUntil(log_time);

        if (DAQState == RECORDING) {

            if (chSemWaitTimeout(&fifoSpace, TIME_IMMEDIATE) != MSG_OK) {
                errors++;
                continue;
            }

            fifoArray[fifoHead] = data;
            errors = 0;

            fifoHead = fifoHead < (FIFO_SIZE - 1) ? fifoHead + 1 : 0;

            chSemSignal(&fifoData);
        }
    }
}

/* ----------------------------  MAIN THREAD  ------------------------------- */
/* -------------------------------------------------------------------------- */

void mainThread() {

    uint16_t fifoTail = 0;
    uint16_t last = 0;

    // Create ADC producer thread
    chThdCreateStatic(adc_thread_wa, sizeof(adc_thread_wa), NORMALPRIO+1,
                      adc_thread, NULL);

    while (true) {

        chSemWait(&fifoData);
        // if (chSemWaitTimeout(&fifoData, TIME_IMMEDIATE) != MSG_OK) {
        //     continue;
        // }

        FifoItem_t* data = &fifoArray[fifoTail];

        logData(&dataFile, data);

        chSemSignal(&fifoSpace);

        fifoTail = fifoTail < (FIFO_SIZE - 1) ? fifoTail + 1 : 0;

    }

    dataFile.close();

}

/* -------------------------------  SETUP  ---------------------------------- */
/* -------------------------------------------------------------------------- */

 void setup() {

     pinMode(LED_BUILTIN, OUTPUT);
     pinMode(A5, INPUT);
     pinMode(A6, INPUT);
     pinMode(A7, INPUT);
     analogReadResolution(ADC_RESOLUTION);

     Serial.begin(9600);
     Serial1.begin(57600);

     // while (!Serial) {}
     // while (!Serial1) {}

     //SD Card Setup
     if(!SD.begin(BUILTIN_SDCARD)){
         while (true) {
             Serial.println("### SD.begin failed! ###");
             digitalWrite(LED_BUILTIN, LOW);
             delay(300);
             digitalWrite(LED_BUILTIN, HIGH);
             delay(300);
         }
     }

     init_dataLog(&dataFile);

     if (!dataFile) {
         Serial1.println(F("### File open failed! ###"));
         while (true) {
             digitalWrite(LED_BUILTIN, LOW);
             delay(300);
             digitalWrite(LED_BUILTIN, HIGH);
             delay(300);
         }
     }

     // interrupt chaining by replacing NVIC vector to point to custom handler
     orig_uart0_irqhandler = _VectorsRam[IRQ_UART0_STATUS+16];
     _VectorsRam[IRQ_UART0_STATUS+16] = custom_uart0_irqhandler;

     chBegin(mainThread);

 }

void loop() {

}
