#ifndef DECODE_H
#define DECODE_H

#include <stdio.h>
#include "common.h"
#include "types.h"

#define MAX_SECRET_BUF_SIZE 1
#define MAX_IMAGE_BUF_SIZE (MAX_SECRET_BUF_SIZE * 8)
#define MAX_FILE_SUFFIX 4

typedef struct _DecodeInfo
{
    /* Destination image info*/
    FILE *fptr_stego_image;
    char *stego_image_fname;

    /* Magic string info*/
    char magic_string[3];

    /* Secret File Info */
    char secret_fname[100];                           // To store the secret file name
    FILE *fptr_secret;                            // To store the secret file address
    char extn_secret_file[MAX_FILE_SUFFIX];       // To store the Secret file extension
    char secret_data[MAX_SECRET_BUF_SIZE];        // To store the secret data
    long size_secret_file;                        // To store the size of the secret data
    long int extn_size;                                // To store the size of extension
} DecodeInfo;

/* Function declarations */


Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo);


Status do_decoding(DecodeInfo *decInfo);


Status open_decode_files(DecodeInfo *decInfo);


Status decode_magic_string(const char *magic_str, DecodeInfo *decInfo);


Status decode_extn_size(int *extn_size, DecodeInfo *decInfo);


Status decode_extn(int extn_size, DecodeInfo *decInfo);


Status decode_secret_file_data_size(DecodeInfo *decInfo);


Status decode_secret_data(DecodeInfo *decInfo);


Status decode_data_from_image(char *data, int size, FILE *fptr_stego_image);


Status decode_byte_from_lsb(char *data, char *image_buffer);


Status decode_size_from_lsb(long int *size, char *image_buffer);

#endif