#!/usr/bin/env bash

BITCOIN_TOOL="./bitcoin-tool"

check () {
	if [ "$2" != "$3" ] ; then
		echo "failed test $1"
		echo "output   : [$2]"
		echo "expected : [$3]"
		return 1
	else
		echo "pass $1"
	fi
}

checkfail () {
	if [ $? -eq 0 ] ; then
		echo "failed test $1"
		echo "expected to return >0 (error) but it returned 0 (success)"
		return 1
	else
		echo "pass $1"
	fi
}

# -----------------------------------------------------------------------------
TEST="1 - convert Casascius mini private key format to address"
NOTE="from example at https://en.bitcoin.it/wiki/Mini_private_key_format"
EXPECTED="1CciesT23BNionJeXrbxmjc7ywfiyM4oLW"
INPUT="S6c56bnXQiBjk9mqSYE7ykVQ7NzrRy"
OUTPUT=$($BITCOIN_TOOL \
	--input-type mini-private-key \
	--input-format raw \
	--input "${INPUT}" \
	--output-type address \
	--output-format base58check )
check "${TEST}" "${OUTPUT}" "${EXPECTED}" || exit 1
# -----------------------------------------------------------------------------
TEST="2 - hex private key to compressed address"
EXPECTED="1BgGZ9tcN4rm9KBzDn7KprQz87SZ26SAMH"
OUTPUT=$($BITCOIN_TOOL \
	--input-type private-key \
	--input-format hex \
	--output-type address \
	--output-format base58check \
	--public-key-compression compressed \
	--network bitcoin \
	--input 0000000000000000000000000000000000000000000000000000000000000001 )
check "${TEST}" "${OUTPUT}" "${EXPECTED}" || exit 1
# -----------------------------------------------------------------------------
TEST="3 - hex private key to uncompressed address"
EXPECTED="1EHNa6Q4Jz2uvNExL497mE43ikXhwF6kZm"
OUTPUT=$($BITCOIN_TOOL \
	--input-type private-key \
	--input-format hex \
	--output-type address \
	--output-format base58check \
	--public-key-compression uncompressed \
	--network bitcoin \
	--input 0000000000000000000000000000000000000000000000000000000000000001 )
check "${TEST}" "${OUTPUT}" "${EXPECTED}" || exit 1
# -----------------------------------------------------------------------------
TEST="4 - WIF uncompressed private key to address"
EXPECTED="16SK7HnxBMRxSpLhhdf8RYcqv8MPJiSF6Q"
OUTPUT=$($BITCOIN_TOOL \
	--input-type private-key-wif \
	--input-format base58check \
	--output-type address \
	--output-format base58check \
	--input 5J1LYLWqNxJBTwdGAmzYnpkqqSuFu48fsHv8jgojFMV2Z8exk9L )
check "${TEST}" "${OUTPUT}" "${EXPECTED}" || exit 1
# -----------------------------------------------------------------------------

