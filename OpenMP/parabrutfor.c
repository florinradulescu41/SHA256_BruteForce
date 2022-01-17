#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include <openssl/sha.h>

#define PASSLEN 5
#define SHA_STRING_LEN 64

typedef unsigned char byte;

int matches(byte *a, byte* b) {
	for (int i = 0; i < 32; i++)
		if (a[i] != b[i])
			return 0;
	return 1;
}


byte* StringHashToByteArray(const char* s) {
	byte* hash = (byte*) malloc(32);
	char two[3];
	two[2] = 0;
	for (int i = 0; i < 32; i++) {
		two[0] = s[i * 2];
		two[1] = s[i * 2 + 1];
		hash[i] = (byte)strtol(two, 0, 16);
	}
	return hash;
}

int main(int argc, char **argv)
{
  int lines = 0;
  FILE *fptr;
  
  // Select the line for what file to read
  // dataset.txt -> 25s on local / 100s on fep
  // dataset_short.txt -> 5s on local / 25s on fep
  
  fptr = fopen("dataset.txt", "r");
  // fptr = fopen("dataset_short.txt", "r"); 
  
  if (fptr == NULL) {
    printf("ERROR opening file dataset.txt or dataset_short.txt\n");
    exit(1);
  }

  char buffer[SHA_STRING_LEN + 1];
  char passes[50][SHA_STRING_LEN + 1] = { 0 };
  char decrypted[50][SHA_STRING_LEN + 1] = { 0 };
  while(fscanf(fptr, " %s ", buffer) != EOF) {
    strcpy(passes[lines], buffer);
    lines++;
  }

  #pragma omp parallel for
  for (int i = 0; i < lines; i++) {
    for (int a = 0; a < 26; a++)
    {
      byte password[PASSLEN] = { 97 + a };
      byte md[100] = { 0 };
      byte* str	= StringHashToByteArray(passes[i]);
      for (password[1] = 97; password[1] < 123; password[1]++) {
        byte *hash = SHA256(password, 2, md);
        if (matches(str, hash)) {
          strncpy(decrypted[i], password, 2);
          continue;
        }
        else {
          for (password[2] = 97; password[2] < 123; password[2]++) {
            byte *hash = SHA256(password, 3, md);
            if (matches(str, hash)) {
              strncpy(decrypted[i], password, 3);
              continue;
            }
            else {
              for (password[3] = 97; password[3] < 123; password[3]++) {
                byte *hash = SHA256(password, 4, md);
                if (matches(str, hash)) {
                  strncpy(decrypted[i], password, 4);
                  continue;
                }
                else {
                  for (password[4] = 97; password[4] < 123; password[4]++) {
                    byte *hash = SHA256(password, 5, md);
                    if (matches(str, hash)) {
                      strncpy(decrypted[i], password, 5);
                      continue;
                    }
                  }
                }
              }
            }
          }
        }
      }
      free(str);
    }
  }

  for (int i = 0; i < lines; i++) {
    printf("%i: %s - %s\n", i, decrypted[i], passes[i]);
  }
  fclose(fptr);
	return 0;
}
