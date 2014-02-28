#include <stdio.h>
#include <stdlib.h>
#include "huff.h"
#include <vector>
#include <map>
#include <string.h>
using namespace std;

#define NAC 128 // not a character
#define EOFC 129 // eof character
unsigned char *data;
size_t size;
char freq[256];
FILE *fp;
Node *tree;
P_Queue queue;
code codes[257];
unsigned char *eof;
/*
   void print(vector<Node *> *v){
   for(int i = 0; i < (*v).size(); i++){
   printf("char: %d, priority: %d\n", (*v)[i] -> c, (*v)[i] -> priority);
   }
   putchar('\n');
   }
   */



//loads the file data and calculates
//frequencies
void load_file(char *file){
    for(int i = 0; i < 256; i++){
        freq[i] = 0;
    }
    fp = fopen(file, "r"); 
    fseek(fp, 0L, SEEK_END);
    size = ftell(fp);
    fseek(fp, 0L, SEEK_SET);
    data = (unsigned char *)calloc(size, sizeof(char));
    fread(data, sizeof(char), size, fp);
    for(int i = 0; i < size; i++){
        freq[128 + data[i]]++;
    }
    for(int i = 0; i < size; i++){
        printf("%X ", data[i]);
    }
    putchar('\n');
    /*
    for(int i = 0; i < 256; i++){
        printf("%X: %d\n", i, freq[i]);
    }*/
}

char masks[] = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01}; 
void b_write(bit_stream *b_stream, unsigned char *buffer, size_t bits){
    // at the beginning of this function call, the buffer should never be full
    // since it should have been flushed at the end of the last call to this function
    // need to shift buffer by the offset
    printf("bits: %d\n", bits);
    int size = (bits + 15) / 8;
    unsigned char *shift = (unsigned char *)calloc(size, sizeof(char));  
    for(int i = size - 2; i >= 0; i--){
        shift[i + 1] |= (buffer[i] & (1 << (b_stream -> offset - 1))) << (8 - b_stream -> offset);
        shift[i] = buffer[i] >> b_stream -> offset;
    }
    for(int i = 0; i < size; i++){
        if(b_stream -> size == b_stream -> capacity){
            // flush buffer
            fwrite(b_stream -> buffer, sizeof(char), b_stream -> capacity, b_stream -> fp);
            b_stream -> size = 0;
            for(int j = 0; j < b_stream -> capacity; j++){
                b_stream -> buffer[j] = 0;
            }
        }
        b_stream -> buffer[size++] |= shift[i];
    }
    b_stream -> offset = (b_stream -> offset + bits) % 8;
}

// flushes and closes the stream
void b_close(bit_stream *b_stream){
    fwrite(b_stream -> buffer, sizeof(char), b_stream -> size, b_stream -> fp);
    fclose(b_stream -> fp);
    free(b_stream -> buffer);
    free(b_stream);
}

bit_stream *b_open(char *output){
    FILE *fp = fopen(output, "w+");
    bit_stream *result = (bit_stream *)malloc(sizeof(bit_stream));
    unsigned char *buffer = (unsigned char *)calloc(1 << 16, sizeof(char));
    *result = (bit_stream){0, 0, 1 << 16, buffer, fp};
    return result;
}