TEST="6 - raw private key file, to uncompressed base58check WIF private key"
EXPECTED="5JZjfs5wJv1gNkJXCmYpyj6VxciqPkwmK4yHW8zMmPN1PW7Hk7F"
INPUT="62A87AD3272B41E67108FEA10C57BA6ED609F2F7A2264A83B690CD45707090D1"
OUTPUT=$($BITCOIN_TOOL \
	--input-type private-key \
	--input-format raw \
	--input-file <(echo "${INPUT}"|xxd -r -p) \
	--output-type private-key-wif \
	--output-format base58check \
	--public-key-compression uncompressed \
	--network bitcoin \
)
check "${TEST}" "${OUTPUT}" "${EXPECTED}" || exit 1
# -----------------------------------------------------------------------------
TEST="7 - raw private key file, to compressed base58check WIF private key"
EXPECTED="KzXVLY4ni4yznz8LJwdUmNoGpUfebSxiakXRqcGAeuhihzaVe3Rz"
INPUT="62A87AD3272B41E67108FEA10C57BA6ED609F2F7A2264A83B690CD45707090D1"
OUTPUT=$($BITCOIN_TOOL \
	--input-type private-key \
	--input-format raw \
	--input-file <(echo "${INPUT}"|xxd -r -p) \
	--output-type private-key-wif \
	--output-format base58check \
	--public-key-compression compressed \
	--network bitcoin \
)
check "${TEST}" "${OUTPUT}" "${EXPECTED}" || exit 1
# -----------------------------------------------------------------------------
TEST="8 - hex ripemd160 hash of public key, to base58check address"
EXPECTED="1A1zP1eP5QGefi2DMPTfTL5SLmv7DivfNa"
INPUT="62E907B15CBF27D5425399EBF6F0FB50EBB88F18"
OUTPUT=$($BITCOIN_TOOL \
	--input-type public-key-rmd \
	--input-format hex \
	--input "${INPUT}" \
	--output-type address \
	--output-format base58check \
	--network bitcoin \
)
check "${TEST}" "${OUTPUT}" "${EXPECTED}" || exit 1
# -----------------------------------------------------------------------------
TEST="9 - base58check address, to hex ripemd160 hash of public key"
EXPECTED="62e907b15cbf27d5425399ebf6f0fb50ebb88f18"
INPUT="1A1zP1eP5QGefi2DMPTfTL5SLmv7DivfNa"
OUTPUT=$($BITCOIN_TOOL \
	--input-type address \
	--input-format base58check \
	--input "${INPUT}" \
	--output-type public-key-rmd \
	--output-format hex )
check "${TEST}" "${OUTPUT}" "${EXPECTED}" || exit 1
# -----------------------------------------------------------------------------
TEST="10 - fix base58check address, by changing 1 character"
EXPECTED="1NaqSiNC4tfbyX42NGca24pBWvJ5L4Bd5J"
INPUT="1A1zP1eP5QGefi2DMPTfTL5SLmv7DivfNa"
OUTPUT=$($BITCOIN_TOOL \
	--input-type private-key-wif \
	--input-format base58check \
	--output-type address \
	--output-format base58check \
	--input 5J5sKGFLpZ4bQXEHiEmDp9Fuf7k36FqF3WoaNKHKDHnLfJYnkUR \
	--fix-base58check )
check "${TEST}" "${OUTPUT}" "${EXPECTED}" || exit 1
# -----------------------------------------------------------------------------
TEST="11 - convert 'Hash 160' to address"
EXPECTED="12c6DSiU4Rq3P4ZxziKxzrL5LmMBrzjrJX"
INPUT="119b098e2e980a229e139a9ed01a469e518e6f26"
OUTPUT=$($BITCOIN_TOOL \
	--input-type public-key-rmd \
	--output-type address \
	--input-format hex \
	--network bitcoin \
	--output-format base58check \
	--input "${INPUT}")
check "${TEST}" "${OUTPUT}" "${EXPECTED}" || exit 1
# -----------------------------------------------------------------------------
TEST="12 - convert 'SHA256' to address"
EXPECTED="1JNC98D5LZbrGHFR8shDwiqLPGfpg15BUM"
INPUT="904b8a01c68095a9e825d28082c04b75b1f56277648256985717620e8913b79b"
OUTPUT=$($BITCOIN_TOOL \
        --input-type public-key-sha \
        --output-type address \
        --input-format hex \
        --network bitcoin \
        --output-format base58check \
        --input "${INPUT}")
check "${TEST}" "${OUTPUT}" "${EXPECTED}" || exit 1
# -----------------------------------------------------------------------------
TEST="13 - convert WIF private key to address bech32"
EXPECTED="bc1qhmc0vk4xzr37ayv7tlyhns7x4dk04tyvflk8ey"
INPUT="L3GzRAGwCqfSNFr6g1NQm7edn29DgAKZJ6owUBqYELpP6Kbim5kM"
OUTPUT=$($BITCOIN_TOOL \
        --input-type private-key-wif \
        --output-type address \
        --input-format base58check \
        --network bitcoin \
        --output-format bech32 \
        --input "${INPUT}")
