/*************************** HEADER FILES ***************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
// #include <openssl/sha.h>
#include <memory.h>
#include "sha256.h"
#include <pthread.h>

/****************************** MACROS ******************************/
#define ROTLEFT(a,b) (((a) << (b)) | ((a) >> (32-(b))))
#define ROTRIGHT(a,b) (((a) >> (b)) | ((a) << (32-(b))))

#define CH(x,y,z) (((x) & (y)) ^ (~(x) & (z)))
#define MAJ(x,y,z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define EP0(x) (ROTRIGHT(x,2) ^ ROTRIGHT(x,13) ^ ROTRIGHT(x,22))
#define EP1(x) (ROTRIGHT(x,6) ^ ROTRIGHT(x,11) ^ ROTRIGHT(x,25))
#define SIG0(x) (ROTRIGHT(x,7) ^ ROTRIGHT(x,18) ^ ((x) >> 3))
#define SIG1(x) (ROTRIGHT(x,17) ^ ROTRIGHT(x,19) ^ ((x) >> 10))

#define PASSLEN 5
#define NR_PROC 8
#define SHA_STRING_LEN 64
#define ALPHABET_SIZE 26
#define SHA256_DIGEST_LENGTH 32

/**************************** VARIABLES *****************************/
static const WORD k[64] = {
	0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
	0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
	0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
	0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
	0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
	0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
	0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
	0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
};

int P;
pthread_barrier_t barrier;
pthread_mutex_t lock;

FILE *fptr;

typedef unsigned char byte;


/*********************** FUNCTION DEFINITIONS ***********************/
void sha256_transform(SHA256_CTX *ctx, const BYTE data[])
{
	WORD a, b, c, d, e, f, g, h, i, j, t1, t2, m[64];

	for (i = 0, j = 0; i < 16; ++i, j += 4)
		m[i] = (data[j] << 24) | (data[j + 1] << 16) | (data[j + 2] << 8) | (data[j + 3]);
	for ( ; i < 64; ++i)
		m[i] = SIG1(m[i - 2]) + m[i - 7] + SIG0(m[i - 15]) + m[i - 16];

	a = ctx->state[0];
	b = ctx->state[1];
	c = ctx->state[2];
	d = ctx->state[3];
	e = ctx->state[4];
	f = ctx->state[5];
	g = ctx->state[6];
	h = ctx->state[7];

	for (i = 0; i < 64; ++i) {
		t1 = h + EP1(e) + CH(e,f,g) + k[i] + m[i];
		t2 = EP0(a) + MAJ(a,b,c);
		h = g;
		g = f;
		f = e;
		e = d + t1;
		d = c;
		c = b;
		b = a;
		a = t1 + t2;
	}

	ctx->state[0] += a;
	ctx->state[1] += b;
	ctx->state[2] += c;
	ctx->state[3] += d;
	ctx->state[4] += e;
	ctx->state[5] += f;
	ctx->state[6] += g;
	ctx->state[7] += h;
}

void sha256_init(SHA256_CTX *ctx)
{
	ctx->datalen = 0;
	ctx->bitlen = 0;
	ctx->state[0] = 0x6a09e667;
	ctx->state[1] = 0xbb67ae85;
	ctx->state[2] = 0x3c6ef372;
	ctx->state[3] = 0xa54ff53a;
	ctx->state[4] = 0x510e527f;
	ctx->state[5] = 0x9b05688c;
	ctx->state[6] = 0x1f83d9ab;
	ctx->state[7] = 0x5be0cd19;
}

void sha256_update(SHA256_CTX *ctx, const BYTE data[], size_t len)
{
	WORD i;

	for (i = 0; i < len; ++i) {
		ctx->data[ctx->datalen] = data[i];
		ctx->datalen++;
		if (ctx->datalen == 64) {
			sha256_transform(ctx, ctx->data);
			ctx->bitlen += 512;
			ctx->datalen = 0;
		}
	}
}

void sha256_final(SHA256_CTX *ctx, BYTE hash[])
{
	WORD i;

	i = ctx->datalen;

	// Pad whatever data is left in the buffer.
	if (ctx->datalen < 56) {
		ctx->data[i++] = 0x80;
		while (i < 56)
			ctx->data[i++] = 0x00;
	}
	else {
		ctx->data[i++] = 0x80;
		while (i < 64)
			ctx->data[i++] = 0x00;
		sha256_transform(ctx, ctx->data);
		memset(ctx->data, 0, 56);
	}

	// Append to the padding the total message's length in bits and transform.
	ctx->bitlen += ctx->datalen * 8;
	ctx->data[63] = ctx->bitlen;
	ctx->data[62] = ctx->bitlen >> 8;
	ctx->data[61] = ctx->bitlen >> 16;
	ctx->data[60] = ctx->bitlen >> 24;
	ctx->data[59] = ctx->bitlen >> 32;
	ctx->data[58] = ctx->bitlen >> 40;
	ctx->data[57] = ctx->bitlen >> 48;
	ctx->data[56] = ctx->bitlen >> 56;
	sha256_transform(ctx, ctx->data);

	// Since this implementation uses little endian byte ordering and SHA uses big endian,
	// reverse all the bytes when copying the final state to the output hash.
	for (i = 0; i < 4; ++i) {
		hash[i]      = (ctx->state[0] >> (24 - i * 8)) & 0x000000ff;
		hash[i + 4]  = (ctx->state[1] >> (24 - i * 8)) & 0x000000ff;
		hash[i + 8]  = (ctx->state[2] >> (24 - i * 8)) & 0x000000ff;
		hash[i + 12] = (ctx->state[3] >> (24 - i * 8)) & 0x000000ff;
		hash[i + 16] = (ctx->state[4] >> (24 - i * 8)) & 0x000000ff;
		hash[i + 20] = (ctx->state[5] >> (24 - i * 8)) & 0x000000ff;
		hash[i + 24] = (ctx->state[6] >> (24 - i * 8)) & 0x000000ff;
		hash[i + 28] = (ctx->state[7] >> (24 - i * 8)) & 0x000000ff;
	}
}

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
