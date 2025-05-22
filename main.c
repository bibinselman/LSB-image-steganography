
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "encode.h"
#include "decode.h"
#include "types.h"

//declaring function to check arg type (-e (encoding) or -d (decoding))
OperationType check_operation_type(char *);

int main(int argc, char *argv[])
{
    //validating no of args
    if(argc >= 3 && argc <= 5)
    {
        //calling function to check operation to do is encode or decode
        if((check_operation_type(argv[1]) == e_encode) && (argc == 4 || argc == 5)) 
        {
            EncodeInfo encodeInfo;
            printf("You've selected to do encoding\n");
            
            //validate encode args
            if(read_and_validate_encode_args(argv,&encodeInfo) == e_success)
            {
                //start encoding
                do_encoding(&encodeInfo);
            }
            else
            {
                exit(1);
            }
        } 
        else if((check_operation_type(argv[1]) == e_decode) && (argc == 3 || argc == 4))
        {
            DecodeInfo decodeInfo;
            printf("You've selected to do decoding\n");
            
            //validate decode args
            if(read_and_validate_decode_args(argv, &decodeInfo) == e_success)
            {
                //start decoding
                do_decoding(&decodeInfo);
            }
            else
            {
                exit(1);
            }

        } 
    }
       
    else
    {
        //if arg numbers are not valid
        printf("Enter correct number of arguments\n");
    }

    return 0;
}
OperationType check_operation_type(char *symbol)
{
    //check argv[1] is -e
    if ((strcmp(symbol, "-e")) == 0)
    {
        return e_encode;
    } 
    //check argv[1] is -d
    else if ((strcmp(symbol, "-d")) == 0)
    {
        return e_decode;
    }
   
}
