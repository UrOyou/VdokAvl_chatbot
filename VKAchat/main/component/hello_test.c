//测试模块化
#include "stdio.h"
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "hello_test.h"
// #include "portmacro.h"

// #define portTICK_PERIOD_MS              ( ( TickType_t ) 1000 / configTICK_RATE_HZ )


void hello_test(void){

    printf("this is a hello for test");

    for (int i = 10; i >= 0; i--) {
        printf("Restarting in %d seconds...\n", i);
        vTaskDelay(1000);
    }
    printf("Restarting now.\n");
    fflush(stdout);
    esp_restart();
}