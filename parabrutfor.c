#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include <openssl/sha.h>

#define PASSLEN 5
#define NR_PROC 4
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

void printResult(byte* password, byte* hash) {
	char sPass[PASSLEN + 1];
	memcpy(sPass, password, PASSLEN);
	sPass[PASSLEN] = 0;
	printf("\nA fost decriptată parola << %s >> din cifrul SHA256 următor:\n", sPass);
	for (int i = 0; i < SHA256_DIGEST_LENGTH; i++)
		printf("%02x", hash[i]);
	printf("\n");
}

int main(int argc, char **argv)
{

  FILE *fptr;
  fptr = fopen("dataset.txt", "r");
  if (fptr == NULL) {
    printf("ERROR opening file dataset.txt\n");
    exit(1);
  }

  char buffer[SHA_STRING_LEN + 1];

  while(fscanf(fptr, "%s", buffer) != EOF) {
    for (int a = 0; a < 26; a++)
    {
      byte password[PASSLEN] = { 97 + a };
      byte* str 	=   StringHashToByteArray(buffer);
      for (password[1] = 97; password[1] < 123; password[1]++) {
        byte *hash = SHA256(password, 2, 0);
        if (matches(str, hash)) {
          printResult(password, hash);
          continue;
        }
        else {
          for (password[2] = 97; password[2] < 123; password[2]++) {
            byte *hash = SHA256(password, 3, 0);
            if (matches(str, hash)) {
              printResult(password, hash);
              continue;
            }
            else {
              for (password[3] = 97; password[3] < 123; password[3]++) {
                byte *hash = SHA256(password, 4, 0);
                if (matches(str, hash)) {
                  printResult(password, hash);
                  continue;
                }
                else {
                  for (password[4] = 97; password[4] < 123; password[4]++) {
                    byte *hash = SHA256(password, 5, 0);
                    if (matches(str, hash)) {
                      printResult(password, hash);
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
  fclose(fptr);

	return 0;
}
