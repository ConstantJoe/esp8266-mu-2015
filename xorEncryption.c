#include <stdio.h>
#include <stdlib.h>
#include <string.h>

unsigned int key;
time_t t;

const char *byte_to_binary(int x)
{
    static char b[9];
    b[0] = '\0';

    int z;
    for (z = 128; z > 0; z >>= 1)
    {
        strcat(b, ((x & z) == z) ? "1" : "0");
    }

    return b;
}

void createKey(){
	srand((unsigned) time(&t));
	key = rand() % 255;
}

char *xorEncrypt(char* str, int len){
	char* encrypted = (char*)malloc(len);
	int i;

	for(i=0;i<len;i++){		
		//XOR it with key
		encrypted[i] = str[i] ^ key;
	}
	return encrypted;
}

char *xorDecrypt(char* str, int len){
	char* decrypted = (char*)malloc(len);
	int i;

	for(i=0;i<len;i++){		
		//XOR it with key
		decrypted[i] = str[i] ^ key;
	}
	return decrypted;
}

int main()
{
	char str[] = "Hello World";

	createKey();
	
	printf("String: %s\n", str);
	printf("Key: %d\n", key);
	char* enc = xorEncrypt(str, (int) sizeof(str));
	printf("Encrypted: %s\n", enc);

	char* dec = xorDecrypt(enc, (int) sizeof(str));
	printf("Decrypted: %s\n", dec);
	
	free(enc);
	free(dec);

	return 0;
}
