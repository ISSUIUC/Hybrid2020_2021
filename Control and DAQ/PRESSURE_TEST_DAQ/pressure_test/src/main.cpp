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

#define PRINT_USB 1
#define PRINT_TLM 0

#define READ_INTERVAL_US    1000
#define READ_TICKS          (chTimeUS2I(READ_INTERVAL_US))

#define FIFO_SIZE           10000
#define ADC_RESOLUTION      13

// static uint32_t counter = 0;
static uint8_t exit_msg = 0;

SEMAPHORE_DECL(fifoData, 0);
SEMAPHORE_DECL(fifoSpace, FIFO_SIZE);

typedef struct {
    uint32_t usec;
    uint16_t value1;
    uint16_t value2;
    uint16_t value3;
    uint16_t errors;
} FifoItem_t;

static FifoItem_t fifoArray[FIFO_SIZE];

/* ------------------------------  UART ISR  -------------------------------- */
/* -------------------------------------------------------------------------- */
// Doesnt work yet :(

void uart0_irqhandler(void) {

    Serial.println("woo irqhandler triggered.");
    digitalWrite(LED_BUILTIN, HIGH);
    delay(500);
    digitalWrite(LED_BUILTIN, LOW);

    // if (UART0_S1 & (UART_S1_RDRF | UART_S1_IDLE)) {
    //     Serial.println("woo irqhandler triggered.");
    //     digitalWrite(LED_BUILTIN, HIGH);
    //     delay(100);
    //     digitalWrite(LED_BUILTIN, LOW);
    // }

}

/* -----------------------------  ADC THREAD  ------------------------------- */
/* -------------------------------------------------------------------------- */

// Declare a stack with 32 bytes beyond task switch and interrupt needs.
static THD_WORKING_AREA(adc_thread_wa, 32);

static THD_FUNCTION(adc_thread, arg) {

    uint16_t fifoHead = 0;  // Index of head data point
    uint16_t errors = 0;    // Number of missed data points

    systime_t log_time = chVTGetSystemTime();   // Get current system time

    while (true) {

        // Sleep until next read time is reached
        log_time = chTimeAddX(log_time, READ_TICKS);
        chThdSleepUntil(log_time);

        if (chSemWaitTimeout(&fifoSpace, TIME_IMMEDIATE) != MSG_OK) {
            errors++;
            continue;
        }

        FifoItem_t* p = &fifoArray[fifoHead];
        p->usec = micros();
        p->value1 = analogRead(A7);
        p->value2 = analogRead(A6);
        p->value3 = analogRead(A5);
        p->errors = errors;
        errors = 0;

        fifoHead = fifoHead < (FIFO_SIZE - 1) ? fifoHead + 1 : 0;

        chSemSignal(&fifoData);
    }
}

/* ----------------------------  MAIN THREAD  ------------------------------- */
/* -------------------------------------------------------------------------- */

int main(void) {

    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(A5, INPUT);
    pinMode(A6, INPUT);
    pinMode(A7, INPUT);

    Serial.begin(9600);
    Serial1.begin(57600);

    // while (!Serial) {}
    // while (!Serial1) {}

    digitalWrite(LED_BUILTIN, HIGH);
    delay(100);
    digitalWrite(LED_BUILTIN, LOW);

    Serial.println("hello there");
    Serial1.println("hello telemetry!");

    attachInterruptVector(IRQ_UART1_STATUS, uart0_irqhandler);
    NVIC_SET_PRIORITY(IRQ_UART1_STATUS, 1);
    NVIC_ENABLE_IRQ(IRQ_UART1_STATUS);

    Serial.println("IRQ Handler set!");
    Serial1.println("telemetry IRQ handler set!");

    analogReadResolution(ADC_RESOLUTION);

    Serial.printf("Size of FifoItem_t is: %d bytes", sizeof(FifoItem_t));
    Serial.println();

    uint16_t fifoTail = 0;
    uint16_t last = 0;

    // Open file.
    File file;
    if (!SD.begin(BUILTIN_SDCARD)) {
        Serial.println(F("SD begin failed."));
        while (true) {}
    }

    file = SD.open("dataFile.CSV", O_CREAT | O_WRITE | O_TRUNC);
    if (!file) {
        Serial.println(F("file open  failed."));
        while (true) {}
    }

    chThdCreateStatic(adc_thread_wa, sizeof(adc_thread_wa), NORMALPRIO+1,
                      adc_thread, NULL);

    chSysInit();

    while (true) {

        chSemWait(&fifoData);

        FifoItem_t* p = &fifoArray[fifoTail];
        file.print(p->usec);
        file.write(',');
        file.print(p->value1);
        file.write(',');
        file.print(p->value2);
        file.write(',');
        file.print(p->value3);

        chSemSignal(&fifoSpace);

        fifoTail = fifoTail < (FIFO_SIZE - 1) ? fifoTail + 1 : 0;

    }

    file.close();

    return 0;

 }
