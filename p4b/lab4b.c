#include <time.h>
#include <math.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <getopt.h>
#include <mraa/gpio.h>
#include <mraa/aio.h>

int period = 1, scale = 'F';
char buffer[16];
time_t current_time;
struct tm *timeinfo;
mraa_aio_context sensor;
mraa_gpio_context button;

void parse_options(int argc, char **argv);
void button_pressed();
float get_temperature(uint16_t value);

int main(int argc, char **argv)
{
    parse_options(argc, argv);

    printf("period: %d, scale: %c\n", period, scale);

    uint16_t value;

    sensor = mraa_aio_init(1);
    button = mraa_gpio_init(60);

    mraa_gpio_dir(button, MRAA_GPIO_IN);
    mraa_gpio_isr(button, MRAA_GPIO_EDGE_RISING, &button_pressed, NULL);

    while (1)
    {
        time(&current_time);
        timeinfo = localtime(&current_time);
        strftime(buffer, 16, "%H:%M:%S", timeinfo);
        value = mraa_aio_read(sensor);

        printf("%s %f\n", buffer, get_temperature(value));
        sleep(period);
    }

    return 0;
}

void parse_options(int argc, char **argv)
{
    int c;
    static struct option long_options[] = {
        {"period", required_argument, NULL, 'p'},
        {"scale", required_argument, NULL, 's'},
        {0, 0, 0, 0}};

    while ((c = getopt_long(argc, argv, "ps", long_options, NULL)) != -1)
    {
        switch (c)
        {
        case 'p':
            period = atoi(optarg);
            break;
        case 's':
            if (strcmp(optarg, "F") == 0 || strcmp(optarg, "f") == 0)
                scale = 'F';
            else if (strcmp(optarg, "C") == 0 || strcmp(optarg, "c") == 0)
                scale = 'C';
            else
            {
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

void button_pressed()
{
    time(&current_time);
    timeinfo = localtime(&current_time);
    strftime(buffer, 16, "%H:%M:%S", timeinfo);

    printf("%s SHUTDOWN\n", buffer);

    mraa_aio_close(sensor);
    mraa_gpio_close(button);

    exit(0);
}

float get_temperature(uint16_t value)
{
    float temperature;
    const int B = 4275, R0 = 100000;
    float R = 1023.0/value-1.0;
    R = R0*R;
    temperature = 1.0/(log(R/R0)/B+1/298.15)-273.15;
    if (scale == 'F')
        temperature = temperature * 1.8 + 32.0;
    return temperature;
}
