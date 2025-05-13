#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "encode.h"
#include "types.h"
#include "common.h"


/* finding size of src img*/

uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width, height;
    // Seek to 18th      
    fseek(fptr_image, 18, SEEK_SET);

    // Read the width (an int)
    fread(&width, sizeof(int), 1, fptr_image);
    printf("width = %u\n", width);

    // Read the height (an int)
    fread(&height, sizeof(int), 1, fptr_image);
    printf("height = %u\n", height);

    // Return image capacity
    return width * height * 3;
}

/* Find the size of the secret file data and return it */

uint get_file_size(FILE *fptr)
{
    fseek(fptr, 0, SEEK_END);  //moving pointer to end         
    long file_size = ftell(fptr);  //storing secret file size   
    rewind(fptr);  //moving back pointer to start

    return file_size;
}


 /* reading and validating arguments passed for encoding */

Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{

    // checking source image argument have .bmp 
    if (strstr(argv[2], ".bmp") != NULL)
    {
        encInfo->src_image_fname = argv[2];  // if yes, storing argument to source file name
    }
    else
    {
        printf("Unsupported source file\n");
        return e_failure;  //if .bmp not present, returning error
    }

    // checking secret file name have .txt / .c / .sh
    if((strstr(argv[3],".txt") != NULL)  || (strstr(argv[3], ".c") != NULL) || (strstr(argv[3], ".sh") != NULL))
    {
        encInfo->secret_fname = argv[3];  // if yes, storing argument to secret_fname
    }
    else
    {
        printf("Unsupported secret text file\n");
        return e_failure;  // if not, returning error
    }

    //checking if dest image name is passed in argument
    if(argv[4] == NULL)
    {
        encInfo->stego_image_fname = "Default.bmp"; // if not, store dest image name as 'Default.bmp'
    }
    else
    {
        //if dest image name is passed
        if(strstr(argv[4],".bmp")) //checking if name includes .bmp
        {
        encInfo->stego_image_fname = argv[4];  // if yes, store argument to stego_image_name
        }
        else
        {
            printf("Invalid destination file name\n"); 
            return e_failure; // if .bmp not present, return error
        }
    }

    // printing success message and returning e_success
    printf("All arguments are valid. Proceeding with encoding...\n");
    return e_success; 

}


/* opening files in EncodeInfo structure */

Status open_files(EncodeInfo *encInfo)
{
    // Src Image file
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "r");
    // Do Error handling
    if (encInfo->fptr_src_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->src_image_fname);

        return e_failure;
    }

    // Secret file
    encInfo->fptr_secret = fopen(encInfo->secret_fname, "r");
    // Checking if file opened successfully
    if (encInfo->fptr_secret == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->secret_fname);

        return e_failure;
    }

    // Stego Image file
    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "w");
    // Checking if file opened successfully
    if (encInfo->fptr_stego_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->stego_image_fname);

        return e_failure;
    }

    // All success, return e_success
    return e_success;
}


/* checking if there's enough capacity in source image */


Status check_capacity(EncodeInfo *encInfo)
{
    encInfo->image_capacity = get_image_size_for_bmp(encInfo->fptr_src_image);   // finding and storing image size
    encInfo->size_secret_file = get_file_size(encInfo->fptr_secret);  // finding and storing secret file size

    //checking image has enough capacity 
    if(encInfo->image_capacity > (54+16+32+32+32+(encInfo->size_secret_file * 8)))
    {
        printf("Enough capacity available \n");
        return e_success; // capacity validated, returning e_success
    }
    else
    {
        printf("ERROR : Not enough space avaialbe\n"); 
        return e_failure; // not enough capacity found, returning e_failure
    }
}


/* copy bmp header data - uptp 54 bytes */

Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
{
    char imageBuffer[54]; // to store 54 bytes
    rewind (fptr_src_image); // setting source image pointer to start

    if(fread(imageBuffer, 1, 54, fptr_src_image) == 54) //reading 54 bytes from source img and validating
    {
        fwrite(imageBuffer, 1, 54, fptr_dest_image); // writing 54 bytes to buffer
        return e_success;
    }
    else
    {
        return e_failure; 
    }
    
}


