Simple tool in C to convert Bitcoin addresses and keys.

Compile with `make`

I created this I couldn't find a offline tool or library able
to create addresses from Bitcoin private keys, and as a
learning exercise in ECDSA.

Some day I'd like to replace the dependancy on OpenSSL with
my own implementation of ECDSA.

### Command-line options

    --input-type : Input data type, can be one of :
        private-key     : ECDSA private key
        public-key      : ECDSA public key
        public-key-sha  : SHA256(public key)
        public-key-rmd  : RIPEMD160(SHA256(public key))
        address         : Bitcoin address (version + hash)

    --input-format : Input data format, can be one of :
        raw             : raw binary data
        hex             : hexadecimal encoded
        base58check     : Base58Check encoded

    --output-type   Output data type, can be one of :
        private-key     : ECDSA private key
        public-key      : ECDSA public key
        public-key-sha  : SHA256(public key)
        public-key-rmd  : RIPEMD160(SHA256(public key))
        address         : Bitcoin address (version + hash)
        all             : all output types as type:value

    --output-format Output data format, can be one of :
        raw             : raw binary data
        hex             : hexadecimal encoded
        base58check     : Base58Check encoded

    --input         Specify input data - can be raw data, private key, public or address
    --input-file    Specify input file name

### Examples

#### Manual address / key generation

Let's manually generate a Bitcoin address and private key for the purpose of an offline wallet (cold storage).

Create private key:
```
$ openssl rand 32 > key.bin
```

Inspect private key:
```
$ hexdump -e '32/1 "%02X" "\n"' key.bin

62A87AD3272B41E67108FEA10C57BA6ED609F2F7A2264A83B690CD45707090D1
```

Convert private key to WIF (Wallet Import Format):
```
$ ./bitcoin-tool \
    --input-type private-key \
    --input-format raw \
    --input-file key.bin \
    --output-type private-key \
    --output-format base58check

5JZjfs5wJv1gNkJXCmYpyj6VxciqPkwmK4yHW8zMmPN1PW7Hk7F
```

Show address for WIF private key:
```
$ ./bitcoin-tool \
    --input-type private-key \
    --input-format base58check \
    --input 5JZjfs5wJv1gNkJXCmYpyj6VxciqPkwmK4yHW8zMmPN1PW7Hk7F \
    --output-type address \
    --output-format base58check
    
1KYv3U6gWcxS5UfbNzP25eDEjd5PHHB5Gh    
```

Convert the WIF private key to a QR code so we can print it and import it easily later:
```
$ qrencode -d 300 -s 3 -l H 5JZjfs5wJv1gNkJXCmYpyj6VxciqPkwmK4yHW8zMmPN1PW7Hk7F -o privkey.png
```

Now you can receive Bitcoins using the address above, but you will need to import the private key at a later time in order to spend them. (`bitcoind importprivkey`, for the official client)
 
#### Generate address from random private key
```
./bitcoin-tool \
    --input-type private-key \
    --input-format raw \
    --input-file <(openssl rand 64) \
    --output-type address \
    --output-format base58check
```
This outputs an address you can send Bitcoins to, if you want to loose them forever (because the private key is never output!).

#### Poor-mans brainwallet

Hash a text phrase with SHA256, which is then used as the private key to generate an address from.

**Never use this example for an actual wallet, it will be stolen almost immediately!**
```
./bitcoin-tool \
    --input-type private-key \
    --input-format raw \
    --input-file <(echo -n sausage|openssl dgst -sha256 -binary) \
    --output-type public-key \
    --output-format hex
```

