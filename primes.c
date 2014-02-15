#include <stdio.h>
#include <stdlib.h>
#include <math.h>

//#define

// returns total number of primes in file
int write_primes(long max, char *file){
    // to force alignment, make max the next multiple of 8;
    max += !!(0x7 & max) << 3;
    max &= ~(0x7);
    unsigned long stop = (unsigned long)sqrt(max);
    //printf("max: %d, stop: %d\n", max, stop);
    char masks[] = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};
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
    if(size >= max){
        printf("File already contains more than %d primes\n", max);
        return -1;
    }
    unsigned char *new_primes = (char *)calloc((max - size) / 8, 
            sizeof(char));
    if(!size)
        new_primes[0] |= 0xC0;
    unsigned long *old_primes = (long *)calloc(4096, sizeof(long));
    //int i; 
    unsigned long offset = 0; // offset in bits in read
    int current_prime_old = 0; // current prime whose multiples to cross out 
    int current_prime_new = 0;
    char current; // current byte in read
    //long mask = 0;
    int chunk_size = 0;
    while(1){ 
        //printf("outer\n");
        long p;
        //printf("offset: %d, size: %d\n", offset, size);
        if(offset < size || current_prime_old < chunk_size){
            //printf("old\n");
            if(!current_prime_old){
                chunk_size = 0;
                int j = 0;// # primes encountered so far
                while(j < 4096 && offset < size){
                    if(!(offset & 0x7)){
                        fread(&current, sizeof(char), 1, read);
                    }
                    if(!(masks[offset & 0x7] & current)){
                        old_primes[j++] = offset;
                        chunk_size++;
                    }
                    offset++; 
                }
                //printf("altered offset: %d\n", offset);
            }
            // at this point prime chunk has been loaded
            p = old_primes[current_prime_old];
            current_prime_old = (current_prime_old + 1) % 4096;
        }else{
            //printf("current_prime_new = %d\n", current_prime_new);
            //printf("size: %d\n", size);
            while(new_primes[(p = current_prime_new + size) / 8] & 
                    masks[p % 8]){ 
                //printf("new_primes[(p = current_prime_new + size) / 8] = %X\n", new_primes[(p = current_prime_new + size) / 8]);
                //printf("p mod 8 = %d\n", p % 8);
                //printf("p: %d\n", p);
                current_prime_new++;
            }

        }
        if(p > stop){
            //printf("break_a\n");
            //printf("p: %d\n", p);
            break;
        }
        //printf("p: %d\n", p);
        //printf("size = %d\n", size);
        for(unsigned long j = p * p; j < max; j += p){
            //printf("j - size: %d\n", j - size);
            //printf("(j - size) % 8: %d\n", (j - size) % 8);
            if((int)(j - size) < 0){
                //printf("continue\n");
                continue;
            }
            new_primes[(j - size) / 8] |= masks[(j - size) % 8];
        }
        current_prime_new++;
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



int main(int argc, char **argv){
    unsigned long in = strtoul(argv[1], NULL, 10); 
    write_primes(in, "primes.txt");
}
