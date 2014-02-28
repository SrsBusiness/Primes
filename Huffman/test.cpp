#include <stdio.h>
#include <stdlib.h>
#include "huff.h"

using namespace std;

int main(){
    P_Queue q;
    for(int i = 0; i < 10; i++)
        q.push((Node *)malloc(sizeof(Node)));
    q[0] = q[1];
    printf("%d\n", q[0]);
}
