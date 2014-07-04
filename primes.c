#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <assert.h>
#include <string.h>

// set the bth bit in a
#define set(a, b) (a |= (1 << b))

/*
 * This is awful code and I apologize. I wrote this at 3am
 * cuz I was bored. Will refactor sometime, honest.
*/

// returns total number of primes in file
char masks[] = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};
FILE *prime_check;

int write_primes(uint64_t max, char *file){
    // to force alignment, make max next mult of 8
    max = (max + 7) & 8;
    uint64_t stop = (uint64_t)sqrt(max); // stopping place
    FILE *read = fopen(file, "r");
    FILE *write;
    uint64_t size;
    if(read){ // file exists
        // grab file size
        fread(&size, sizeof(long), 1, read);
        write = fopen(file, "r+");
    }else{
        size = 0;
        write = fopen(file, "w+");
    }
    if(size >= max){
        printf("File already contains more than %d primes\n", max);
        fclose(read);
        fclose (write);
        return -1;
    }
    // need 0 initialized buffer
    uint8_t *new_primes = calloc(1 << 16, sizeof(char));
    uint64_t buffer_length = 0; // in bits
    uint64_t current_prime;
    uint64_t offset = size / 8;
    // reading from file
    while(current_prime <= max){     
        if(buffer_length >= 1 << (16 + 3)) {
            fwrite(new_primes, sizeof(char), 1 << 16, write);
            buffer_length = 0;
            memset(new_primes, 0, 1 << 16);
        }
    }
    // generating new primes
    for(;;){
    }
    if(!size)
        new_primes[0] |= 0xC0;

}

/*
int write_primes(uint64_t max, char *file){
    // 1 means we're reading old primes, 0 means calculating new ones
    char b;
    // to force alignment, make max the next multiple of 8;
    max += !!(0x7 & max) << 3;
    max &= ~(0x7);
    uint64_t stop = (uint64_t)sqrt(max);
    FILE *read = fopen(file, "r");
    FILE *write;
    uint64_t size;
    if(read){ // file exists
        fread(&size, sizeof(long), 1, read);
        write = fopen(file, "r+");
    }else{
        size = 0;
        write = fopen(file, "w+");
    }
    b = size > 0;
    if(size >= max){
        printf("File already contains more than %d primes\n", max);
        return -1;
    }
    uint8_t *new_primes = calloc((max - size) / 8, sizeof(char));
    if(!size)
        new_primes[0] |= 0xC0;
    uint64_t *old_primes = calloc(4096, sizeof(long));
    uint64_t offset = 0; // offset in bits in read
    uint32_t current_prime_old = 0; // index of current prime whose multiples to cross out 
    uint64_t current_prime_new = 0;
    char current; // current byte in read
    int chunk_size = 0;
    while(1){ 
        uint64_t p = 0;
        if(b){
            if(!current_prime_old){
                chunk_size = 0;
                int j = 0;// # primes encountered so far
                while(j < 4096 && offset < size && offset <= stop){
                    //printf("herp\n");
                    if(!(offset & 0x7)){
                        fread(&current, sizeof(char), 1, read);
                    }
                    if(!(masks[offset & 0x7] & current)){
                        old_primes[j++] = offset;
                        chunk_size++;
                    }
                    offset++; 
                }
            }
            // at this point prime chunk has been loaded
            p = old_primes[current_prime_old];
            if(chunk_size)
                current_prime_old = (current_prime_old + 1) % chunk_size;
            // if offset >= size b needs to be 0
            // if offset > stop b needs to be 0
            if(offset >= size || offset > stop && current_prime_old >= chunk_size)
                b = 0;
        }else{
            while(new_primes[current_prime_new / 8] & 
                    masks[current_prime_new  % 8]){ 
                current_prime_new++;
            }
            p = current_prime_new + size;
            current_prime_new++;
        }
        assert(p);
        if(p > stop){
            break;
        }
        for(uint64_t j = p * p; j < max; j += p){
            if((long)(j - size) < 0){
                //printf("continue\n");
                continue;
            }
            new_primes[(j - size) / 8] |= masks[(j - size) % 8];
        }
    }
    fseek(write, 0, SEEK_SET);
    fwrite(&max, sizeof(long), 1, write);
    fseek(write, 0, SEEK_END);
    fwrite(new_primes, sizeof(char), (max - size) / 8, write);
    if(read)
        fclose(read);
    fclose(write);
}   

*/
int prime_init(char *file){
    prime_check = fopen(file, "r");
    return !prime_check;
}

int is_prime(uint64_t l){
    fseek(prime_check, 8 + l / 8, SEEK_SET);
    char c;
    fread(&c, sizeof(char), 1, prime_check);
    return !(c & masks[l % 8]);
}

int main(int argc, char **argv){
    uint64_t in = strtoul(argv[1], NULL, 10); 
    write_primes(in, argv[2]);
    printf("Write Finished.\n");
    char buffer[20];
    printf("Enter file to scan: ");
    fflush(stdin);
    scanf("%s", buffer);
    prime_init(buffer);
    uint64_t l;
    while(1){
        scanf("%lu", &l);
        printf("Is %lu prime: %d\n", l, is_prime(l));
    }
}
