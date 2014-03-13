#include <stdio.h>
#include <stdlib.h>
#include "huff.h"
#include <vector>
#include <map>
#include <string.h>
#include <assert.h>
#include <unordered_map>
#include <string>
using namespace std;

/*
32 byte bit-field representing alphabet
1 means character is present, 0 otherwise
If preset, expect it's code length in a byte
Max dict size: 32 + 256 + 1 bytes

Followed by concatenation of huffman codes

*/
#define NAC 257 // not a character
#define EOFC 256 // eof character
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

void free_tree(Node *n){
    if(n -> left)
        free_tree(n -> left);
    if(n -> right)
        free_tree(n -> right);
    free(n);
}

void c_load_file(char *file){
    for(int i = 0; i < 256; i++){
        freq[i] = 0;
    }
    fp = fopen(file, "r"); 
    fseek(fp, 0L, SEEK_END);
    size = ftell(fp);
    fseek(fp, 0L, SEEK_SET);
    data = (unsigned char *)calloc(size, sizeof(char));
    fread(data, sizeof(char), size, fp);
    fclose(fp);
    for(int i = 0; i < size; i++){
        freq[data[i]]++;
    }
    /*
    for(int i = 0; i < size; i++){
        printf("%X ", data[i]);
    }
    putchar('\n');
    */
    /*
       for(int i = 0; i < 256; i++){
       printf("%X: %d\n", i, freq[i]);
       }*/
}

void d_load_file(char *file){
    fp = fopen(file, "r");
    fseek(fp, 0L, SEEK_END);
    size = ftell(fp);
    printf("size: %u\n", size);
    fseek(fp, 0L, SEEK_SET);
    data = (unsigned char *)calloc(size, sizeof(char));
    fread(data, sizeof(char), size, fp);
    fclose(fp);
}

char masks[] = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01}; 
void b_write(bit_stream *b_stream, unsigned char *buffer, size_t bits){
    // at the beginning of this function call, the buffer should never be full
    // since it should have been flushed at the end of the last call to this function
    // need to shift buffer by the offset
    //printf("bits: %d\n", bits);
    //printf("bits: %d, b_stream -> size: %d\n", bits, b_stream -> size);
    int size = (bits + 15) / 8;
    //printf("size: %d\n", size);
    unsigned char *shift = (unsigned char *)calloc(size, sizeof(char));  
    char offset = b_stream -> size % 8;
    int old_size = b_stream -> size;
    for(int i = size - 2; i >= 0; i--){
        shift[i + 1] |= (buffer[i] & ((1 << offset) - 1)) << (8 - offset);
        shift[i] = buffer[i] >> offset;
    }
    /*
        can only flush full bytes to file
        
    */ 
    for(int i = 0; i < size; i++){
        // flush buffer if 
        if(b_stream -> size + 8 >= b_stream -> capacity * 8){
            // flush buffer
            fwrite(b_stream -> buffer, sizeof(char), b_stream -> capacity, b_stream -> fp);
            old_size = b_stream -> size = 0;
            for(int j = 0; j < b_stream -> capacity; j++){
                b_stream -> buffer[j] = 0;
            }
            b_stream -> size = 0;
        }
        //printf("size: %d, i: %d\n", b_stream -> size / 8, i);  
        b_stream -> buffer[b_stream -> size / 8] |= shift[i];
        b_stream -> size += 8;
    }
    b_stream -> size = old_size + bits % (1 << 16);
    free(shift);
    //printf("b_stream -> size after: %d\n", b_stream -> size);
    //assert(b_stream -> size % 8 == bits % 8);
}

// flushes and closes the stream
void b_close(bit_stream *b_stream){
    fwrite(b_stream -> buffer, sizeof(char), (b_stream -> size + 7) / 8, b_stream -> fp);
    fclose(b_stream -> fp);
    free(b_stream -> buffer);
    free(b_stream);
}

bit_stream *b_open(char *output){
    FILE *out = fopen(output, "w+");
    bit_stream *result = (bit_stream *)malloc(sizeof(bit_stream));
    unsigned char *buffer = (unsigned char *)calloc(1 << 16, sizeof(char));
    *result = (bit_stream){0, 1 << 16, buffer, out};
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
        int length = (c.length + 7) / 8;
        printf("length: %d\n", length);
        unsigned char *tmp = (unsigned char *)calloc(length, sizeof(char)); 
        printf("c.length: %d, c.capacity: %d\n", c.length, c.capacity);
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
    for(int j = 0; j < c.length; j++){
        putchar('0' + ((c.code_seq[j / 8] >> (7 - (j % 8))) & 1));
    }
    putchar('\n');
}

