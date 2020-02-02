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

    char *a = "h";
    char b[50], c[50];
    unsigned long size = 0;

    printf("Uncompressed: %s, length: %lu\n", a, strlen(a));

    //size = def(a, b, 50, 50);

    def_stream.avail_in = (uInt)strlen(a) + 1;
    def_stream.next_in = (Bytef *)a;
    def_stream.avail_out = (uInt)sizeof(b);
    def_stream.next_out = (Bytef *)b;
    deflate(&def_stream, Z_SYNC_FLUSH);
    do
    {
        deflate(&def_stream, Z_SYNC_FLUSH);
    } while (def_stream.avail_in > 0);

    printf("Compressed: %s, length: %lu\n", b, strlen(b));

    //size = inf(b, c, 50, 50);

    inf_stream.avail_in = (uInt)((char *)def_stream.next_out - b);
    inf_stream.next_in = (Bytef *)b;
    inf_stream.avail_out = (uInt)sizeof(c);
    inf_stream.next_out = (Bytef *)c;
    do
    {
        inflate(&inf_stream, Z_SYNC_FLUSH);
    } while (inf_stream.avail_in > 0);

    printf("Decompressed: %s, length: %lu\n", c, strlen(c));

    return 0;
}

int def(char *src, char *dest, int src_size, int dest_size)
{
    def_stream.avail_in = (uInt)src_size;
    def_stream.next_in = (Bytef *)dest;
    def_stream.avail_out = (uInt)dest_size;
    def_stream.next_out = (Bytef *)src;

    do
    {
        deflate(&def_stream, Z_SYNC_FLUSH);
    } while (def_stream.avail_in > 0);

    return dest_size - def_stream.avail_out;
}

int inf(char *src, char *dest, int src_size, int dest_size)
{
    inf_stream.avail_in = (uInt)src_size;
    inf_stream.next_in = (Bytef *)dest;
    inf_stream.avail_out = (uInt)dest_size;
    inf_stream.next_out = (Bytef *)src;
    inflate(&inf_stream, Z_SYNC_FLUSH);

    return dest_size - inf_stream.avail_out;
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