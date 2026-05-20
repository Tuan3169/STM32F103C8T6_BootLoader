/*****************************************************************************************************************
 * @Filename: HashBinaryTool.c
 * @Description: Add SHA256 + CRC32 at (data_size - 36)
 ****************************************************************************************************************/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "Crc.h"
#include "Sha256.h"

#define TAIL_SIZE 1024

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        printf("Usage:\n");
        printf("tool input.bin output.bin\n");
        return -1;
    }

    char *input = argv[1];
    char *output = argv[2];



    FILE *fi = fopen(input, "rb");

    if (!fi)
    {
        printf("open input error\n");
        return -1;
    }

    fseek(fi, 0, SEEK_END);
    uint32_t input_size = ftell(fi);
    rewind(fi);

    printf("Input size : %04x\n", input_size);

    uint8_t *buffer = malloc(input_size);

    if (!buffer)
    {
        printf("malloc fail\n");
        return -1;
    }

    memset(buffer, 0xFF, input_size);

    fread(buffer, 1, input_size, fi);
    fclose(fi);

    // SHA256
    Sha256Ctx ctx;
    uint8_t hash[32];

    sha256_init(&ctx);
    sha256_update(&ctx, buffer, input_size);
    sha256_final(&ctx, hash);

    printf("SHA256: ");
    for (int i = 0; i < 32; i++)
        printf("%02x", hash[i]);
    printf("\n");

    // CRC32
    uint32_t crc = crc32_calc(CRC_DEFAULT_INIT, buffer, input_size);

    printf("CRC32 : %08x\n", crc);

    uint8_t output_data[40];
    // write hash + crc into buffer
    output_data[0] = ((uint8_t*)&input_size)[0];
    output_data[1] = ((uint8_t*)&input_size)[1];
    output_data[2] = ((uint8_t*)&input_size)[2];
    output_data[3] = ((uint8_t*)&input_size)[3];
    memcpy(&output_data[4], hash, 32);
    memcpy(&output_data[36], &crc, 4);

    FILE *fo = fopen(output, "wb");

    if (!fo)
    {
        printf("open output error\n");
        return -1;
    }

    fwrite(output_data, 1, 40, fo);

    fclose(fo);
    free(buffer);

    printf("Done\n");

    return 0;
}