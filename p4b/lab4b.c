#include <time.h>
#include <math.h>
#include <poll.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <mraa/gpio.h>
#include <mraa/aio.h>

int period = 1, scale = 'F', logfd = 1, stop = 0;
char time_buffer[16];
time_t current_time;
struct tm *timeinfo;
mraa_aio_context sensor;
mraa_gpio_context button;

void parse_options(int argc, char **argv);
void process_command(char *command);
void button_pressed();
float get_temperature(uint16_t value);

int main(int argc, char **argv)
{
    parse_options(argc, argv);

    int i, start, bytes_read;
    uint16_t value;
    char command_buffer[32], buffer[256];
    struct pollfd pfds[1];

    pfds[0].fd = STDIN_FILENO;
    pfds[0].events = POLLIN | POLLHUP | POLLERR;

    sensor = mraa_aio_init(1);
    button = mraa_gpio_init(60);

    mraa_gpio_dir(button, MRAA_GPIO_IN);
    mraa_gpio_isr(button, MRAA_GPIO_EDGE_RISING, &button_pressed, NULL);

    while (1)
    {
        poll(pfds, (nfds_t)2, 0);
        if (pfds[0].revents & POLLIN)
        {
            bytes_read = read(pfds[0].fd, buffer, 256);
            if (!bytes_read)
            {
                printf("Poll closed\n");
                exit(0);
            }
            for (i = 0, start = 0; i < bytes_read && start < bytes_read; i++)
            {
                if (buffer[i] == '\n')
                {
                    strncpy(command_buffer, buffer, i - start);
                    command_buffer[start] = '\0';
                    process_command(command_buffer);
                    start = i;
                    if (logfd > 1)
                        write(logfd, command_buffer, i - start);
                }
            }
        }

        if (!stop)
        {
            time(&current_time);
            timeinfo = localtime(&current_time);
            strftime(time_buffer, 16, "%H:%M:%S", timeinfo);
            value = mraa_aio_read(sensor);

            printf("%s %.1f\n", time_buffer, get_temperature(value));
            if (logfd > 1)
                dprintf(logfd, "%s %.1f\n", time_buffer, get_temperature(value));
            sleep(period);
        }
    }

    return 0;
}

void parse_options(int argc, char **argv)
{
    int c;
    static struct option long_options[] = {
        {"period", required_argument, NULL, 'p'},
        {"scale", required_argument, NULL, 's'},
        {"log", required_argument, NULL, 'l'},
        {0, 0, 0, 0}};

    while ((c = getopt_long(argc, argv, "psl", long_options, NULL)) != -1)
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
        case 'l':
            logfd = open(optarg, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
            if (logfd < 0)
            {
                fprintf(stderr, "Unable to open/create output file '%s': %s\n", optarg, strerror(errno));
                exit(1);
            }
            break;
        default:
            fprintf(stderr, "Unable to retrieve option\n");
            exit(1);
        }
    }
}

void process_command(char *command)
{
    if (strcmp(command, "SCALE=F") == 0)
    {
        scale = 'F';
    }
    else if (strcmp(command, "SCALE=C\n") == 0)
    {
        scale = 'C';
    }
    else if (strncmp(command, "PERIOD=", 7) == 0)
    {
        char str[8];
        strncpy(str, command + 7 * sizeof(char), 7);
        str[7] = '\0';
        period = atoi(str);
    }
    else if (strcmp(command, "STOP\n") == 0)
    {
        stop = 1;
    }
    else if (strcmp(command, "START\n") == 0)
    {
        stop = 0;
    }
    else if (strncmp(command, "LOG ", 4) == 0)
    {
        // do nothing
    }
    else if (strcmp(command, "OFF\n") == 0)
    {
        button_pressed();
    }
}

void button_pressed()
{
    time(&current_time);
    timeinfo = localtime(&current_time);
    strftime(time_buffer, 16, "%H:%M:%S", timeinfo);

    printf("%s SHUTDOWN\n", time_buffer);

    mraa_aio_close(sensor);
    mraa_gpio_close(button);

    exit(0);
}

float get_temperature(uint16_t value)
{
    float temperature;
    const int B = 4275, R0 = 100000;
    float R = 1023.0 / value - 1.0;
    R = R0 * R;
    temperature = 1.0 / (log(R / R0) / B + 1 / 298.15) - 273.15;
    if (scale == 'F')
        temperature = temperature * 1.8 + 32.0;
    return temperature;
}