void get_lengths(Node *n, int level, unsigned char lengths[257]){
    if(n -> left) 
        get_lengths(n -> left, level + 1, lengths);
    if(n -> right)
        get_lengths(n -> right, level + 1, lengths);
    else if(!n -> left && !n -> right){
        lengths[n -> c] = level; 
    }
}

void inc_code(code *c){
    int length = c -> length;
    unsigned char *tmp = c -> code_seq;
    int last = (length - 1) / 8;
    //printf("%d\n", last);
    int overflow = (tmp[last] ==(unsigned char) ~((1 << (7 - (length + 7) % 8)) - 1));
    /*
    if(overflow)
        printf("overflow: %d\n", overflow);
    */
    //printf("1 << ~((7 - (length - 1) % 8) - 1)): %d\n", (unsigned char)~((1 << (7 - (length - 1) % 8)) - 1));
    //printf("tmp[last]: %d\n", tmp[last]);
    //printf("(int)0x80 >> ((c -> length - 1) % 8): %d\n", (char)0x80 >> ((c -> length - 1) % 8));
    tmp[last] += 1 << (7 - (length - 1) % 8);
    //printf("overflow: %d\n", overflow);
    while(overflow && --last >= 0){
        overflow = (tmp[last] == 0xFF);
        tmp[last] += 1; 
    }
    //printf("%d\n", 1 << (7 - (c -> length - 1) % 8));
}

void compress(char *input, char *compressed){
    //printf("size: %d\n", queue.size());
    c_load_file(input);
    for(int i = 0; i < 256; i++){
        if(!freq[i])
            continue;
        printf("i = %c, freq = %d\n", i, freq[i]);
        Node *n = (Node *)malloc(sizeof(Node));
        *n = (Node){i, freq[i], 0, NULL, NULL};
        queue.push(n);
    }
    // push eof
    Node *eof = (Node *)malloc(sizeof(Node));
    *eof = (Node){EOFC, 1, 0, NULL, NULL};
    queue.push(eof);
    while(queue.size() > 1){
        Node *n1 = queue.pop();
        Node *n2 = queue.pop();
        Node *n3 = (Node *)malloc(sizeof(Node));
        *n3 = (Node){NAC, n1 -> priority + n2 -> priority, 0, n1, n2};
        queue.push(n3);
    }
    tree = queue.pop();
    //printf("Queue Tree:\n");
    //print_tree(tree, 0);
    /*
        traverse tree and note the lenghts for each character
     */
    unsigned char lengths[257] = {0};
    get_lengths(tree, 0, lengths);
    // should free tree here;
    free_tree(tree);
    P_Queue sort_by_length;
    for(int i = 0; i < 257; i++){
        if(lengths[i]){ 
            Node *n = (Node *)malloc(sizeof(Node));
            *n = (Node){i, lengths[i], 0, NULL, NULL};
            sort_by_length.push(n);
        }
    }
    sort_by_length.print();
        printf("\n\n\n");
    code c = (code){(unsigned char *)calloc(4, sizeof(char)), 1, 4}; 
    for(int i = 0; i < 257; i++)
        codes[i] = (code){0, 0, 0};
    while(sort_by_length.size() > 0){
        Node *n = sort_by_length.pop();
        sort_by_length.print();
        printf("\n\n\n");
        c.length = n -> priority; 
        unsigned char *code_seq = (unsigned char *)calloc(4, sizeof(char));
        memcpy(code_seq, c.code_seq, 4);
        codes[n -> c] = (code){code_seq, c.length, c.capacity};
        //printf("%d %c ",n -> n, n -> c);
        //print_code(codes[n -> c]);
        inc_code(&c);
        free(n);
    }
    free(c.code_seq);
    for(int i = 0; i < 257; i++){
        code c = codes[i];
        if(c.length){
            printf("char: %c ", i);
            print_code(c);
        }
    }
    bit_stream *b = b_open(compressed);
    unsigned char bitfield[32] = {0};
    unsigned char code_lengths[257];
    int alpha_size = 0;
    for(int i = 0; i < 256; i++)
        if(freq[i] > 0){
            bitfield[i / 8] |= 1 << (7 - i % 8);
            code_lengths[alpha_size++] = codes[i].length;
        }
    code_lengths[alpha_size++] = codes[EOFC].length;
    b_write(b, bitfield, 256);
    b_write(b, code_lengths, alpha_size * 8);
    printf("size: %u\n", size);
    for(int i = 0; i < size; i++){
        code c = codes[data[i]];
        if(!c.length)
            continue;
        //printf("length: %d, character: %X\n", c.length, data[i]);
        b_write(b, c.code_seq, c.length);
    }
    b_write(b, codes[EOFC].code_seq, codes[EOFC].length);
    b_close(b);
    for(int i = 0; i < 257; i++)
        if(codes[i].code_seq)
            free(codes[i].code_seq);
    free(data);
}

