// NAME: Jiaping Zeng
// EMAIL: jiapingzeng@ucla.edu
// ID: 905363270

#include <time.h>
#include <math.h>
#include <poll.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <getopt.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <mraa/gpio.h>
#include <mraa/aio.h>
#include <openssl/ssl.h>

int period = 1, scale = 'F', logfd = 1, stop = 0, pressed = 0, id, port, sockfd = 1;
char host[64], time_buffer[16], ssl_buffer[256];
time_t current_time;
struct tm *timeinfo;
mraa_aio_context sensor;
SSL *ssl;

void parse_options(int argc, char **argv);
void run_command(char *command);
void initialize();
void button_pressed();
float get_temperature(uint16_t value);
void on_error(char *message);

int main(int argc, char **argv)
{
    parse_options(argc, argv);

    int i, start, bytes_read;
    uint16_t value;
    char command_buffer[128], buffer[2048];
    struct pollfd pfds[1];

    initialize();

    sprintf(ssl_buffer, "ID=%d\n", id);
    if (SSL_write(ssl, ssl_buffer, strlen(ssl_buffer)) <= 0)
        on_error("Failed to write SSL");
    dprintf(logfd, "ID=%d\n", id);

    pfds[0].fd = sockfd;
    pfds[0].events = POLLIN | POLLHUP | POLLERR;

    while (!pressed)
    {
        poll(pfds, (nfds_t)2, 0);
        if (pfds[0].revents & POLLIN)
        {
            bytes_read = SSL_read(ssl, buffer, 2048);
            if (bytes_read <= 0)
                button_pressed();
            for (i = 0, start = 0; i < bytes_read; i++)
            {
                if (buffer[i] == '\n')
                {
                    strncpy(command_buffer, buffer + start, i - start);
                    command_buffer[i - start] = '\0';
                    run_command(strtok(command_buffer, "\n"));
                    start = i;
                }
            }
        }

        if (!stop && !pressed)
        {
            time(&current_time);
            timeinfo = localtime(&current_time);
            strftime(time_buffer, 16, "%H:%M:%S", timeinfo);
            value = mraa_aio_read(sensor);

            sprintf(ssl_buffer, "%s %.1f\n", time_buffer, get_temperature(value));
            if (SSL_write(ssl, ssl_buffer, strlen(ssl_buffer)) <= 0)
                on_error("Failed to write SSL");
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
        {"id", required_argument, NULL, 'i'},
        {"host", required_argument, NULL, 'h'},
        {0, 0, 0, 0}};

    while ((c = getopt_long(argc, argv, "pslih", long_options, NULL)) != -1)
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
        case 'i':
            id = atoi(optarg);
            break;
        case 'h':
            strncpy(host, optarg, 64);
            break;
        default:
            on_error("Unable to retrieve option");
        }
    }

    if (optind < argc)
        port = atoi(argv[optind]);
    else
        on_error("Port argument not found");
}

void run_command(char *command)
{
    if (logfd > 1)
        dprintf(logfd, "%s\n", command);
    if (strncmp(command, "SCALE=F", 7) == 0)
    {
        scale = 'F';
    }
    else if (strncmp(command, "SCALE=C", 7) == 0)
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
    else if (strncmp(command, "STOP", 4) == 0)
    {
        stop = 1;
    }
    else if (strncmp(command, "START", 5) == 0)
    {
        stop = 0;
    }
    else if (strncmp(command, "LOG ", 4) == 0)
    {
        // do nothing
    }
    else if (strncmp(command, "OFF", 3) == 0)
    {
        button_pressed();
    }
}

void initialize()
{
    struct sockaddr_in addr;
    struct hostent *server;
    SSL_CTX *ctx;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        on_error("Unable to open socket");

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    server = gethostbyname(host);
    if (!server)
        on_error("Unable to get host by name");
    memcpy((char *)&addr.sin_addr.s_addr, (char *)server->h_addr, server->h_length);
    if (connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
        on_error("Connection failed");

    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();

    ctx = SSL_CTX_new(TLSv1_client_method());
    if (!ctx)
        on_error("Unable to create context");
    ssl = SSL_new(ctx);
    if (!ssl)
        on_error("Unable to create SSL structure");
    if (SSL_set_fd(ssl, sockfd) != 1)
        on_error("Unable to set SSL file descriptor");
    if (SSL_connect(ssl) != 1)
        on_error("Unable to connect to SSL");

    sensor = mraa_aio_init(1);
}

void button_pressed()
{
    time(&current_time);
    timeinfo = localtime(&current_time);
    strftime(time_buffer, 16, "%H:%M:%S", timeinfo);

    if (logfd > 1)
        dprintf(logfd, "%s SHUTDOWN\n", time_buffer);

    pressed = 1;

    SSL_shutdown(ssl);
    SSL_free(ssl);

    mraa_aio_close(sensor);
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

void on_error(char *message)
{
    fprintf(stderr, "%s\n", message);
    exit(2);
}
