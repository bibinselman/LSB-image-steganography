#include <stdio.h>
#include <string.h>
#include "decode.h"
#include "types.h"
#include "common.h"



/* Read and validate decode arguments*/

Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo)
{
   // checking dest image argument have .bmp 
   if (strstr(argv[2], ".bmp") != NULL)
   {
       decInfo->stego_image_fname = argv[2];  // if yes, storing argument to source file name
   }
   else
   {
       printf("Unsupported stego image file\n");
       return e_failure;  //if .bmp not present, returning error
   }

   //checking output file argument
   if(argv[3] == NULL)
   {
    strcpy(decInfo->secret_fname, "secret_out");
   }
   else
   {
   
    char temp[100]; // declaring temp char array 
    strcpy(temp, argv[3]);  // copy argv to temp to do oprtn
    char *extn_cut = strtok(temp, "."); // remove extn part from name (will be added after decoding extn)
    strcpy(decInfo->secret_fname, extn_cut); // copy file name without extn to secret_fname

   }

   //return  success
   return e_success;
}


/* Open decode files*/

Status open_decode_files(DecodeInfo *decInfo)
{
    
    // Stego Image file
    decInfo->fptr_stego_image = fopen(decInfo->stego_image_fname, "rb"); //rb

    // Do Error handling
    if (decInfo->fptr_stego_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", decInfo->stego_image_fname);
        return e_failure;
    }

    // Output file
    decInfo->fptr_secret = fopen(decInfo->secret_fname, "wb");//wb

    // Do Error handling
    if (decInfo->fptr_secret == NULL)
    { 
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", decInfo->secret_fname);

        return e_failure;
    }

    // return e_success
    return e_success;
}


/* decode magic string */

Status decode_magic_string(const char *magic_string, DecodeInfo *decInfo)
{
    // storing magic string length
    int size = strlen(magic_string);
    char M_string[size + 1];  // creating a char array to store extracted #MS
    M_string[size] = '\0'; // storing /0 to last position

    fseek(decInfo->fptr_stego_image, 54, SEEK_SET);  // moving pointer to 54 th pos in stego image
    decode_data_from_image(M_string, size, decInfo->fptr_stego_image); //

    //printf("Decoded Magic String: %s\n", M_string);
    //printf("Encoded Magic String: %s\n", magic_string);

    if (strcmp(M_string, magic_string) == 0)
    {
        char n_magic_string[50];
        label:
        printf("Enter the magic string to continue\n");
        scanf("%s",n_magic_string);

        if (strcmp(n_magic_string, M_string) == 0)
        {
            return e_success;
        }
        else
        {
            printf("Entered magic string is wrong\n");
            goto label;
        }
        
    }
    else
    {
        printf("Magic string not matched\n");
        return e_failure;
    }
}


/* decode secret file extension size*/

Status decode_secret_file_extension_size(DecodeInfo *decInfo)
{
    char Buffer[32]; // char array to read from stego img

    fread(Buffer, 32, 1, decInfo->fptr_stego_image);  // read 32 bytes from stego img
    //calling function to decode extn size
    decode_size_from_lsb(&decInfo->extn_size, Buffer); 

    return e_success;
}


/* decode secret file extension */

Status decode_secret_file_extension(DecodeInfo *decInfo)
{
    char extn[decInfo->extn_size+1]; // char array to store extn - with extn size as size
    extn[decInfo->extn_size] = '\0';  // initializing last element as null '/0'

    // calling function  to decode data 
    decode_data_from_image(extn, decInfo->extn_size, decInfo->fptr_stego_image);

    //add extn with output file
    strcat(decInfo->secret_fname,".");  // adding '.' of '.extn' to the file name
    strcat(decInfo->secret_fname, extn); // adding remaining extn name

    // Reopen secret file with new extension
    fclose(decInfo->fptr_secret); // closing secret file
    decInfo->fptr_secret = fopen(decInfo->secret_fname, "wb"); // opening secret file after new naming in "wb" mode
    
    //checking file opened successfully
    if(decInfo->fptr_secret == NULL)
    {
        return e_failure; // if failed to open
    }
    else
    {
        return e_success; // successfully opened

    }
 
}