/*
   int find_eof(Node *n, unsigned char *eof_code, int length, int capacity){
   int found;
   if(n -> left){
   if(length / 8 == capacity){
   unsigned char *tmp = (unsigned char *)calloc(capacity * 2, sizeof(char));
   for(int i = 0; i < capacity; i++)
   tmp[i] = eof_code[i];
   capacity *= 2;
   free(eof_code);
   eof_code = tmp;
   }
   found = find_eof(n -> left, eof_code, length + 1, capacity);
   }
   if(found)
   return found;
   if(n -> c == EOFC){
// set eof to the eof bit sequence 
eof = eof_code;
return 1; 
}
if(n -> right){
if(length / 8 == capacity){
unsigned char *tmp = (unsigned char *)calloc(capacity * 2, sizeof(char));
for(int i = 0; i < capacity; i++)
tmp[i] = eof_code[i];
capacity *= 2;
free(eof_code);
eof_code = tmp;
}
eof_code[length / 8] |= (1 << (length % 8));
int result = find_eof(n -> right, eof_code, length + 1, capacity);
eof_code[length / 8] &= ~(1 << (length % 8));
return result;
}
}
*/
void store_codes(Node * n, code c){
    if(c.length / 8 == c.capacity){
        unsigned char *tmp = (unsigned char *)calloc(c.capacity * 2, sizeof(char)); 
        memcpy(tmp, c.code_seq, c.capacity);
        c.capacity *= 2;
        free(c.code_seq);
        c.code_seq = tmp;
    }
    if(n -> left){
        c.length++;
        store_codes(n -> left, c);
        c.length--;
    }
    if(n -> right){
        c.length++;
        c.code_seq[c.length / 8] |= (1 << (c.length % 8));
        store_codes(n -> right, c);
        c.code_seq[c.length / 8] &= ~(1 << (c.length % 8));
        c.length--;
    }
    if(!n -> left && !n -> right){
        printf("byte: %X\n", n -> c);
        int length = (c.length + 7) / sizeof(char);
        unsigned char *tmp = (unsigned char *)calloc(length, sizeof(char)); 
        memcpy(tmp, c.code_seq, length);
        //codes.insert(pair<int, code>(n -> c, (code){tmp, c.length, length}));
        codes[n -> c] = (code){tmp, c.length, length};
        printf("length: %d\n", c.length);
    }
}

void compress(char *compressed){
    queue.size();
    for(int i = 0; i < 256; i++){
        if(!freq[i])
            continue;
        Node *n = (Node *)malloc(sizeof(Node));
        *n = (Node){i, freq[i], 0, NULL, NULL};
        queue.push(n);
    }
    while(queue.size() > 1){
        Node *n1 = queue.pop();
        Node *n2 = queue.pop();
        Node *n3 = (Node *)malloc(sizeof(Node));
        *n3 = (Node){NAC, n1 -> priority + n2 -> priority, 0, n1, n2};
        queue.push(n3);
    }
    tree = queue.pop();
    //unsigned char *eof_code = (unsigned char *)calloc(4, sizeof(char));
    //find_eof(tree, eof_code, 0, 4);
    unsigned char *code_seq = (unsigned char *)calloc(4, sizeof(char));
    store_codes(tree, (code){code_seq, 0, 4});
    bit_stream *b = b_open(compressed);
    for(int i = 0; i < size; i++){
        code c = codes[data[i] + 128];
        printf("hi\n");
        b_write(b, c.code_seq, c.length);
    }
    b_write(b, codes[257].code_seq, codes[257].length);
    b_close(b);
}

int main(int argc, char **argv){
    /*
    Node *n;
    // refactor this, it's ugly
    P_Queue v = P_Queue();
    for(int i = 10; i > 0; i--){
        n = (Node *)malloc(sizeof(Node));
        *n = (Node){i, (i + 1) / 2, 0, NULL, NULL};
        v.push(n);
    }
    //printf("Size of queue: %d\n", v.size());
    size_t size = v.size();
    for(int i = 0; i < size; i++){
        Node *tmp = v[i];
        printf("char: %d, priority: %d\n", tmp -> c, tmp -> priority);
    }
    printf("Popping now\n");
    for(int i = 0; i < size; i++){
        //printf("Size before: %d\n", v.size());
        Node *tmp = v.pop();
        //printf("Size after: %d\n", v.size());
        printf("char: %d, priority: %d\n", tmp -> c, tmp -> priority);
    }
    return 0;
    */
    if(argc < 3)
        return 1;
    load_file(argv[1]);
    compress(argv[2]);
}
