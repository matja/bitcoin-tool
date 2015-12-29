## Introduction
bitcoin-tool is a simple tool written in C to convert Bitcoin keys to addresses,
and various other conversions of keys.

Disclaimer: THIS CODE IS EXPERIMENTAL, IT IS PROBABLY BUGGY. PLEASE DO NOT
TRUST YOUR BITCOINS WITH IT.  IT IS EASY TO MAKE AN IRREVERSIBLE MISTAKE AND
SEND YOUR BITCOINS TO A ADDRESS WITH NO WAY OF GETTING THEM BACK.

## Build Instructions
Run `make test` to compile and run all tests.

### Requirements
* A C compiler
* OpenSSL headers and libraries (with elliptic curve support)

## Platform-specific Instructions
### Gentoo Linux
Gentoo by default enables the `bindist` flag on the openssl package, which disables
elliptic curve support, presumably because of software patent concerns.  Since
the `openssh` package also has `bindist` USE-flag set, the USE-flag must be disabled
in both, then re-emerged to get an OpenSSL library with elliptic curve support.

## Description
I created this because I couldn't find an offline tool or library able
to create addresses from Bitcoin private keys, and as a learning exercise in
Bitcoin address formats and ECDSA.

Some day I'd like to replace the dependancy on OpenSSL with my own
implementation of ECDSA.

The option names are a little verbose but I wanted to make it clear exactly what
each one is referring to, especially when it is possible to make a costly
mistake.

I've tried to add as much sanity checking as possible, to remove the scope
for errors and misinterpretation of data.  This sometimes boreders on the
pedantic and annoying.  For example, if the file for `--input-file` contains
more data than is expected, then it'll refuse to process it at all.

### Command-line options
```
  --input-type : Input data type, must be one of :
      mini-private-key : 30 character Casascius mini private key
      private-key      : 32 byte ECDSA private key
      private-key-wif  : 33/34 byte ECDSA WIF private key
      public-key       : 33/65 byte ECDSA public key
      public-key-sha   : 32 byte SHA256(public key) hash
      public-key-rmd   : 20 byte RIPEMD160(SHA256(public key)) hash
      address          : 21 byte Bitcoin address (prefix + hash)
  --input-format : Input data format, must be one of :
      raw         : Raw binary
      hex         : Hexadecimal encoded
      base58      : Base58 encoded
      base58check : Base58Check encoded (most common)
  --output-type  : Output data type, must be one of :
      all              : All output types, as type:value pairs, most of which
                         are never commonly used, probably for good reason.
      mini-private-key : 30 character Casascius mini private key
      private-key      : 32 byte ECDSA private key
      private-key-wif  : 33/34 byte ECDSA WIF private key
      public-key       : 33/65 byte ECDSA public key
      public-key-sha   : 32 byte SHA256(public key) hash
      public-key-rmd   : 20 byte RIPEMD160(SHA256(public key)) hash
      address          : 21 byte Bitcoin address (prefix + hash)
  --output-format : Output data format, must be one of :
      raw         : Raw binary
      hex         : Hexadecimal encoded
      base58      : Base58 encoded
      base58check : Base58Check encoded (most common)

  --input               : Specify input data on command line
  --input-file          : Specify file name to read for input ('-' for stdin)
  --batch               : Read multiple lines of input from --input-file
  --ignore-input-errors : Continue processing batch input if errors are found.

  --public-key-compression : Can be one of :
      auto         : determine compression from base58 private key (default)
      compressed   : force compressed public key
      uncompressed : force uncompressed public key
    (must be specified for raw/hex keys, should be auto for base58)
  --network        : Network type of keys, one of :
      bitcoin
      bitcoin-testnet
      litecoin
      litecoin-testnet
      feathercoin
      feathercoin-testnet
      dogecoin
      dogecoin-testnet
      quarkcoin
      quarkcoin-testnet
      darkcoin
      darkcoin-testnet
      jumbucks
      jumbucks-testnet
  --fix-base58check : Attempt to fix a Base58Check string by changing
                      characters until the checksum matches.
  --fix-base58check-change-chars : Maximum number of characters to change
                                   (default=3)
```
The `mini-private-key` input-type requires --input to be a 30 character ASCII
string in valid mini private key format and --input-format to be `raw`.

If raw keys are input and an address output is required, then the key type
prefix must be specified via --network

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

Convert private key to WIF (Wallet Import Format).  Since it is a raw key, the
network type must be explicitally set (to bitcoin in this case) because it
cannot be determined from the raw key :
```
$ ./bitcoin-tool \
    --network bitcoin \
    --input-type private-key \
    --input-format raw \
    --input-file key.bin \
    --output-type private-key-wif \
    --output-format base58check \
    --public-key-compression uncompressed

5JZjfs5wJv1gNkJXCmYpyj6VxciqPkwmK4yHW8zMmPN1PW7Hk7F
```
Specifying --public-key-compression is mandatory because the WIF output is
different depending on which public key compression type you choose, and there
is no way to guess from a raw private key.

Same again but compressed public key :
```
$ ./bitcoin-tool \
    --network bitcoin \
    --input-type private-key \
    --input-format raw \
    --input-file key.bin \
    --output-type private-key-wif \
    --output-format base58check \
    --public-key-compression compressed

KzXVLY4ni4yznz8LJwdUmNoGpUfebSxiakXRqcGAeuhihzaVe3Rz
```

Note that the WIF private key is longer with public key compression on, because
an extra byte flag is stored to indicate that the public key should be compressed
(the private key is exactly the same).

