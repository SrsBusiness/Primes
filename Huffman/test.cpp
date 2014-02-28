#include <stdio.h>
#include <vector>

using namespace std;

int main(){
    vector<int> v;
    for(int i = 0; i < 10; i++)
        v.push_back(i);
    v[0] = v[1];
    printf("%d\n", v[0]);
}