check "${TEST}" "${OUTPUT}" "${EXPECTED}" || exit 1
# -----------------------------------------------------------------------------
TEST="14 - bech32 address, to hex ripemd160 hash of public key"
EXPECTED="bef0f65aa610e3ee919e5fc979c3c6ab6cfaac8c"
INPUT="bc1qhmc0vk4xzr37ayv7tlyhns7x4dk04tyvflk8ey"
OUTPUT=$($BITCOIN_TOOL \
	--input-type address \
	--input-format bech32 \
	--input "${INPUT}" \
	--output-type public-key-rmd \
	--output-format hex )
check "${TEST}" "${OUTPUT}" "${EXPECTED}" || exit 1
# -----------------------------------------------------------------------------
TEST="15 - convert WIF private key to ripemd160 hash of public key"
EXPECTED="bef0f65aa610e3ee919e5fc979c3c6ab6cfaac8c"
INPUT="L3GzRAGwCqfSNFr6g1NQm7edn29DgAKZJ6owUBqYELpP6Kbim5kM"
OUTPUT=$($BITCOIN_TOOL \
        --input-type private-key-wif \
        --output-type public-key-rmd \
        --input-format base58check \
        --network bitcoin \
        --output-format hex \
        --input "${INPUT}")
check "${TEST}" "${OUTPUT}" "${EXPECTED}" || exit 1
# -----------------------------------------------------------------------------
TEST="16 - private key to uncompressed public key to bech32 should fail"
OUTPUT=$($BITCOIN_TOOL \
	--input-type private-key \
	--input-format hex \
	--input 00112233445566778899aabbccddeeff00112233445566778899aabbccddeeff \
	--public-key-compression uncompressed \
	--network bitcoin \
	--output-type address \
	--output-format bech32)
