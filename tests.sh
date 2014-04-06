#!/bin/sh

BITCOIN_TOOL="./bitcoin-tool"

check () {
	echo check $1
	if [ "$2" != "$3" ];then
		echo "failed test $1"
		echo "output   : [$2]"
		echo "expected : [$3]"
		return 1
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
check "${TEST}" "${OUTPUT}" "${EXPECTED}" || return 1
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
check "${TEST}" "${OUTPUT}" "${EXPECTED}" || return 1
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
check "${TEST}" "${OUTPUT}" "${EXPECTED}" || return 1
# -----------------------------------------------------------------------------
TEST="4 - WIF uncompressed private key to address"
EXPECTED="16SK7HnxBMRxSpLhhdf8RYcqv8MPJiSF6Q"
OUTPUT=$($BITCOIN_TOOL \
	--input-type private-key-wif \
	--input-format base58check \
	--output-type address \
	--output-format base58check \
	--input 5J1LYLWqNxJBTwdGAmzYnpkqqSuFu48fsHv8jgojFMV2Z8exk9L )
check "${TEST}" "${OUTPUT}" "${EXPECTED}" || return 1
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
check "${TEST}" "${OUTPUT}" "${EXPECTED}" || return 1
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
check "${TEST}" "${OUTPUT}" "${EXPECTED}" || return 1
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
check "${TEST}" "${OUTPUT}" "${EXPECTED}" || return 1
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
check "${TEST}" "${OUTPUT}" "${EXPECTED}" || return 1
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
check "${TEST}" "${OUTPUT}" "${EXPECTED}" || return 1
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
check "${TEST}" "${OUTPUT}" "${EXPECTED}" || return 1

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
check "${TEST}" "${OUTPUT}" "${EXPECTED}" || return 1
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
check "${TEST}" "${OUTPUT}" "${EXPECTED}" || return 1
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
check "${TEST}" "${OUTPUT}" "${EXPECTED}" || return 1
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
check "${TEST}" "${OUTPUT}" "${EXPECTED}" || return 1
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
check "${TEST}" "${OUTPUT}" "${EXPECTED}" || return 1
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
check "${TEST}" "${OUTPUT}" "${EXPECTED}" || return 1
# -----------------------------------------------------------------------------



echo "all tests passed"
