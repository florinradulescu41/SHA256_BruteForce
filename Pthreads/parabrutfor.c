#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sha256.h"
#include <pthread.h>


#define PASSLEN 5
#define NR_PROC 8
#define SHA_STRING_LEN 64
#define ALPHABET_SIZE 26
#define SHA256_DIGEST_LENGTH 32


int P;
pthread_barrier_t barrier;
pthread_mutex_t lock;

FILE *fptr;

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

void *thread_function(void *arg) {

	int thread_id = *(int *)arg;

	// double arr = (double) ALPHABET_SIZE / P;
	// int last = ((thread_id + 1) * arr);
	//
	// int start = thread_id * arr;
	// int end = (ALPHABET_SIZE < last ? ALPHABET_SIZE : last);

	char buffer[SHA_STRING_LEN + 1];

	while(1){
		pthread_mutex_lock(&lock);
		if (fscanf(fptr, "%s", buffer) != EOF) {
				printf("Am citit %s în thread %d\n", buffer, thread_id);
				pthread_mutex_unlock(&lock);
				for (int a = 0; a < ALPHABET_SIZE; a++)
				{
					byte password[PASSLEN] = { 97 + a };
					byte* str = StringHashToByteArray(buffer);

					byte md[1000] = {0};

					for (password[1] = 97; password[1] < 123; password[1]++) {
						byte hash[SHA256_BLOCK_SIZE];
						SHA256_CTX ctx;
						sha256_init(&ctx);
						sha256_update(&ctx, password, 2);
						sha256_final(&ctx, hash);
						if (matches(str, hash)) {
							printResult(password, hash);
							continue;
						}
						else {
							for (password[2] = 97; password[2] < 123; password[2]++) {
								byte hash[SHA256_BLOCK_SIZE];
								SHA256_CTX ctx;
								sha256_init(&ctx);
								sha256_update(&ctx, password, 3);
								sha256_final(&ctx, hash);
								if (matches(str, hash)) {
									printResult(password, hash);
									continue;
								}
								else {
									for (password[3] = 97; password[3] < 123; password[3]++) {
										byte hash[SHA256_BLOCK_SIZE];
										SHA256_CTX ctx;
										sha256_init(&ctx);
										sha256_update(&ctx, password, 4);
										sha256_final(&ctx, hash);
										if (matches(str, hash)) {
											printResult(password, hash);
											continue;
										}
										else {
											for (password[4] = 97; password[4] < 123; password[4]++) {
											byte hash[SHA256_BLOCK_SIZE];
											SHA256_CTX ctx;
											sha256_init(&ctx);
											sha256_update(&ctx, password, 5);
											sha256_final(&ctx, hash);
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
		else {
			pthread_mutex_unlock(&lock);
			pthread_exit(NULL);
		}
	}
	pthread_exit(NULL);
}

int main(int argc, char **argv)
{

  // Select the line for what file to read
  // dataset.txt -> 25s on local / 100s on fep
  // dataset_short.txt -> 5s on local / 25s on fep

  fptr = fopen("dataset.txt", "r");
	// fptr = fopen("dataset_short.txt", "r");

  if (fptr == NULL) {
    printf("ERROR opening file dataset.txt or dataset_short.txt\n");
    exit(1);
  }

	if (pthread_mutex_init(&lock, NULL) != 0) {
    printf("\n mutex init has failed\n");
    return 1;
  }

  // char buffer[SHA_STRING_LEN + 1];

	int thread_id[NR_PROC];
	pthread_t tid[NR_PROC];
	P = NR_PROC;

	pthread_barrier_init(&barrier, NULL, P);

	for (int i = 0; i < P; i++) {
		thread_id[i] = i;
		pthread_create(&tid[i], NULL, thread_function, &thread_id[i]);
	}

	for (int i = 0; i < P; i++) {
		pthread_join(tid[i], NULL);
	}

	fclose(fptr);
	pthread_mutex_destroy(&lock);


	return 0;
}