checkfail "${TEST}" || exit 1
# -----------------------------------------------------------------------------
TEST="17 - 0-padded base58 decoding prefix initialization"
# test all prefix-0x00 addresses from all byte 0x00 to all byte 0xff
# test all prefix-0x05 addresses from all byte 0x00 to all byte 0xff
# test initialization of the address prefix byte with a leading-zero address
OUTPUT=$($BITCOIN_TOOL \
	--batch \
	--input-type address \
	--input-format base58check \
	--output-type public-key-rmd \
	--output-format hex \
	--network bitcoin \
	--input-file <(
cat << EOF
31h1vYVSYuKP6AhS86fbRdMw9XHieotbST
31h1vYVSYuKP6AhS86fbRdMw9XJDQPwwHs
31h1vYVSYuKP6AhS86fbRdMw9ZVCQhvrL5
31h1vYVSYuKP6AhS86fbRdMwKDxjLM8dc6
31h1vYVSYuKP6AhS86fbRdNfy3VH8G4Sg9
31h1vYVSYuKP6AhS86fbRgcvtpKpcWLBoH
31h1vYVSYuKP6AhS86fbg1YpEiaWZGh5rt
31h1vYVSYuKP6AhS86gguZPS5MQxGC1QE1
31h1vYVSYuKP6AhS8BVni5ycjwEYTkpSdW
31h1vYVSYuKP6AhSVRPQ56rh3DHErpieSv
31h1vYVSYuKP6Aj4EjBUpzHfJ3Bbeobh3f
31h1vYVSYuKP6HrqPGaUXzVxjUHWRShSQS
31h1vYVSYuKPdnC75r5BWvnvzGNThRohnd
31h1vYVSYuMncqFQBgn4pZhEHgEaSjuN1c
31h1vYVSZ5wESKnsxfEWfSXb91S8EpSRiP
31h1vYVTMmZC4JjUTqxaPoycskQdbaPPEF
31h1vYZ1QeGgsdhmHBd3QUsHXRpBQhqGxd
31h1vpEU6VRsmN9zDyQGN8usqMkYNx1zq7
31h382QFyTyx6ju8431U24MJyRh2Sezzsw
31nJbjcky5Z5ciHgBxNuPvcGapAkXv1ZMT
3R2cuenjG5nFubqX9Wzuukdin2YfBbQ6Kw
1111111111111111111114oLvT2
11111111111111111111Vp5gvNh
11111111111111111113CUpEqkk2
111111111111111111Ahg1nPYUbR
11111111111111111jpXCZcHpsAq
11111111111111114FzkJ371115Z2
111111111111111FPBt6CHnzxAWAZ
1111111111111126Uw2Vvq8EiC1ZQH
11111111111115qCHTcgbQwpuTX8cA
111111111111NKioeUVktgzXJkQp7x
111111111112d7dWtQMvj9Wtt7EvWou
11111111118AQGAut7N92awznrLKxEs
111111111YcVfxkQb6JRzqk5kEMSuu3
111111113QXfYy4b7UPwLJ99wrsF6tRK
1111111BcrMA6SqZZvEpAezV9QmS2UWs
111111osEoy933LkHyyBcgjE7v5mHW44
111114ZrjxJnU1LA5xSyrWMNuXTtz1Cfb
1111Gk2Yb7VgCTZ6sjfwWYwgqTpqCjdnG
1112CUupRZfa1aCgvwLsbRzNpuQJpnvta6
116HgC8KRBEhXYbF4riJyJFLSHt31kYasP
1QLbz7JHiBTspS962RLKV8GndWFwi5j6Qr
EOF
))
EXPECTED='0000000000000000000000000000000000000000
00000000000000000000000000000000000000ff
000000000000000000000000000000000000ffff
0000000000000000000000000000000000ffffff
00000000000000000000000000000000ffffffff
000000000000000000000000000000ffffffffff
0000000000000000000000000000ffffffffffff
00000000000000000000000000ffffffffffffff
000000000000000000000000ffffffffffffffff
0000000000000000000000ffffffffffffffffff
00000000000000000000ffffffffffffffffffff
000000000000000000ffffffffffffffffffffff
0000000000000000ffffffffffffffffffffffff
00000000000000ffffffffffffffffffffffffff
000000000000ffffffffffffffffffffffffffff
0000000000ffffffffffffffffffffffffffffff
00000000ffffffffffffffffffffffffffffffff
000000ffffffffffffffffffffffffffffffffff
0000ffffffffffffffffffffffffffffffffffff
00ffffffffffffffffffffffffffffffffffffff
ffffffffffffffffffffffffffffffffffffffff
0000000000000000000000000000000000000000
00000000000000000000000000000000000000ff
000000000000000000000000000000000000ffff
0000000000000000000000000000000000ffffff
00000000000000000000000000000000ffffffff
000000000000000000000000000000ffffffffff
0000000000000000000000000000ffffffffffff
00000000000000000000000000ffffffffffffff
000000000000000000000000ffffffffffffffff
0000000000000000000000ffffffffffffffffff
00000000000000000000ffffffffffffffffffff
000000000000000000ffffffffffffffffffffff
0000000000000000ffffffffffffffffffffffff
00000000000000ffffffffffffffffffffffffff
000000000000ffffffffffffffffffffffffffff
0000000000ffffffffffffffffffffffffffffff
00000000ffffffffffffffffffffffffffffffff
000000ffffffffffffffffffffffffffffffffff
0000ffffffffffffffffffffffffffffffffffff
00ffffffffffffffffffffffffffffffffffffff
ffffffffffffffffffffffffffffffffffffffff'
check "${TEST}" "${OUTPUT}" "${EXPECTED}" || exit 1
# -----------------------------------------------------------------------------
# Test various different network prefixes
# -----------------------------------------------------------------------------
TEST="prefix1 - WIF compressed private key to address (bitcoin)"
EXPECTED="1NFeCVtA3zuCUAmYheRvfyABnSZCHfrR3j"
INPUT="Kx4VFK8gXu4qBv73x9b1KFnWYqKekkprYyfX9QhFUMQhrTUooXKc"
OUTPUT=$($BITCOIN_TOOL \
	--input-type private-key-wif \
	--input-format base58check \
	--output-type address \
	--output-format base58check \
	--input "${INPUT}")
