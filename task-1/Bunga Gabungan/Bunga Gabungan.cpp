#include <iostream> 
using namespace std; 

int main (){ 
    int P; 
    int Q; 
    cin >> P; 
    cin >> Q; 
    
    int total_bunga = P*P+Q*Q+1; 
    int banyak_bunga = total_bunga/4;

    if (total_bunga % 4 == 0){ 
        cout << total_bunga << endl; 
    }
    else { 
        cout << -1 << endl;
    } 

    return 0;
}