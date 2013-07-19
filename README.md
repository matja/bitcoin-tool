Simple tool in C to convert Bitcoin addresses and keys.

I created this I couldn't find a offline tool or library able
to create addresses from Bitcoin private keys, and as a
learning exercise in ECDSA.

Some day I'd like to replace the dependancy on OpenSSL with
my own implementation of ECDSA.
	
## Command-line options

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

== Examples ==

Show address for specified base58 private key
    --input-type private-key \\
    --input-format base58check \\
    --input 5J2YUwNA5hmZFW33nbUCp5TmvszYXxVYthqDv7axSisBjFJMqaT \\
    --output-type address \\
    --output-format base58check
 
Show address for random private key (generate random address)
    --input-type private-key \\
    --input-format raw \\
    --input-file <\(openssl rand 64\) \\
    --output-type address \\
    --output-format base58check

Show hex public key for SHA256-hashed string used as private key
    --input-type private-key \\
    --input-format raw \\
    --input-file <\(echo -n sausage|openssl dgst -sha256 -binary\) \\
    --output-type public-key \\
    --output-format hex
 
