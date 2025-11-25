#include <stdio.h>      
#include <stdlib.h>     
#include <unistd.h>    
#include <fcntl.h>      
#include <string.h>     
#include <stdint.h>     
#include <errno.h>      

void xor_crypt(int input_descriptor, int output_descriptor, unsigned char *key, size_t key_length){
    unsigned char buffer[4096]; 
    ssize_t bytesread; 
    size_t keyIndex = 0; 

    while ((bytesread = read(input_descriptor, buffer, sizeof(buffer))) > 0) {
        for (ssize_t i = 0; i < bytesread; i++){
            buffer[i] ^= key[keyIndex % key_length];
            keyIndex++; 
        }
        if (write(output_descriptor, buffer, bytesread) != bytesread){
            perror("write");
            exit(1); 
        }
    }
    if (bytesread < 0){
        perror("read");
        exit(1); 
    }
}

unsigned char roll_left(unsigned char temp, int set){
    set = set % 8;

    for (int i = 0; i < set; i++) {
        unsigned char carry = (temp & 0x80) >> 7;  // bit 7
        temp = (unsigned char)((temp << 1) | carry);
    }
    return temp;
}

unsigned char roll_right(unsigned char temp, int set){
    set = set % 8;

    for (int i = 0; i < set; i++) {
        unsigned char carry = (temp & 0x01) << 7;  // bit 0
        temp = (unsigned char)((temp >> 1) | carry);
    }
    return temp;
}


void bit_crypt(int input_descriptor, int output_descriptor, unsigned char *key, size_t key_length, int decrypt_flag){
    (void)key_length;
    unsigned char buffer[4096];
    ssize_t bytesread; 

    int shift = key[0] % 8;
    if (shift == 0) { shift = 1; }

    while ((bytesread = read(input_descriptor, buffer, sizeof(buffer))) > 0) {
        for (ssize_t i = 0; i < bytesread; i++){
            if (decrypt_flag)
                buffer[i] = roll_right(buffer[i], shift);
            else
                buffer[i] = roll_left(buffer[i], shift);
        }

        if (write(output_descriptor, buffer, bytesread) != bytesread){
            perror("write");
            exit(1); 
        }
    }

    if (bytesread < 0){
        perror("read");
        exit(1); 
    }
}



int main(int argc, char *argv[]){

    int opt; 
    int inputFd, outputFd; 
    int encrypt = 0, decrypt = 0, prompt = 0;

    char *input = NULL;
    char *output = NULL;
    char *key = NULL;
    char *algorithm = NULL;

    while ((opt = getopt(argc, argv, "eda:i:o:k:P")) != -1){
        switch (opt) {
            case 'e': encrypt = 1; break;
            case 'd': decrypt = 1; break;
            case 'i': input = optarg; break;
            case 'o': output = optarg; break;
            case 'a': algorithm = optarg; break;
            case 'k': key = optarg; break;
            case 'P': prompt = 1; break;
        }
    }

    if (!algorithm || (strcmp(algorithm, "xor") != 0 && strcmp(algorithm, "rol") != 0)) {
        fprintf(stderr, "Error: You must specify -a xor or -a rol.\n");
        exit(1);
    }

    if (!input || !output){
        fprintf(stderr, "Error: You must provide input and output files.\n");
        exit(1);
    }

    if ((!key && !prompt) || (key && prompt)) {
        fprintf(stderr, "Error: Provide a key with -k OR -P, but not both.\n");
        exit(1);
    }

    if (encrypt == 0 && decrypt == 0){
        fprintf(stderr, "Error: Choose -e (encrypt) or -d (decrypt).\n");
        exit(1); 
    }

    if (encrypt == 1 && decrypt == 1){
        fprintf(stderr, "Error: Cannot choose both -e and -d.\n");
        exit(1); 
    }

    inputFd = open(input, O_RDONLY);
    if (inputFd == -1) { perror("open input"); exit(1); }

    outputFd = open(output, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (outputFd == -1) { perror("open output"); exit(1); }

    unsigned char *keyBuf = NULL;
    size_t key_length = 0;

    if (key){
        int keyFd = open(key, O_RDONLY);
        if (keyFd == -1){ perror("open keyfile"); exit(1); }

        off_t size = lseek(keyFd, 0, SEEK_END);
        if (size <= 0){
            fprintf(stderr, "Error: Key file is empty.\n");
            close(keyFd);
            exit(1);
        }

        lseek(keyFd, 0, SEEK_SET);
        keyBuf = malloc(size);
        if (!keyBuf){ fprintf(stderr, "Error: Memory allocation failed.\n"); close(keyFd); exit(1); }

        if (read(keyFd, keyBuf, size) != size){
            perror("read keyFile");
            free(keyBuf);
            close(keyFd);
            exit(1);
        }

        key_length = (size_t)size;
        close(keyFd);
    }

    if (prompt){
        char temp[256];
        printf("Enter key: ");
        fflush(stdout);

        if (!fgets(temp, sizeof(temp), stdin)){
            fprintf(stderr, "Error: Failed to read key.\n");
            exit(1);
        }

        temp[strcspn(temp, "\n")] = 0;

        key_length = strlen(temp);
        if (key_length == 0){ fprintf(stderr, "Error: Empty key not allowed.\n"); exit(1); }

        keyBuf = malloc(key_length);
        if (!keyBuf){ fprintf(stderr, "Error: Memory allocation failed.\n"); exit(1); }

        memcpy(keyBuf, temp, key_length);
    }

    if (strcmp(algorithm, "xor") == 0){
        xor_crypt(inputFd, outputFd, keyBuf, key_length);
    } else {
        bit_crypt(inputFd, outputFd, keyBuf, key_length, decrypt);
    }

    close(inputFd);
    close(outputFd);
    free(keyBuf);

    return 0; 
}