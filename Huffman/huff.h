// node for the sorted binary tree
#include <vector>
#include <stdio.h>
using namespace std;

#ifndef HUFF_H
#define HUFF_H

struct code{
    unsigned char *code_seq;
    size_t length;
    size_t  capacity;
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
                if(v[index] -> priority < v[parent = (index - 1) / 2] -> priority){
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
    while((index + 1) * 2 < size && (v[index] -> priority > 
                v[index * 2 + 1] -> priority || v[index] -> priority >
                v[(index + 1) * 2] -> priority)){
        /*
           if((index + 1) * 2 >= size)
           v[index] = (Node *)((long)v[index] ^ (long)v[index * 2 + 1]);
           v[index * 2 + 1] = (Node *)((long)v[index] ^ (long)v[index * 2 + 1]);
           v[index] = (Node *)((long)v[index] ^ (long)v[index * 2 + 1]);
           */
        // if left child has greater priority than right child
        // swap current with left child
        long p1 =  v[index * 2 + 1] -> priority;
        long p2 = v[(index + 1) * 2] -> priority; 
        unsigned long n1 = v[index * 2 + 1] -> n;
        unsigned long n2 = v[(index + 1) * 2] -> n;
        if(p1 < p2 || (p1 == p2 && n1 < n2)){
            v[index] = (Node *)((long)v[index] ^ (long)v[index * 2 + 1]);
            v[index * 2 + 1] = (Node *)((long)v[index] ^ (long)v[index * 2 + 1]);
            v[index] = (Node *)((long)v[index] ^ (long)v[index * 2 + 1]);
            index = index * 2 + 1;
        }else{ // else swap with right child
            v[index] = (Node *)((long)v[index] ^ (long)v[(index + 1) * 2]);
            v[(index + 1) * 2] = (Node *)((long)v[index] ^ (long)v[(index + 1) * 2]);
            v[index] = (Node *)((long)v[index] ^ (long)v[(index + 1) * 2]);
            index = (index + 1) * 2;
        }
    }
    // if v[index] has a left child with a greater priority than it

    //printf("Current: char: %d, priority: %d\n", v[index] -> c, v[index] -> priority);
    if(index * 2 + 1 < size && v[index] -> priority > 
            v[index * 2 + 1] -> priority){
        v[index] = (Node *)((long)v[index] ^ (long)v[index * 2 + 1]);
        v[index * 2 + 1] = (Node *)((long)v[index] ^ (long)v[index * 2 + 1]);
        v[index] = (Node *)((long)v[index] ^ (long)v[index * 2 + 1]);
    }
    //print(q);
    return ret;
}

struct bit_stream{
    size_t size; // in bits
    size_t capacity; // in bytes
    unsigned char *buffer;
    FILE *fp;
} typedef bit_stream;

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
