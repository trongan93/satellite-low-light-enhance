#include <iostream>
#include "zeroDCE.h"
using namespace std;
int main() {
    cout << "IN Processing Zero DCE" << endl;
    qDCENet();
    cout <<"DONE, processed the Zero DCE";
    return 0;
}
