#include <bits/stdc++.h>
using namespace std;

int main() {
    string S;
    cin >> S;

    for (int i = 0; i < S.length(); i++) {
        if (S[i] >= 'A' && S[i] <= 'Z') {
            S[i] = S[i] + 32;
        } 
        else if (S[i] >= 'a' && S[i] <= 'z') {
            S[i] = S[i] - 32;
        }
    }

    cout << S << endl;
    return 0;
}