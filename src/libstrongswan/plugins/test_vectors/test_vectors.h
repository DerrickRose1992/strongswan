/*
 * Copyright (C) 2009 Martin Willi
 * Hochschule fuer Technik Rapperswil
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

TEST_VECTOR_CRYPTER(aes_cbc1)
TEST_VECTOR_CRYPTER(aes_cbc2)
TEST_VECTOR_CRYPTER(aes_cbc3)
TEST_VECTOR_CRYPTER(aes_cbc4)
TEST_VECTOR_CRYPTER(aes_cbc5)
TEST_VECTOR_CRYPTER(aes_cbc6)
TEST_VECTOR_CRYPTER(blowfish1)
TEST_VECTOR_CRYPTER(blowfish2)
TEST_VECTOR_CRYPTER(camellia_cbc1)
TEST_VECTOR_CRYPTER(camellia_cbc2)
TEST_VECTOR_CRYPTER(camellia_cbc3)
TEST_VECTOR_CRYPTER(camellia_cbc4)
TEST_VECTOR_CRYPTER(camellia_cbc5)
TEST_VECTOR_CRYPTER(camellia_cbc6)
TEST_VECTOR_CRYPTER(cast1)
TEST_VECTOR_CRYPTER(des3_cbc1)
TEST_VECTOR_CRYPTER(des3_cbc2)
TEST_VECTOR_CRYPTER(twofish_cbc1)
TEST_VECTOR_CRYPTER(twofish_cbc2)
TEST_VECTOR_CRYPTER(twofish_cbc3)

TEST_VECTOR_SIGNER(aes_xcbc_s1)
TEST_VECTOR_SIGNER(aes_xcbc_s2)
TEST_VECTOR_SIGNER(aes_xcbc_s3)
TEST_VECTOR_SIGNER(aes_xcbc_s4)
TEST_VECTOR_SIGNER(aes_xcbc_s5)
TEST_VECTOR_SIGNER(md5_hmac_s1)
TEST_VECTOR_SIGNER(md5_hmac_s2)
TEST_VECTOR_SIGNER(md5_hmac_s3)
TEST_VECTOR_SIGNER(md5_hmac_s4)
TEST_VECTOR_SIGNER(sha1_hmac_s1)
TEST_VECTOR_SIGNER(sha1_hmac_s2)
TEST_VECTOR_SIGNER(sha1_hmac_s3)
TEST_VECTOR_SIGNER(sha1_hmac_s4)
TEST_VECTOR_SIGNER(sha1_hmac_s5)
TEST_VECTOR_SIGNER(sha1_hmac_s6)
TEST_VECTOR_SIGNER(sha256_hmac_s1)
TEST_VECTOR_SIGNER(sha256_hmac_s2)
TEST_VECTOR_SIGNER(sha256_hmac_s3)
TEST_VECTOR_SIGNER(sha384_hmac_s1)
TEST_VECTOR_SIGNER(sha384_hmac_s2)
TEST_VECTOR_SIGNER(sha384_hmac_s3)
TEST_VECTOR_SIGNER(sha512_hmac_s1)
TEST_VECTOR_SIGNER(sha512_hmac_s2)
TEST_VECTOR_SIGNER(sha512_hmac_s3)

TEST_VECTOR_HASHER(md5_1)
TEST_VECTOR_HASHER(md5_2)
TEST_VECTOR_HASHER(md5_3)
TEST_VECTOR_HASHER(md5_4)
TEST_VECTOR_HASHER(md5_5)
TEST_VECTOR_HASHER(md5_6)
TEST_VECTOR_HASHER(md5_7)
TEST_VECTOR_HASHER(sha1_1)
TEST_VECTOR_HASHER(sha1_2)
TEST_VECTOR_HASHER(sha1_3)
TEST_VECTOR_HASHER(sha256_1)
TEST_VECTOR_HASHER(sha256_2)
TEST_VECTOR_HASHER(sha256_3)
TEST_VECTOR_HASHER(sha384_1)
TEST_VECTOR_HASHER(sha384_2)
TEST_VECTOR_HASHER(sha384_3)
TEST_VECTOR_HASHER(sha512_1)
TEST_VECTOR_HASHER(sha512_2)
TEST_VECTOR_HASHER(sha512_3)

TEST_VECTOR_PRF(aes_xcbc_p1)
TEST_VECTOR_PRF(aes_xcbc_p2)
TEST_VECTOR_PRF(aes_xcbc_p3)
TEST_VECTOR_PRF(aes_xcbc_p4)
TEST_VECTOR_PRF(aes_xcbc_p5)
TEST_VECTOR_PRF(aes_xcbc_p6)
TEST_VECTOR_PRF(aes_xcbc_p7)
TEST_VECTOR_PRF(md5_hmac_p1)
TEST_VECTOR_PRF(md5_hmac_p2)
TEST_VECTOR_PRF(md5_hmac_p3)
TEST_VECTOR_PRF(md5_hmac_p4)
TEST_VECTOR_PRF(md5_hmac_p5)
TEST_VECTOR_PRF(md5_hmac_p6)
TEST_VECTOR_PRF(sha1_hmac_p1)
TEST_VECTOR_PRF(sha1_hmac_p2)
TEST_VECTOR_PRF(sha1_hmac_p3)
TEST_VECTOR_PRF(sha1_hmac_p4)
TEST_VECTOR_PRF(sha1_hmac_p5)
TEST_VECTOR_PRF(sha1_hmac_p6)
TEST_VECTOR_PRF(sha256_hmac_p1)
TEST_VECTOR_PRF(sha256_hmac_p2)
TEST_VECTOR_PRF(sha256_hmac_p3)
TEST_VECTOR_PRF(sha256_hmac_p4)
TEST_VECTOR_PRF(sha256_hmac_p5)
TEST_VECTOR_PRF(sha256_hmac_p6)
TEST_VECTOR_PRF(sha384_hmac_p1)
TEST_VECTOR_PRF(sha384_hmac_p2)
TEST_VECTOR_PRF(sha384_hmac_p3)
TEST_VECTOR_PRF(sha384_hmac_p4)
TEST_VECTOR_PRF(sha384_hmac_p5)
TEST_VECTOR_PRF(sha384_hmac_p6)
TEST_VECTOR_PRF(sha512_hmac_p1)
TEST_VECTOR_PRF(sha512_hmac_p2)
TEST_VECTOR_PRF(sha512_hmac_p3)
TEST_VECTOR_PRF(sha512_hmac_p4)
TEST_VECTOR_PRF(sha512_hmac_p5)
TEST_VECTOR_PRF(sha512_hmac_p6)
TEST_VECTOR_PRF(fips_prf_1)

TEST_VECTOR_RNG(rng_monobit_1)
TEST_VECTOR_RNG(rng_monobit_2)
TEST_VECTOR_RNG(rng_monobit_3)
TEST_VECTOR_RNG(rng_poker_1)
TEST_VECTOR_RNG(rng_poker_2)
TEST_VECTOR_RNG(rng_poker_3)
TEST_VECTOR_RNG(rng_runs_1)
TEST_VECTOR_RNG(rng_runs_2)
TEST_VECTOR_RNG(rng_runs_3)

