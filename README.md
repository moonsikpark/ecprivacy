# ECPrivacy

## Why

> Grandpa, what did you do when PGP was bloated intentionally to block people from using encryption?

## Warning

Not tested at all. Do not use this as a replacement for battle-tested programs.

## Encryption

ECPrivacy uses [Eliptic Curve Cryptography](https://en.wikipedia.org/wiki/Elliptic-curve_cryptography) with libsodium.

For signing and verifying, a twisted edwards curve variant Ed25519 is used. For encryption and decryption, a Montgomery curve variant Curve25519 implementation X25519, stream cipher XSalsa20, and Poly1305 MAC is used. Since Curve25519 is birationally equivalent with Ed25519, a single key pair can be used for both encryption and sign operation.

## Install

You need gcc, libsodium and libsodium headers to compile.

    dnf install gcc libsodium libsodium-devel
    git clone https://github.com/moonsikpark/ecprivacy.git
    cd ecprivacy
    gcc -o ecprivacy src/ecprivacy.c -lsodium
    cp ecprivacy /usr/bin

## Syntax

    Syntax: ecprivacy [command]

    Commands:
     help                                           Show this help text
     genkey                                         Generate a new key pair
     sign    [my privatekey]                        Sign message
     verify                                         Verify message
     encrypt [receiver's publickey] [my privatekey] Encrypt message
     decrypt [my privatekey]                        Decrypt message

## Usage

### Keypair generation
    [alice@test ~]# ecprivacy genkey
    ecprivacy 0.1alpha

    A new keypair has been generated.

    Public key: pubI2GqdwLLa9io9ofTsDG6h4MMLtHRuo5ptMlQSGIZ3LQ=pub
    Private key: privHfANEdqs0o7j2lCjkO8aBCGfAdLwgu0Ga7wzp43vdQjYap3Astr2Kj2h9OwMbqHgwwu0dG6jmm0yVBIYhnctA==pri

### Signing

    [alice@test ~]# ecprivacy sign privHfANEdqs0o7j2lCjkO8aBCGfAdLwgu0Ga7wzp43vdQjYap3Astr2Kj2h9OwMbqHgwwu0dG6jmm0yVBIYhnctA==pri
    ecprivacy 0.1alpha

    Please enter the message; To finish, press Enter, then EOF(Ctrl + D).

    Hello World!


    Your message has been signed.

    ===ECPRIVACY SIGNED MESSAGE===
    Hello World!


    /s/pubI2GqdwLLa9io9ofTsDG6h4MMLtHRuo5ptMlQSGIZ3LQ=pub/
    /xvNqy8aV25ihNTDzVG9oDN1RqVRBGA16Jv-WoKfZMXQYPJtqiM0NP5j4F5eTMl0OA2AagxXp13wo32d739RoAQ==/
    ===END MESSAGE===

### Verifying

    [bob@test ~]# ecprivacy verify
    ecprivacy 0.1alpha

    Please enter the message to verify; To finish, press Enter, then EOF(Ctrl + D).

    ===ECPRIVACY SIGNED MESSAGE===
    Hello World!


    /s/pubI2GqdwLLa9io9ofTsDG6h4MMLtHRuo5ptMlQSGIZ3LQ=pub/
    /xvNqy8aV25ihNTDzVG9oDN1RqVRBGA16Jv-WoKfZMXQYPJtqiM0NP5j4F5eTMl0OA2AagxXp13wo32d739RoAQ==/
    ===END MESSAGE===


    Message is verified and is the exact copy.
    [bob@test ~]# ecprivacy verify
    ecprivacy 0.1alpha

    Please enter the message to verify; To finish, press Enter, then EOF(Ctrl + D).

    ===ECPRIVACY SIGNED MESSAGE===
    Hello World.


    /s/pubI2GqdwLLa9io9ofTsDG6h4MMLtHRuo5ptMlQSGIZ3LQ=pub/
    /xvNqy8aV25ihNTDzVG9oDN1RqVRBGA16Jv-WoKfZMXQYPJtqiM0NP5j4F5eTMl0OA2AagxXp13wo32d739RoAQ==/
    ===END MESSAGE===


    WARNING! Message is not verified. It might have been tampered with or not signed by the key holder.

### Encrypting

    [bob@test ~]# ecprivacy genkey
    ecprivacy 0.1alpha

    A new keypair has been generated.

    Public key: pub7JTQXp02LDNzTBK7EPGYxwfVzZcXNy8En05sp6Rw0p4=pub
    Private key: priUbhSdleGCS2cXJhV6NS5g2tzlSd0hnNzJ3wKZ3Kw9KXslNBenTYsM3NMErsQ8ZjHB9XNlxc3LwSfTmynpHDSng==pri

    [alice@test ~]# ecprivacy encrypt pub7JTQXp02LDNzTBK7EPGYxwfVzZcXNy8En05sp6Rw0p4=pub privHfANEdqs0o7j2lCjkO8aBCGfAdLwgu0Ga7wzp43vdQjYap3Astr2Kj2h9OwMbqHgwwu0dG6jmm0yVBIYhnctA==pri
    ecprivacy 0.1alpha

    Please enter the message to encrypt; To finish, press Enter, then EOF(Ctrl + D).

    Hello World!


    Your message has been encrypted.

    ===ECPRIVACY ENCRYPTED MESSAGE===
    ZWFocmd0cmRlZmhiZHJzZnRoYnRmcmRhd3JzZWdiaG
    FTUldFaGRndHllYXJobmJkdHJneWhqYXRlZGdmaGIg
    ZnhkZXRhIG5qaGJlYXQ0NmpodWFldGQ0aGJkZnRlcj
    VnaGVkcnQ1aA==
    ===END MESSAGE===
    
### Decrypting

    [bob@test ~]# ecprivacy decrypt priUbhSdleGCS2cXJhV6NS5g2tzlSd0hnNzJ3wKZ3Kw9KXslNBenTYsM3NMErsQ8ZjHB9XNlxc3LwSfTmynpHDSng==pri
    ecprivacy 0.1alpha

    Please enter the message to decrypt; To finish, press Enter, then EOF(Ctrl + D).

    ===ECPRIVACY ENCRYPTED MESSAGE===
    ZWFocmd0cmRlZmhiZHJzZnRoYnRmcmRhd3JzZWdiaG
    FTUldFaGRndHllYXJobmJkdHJneWhqYXRlZGdmaGIg
    ZnhkZXRhIG5qaGJlYXQ0NmpodWFldGQ0aGJkZnRlcj
    VnaGVkcnQ1aA==
    ===END MESSAGE===


    Your message has been decrypted.

    Hello World!
