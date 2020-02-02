#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "zlib.h"

z_stream defstream, infstream;

void init();
void cleanup();

int main() {

    init();
    atexit(cleanup);
    
    char a[50] = "hellohellohellohellohello";
    char b[50], c[50];

    printf("Uncompressed: %s, length: %lu\n", a, strlen(a));

    defstream.avail_in = (uInt)strlen(a)+1;
    defstream.next_in = (Bytef *)a;
    defstream.avail_out = (uInt)sizeof(b);
    defstream.next_out = (Bytef *)b;

    deflate(&defstream, Z_SYNC_FLUSH);

    printf("Compressed: %s, length: %lu\n", b, strlen(b));

    infstream.avail_in = (uInt)((char*)defstream.next_out - b);
    infstream.next_in = (Bytef *)b;
    infstream.avail_out = (uInt)sizeof(c);
    infstream.next_out = (Bytef *)c;

    inflate(&infstream, Z_SYNC_FLUSH);

    printf("Decompressed: %s, length: %lu\n", c, strlen(c));

    return 0;
}

void init() {
    defstream.zalloc = Z_NULL;
    defstream.zfree = Z_NULL;
    defstream.opaque = Z_NULL;

    deflateInit(&defstream, Z_DEFAULT_COMPRESSION);

    infstream.zalloc = Z_NULL;
    infstream.zfree = Z_NULL;
    infstream.opaque = Z_NULL;

    inflateInit(&infstream);
}

void cleanup() {
    deflateEnd(&defstream);
    inflateEnd(&infstream);
}