check "${TEST}" "${OUTPUT}" "${EXPECTED}" || exit 1
# -----------------------------------------------------------------------------
TEST="prefix2 - WIF compressed private key to address (bitcoin-testnet)"
EXPECTED="mxToLbBqPcSNnqPCSnrYjFv172TFPLjVNf"
INPUT="92Wn1EBgiwDNT8SC7WMZfcSk2y3mQkLUPAQtwMNYZQGAzCFUTdu"
OUTPUT=$($BITCOIN_TOOL \
	--input-type private-key-wif \
	--input-format base58check \
	--output-type address \
	--output-format base58check \
	--input "${INPUT}")
check "${TEST}" "${OUTPUT}" "${EXPECTED}" || exit 1
# -----------------------------------------------------------------------------
TEST="prefix3 - WIF compressed private key to address (litecoin)"
EXPECTED="LMzBLYQG2opHvMBihMQgJBboxunoj5pssC"
INPUT="6vVAeKejJRV5wgrAqtqi7eQsS4Zf79nkw8xuYntU3JwHCiexYaJ"
OUTPUT=$($BITCOIN_TOOL \
	--input-type private-key-wif \
	--input-format base58check \
	--output-type address \
	--output-format base58check \
	--input "${INPUT}")
check "${TEST}" "${OUTPUT}" "${EXPECTED}" || exit 1
# -----------------------------------------------------------------------------
TEST="prefix4 - WIF compressed private key to address (dogecoin)"
EXPECTED="DHEPGdnS46dHT79tkfm5DyhGAbQj4Xi8Ni"
INPUT="6KayMYAEQfFACQhZUzbBpFhvGzDWSmRtaY9NrPQGig9qVzRCzQf"
OUTPUT=$($BITCOIN_TOOL \
	--input-type private-key-wif \
	--input-format base58check \
	--output-type address \
	--output-format base58check \
	--input "${INPUT}")
check "${TEST}" "${OUTPUT}" "${EXPECTED}" || exit 1
# -----------------------------------------------------------------------------
TEST="prefix5 - WIF compressed private key to address (feathercoin)"
EXPECTED="6ggpjHf7iq2vdvq8HNfZo9XQSLeVfxezzX"
INPUT="5nU4VfsTK8B4ra3ytR4YqSWH8txuGxZUCuVSyKKtU9pdENQkpTi"
OUTPUT=$($BITCOIN_TOOL \
	--input-type private-key-wif \
	--input-format base58check \
	--output-type address \
	--output-format base58check \
	--input "${INPUT}")
check "${TEST}" "${OUTPUT}" "${EXPECTED}" || exit 1
# -----------------------------------------------------------------------------
TEST="prefix6 - WIF compressed private key to address (quarkcoin)"
EXPECTED="QUARKbonge6S9FyPKSa5HgxAsUercoU8CL"
INPUT="7EoLLiHnKZnbvsgfS91NHBS5r11N1G2RxGaepGNH4gRVeDYRgWB"
OUTPUT=$($BITCOIN_TOOL \
	--input-type private-key-wif \
	--input-format base58check \
	--output-type address \
	--output-format base58check \
	--input "${INPUT}")
check "${TEST}" "${OUTPUT}" "${EXPECTED}" || exit 1
# -----------------------------------------------------------------------------
TEST="prefix7 - WIF compressed private key to address (namecoin)"
EXPECTED="MyMfA2KMf1ppNGhvYKEqfSohKwGFyK1P5e"
INPUT="5KhxuVoUB2AUpyqKb9bgpp1468vpTaKc1Pcd48qNzgtdpWwknBE"
OUTPUT=$($BITCOIN_TOOL \
	--input-type private-key-wif \
	--input-format base58check \
	--output-type address \
	--output-format base58check \
	--network namecoin \
	--input "${INPUT}")
check "${TEST}" "${OUTPUT}" "${EXPECTED}" || exit 1
# -----------------------------------------------------------------------------




echo "all tests passed"
