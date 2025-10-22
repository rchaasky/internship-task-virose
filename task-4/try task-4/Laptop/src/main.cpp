#include "Laptop.h"

int main() {
    Laptop laptop("data/data.json");

    laptop.readJSON();
    laptop.splitData();
    laptop.sendToESPBridge();
    
    return 0;
}