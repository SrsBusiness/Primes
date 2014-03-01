#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
//#define

// returns total number of primes in file
char masks[] = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};
FILE *prime_check;
int write_primes(unsigned long max, char *file){
    // 1 means we're reading old primes, 0 means calculating new ones
    char b;
    // to force alignment, make max the next multiple of 8;
    max += !!(0x7 & max) << 3;
    max &= ~(0x7);
    unsigned long stop = (unsigned long)sqrt(max);
    //printf("max: %d, stop: %d\n", max, stop);
    FILE *read = fopen(file, "r");
    FILE *write;
    unsigned long size;
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
    //printf("size: %d\n", size);
    unsigned char *new_primes = (char *)calloc((max - size) / 8, 
            sizeof(char));
    if(!size)
        new_primes[0] |= 0xC0;
    unsigned long *old_primes = (long *)calloc(4096, sizeof(long));
    //int i; 
    unsigned long offset = 0; // offset in bits in read
    unsigned int current_prime_old = 0; // index of current prime whose multiples to cross out 
    unsigned long current_prime_new = 0;
    char current; // current byte in read
    //long mask = 0;
    int chunk_size = 0;
    while(1){ 
        //printf("%d\n", 1);
        //printf("outer\n");
        unsigned long p = 0;
        //printf("offset: %d, size: %d\n", offset, size);
        if(b){
            //printf("old\n");
            //printf("chunk_size before: %d\n", chunk_size);
            //printf("current_prime_old before: %d\n", current_prime_old);
            //printf("old\n");
            if(!current_prime_old){
                chunk_size = 0;
                int j = 0;// # primes encountered so far
                //printf("offset: %d, size: %d, stop: %d\n", offset, size, stop);
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
            //printf("chunk_size after: %d\n", chunk_size);
            //printf("current_prime_old after: %d\n", current_prime_old);
            // at this point prime chunk has been loaded
            //printf("%d\n", 2);
            p = old_primes[current_prime_old];
            if(chunk_size)
                current_prime_old = (current_prime_old + 1) % chunk_size;
            // if offset >= size b needs to be 0
            // if offset > stop b needs to be 0
            if(offset >= size || offset > stop && current_prime_old >= chunk_size)
                b = 0;
        }else{
            //printf("Shouldn't see this\n");
            //printf("new\n");
            //printf("current_prime_new = %d\n", current_prime_new);
            while(new_primes[current_prime_new / 8] & 
                    masks[current_prime_new  % 8]){ 
                //printf("new_primes[(p = current_prime_new + size) / 8] = %X\n", new_primes[(p = current_prime_new + size) / 8]);
                //printf("p mod 8 = %d\n", p % 8);
                //printf("p: %d\n", p);
                current_prime_new++;
            }
            //printf("%d\n", 3);
            p = current_prime_new + size;
            current_prime_new++;
        }
        assert(p);
        if(p > stop){
            //printf("break_a\n");
            //printf("p: %d\n", p);
            break;
        }
        //printf("P is %d\n", p);
        //printf("size = %d\n", size);
        for(unsigned long j = p * p; j < max; j += p){
            //printf("%d\n", 4);
            //printf("j - size: %d\n", j - size);
            //printf("(j - size) % 8: %d\n", (j - size) % 8);
            if((long)(j - size) < 0){
                //printf("continue\n");
                continue;
            }
            new_primes[(j - size) / 8] |= masks[(j - size) % 8];
        }
        //printf("herp p: %d\n", p);
    }
    /*
       for(int i = 0; i < (max - size) / 8; i++)
       printf("%hhd\n", new_primes[i]);
       */
    fseek(write, 0, SEEK_SET);
    fwrite(&max, sizeof(long), 1, write);
    fseek(write, 0, SEEK_END);
    fwrite(new_primes, sizeof(char), (max - size) / 8, write);
    if(read)
        fclose(read);
    fclose(write);
}   

int prime_init(char *file){
    prime_check = fopen(file, "r");
    return !prime_check;
}

int is_prime(unsigned long l){
    fseek(prime_check, 8 + l / 8, SEEK_SET);
    char c;
    fread(&c, sizeof(char), 1, prime_check);
    return !(c & masks[l % 8]);
}

int main(int argc, char **argv){
    unsigned long in = strtoul(argv[1], NULL, 10); 
    write_primes(in, argv[2]);
    printf("Write Finished.\n");
    /*
    char buffer[20];
    printf("Enter file to scan: ");
    fflush(stdin);
    scanf("%s", buffer);
    prime_init(buffer);
    unsigned long l;
    while(1){
        scanf("%lu", &l);
        printf("Is %lu prime: %d\n", l, is_prime(l));
    }
    */
}