/* decode secret file size */

Status decode_secret_file_size(DecodeInfo *decInfo)
{
    char Buffer[32]; // char array to store secret file size (int)
    fread(Buffer, 32, 1, decInfo->fptr_stego_image); // reading 32 bytes from stego img

    // calling function to decode size 
    decode_size_from_lsb(&decInfo->size_secret_file, Buffer); // 

    return e_success; // success
}


/* decode secret file data */

Status decode_secret_data(DecodeInfo *decInfo)
{
    char data; // char variable to store data

    for(int i=0; i<decInfo->size_secret_file; i++)
    {
        char Buffer[8]; // char variable to store 8 bytes at a time
        fread(Buffer, 8, 1, decInfo->fptr_stego_image); // reading 8 bytes from stego img
        // calling function to decode data from lsb bits
        decode_byte_from_lsb(&data, Buffer); 
        fwrite(&data, sizeof(char), 1, decInfo->fptr_secret); // writing data extracted from stego to secret output file
    }

    return e_success; // success
}



/* decode data from image */

Status decode_data_from_image(char *data, int size, FILE *fptr_stego_image)
{
    char imageBuffer[8]; // char array to store 8 bytes from stego img

    for (int i = 0; i < size; i++)
    {
        fread(imageBuffer, 8, 1, fptr_stego_image);           // Read 8 bytes from stego img
        //calling function to decode each byte of data
        decode_byte_from_lsb(&data[i], imageBuffer);          
    }

    return e_success; // success
}


/* decode byte from lsb */

Status decode_byte_from_lsb(char *data, char *image_buffer)
{
    *data = 0; // storing data with zero

    for (int i = 0; i < 8; i++)
    {
        *data = *data << 1; // left shift in each iteration
        *data = *data | (image_buffer[i] & 1); // getting lsb from each byte and stores to data
    }

    return e_success; // success
}


/* decode size from lsb */

Status decode_size_from_lsb(long int *size, char *image_buffer)
{
    *size = 0; // storing size with zero

    for (int i = 0; i < 32; i++) // running loop 32 times to get 32 bits (4 bytes)
    {
        *size = *size << 1;                         // left shifting to get eacg bit
        *size |= (image_buffer[i] & 1);             // getting lsb from each byte and stores in size
    }

    return e_success; // success
}


// to do decoding step by step
Status do_decoding(DecodeInfo *decInfo)
{
    //opening files
    if(open_decode_files(decInfo) == e_failure)
    {
        printf("Error : failed to open decoding files\n");
        return e_failure;
    }
    else
    {
        printf("Files opened successfully\n");
    }

    //decode magic string from image
    if(decode_magic_string(MAGIC_STRING,decInfo) == e_failure)
    {
        printf("Error : fialed to decode magic string\n");
        return e_failure;
    }
    else
    {
        printf("Magic string decoded successfully\n");
    }

    //decode secret file extension size
    if(decode_secret_file_extension_size(decInfo) == e_failure)
    {
        printf("Error : failed to decode secret file extension size\n");
        return e_failure;
    }
    else
    {
        printf("Secert file extension size decoded successfully\n");
    }

    //decode secret file extension
    if(decode_secret_file_extension(decInfo) == e_failure)
    {
        printf("Error : failed to decode secret file extension\n");
        return e_failure;
    }
    else
    {
        printf("Secret file extension decoded successfully\n");
    }


    //decode secret file size
    if(decode_secret_file_size(decInfo) == e_failure)
    {
        printf("Error : failed to decode secret file size");
        return e_failure;
    }
    else
    {
        printf("Secret file size decoded successfully\n");
    }

    //decode secret file data
    if(decode_secret_data(decInfo) == e_failure)
    {
        printf("Error : failed to decode secret file data\n");
        return e_failure;
    }
    else
    {
        printf("Secret file data decoded successfully\n");
    }

    //all operations are success
    return e_success;

}




