/* encoding magic string to image*/

Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo)
{
    int size = strlen(magic_string); // finding and storing size of magic string

    //encoding magic string
    if(encode_data_to_image((char*)magic_string, size, encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_success)
    {
        printf("Magic string encoded successfully\n"); // successfully encoded
        return e_success;
    }
    else
    {
        printf("ERROR : Encoding magic string failes\n"); //failed to encode
        return e_failure;
    }

}


/* encoding secret file extension size to image*/

Status encode_secret_file_extn_size(int size, EncodeInfo *encInfo)
{

    char imageBuffer[32]; // char array to store data from src img
    if(fread(imageBuffer, 1, 32, encInfo->fptr_src_image) == 32)
    {
        // calling function to encode
        encode_size_to_lsb(size, imageBuffer);
        fwrite(imageBuffer, 1, 32,encInfo->fptr_stego_image);
        return e_success;
    }
    else
    {
        return e_failure;
    }

}


/* encoding secret file extension to image*/

Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo)
{

    // calling function to encode  data and validating
    if(encode_data_to_image(encInfo->extn_secret_file, 4, encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_success)
    {
        return e_success;
    }
    else
    {
        return e_failure;
    }
}


/* encode secret file size to image*/

Status encode_secret_file_size(long file_size, EncodeInfo *encInfo)
{

    char imageBuffer[32]; // char array to store data from src img

    //reading and validating 32 bytes from src img
    if(fread(imageBuffer, 1, 32, encInfo->fptr_src_image) == 32)
    {
        //calling function to do encoding
        encode_size_to_lsb(encInfo->size_secret_file,imageBuffer);
        fwrite(imageBuffer,1,32,encInfo->fptr_stego_image);
        return e_success;
    }
    else
    {
        printf("ERROR: failed to encode secret file size\n");
        return e_failure;
    }

}


/* encoding secret file data to image*/

Status encode_secret_file_data(EncodeInfo *encInfo)
{

    char data[encInfo->size_secret_file]; // char array to store data frok src img

    rewind(encInfo->fptr_secret); // setting pointer to start

    //reading data
    if(fread(data, 1, encInfo->size_secret_file, encInfo->fptr_secret) == encInfo->size_secret_file)
    {
        //calling function to encode data
        encode_data_to_image(data, encInfo->size_secret_file, encInfo->fptr_src_image, encInfo->fptr_stego_image);
        return e_success;
    }
    else
    {
        return e_failure;
    }
}


/* copying remainig data from src to dest image*/

Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest)
{
    char ch; // char variable to copy and paste data
    while((fread((&ch),1,1,fptr_src)) > 0) // reading data from src img
    {
        fwrite(&ch,1,1,fptr_dest); // pasting data to dest img
    }

    printf("Remaining image data copied successfully\n");

    return e_success;
}



/* function to encode data to the image*/

Status encode_data_to_image(char *data, int size, FILE *fptr_src_image, FILE *fptr_stego_image)
{
    char imageBuffer[8]; // char array to store data from src img

    //loop to pass and encode data byte by byte
    for(int i = 0; i < size; i++)
    {
    fread(imageBuffer, 8, 1, fptr_src_image); // reading data from src img
    encode_byte_to_lsb(data[i], imageBuffer); // calling function to encode
    fwrite(imageBuffer, 8, 1, fptr_stego_image); // writing data to dest img
    }
        return e_success;

}


/* function to encode data byte to lsb of bytes of image*/

Status encode_byte_to_lsb(char data, char *image_buffer)
{
    for (int i = 0; i < 8; i++)
    {
    image_buffer[i] &= ~1;
    char bit = (data >> (7 - i)) & 1;  // Encode from MSB to LSB
    image_buffer[i] |= bit;
    }

    return e_success;

}


/* function to encode sizes to image bytes lsb */

Status encode_size_to_lsb(int size, char *image_buffer)
{
    for (int i = 0; i < 32; i++) 
    {

    image_buffer[i] &= ~1; 
        
    int bit = (size >> (31 - i)) & 1;
        
    image_buffer[i] |= bit;

    }

    return e_success;

}

// encoding the datas to image step by step
Status do_encoding(EncodeInfo *encInfo)
{

    //opening files
    if(open_files(encInfo) == e_failure)
    {
        printf("Opening files has failed\n");
        return e_failure;
    }
    else
    {
        printf("Files opened successfully\n");
    }
 
    //checking capacity
    if(check_capacity(encInfo) == e_failure)
    {
        printf("Capacity is not enough\n");
        return e_failure;
    }
    else
    {
        printf("Found enough capacity\n");
    }

    //Copying bmp header
    if(copy_bmp_header(encInfo->fptr_src_image,encInfo->fptr_stego_image) == e_failure)
    {
        printf("Copying bmp header has failed\n");
        return e_failure;
    }

    //encoding magic string
    if(encode_magic_string(MAGIC_STRING,encInfo) == e_failure)
    {
        printf("Magic string encoding failed\n");
        return e_failure;
    }
    else
    {
        printf("Magic string succesfully encoded\n");
    }

    //extracting extension from secret file name
    char *extn = strrchr(encInfo->secret_fname, '.');
    if (extn != NULL)
    {
        encInfo->extn_size = strlen(extn);

        strcpy(encInfo->extn_secret_file, extn);
        encInfo->extn_secret_file[encInfo->extn_size] = '\0'; // Ensure null termination
    }
    else
    {
        printf("Error : Could not extract file extension.\n");
        return e_failure;
    }
    printf("Extracted secret file extension: %s\n", encInfo->extn_secret_file);


    //encoding secret file extn size
    if(encode_secret_file_extn_size(encInfo->extn_size,encInfo) == e_failure)
    {
        printf("Error : Failed to encode secret file extension size\n");
        return e_failure;
    } 


    //encoding secret file extn
    if(encode_secret_file_extn(encInfo->extn_secret_file,encInfo) == e_failure)
    {
        printf("Encoding secret file extension failed\n");
        return e_failure;
    }


    //encoding secert file size
    if(encode_secret_file_size(encInfo->size_secret_file,encInfo) == e_failure)
    {
        printf("Error : failed to encode secret file size\n");
        return e_failure;
    }


    //calling secret file data
    if(encode_secret_file_data(encInfo) == e_failure)
    {
        printf("Error : failed encode secret file data\n");
        return e_failure;
    }


    //copying remaining data
    if(copy_remaining_img_data(encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_failure)
    {
        printf("Error : failed to copy remaining image data\n");
        return e_failure;
    }


    //closing file pointers
    fclose(encInfo->fptr_src_image);
    fclose(encInfo->fptr_secret);
    fclose(encInfo->fptr_stego_image);
    printf("INFO: All files closed successfully.\n");

    return e_success;

    
}
