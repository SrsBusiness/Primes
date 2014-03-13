// node for the sorted binary tree
#include <vector>
#include <stdio.h>
#include <string.h>
#include <string>
using namespace std;

#ifndef HUFF_H
#define HUFF_H

struct code{
    unsigned char *code_seq;
    size_t length; // in bits
    size_t  capacity; // in bytes
    bool operator==(const code &other) const{
        return (bool)(!strcmp((char *)code_seq, (char *)other.code_seq) && length == other.length);
    }
} typedef code;

struct Node{
    unsigned int c;
    long priority;
    unsigned long n;
    struct Node *left;
    struct Node *right;
} typedef Node;

class P_Queue{
    public:
        vector<Node *> v;
        unsigned long count; // counts total number of pops called
        P_Queue(){
            count = 0;
            //v = vector<Node *>();
        }
        void push(Node *n){
            n -> n = count++;
            v.push_back(n);
            size_t index = v.size() - 1;
            while(index > 0){
                size_t parent;
                if(v[index] -> priority < v[parent = (index - 1) / 2] -> priority || 
                        (v[index] -> priority < v[parent = (index - 1) / 2] -> priority && v[index] -> n < v[parent] -> n)){
                    v[index] = (Node *)((long)v[parent] ^ (long)v[index]); 
                    v[parent] = (Node *)((long)v[parent] ^ (long)v[index]); 
                    v[index] = (Node *)((long)v[parent] ^ (long)v[index]); 
                }
                index = parent;
            }
        }
        unsigned long size(){
            return v.size();
        }
        Node *pop();
        Node *& operator[](size_t index){
            Node *& result = v[index];
            return result;
        }

        void print(){
            print(0, 0);    
        }
        void print(size_t current, int indent){
            int left = current * 2 + 1;
            int right = (current + 1) * 2;
            if(right < v.size())
                print(right, indent + 4);
            for(int i = 0; i < indent; i++)
                putchar(' ');
            printf("%u %d %lu\n", v[current] -> c, v[current] -> priority, v[current] -> n);
            if(left < v.size())
                print(left, indent + 4);
                            
        }
};

Node *P_Queue::pop(){
    size_t size = v.size();
    if(!size)
        return NULL;
    Node* ret = v[0];

    if(size == 1){
        v.pop_back();
        return ret;
    }
    //swap root with last element
    //printf("First before swap: char: %d, priority: %d\n", v[0] -> c, v[0] -> priority);
    v[0] = (Node *)((long)v[0] ^ (long)v[size - 1]); 
    v[size - 1] = (Node *)((long)v[0] ^ (long)v[size - 1]); 
    v[0] = (Node *)((long)v[0] ^ (long)v[size - 1]); 
    //printf("First after swap: char: %d, priority: %d\n", v[0] -> c, v[0] -> priority);
    // remove last element
    v.pop_back();
    size--;
    // now rotate top element down
    size_t index = 0;
    // while has 2 children and is less than one of them
    while((index + 1) * 2 < size && (v[index] -> priority >= 
                v[index * 2 + 1] -> priority || v[index] -> priority >=
                v[(index + 1) * 2] -> priority)){
        /*
           if((index + 1) * 2 >= size)
           v[index] = (Node *)((long)v[index] ^ (long)v[index * 2 + 1]);
           v[index * 2 + 1] = (Node *)((long)v[index] ^ (long)v[index * 2 + 1]);
           v[index] = (Node *)((long)v[index] ^ (long)v[index * 2 + 1]);
           */
        // if left child has greater priority than right child
        // swap current with left child
        // we know parent should perhaps swap with one
        // need to check for tiebreak
        size_t child;
        long p1 =  v[index * 2 + 1] -> priority;
        long p2 = v[(index + 1) * 2] -> priority; 
        unsigned long n1 = v[index * 2 + 1] -> n;
        unsigned long n2 = v[(index + 1) * 2] -> n;
        // swapt with left child (p1, n1)
        if(p1 < p2 || (p1 == p2 && n1 < n2)){
            child = index * 2 + 1;
            /*
            v[index] = (Node *)((long)v[index] ^ (long)v[index * 2 + 1]);
            v[index * 2 + 1] = (Node *)((long)v[index] ^ (long)v[index * 2 + 1]);
            v[index] = (Node *)((long)v[index] ^ (long)v[index * 2 + 1]);
            index = index * 2 + 1;
            */
        }else{ // else swap with right child
            child = (index + 1) * 2;
            /*
            v[index] = (Node *)((long)v[index] ^ (long)v[(index + 1) * 2]);
            v[(index + 1) * 2] = (Node *)((long)v[index] ^ (long)v[(index + 1) * 2]);
            v[index] = (Node *)((long)v[index] ^ (long)v[(index + 1) * 2]);
            index = (index + 1) * 2;
            */
        }
        if(v[index] -> priority != v[child] -> priority || v[index] -> n < v[child] -> n){
            v[index] = (Node *)((long)v[index] ^ (long)v[child]);
            v[child] = (Node *)((long)v[index] ^ (long)v[child]);
            v[index] = (Node *)((long)v[index] ^ (long)v[child]);
            index = child;
        }else
            break;
    }
    // if v[index] has a left child with a greater priority than it

    //printf("Current: char: %d, priority: %d\n", v[index] -> c, v[index] -> priority);
    size_t left = index * 2 + 1;
    if(left < size && (v[index] -> priority > 
            v[left] -> priority ||
            (v[index] -> priority == 
            v[left] -> priority && v[index] -> n > v[left] -> n))){
        v[index] = (Node *)((long)v[index] ^ (long)v[index * 2 + 1]);
        v[index * 2 + 1] = (Node *)((long)v[index] ^ (long)v[index * 2 + 1]);
        v[index] = (Node *)((long)v[index] ^ (long)v[index * 2 + 1]);
    }
    //print(q);
    return ret;
}
struct code_hasher{
    size_t operator()(const code c) const{
        hash<std::string> h;
        std::string str((char *)c.code_seq); 
        //printf("Code: ");
        //print_code(c);
        //printf("hash: %d\n", h(str) ^ c.length);
        return h(str) ^ c.length;
    }
    
} typedef code_hasher;

struct bit_stream{
    size_t size; // in bits
    size_t capacity; // in bytes
    unsigned char *buffer;
    FILE *fp;
} typedef bit_stream;

struct file_buffer{
    size_t size; // in bytes
    size_t capacity; // in bytes
    unsigned char *buffer;
    FILE *fp;
} typedef file_buffer;
/*
   class P_Queue{
   vector<Node *> *v;
   unsigned long count; // counts total number of pops called
   public:
   P_Queue() : Vector<Node *> *v{};
   };
   */
// priority queue nodes
#endif
