#include <stdio.h>
#include <stdlib.h>
#include "huff.h"
#include <vector>
#include <map>
#include <string.h>
using namespace std;

#define NAC 257 // not a character
//#define EOFC 256 // eof character
unsigned char *data;
size_t size;
int freq[256];
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

void print_code(code);

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
        freq[data[i]]++;
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
    //printf("bits: %d\n", bits);
    int size = (bits + 15) / 8;
    //printf("size: %d\n", size);
    unsigned char *shift = (unsigned char *)calloc(size, sizeof(char));  
    char offset = b_stream -> size % 8;
    for(int i = size - 2; i >= 0; i--){
        shift[i + 1] |= (buffer[i] & ((1 << offset) - 1)) << (8 - offset);
        shift[i] = buffer[i] >> offset;
    }
    for(int i = 0; i < size + 7; i++){
        if(b_stream -> size == b_stream -> capacity * 8){
            // flush buffer
            fwrite(b_stream -> buffer, sizeof(char), b_stream -> capacity, b_stream -> fp);
            b_stream -> size = 0;
            for(int j = 0; j < b_stream -> capacity; j++){
                b_stream -> buffer[j] = 0;
            }
            b_stream -> size = 0;
        }
        //printf("size + 1: %d, i: %d\n", size + 1, i);  
        b_stream -> buffer[b_stream -> size / 8 + i] |= shift[i];
    }
    b_stream -> size += bits;
}

// flushes and closes the stream
void b_close(bit_stream *b_stream){
    fwrite(b_stream -> buffer, sizeof(char), (b_stream -> size + 7) / 8, b_stream -> fp);
    fclose(b_stream -> fp);
    free(b_stream -> buffer);
    free(b_stream);
}

bit_stream *b_open(char *output){
    FILE *fp = fopen(output, "w+");
    bit_stream *result = (bit_stream *)malloc(sizeof(bit_stream));
    unsigned char *buffer = (unsigned char *)calloc(1 << 16, sizeof(char));
    *result = (bit_stream){0, 1 << 16, buffer, fp};
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
return resultntel xeon;
}
}
*/
void print_tree(Node *n, int indent){
    if(n -> right)
        print_tree(n -> right, indent + 4);
    for(int i = 0; i < indent; i++)
        putchar(' ');
    printf("%X, %d\n", n -> c, n -> priority);
    if(n -> left)
        print_tree(n -> left, indent + 4);
}
void store_codes(Node * n, code c){
    if(c.length == 8 * c.capacity){
        unsigned char *tmp = (unsigned char *)calloc(c.capacity * 2, sizeof(char)); 
        memcpy(tmp, c.code_seq, c.capacity);
        c.capacity *= 2;
        free(c.code_seq);
        c.code_seq = tmp;
    }
    /*
    if(n -> c != NAC){
        printf("char %X, before:\n", n -> c);
        print_code(c);
    }
    */
    if(n -> left){
        //printf("left\n");
        c.length++;
        store_codes(n -> left, c);
        c.length--;
    } 
    if(n -> right){
        //printf("right\n");
        c.code_seq[c.length / 8] |= (1 << (7 - (c.length % 8)));
        c.length++;
        //printf("right_code\n");
        //print_code(c);
        store_codes(n -> right, c);
        c.length--;
        c.code_seq[c.length / 8] &= ~(1 << (7 - (c.length % 8)));
    }
    /*
    if(n -> c != NAC){
        printf("char %X, after:\n", n -> c);
        print_code(c);
    }
    */
    if(!n -> left && !n -> right){
        //printf("added\n");
        //printf("byte: %X\n", n -> c);
        int length = (c.length + 7) / sizeof(char);
        unsigned char *tmp = (unsigned char *)calloc(length, sizeof(char)); 
        memcpy(tmp, c.code_seq, length);
        //codes.insert(pair<int, code>(n -> c, (code){tmp, c.length, length}));
        codes[n -> c] = (code){tmp, c.length, length};
        /*
        if(n -> c == EOFC)
            printf("EOF character added. Length: %d\n", c.length);
        */
        //printf("length: %d\n", c.length);
    }
}

/*
   void print_codes(){
   printf("Codes:\n");
   for(int i = 0; i < 257; i++){
   code c = codes[i];
   if(!c.length)
   continue;
   printf("%X: ", i);
   for(int j = 0; j < c.length; j++){
   putchar('0' + ((c.code_seq[j / 8] >> (7 - j)) & 1));
   }
   putchar('\n');
   }
   }
   */

void print_code(code c){
    for(int j = 0; j < c.length; j++)
        putchar('0' + ((c.code_seq[j / 8] >> (7 - (j % 8))) & 1));
    putchar('\n');
}

void compress(char *compressed){
    //printf("size: %d\n", queue.size());
    for(int i = 0; i < 256; i++){
        if(!freq[i])
            continue;
        printf("i = %c, freq = %d\n", i, freq[i]);
        Node *n = (Node *)malloc(sizeof(Node));
        *n = (Node){i, freq[i], 0, NULL, NULL};
        queue.push(n);
    }
    // push eof
    /*
    Node *eof = (Node *)malloc(sizeof(Node));
    *eof = (Node){EOFC, 0, 0, NULL, NULL};
    queue.push(eof);
    */
    while(queue.size() > 1){
        Node *n1 = queue.pop();
        Node *n2 = queue.pop();
        Node *n3 = (Node *)malloc(sizeof(Node));
        *n3 = (Node){NAC, n1 -> priority + n2 -> priority, 0, n1, n2};
        queue.push(n3);
    }
    tree = queue.pop();
    printf("Queue Tree:\n");
    print_tree(tree, 0);
    //unsigned char *eof_code = (unsigned char *)calloc(4, sizeof(char));
    //find_eof(tree, eof_code, 0, 4);
    unsigned char *code_seq = (unsigned char *)calloc(4, sizeof(char));
    for(int i = 0; i < 257; i++)
        codes[i] = (code){0, 0, 0};
    store_codes(tree, (code){code_seq, 0, 4});
    //print_codes();
    printf("Codes\n");
    fflush(stdin);
    for(int i = 0; i < 257; i++){
        code c = codes[i];
        if(c.length){
            printf("char: %X ", i);
            print_code(c);
        }
    }
    bit_stream *b = b_open(compressed);
    for(int i = 0; i < size; i++){
        code c = codes[data[i]];
        if(!c.length)
            continue;
        //printf("length: %d, character: %X\n", c.length, data[i]);
        b_write(b, c.code_seq, c.length);
    }
    //b_write(b, codes[EOFC].code_seq, codes[EOFC].length);
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
