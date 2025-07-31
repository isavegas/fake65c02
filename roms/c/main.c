#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/*int strlen(char* string) {
  int len = 0;
  while (*string) {
    string++;
  }
  return len;
}*/

// This works as a PoC!
int _putchar(char c) {
  __asm__ ("ldy #%o", c);
  __asm__ ("lda (sp),y");
  __asm__ ("sta $8001");
  return 0;
}

char _ia(char c) {
  switch (c) {
    case 0:
      return '0';
    case 1:
      return '1';
    case 2:
      return '2';
    case 3:
      return '3';
    case 4:
      return '4';
    case 5:
      return '5';
    case 6:
      return '6';
    case 7:
      return '7';
    case 8:
      return '8';
    case 9:
      return '9';
    case 10:
      return 'A';
    case 11:
      return 'B';
    case 12:
      return 'C';
    case 13:
      return 'D';
    case 14:
      return 'E';
    case 15:
      return 'F';
  }
  return '!';
}

int main() {
  char* str = "abcdefgh";
  //char* buffer;// = "000000000000000";
  //int i = 0;
  //__asm__ ("lda #$f9");
  //__asm__ ("sta $8000");
  //return 1;
  //buffer = calloc(1, 16);
  //puts("...");
  //utoa(1111111, buffer, 10);
  //puts("abc");
  //puts(buffer);
  //puts("def");
  //for (i = 0; i < 16; i++) {
  //  putchar(_ia(buffer[i]));
  //}
  //putchar('\n');
  //puts("ghi");
  //free(buffer);
  // printf(">%s\n", str);
  // if (puts("Test") == 0xffff) {
  //   puts("Error");
  // } else {
  //   puts("Pass");
  // }
  return 0;
}
