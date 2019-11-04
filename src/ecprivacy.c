/*
Copyright (c) 2019, Moonsik Park
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the <organization> nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <sodium.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define VERSION "0.1alpha"

#define B64_PUBKEY_LEN 44

#define B64_PRIVKEY_LEN 88

#define PADDING 6

#define SIGNED_MESSAGE_HEADER "===ECPRIVACY SIGNED MESSAGE===\n"

#define SIGNED_MESSAGE_PRIVKEY_AND_SIGNATURE "/s/pub%spub/\n/%s/\n"

#define ENCRYPTED_MESSAGE_HEADER "===ECPRIVACY ENCRYPTED MESSAGE===\n"

#define MESSAGE_FOOTER "===END MESSAGE===\n"



const char *program_name = "ecprivacy " VERSION;

char buffer[4096];
char buffer2[4096];

void
show_help(void)
{
  fprintf(stdout, "Syntax: ecprivacy [command]\n\n"
          "Commands:\n"
          " help                                           Show this help text\n"
          " genkey                                         Generate a new key pair\n"
          " sign    [my privatekey]                        Sign message\n"
          " verify                                         Verify message\n"
          " encrypt [receiver's publickey] [my privatekey] Encrypt message\n"
          " decrypt [my privatekey]                        Decrypt message\n");
}

int
parse_privkey(unsigned char privkey[crypto_sign_SECRETKEYBYTES], char *raw_key)
{
    if(!raw_key || strlen(raw_key) != B64_PRIVKEY_LEN + PADDING) {
        return -1;
    }
    
    raw_key[strlen(raw_key)-3] = '\0';
    raw_key = raw_key + 3;
    sodium_base642bin(privkey, crypto_sign_SECRETKEYBYTES, raw_key, strlen(raw_key), NULL, NULL, NULL, sodium_base64_VARIANT_URLSAFE);
}

int
parse_pubkey(unsigned char pubkey[crypto_sign_PUBLICKEYBYTES], char *raw_key)
{
    if(!raw_key || strlen(raw_key) != B64_PUBKEY_LEN + PADDING) {
        return -1;
    }
    
    raw_key[strlen(raw_key)-3] = '\0';
    raw_key = raw_key + 3;
    sodium_base642bin(pubkey, crypto_sign_PUBLICKEYBYTES, raw_key, strlen(raw_key), NULL, NULL, NULL, sodium_base64_VARIANT_URLSAFE);
}

int
genkey(void)
{
    unsigned char pubkey[crypto_sign_PUBLICKEYBYTES], privkey[crypto_sign_SECRETKEYBYTES];

    if (crypto_sign_keypair(pubkey, privkey) != 0) {
        fprintf(stderr, "Failed to generate keypair.\n");
        return -1;
    }
    puts("A new keypair has been generated.\n");
    printf("Public key: pub%spub\n", sodium_bin2base64(buffer, sizeof(buffer), pubkey, sizeof(pubkey), sodium_base64_VARIANT_URLSAFE));
    printf("Private key: pri%spri\n", sodium_bin2base64(buffer, sizeof(buffer), privkey, sizeof(privkey), sodium_base64_VARIANT_URLSAFE));

    return 0;
}

int
sign(char* message, unsigned char* privkey)
{
    unsigned char sig[crypto_sign_BYTES];

    crypto_sign_detached(sig, NULL, message, strlen(message), privkey);

    unsigned char pubkey[crypto_sign_PUBLICKEYBYTES];
    crypto_sign_ed25519_sk_to_pk(pubkey, privkey);
    puts("Your message has been signed.\n");
    printf(SIGNED_MESSAGE_HEADER
            "%s\n\n"
            SIGNED_MESSAGE_PRIVKEY_AND_SIGNATURE
            MESSAGE_FOOTER,
                message,
                sodium_bin2base64(buffer, sizeof(buffer), pubkey, sizeof(pubkey), sodium_base64_VARIANT_URLSAFE),
                sodium_bin2base64(buffer2, sizeof(buffer2), sig, sizeof(sig), sodium_base64_VARIANT_URLSAFE));
    return 0;
}

int
verify(char* signed_message)
{
    if (signed_message[strlen(signed_message) - 1] != '\n') {
        char* add = signed_message;
        for (;*add;add++);
        *add++ = '\n';
        *add++ = 0;
    }
    if(strncmp(SIGNED_MESSAGE_HEADER, signed_message, strlen(SIGNED_MESSAGE_HEADER)) != 0 ||
        strcmp(signed_message + strlen(signed_message) - strlen(MESSAGE_FOOTER), MESSAGE_FOOTER) != 0) {
        fprintf(stderr, "Message format incorrect. Please check your message.\n");
        return -1;
    }

    char* metadata = strstr(signed_message, "/s/pub");
    
    if (metadata == NULL || strlen(metadata) != 164){
        fprintf(stderr, "Message format incorrect. Please check your message.\n");
        return -1;
    }

    char *data[5];
    int i = 0;

    data[i] = strtok(metadata, "/");

    while(data[i] != NULL || i < 5)
    {
        data[++i] = strtok(NULL, "/");
    }

    unsigned char pubkey[crypto_sign_PUBLICKEYBYTES];

    if(parse_pubkey(pubkey, data[1]) != 0) {
        fprintf(stderr, "Failed to parse public key from message.\n");
        return -1;
    }

    unsigned char sig[crypto_sign_BYTES];

    sodium_base642bin(sig, crypto_sign_BYTES, data[3], strlen(data[3]), NULL, NULL, NULL, sodium_base64_VARIANT_URLSAFE);

    signed_message = signed_message + strlen(SIGNED_MESSAGE_HEADER);
    signed_message[strlen(signed_message) - 4] = '\0';

    if (crypto_sign_verify_detached(sig, signed_message, strlen(signed_message), pubkey) != 0) {
        printf("WARNING! Message is not verified. It might have been tampered with or not signed by the key holder.\n");
        return 1;
    } else {
        printf("Message is verified and is the exact copy.\n");
        return 0;
    }
}

int
main(int argc, char** argv)
{
    fprintf(stdout, "%s\n\n", program_name);

    if (sodium_init() < 0) {
        fprintf(stderr, "libsodium can't be initalized. Exiting\n");
        return -1;
    }

    if(argc < 2) {
        show_help();
        return 0;
    }

    if (strcmp(argv[1], "help") == 0) {
        show_help();
        return 0;
    }

    if (strcmp(argv[1], "genkey") == 0) {
        return genkey();
    }

    if (strcmp(argv[1], "sign") == 0) {
        if (argc < 3) {
            puts("Please specify a private key to sign the message.\n");
            return -1;
        }
        if (argc > 3) {
            puts("Too many arguments.\n");
            return -1;
        }

        char *message = NULL;
        size_t size = 0;
        puts("Please enter the message; To finish, press Enter, then EOF(Ctrl + D).\n");

        ssize_t result = getdelim(&message, &size, '\x1A', stdin);

        puts("\n");
        unsigned char privkey[crypto_sign_SECRETKEYBYTES];

        if (parse_privkey(privkey, argv[2]) != 0){
            puts("Failed to parse private key.");
        }

        int ret = sign(message, privkey);
        
        free(message);

        return ret;
    }

    if (strcmp(argv[1], "verify") == 0){
        char *message = NULL;
        size_t size = 0;
        puts("Please enter the message to verify; To finish, press Enter, then EOF(Ctrl + D).\n");

        ssize_t result = getdelim(&message, &size, '\x1A', stdin);

        puts("\n");

        
        int ret = verify(message);
        
        free(message);

        return ret;
    }

}