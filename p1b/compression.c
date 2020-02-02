#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "zlib.h"

z_stream def_stream, inf_stream;

int def(char *src, char *dest, int src_size, int dest_size);
int inf(char *src, char *dest, int src_size, int dest_size);
void init();
void cleanup();

int main()
{
    init();
    atexit(cleanup);

    char a[50] = "sendingsendingsendingsendingsending";
    char b[50], c[50];
    long size = 0;

    write(STDOUT_FILENO, a, strlen(a));
    write(STDOUT_FILENO, "\n", 1);
    printf("length: %ld\n", strlen(a));

    size = def(a, b, strlen(a), 50);
    printf("\nsize: %ld\n", size);

    write(STDOUT_FILENO, b, def_stream.total_out);
    write(STDOUT_FILENO, "\n", 1);
    printf("length: %ld\n", def_stream.total_out);

    size = inf(b, c, def_stream.total_out, 50);
    printf("\nsize: %ld\n", size);

    write(STDOUT_FILENO, c, inf_stream.total_out);
    write(STDOUT_FILENO, "\n", 1);
    printf("length: %ld\n", inf_stream.total_out);

    return 0;
}

int def(char *src, char *dest, int src_size, int dest_size)
{
    def_stream.avail_in = (uInt)src_size;
    def_stream.next_in = (Bytef *)src;
    def_stream.avail_out = (uInt)dest_size;
    def_stream.next_out = (Bytef *)dest;
    do
    {
        deflate(&def_stream, Z_SYNC_FLUSH);
    } while (def_stream.avail_in > 0);
    return def_stream.total_out;
}

int inf(char *src, char *dest, int src_size, int dest_size)
{
    inf_stream.avail_in = (uInt)src_size;
    inf_stream.next_in = (Bytef *)src;
    inf_stream.avail_out = (uInt)dest_size;
    inf_stream.next_out = (Bytef *)dest;
    do
    {
        inflate(&inf_stream, Z_SYNC_FLUSH);
    } while (inf_stream.avail_in > 0);
    return inf_stream.total_out;
}

void init()
{
    def_stream.zalloc = Z_NULL;
    def_stream.zfree = Z_NULL;
    def_stream.opaque = Z_NULL;

    deflateInit(&def_stream, Z_DEFAULT_COMPRESSION);

    inf_stream.zalloc = Z_NULL;
    inf_stream.zfree = Z_NULL;
    inf_stream.opaque = Z_NULL;

    inflateInit(&inf_stream);
}

void cleanup()
{
    deflateEnd(&def_stream);
    inflateEnd(&inf_stream);
}