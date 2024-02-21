/*
static unsigned serial_out = 0;
void o(unsigned char c) {
    serial_out = c;
    __asm__ ("lda %v", serial_out);
    __asm__ ("sta $8002");
}*/

int main(){
  int a = 1;
  if (a == 0) {
    return 31;
  } else {
    return 0;
  }
}