Show address for uncompressed WIF private key:
```
$ ./bitcoin-tool \
    --input-type private-key-wif \
    --input-format base58check \
    --input 5JZjfs5wJv1gNkJXCmYpyj6VxciqPkwmK4yHW8zMmPN1PW7Hk7F \
    --output-type address \
    --output-format base58check

1KYv3U6gWcxS5UfbNzP25eDEjd5PHHB5Gh
```

Show address for compressed WIF private key:
```
$ ./bitcoin-tool \
    --input-type private-key-wif \
    --input-format base58check \
    --input KzXVLY4ni4yznz8LJwdUmNoGpUfebSxiakXRqcGAeuhihzaVe3Rz \
    --output-type address \
    --output-format base58check

1Lm2DPqbhsutDkKoK9ZPPUkDKnGxQfpJLW
```
This demonstrates why it is necessary to be careful when converting raw private
keys to addresses; the same private key will (almost definitely) result in two
seperate addresses, one for each intermediate form of the public key.

Convert the WIF private key to a QR code so we can print it and import it
easily later:
```
$ qrencode -d 300 -s 3 -l H 5JZjfs5wJv1gNkJXCmYpyj6VxciqPkwmK4yHW8zMmPN1PW7Hk7F -o privkey.png
```

Now you can receive Bitcoins using the address above, but you will need to
import the private key into your wallet at a later time in order to spend them
(`bitcoind importprivkey`, for the official client), or at least be able to
sign transactions with that key (not necessarily online).

#### Generate address from random private key
```
./bitcoin-tool \
    --network bitcoin \
    --input-type private-key \
    --input-format raw \
    --input-file <(openssl rand 32) \
    --output-type address \
    --output-format base58check \
    --public-key-compression compressed
```
This outputs an address you can send Bitcoins to, if you want to loose them forever (because the private key is never output!).

#### Poor-mans brainwallet

Hash a text phrase with SHA256, which is then used as the private key to generate an address from.

**Never use this example for an actual wallet, it will be stolen almost immediately!** (I did a test with another dictionary word and it took all of 4 seconds for someone to steal it!)

This shows the `--output-type all` option, which spews out lots of unnecessary
garbage which I can't imagine would ever be useful, but it does so because it can.
So There.
```
./bitcoin-tool \
    --input-type private-key \
    --input-format raw \
    --input-file <(echo -n sausage|openssl dgst -sha256 -binary) \
    --public-key-compression uncompressed \
    --network bitcoin \
    --output-type all

address.hex:000511096ab078473911e0222fcbc3375314e2bab1
address.base58:156T6Af12SKCQGbjEWNeTkADhJNk
address.base58check:1TnnhMEgic5g4ttrCQyDopwqTs4hheuNZ
public-key-ripemd160.hex:0511096ab078473911e0222fcbc3375314e2bab1
public-key-ripemd160.base58:56T6Af12SKCQGbjEWNeTkADhJNk
public-key-ripemd160.base58check:TnnhMEgic5g4ttrCQyDopwqTs4k6XbAK
public-key-sha256.hex:b17978b7528353483429a758fb9ec833882a5ddbb27c1fc2bb4a66436f7e342f
public-key-sha256.base58:CwnbNMmu9yCkXE32543pfPAgVSynE2wjGYv9Mip4yrb8
public-key-sha256.base58check:2MAMBCve8eVyrbxxBzqn5HLNqqyc8CysKPdfaKPzA81mHxPvyu
public-key.hex:04a32ed011213146495f58d3ed83a6cc3fc0fd107d5fa2887bbc2fcea81e8bc84f650e81f4ddc84424daab546945f0d7d9dfd4dce39ce3776ee6b8ba78e6eddc7a
public-key.base58:QjfX2h4LdAA21NTa2K5dVcxcuQVTtvT3dL5JFLvxAMuCGKY3t8yCKNzJid8MHWbYmoHSRXAS9hggkhQUDiwaaGAV
public-key.base58check:3gKQTqtZhdBHDDe1echja7ac39tup3SnNSzwZSrnHb417QbL7T8JcTfW7GgEQsvhYrPqLsiraabne6xDrSGZ6bBB4S5YGM
private-key-wif.hex:8030caae2fcb7c34ecadfddc45e0a27e9103bd7cfc87730d7818cc096b1266a683
private-key-wif.base58:f5g1GA5uH4gsfEU6ANnGCzoe1VZvnZ1mYh3frnVSPR1nJ
private-key-wif.base58check:5JBmuBc64pVrKLyDc8ktyXJmAeEwKQogn6jsk6taeq8zRMtGZrE
private-key.hex:30caae2fcb7c34ecadfddc45e0a27e9103bd7cfc87730d7818cc096b1266a683
private-key.base58:4HTpd7gVSeVJDurhJKYGEYyFWMZRCNjSnXaEcan9K6Gz
private-key.base58check:NVKW9zzMvs4LawZwJztUZdx3R27Gwc4Hg6WvqqQxHMFkbn3Wz
```

#### Batch processing

You can read multiple lines of input from a text file and process individually
with the `--batch` option.  This requires the `--input-file` option to be
set.  This will be faster than spawning a new instance of bitcoin-tool for
each line of a line - from a shell script, for example.

**Generate 1000 random private keys in hex format**
`keys=1000 ; openssl rand $[32*keys] | xxd -p -c32 > hexkeys`

**Convert all the private keys to addresses**
```
./bitcoin-tool \
--batch \
--input-file hexkeys \
--input-format hex \
--input-type private-key \
--network bitcoin \
--public-key-compression compressed \
--output-type address \
--output-format base58check