// next_bit = index of next bit
void next_bit(size_t next_bit, code *c){
    //printf("data[%d / 8]: %X\n", next_bit, data[next_bit / 8]);
    c -> code_seq[c -> length / 8] |= ((data[next_bit / 8] >> (7 - next_bit % 8)) & 1) << (7 - c -> length % 8);
    c -> length++;
}

file_buffer *fb_open(char *out){
    file_buffer *result = (file_buffer *)malloc(sizeof(file_buffer));
    *result = (file_buffer){0, 1 << 16, (unsigned char *)calloc(1 << 16, sizeof(char)), fopen(out, "w+") };
    return result;
}

void fb_write(file_buffer *fb, unsigned char c){
    if(fb -> size == fb -> capacity){
        fwrite(fb -> buffer, sizeof(char), fb -> capacity, fb -> fp);
        fb -> size = 0;
    }
    fb -> buffer[fb -> size++] = c;
}

void fb_close(file_buffer *fb){
    printf("file size: %d\n", fb -> size);
    if(fb -> size)
        fwrite(fb -> buffer, sizeof(char), fb -> size, fb -> fp);
    fclose(fb -> fp);
    free(fb -> buffer);
    free(fb);
}
void decompress(char *in, char *out){
    d_load_file(in);
    int alpha_size = 0;
    unordered_map<code, unsigned int, code_hasher> codes_found;
    unsigned char lengths[257] = {0};
    for(int i = 0; i < 256; i++){
        if((data[i / 8] >> (7 - (i % 8))) & 1){
            lengths[i] = 1;
            alpha_size++;
        }
    }
    printf("Alphabet size: %d\n", alpha_size);
    lengths[256] = 1;
    // lengths[i] == 1 if i is in the alphabet
    int index = 0;
    for(int i = 0; i < alpha_size + 1; i++){
        while(!lengths[index])
            index++;
        lengths[index++] = data[i + 32];
    }
    /*
    for(int i = 0; i < 257; i++){
        if(lengths[i])
            printf("Character: %c, length: %u\n", i, lengths[i]);
    }
    */
    P_Queue q;
    //code c = (code){(unsigned char *)calloc(4, sizeof(char)), 0, 4};
    for(int i = 0; i < 257; i++){
        if(lengths[i]){
            Node *n = (Node *)malloc(sizeof(Node));
            *n = (Node){i, lengths[i], 0, NULL, NULL};
            q.push(n);
        }
    }
    //for(int i = 0; i < 257; i++)
    //    codes[i] = (code){0, 0, 0};
    code c = (code){(unsigned char *)calloc(4, sizeof(char)), 0, 4};
    while(q.size() > 0){
        Node *n = q.pop();
        //q.print();
        printf("\n\n\n");
        c.length = n -> priority; 
        unsigned char *code_seq = (unsigned char *)calloc(4, sizeof(char));
        memcpy(code_seq, c.code_seq, 4);
        code tmp = (code){code_seq, c.length, c.capacity};
        codes_found[tmp] = n -> c;
        //printf("%d %c ",n -> n, n -> c);
        //print_code(codes[n -> c]);
        inc_code(&c);
        printf("char: %c\n ", n -> c);
        free(n);
    }
    size_t seek = 256 + (alpha_size + 1) * 8;
    file_buffer *fb = fb_open(out);
    for(int i = 0; i < 4; i++)
        c.code_seq[i] = 0;
    c.length = 0;
    while(1){
        next_bit(seek++, &c);
        //print_code(c);
        if(codes_found.find(c) != codes_found.end()){
            print_code(c);
            unsigned int character = codes_found[c];
            if(character == EOFC)
                break;
            fb_write(fb, codes_found[c]);
            c.length = 0;
            for(int i = 0; i < 4; i++)
                c.code_seq[i] = 0;
        }
    }
    fb_close(fb); 
    /*
    for(int i = 0; i < 32; i++){
        next_bit(i + 256 + (alpha_size + 1) * 8, &c); 
        //print_code(c);
    }
    print_code(c);
    */
    // all the lengths have been read
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
    //compress(argv[1], argv[2]);
    decompress(argv[1], argv[2]);
    /*
    code c = (code){(unsigned char *)calloc(4, sizeof(char)), 5, 4};
    for(int i = 0; i < 32; i++){
        print_code(c);
        inc_code(&c); 
    } 
    */
}
