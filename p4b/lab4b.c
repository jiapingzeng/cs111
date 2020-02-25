#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <getopt.h>
#include <mraa/gpio.h>
#include <mraa/aio.h>

sig_atomic_t volatile run_flag = 1;
int period = 1, scale = 'F';

void parse_options(int argc, char **argv);
void button_pressed();

int main(int argc, char **argv) {

    parse_options(argc, argv);

    printf("period: %d, scale: %c\n", period, scale);

    uint16_t value;
    char buffer[16];
    time_t current_time;
    struct tm *timeinfo;
    mraa_aio_context sensor;
    mraa_gpio_context button;

    sensor = mraa_aio_init(1);
    button = mraa_gpio_init(60);

    mraa_gpio_dir(button, MRAA_GPIO_IN);

    mraa_gpio_isr(button, MRAA_GPIO_EDGE_RISING, &button_pressed, NULL); 

    while (run_flag) {
        time(&current_time);
        timeinfo = localtime(&current_time);
        strftime(buffer, 16, "%H:%M:%S", timeinfo);
        value = mraa_aio_read(sensor);
        
        printf("%s %d\n", buffer, value);
        sleep(period);
    }

    mraa_aio_close(sensor);
    mraa_gpio_close(button);

    return 0;
}

void parse_options(int argc, char **argv) {
    int c;
    static struct option long_options[] = {
        {"period", required_argument, NULL, 'p'},
        {"scale", required_argument, NULL, 's'},
        {0, 0, 0, 0}
    };

    while ((c = getopt_long(argc, argv, "ps", long_options, NULL)) != -1) {
        switch (c) {
            case 'p':
                period = atoi(optarg);
                break;
            case 's':
                if (strcmp(optarg, "F") == 0 || strcmp(optarg, "f") == 0)
                    scale = 'F';
                else if (strcmp(optarg, "C") == 0 || strcmp(optarg, "c") == 0)
                    scale = 'C';
                else {
                    fprintf(stderr, "Invalid scale option: %s\n", optarg);
                    exit(1);
                }
                break;
            default:
                fprintf(stderr, "Unable to retrieve option\n");
                exit(1);
        }
    }
}

void button_pressed() {
    printf("button pressed\n");
    run_flag = 0;
}
