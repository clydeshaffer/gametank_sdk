static char tmp;
void delayMicroseconds(char c) {
    tmp = c;
    while(tmp) {
        --tmp;
    }
}

char cabs(char x) {
    if(x & 128) return -x;
    return x;
}