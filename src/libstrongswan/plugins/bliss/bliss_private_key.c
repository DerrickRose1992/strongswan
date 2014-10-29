/*
 * Copyright (C) 2014 Andreas Steffen
 * HSR Hochschule fuer Technik Rapperswil
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

#include "bliss_private_key.h"
#include "bliss_param_set.h"
#include "bliss_fft.h"

#define _GNU_SOURCE
#include <stdlib.h>

typedef struct private_bliss_private_key_t private_bliss_private_key_t;

/**
 * Private data of a bliss_private_key_t object.
 */
struct private_bliss_private_key_t {
	/**
	 * Public interface for this signer.
	 */
	bliss_private_key_t public;

	/**
	 * BLISS signature parameter set
	 */
	bliss_param_set_t *set;

	/**
	 * reference count
	 */
	refcount_t ref;
};

METHOD(private_key_t, get_type, key_type_t,
	private_bliss_private_key_t *this)
{
	return KEY_BLISS;
}

METHOD(private_key_t, sign, bool,
	private_bliss_private_key_t *this, signature_scheme_t scheme,
	chunk_t data, chunk_t *signature)
{
	switch (scheme)
	{
		case SIGN_BLISS_I_SHA256:
			return FALSE;
		case SIGN_BLISS_IV_SHA384:
			return FALSE;
		default:
			DBG1(DBG_LIB, "signature scheme %N not supported with BLISS",
				 signature_scheme_names, scheme);
			return FALSE;
	}
}

METHOD(private_key_t, decrypt, bool,
	private_bliss_private_key_t *this, encryption_scheme_t scheme,
	chunk_t crypto, chunk_t *plain)
{
	DBG1(DBG_LIB, "encryption scheme %N not supported",
				   encryption_scheme_names, scheme);
	return FALSE;
}

METHOD(private_key_t, get_keysize, int,
	private_bliss_private_key_t *this)
{
	return this->set->strength;
}

METHOD(private_key_t, get_public_key, public_key_t*,
	private_bliss_private_key_t *this)
{
	public_key_t *public = NULL;

	return public;
}

METHOD(private_key_t, get_encoding, bool,
	private_bliss_private_key_t *this, cred_encoding_type_t type,
	chunk_t *encoding)
{
	bool success = TRUE;

	*encoding = chunk_empty;

	return success;
}

METHOD(private_key_t, get_fingerprint, bool,
	private_bliss_private_key_t *this, cred_encoding_type_t type, chunk_t *fp)
{
	bool success = FALSE;

	return success;
}

METHOD(private_key_t, get_ref, private_key_t*,
	private_bliss_private_key_t *this)
{
	ref_get(&this->ref);
	return &this->public.key;
}

METHOD(private_key_t, destroy, void,
	private_bliss_private_key_t *this)
{
	if (ref_put(&this->ref))
	{
		free(this);
	}
}

/**
 * Internal generic constructor
 */
static private_bliss_private_key_t *bliss_private_key_create_empty(void)
{
	private_bliss_private_key_t *this;

	INIT(this,
		.public = {
			.key = {
				.get_type = _get_type,
				.sign = _sign,
				.decrypt = _decrypt,
				.get_keysize = _get_keysize,
				.get_public_key = _get_public_key,
				.equals = private_key_equals,
				.belongs_to = private_key_belongs_to,
				.get_fingerprint = _get_fingerprint,
				.has_fingerprint = private_key_has_fingerprint,
				.get_encoding = _get_encoding,
				.get_ref = _get_ref,
				.destroy = _destroy,
			},
		},
		.ref = 1,
	);
	return this;
}

/**
 * Compute the scalar product of a vector x with a negative wrapped vector y
 */
static int16_t wrapped_product(int16_t *x, int16_t *y, int n, int shift)
{
	int16_t product = 0;
	int i;

	for (i = 0; i < n - shift; i++)
	{
		product += x[i] * y[i + shift];
	}
	for (i = n - shift; i < n; i++)
	{
		product -= x[i] * y[i + shift - n];
	}
	return product;
}

/**
 * Apply a negative wrapped rotation to a vector x
 */
static void wrap(int16_t *x, int n, int shift, int16_t *x_wrapped)
{
	int i;

	for (i = 0; i < n - shift; i++)
	{
		x_wrapped[i + shift] = x[i];
	}
	for (i = n - shift; i < n; i++)
	{
		x_wrapped[i + shift - n] = -x[i];
	}
}

static int compare(const int16_t *a, const int16_t *b)
{
	int16_t temp = *a - *b;

	if (temp > 0)
	{
		return 1;
	}
	else if (temp < 0)
	{
		return -1;
	}
	else
	{
		return 0;
	}
}

/**
 * Compute the Nk(S) norm of S = (s1, s2)
 */
static uint32_t nks_norm(int16_t *s1, int16_t *s2, int n, uint16_t kappa)
{
	int16_t t[n], t_wrapped[n], max_kappa[n];
	uint32_t nks = 0;
	int i, j;

	for (i = 0; i < n; i++)
	{
		t[i] = wrapped_product(s1, s1, n, i) + wrapped_product(s2, s2, n, i);
	}

	for (i = 0; i < n; i++)
	{
		wrap(t, n, i, t_wrapped);
		qsort(t_wrapped, n, sizeof(int16_t), (__compar_fn_t)compare);
		max_kappa[i] = 0;

		for (j = 1; j <= kappa; j++)
		{
			max_kappa[i] += t_wrapped[n - j];
		}
	}
	qsort(max_kappa, n, sizeof(int16_t), (__compar_fn_t)compare);

	for (i = 1; i <= kappa; i++)
	{
		nks += max_kappa[n - i];
	}
	return nks;
}

/**
 * Compute the inverse x1 of x modulo q as x^(-1) = x^(q-2) mod q
 */
static uint32_t invert(uint32_t x, uint16_t q)
{
	uint32_t x1, x2;
	uint16_t q2;
	int i, i_max;

	q2 = q - 2;
	x1 = (q2 & 1) ? x : 1;
	x2 = x;
	i_max = 15;

	while ((q2 & (1 << i_max)) == 0)
	{
		i_max--;
	}
	for (i = 1; i <= i_max; i++)
	{
		x2 = (x2 * x2) % q;

		if (q2 & (1 << i))
		{
			x1 = (x1 * x2) % q;
		}
	}

	return x1;
}

/**
 * See header.
 */
bliss_private_key_t *bliss_private_key_gen(key_type_t type, va_list args)
{
	private_bliss_private_key_t *this;
	u_int key_size = 1;
	int i;
	uint32_t *a, *A, *F, *G, nks;
	uint16_t q, n, l2_norm;
	bliss_param_set_t *set;
	bliss_fft_t *fft;

	int16_t f_bliss1[] = {
		 0,  0,  0,  0,  1,  1,  0, -1,  0,  1, 
		 0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 
		 0,  0, -1,  0,  0,  0, -1,  1,  0,  0, 
		 1,  1, -1,  0,  1,  1,  0,  0,  0,  0, 
		 0,  0,  0,  0,  1,  0, -1,  0, -1,  0, 
		 0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 
		-1,  0,  1,  0,  1,  0,  0,  0,  0,  0, 
		 0,  0,  1,  0,  0,  1,  0, -1,  0,  0, 
		 0, -1,  0,  0,  1, -1,  0,  0,  0,  0, 
		 0,  0,  0,  0,  1,  0,  0,  0,  0,  0, 

		 0,  0,  0,  0,  0,  1,  0, -1,  0,  1, 
		 1, -1,  0,  1,  0,  1,  0,  0,  1,  0, 
		-1,  0,  0,  0,  1, -1,  1,  0,  1,  0, 
		 0,  0,  0, -1, -1,  0,  0,  0, -1,  0, 
		-1,  0,  0,  0,  1,  0,  1,  0,  0,  1, 
		 0, -1,  0,  0,  0, -1,  0, -1,  0,  0, 
		-1,  0,  0, -1,  1,  1, -1,  0,  0, -1, 
		 0,  0,  1, -1,  0, -1,  0,  0,  1,  0, 
		 0,  1,  1,  0,  0,  0, -1,  0,  0, -1, 
		 0,  0,  0, -1,  1,  0,  0, -1,  0,  1, 

		 0,  0,  1, -1, -1,  0,  0,  0,  0,  1, 
		 0,  0,  0,  0,  0,  0,  0,  0, -1,  0, 
		 0,  0,  0,  0,  0,  0,  0,  0,  0,  1, 
		 1,  0, -1,  0,  0,  0,  0,  0,  1,  0, 
		 0,  1,  0,  1,  0,  0,  0,  0,  0,  1, 
		 0,  0,  0,  0,  0,  1,  0,  0, -1,  0, 
		 1, -1,  0,  0,  0,  0,  1,  0, -1,  0, 
		-1,  0, -1,  1,  0,  0,  1,  1,  0,  0, 
		 0,  0,  0,  0,  0,  1,  0,  0, -1,  0, 
		 0,  1, -1,  0,  1,  0,  0,  0,  1, -1, 

		-1,  0,  0,  0, -1,  0,  1,  0,  0,  0, 
		 0,  0,  1, -1,  1,  0,  0,  0,  0,  0, 
		 0, -1,  0,  1,  0,  0,  0,  0,  0, -1, 
		 0,  0,  0,  0,  0,  0,  0,  1,  0,  1, 
		 0,  0,  0,  0, -1,  1,  0,  0,  0,  0, 
		 0,  0,  0,  0, -1,  0,  0,  0,  0,  1, 
		 0, -1,  0,  0,  0,  1,  0,  1, -1,  1, 
		 0,  0,  0,  0,  1,  1,  0,  1,  0,  0, 
		 0, -1,  1,  0,  1, -1,  0,  0, -1,  0, 
		 0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 

		 0,  0,  1,  0,  0,  0,  0,  0, -1,  0, 
		 0, -1,  0,  0,  0,  1,  0,  0,  0,  1, 
		 0,  0,  0,  0,  0,  0,  0, -1,  0,  0, 
		 0,  0, -1,  1,  0,  1,  0,  0, -1,  0, 
		 0, -1,  0,  0,  0,  0,  0,  0,  0,  0, 
		 0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 
		-1,  0,  0,  1,  0, -1,  1,  0,  1,  0, 
		-1,  1,  0,  0,  0,  0,  0,  0,  0, -1, 
		 0,  0,  0,  0,  0,  1,  1,  0, -1,  0, 
		-1,  0,  0,  0, -1, -1,  0,  0,  0,  0, 

		 0,  0, -1, -1,  0,  1, -1,  0,  0,  0, 
		 0, -1
	};

int16_t g_bliss1[] = {
		-1,  0,  0,  0,  0,  0,  0,  0,  1,  0, 
		 0,  0,  0,  0,  0,  0,  0,  1,  1,  0, 
		 1,  0,  0,  0,  1,  0, -1,  0,  0,  0, 
		 0,  0,  0,  0, -1,  1,  0,  0,  0,  0, 
		 0,  0, -1,  1,  0,  0,  0,  0,  0, -1, 
		-1,  0, -1,  0,  1, -1,  0,  0,  0,  0, 
		 0,  0,  0,  0,  1, -1,  0,  0,  1,  0, 
		 0,  1,  0, -1,  1,  0,  0,  0,  0,  0, 
		 0,  0,  0,  0, -1,  1,  0, -1,  0,  1, 
		 0,  1,  0,  0,  1,  0, -1,  1,  0,  0, 

		 0,  0,  0,  0,  0,  0,  1,  1,  0,  0, 
		 0,  0,  0,  0,  0,  0,  0,  0,  1,  0, 
		 0,  0,  0,  0,  0,  0,  1,  0,  0,  1, 
		 1,  1,  1,  0,  0,  0,  0,  0, -1,  0, 
		 1,  1,  0,  0,  1,  0, -1,  1,  0,  0, 
		-1,  0,  0, -1,  0,  0,  0,  0,  0,  0, 
		 0,  0,  0, -1,  0, -1,  1,  0,  0, -1, 
		 0,  0, -1,  0,  0,  0,  0,  0,  0, -1, 
		 0,  0,  1,  0,  0,  0,  0, -1,  0,  1, 
		 0,  0, -1,  0,  0,  0,  0,  0,  0,  0, 

		 0,  0,  0,  0,  0,  0,  0,  1,  0,  1, 
		 1,  0, -1,  0,  0,  0,  0,  1,  1,  0, 
		 0, -1,  1,  1,  0,  0,  0, -1,  1,  0, 
		 0,  1,  0,  0,  0,  0, -1,  1,  0,  0, 
		-1,  0,  1,  0,  0,  0, -1, -1,  0,  0, 
		 0, -1,  1,  1,  0,  1,  0,  0,  0,  0, 
		 0,  0,  1,  0,  0,  1,  1,  1,  1,  0, 
		 0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 
		 0,  0,  0,  0,  1,  0,  0,  0,  0,  1, 
		 0, -1, -1,  0, -1,  0, -1, -1,  1, -1, 

		-1,  0,  0,  0, -1,  0,  0,  0,  0,  0, 
		-1,  1,  0, -1,  0,  0,  0,  0,  0,  0, 
		 1,  0,  0,  0,  0,  0, -1,  0,  0, -1, 
		 1,  0,  0, -1,  0,  0,  0,  0,  0, -1, 
		 0,  0, -1,  0,  0,  0, -1,  0,  0,  0, 
		 0,  0,  0,  1,  1,  0,  0,  0,  0,  0, 
		-1,  0,  0,  0,  0,  0, -1,  0,  0,  0, 
		 1,  1,  1,  0,  0,  0,  0, -1, -1,  0, 
		-1,  0,  1,  0,  0,  0,  0,  0, -1,  0, 
		-1,  0,  0,  0,  0,  1,  0,  0, -1, -1, 

		 1,  0,  0,  0,  0,  0,  0,  0, -1,  0, 
		 0,  0,  1, -1,  0,  0,  1,  0,  0,  1, 
		-1,  0,  1,  0,  1,  1,  0,  0,  0,  0, 
		 0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 
		 0, -1,  0,  0,  0,  0,  1,  1,  0,  0, 
		 1,  0, -1,  0,  0,  1,  1,  0,  0,  0, 
		 0,  0, -1,  0,  0,  0,  0,  0,  1,  0, 
		 0,  0,  1, -1, -1,  0,  0,  0,  0,  1, 
		-1, -1,  1,  0,  0,  0,  0,  0,  0,  1, 
		 0,  0,  0,  1,  0,  0,  0,  1,  1,  0, 

		-1,  0,  1,  0,  0,  0,  0,  0,  0,  0, 
		 0, -1 
};

	int16_t f[] = {
		 0, -1, -1,  0,  0,  1, -1, -1,  0,  0, 
		 0,  0,  1,  1,  0,  0,  0,  1, -1,  1, 
		-2,  1,  0,  0, -1,  0,  0,  0, -1,  0, 
		 0, -1,  0,  1,  1, -1,  0,  1, -2, -1, 
		 1,  0,  0,  0,  0, -1, -1,  0,  1,  2, 
		 0,  0,  1,  0, -1,  0,  1,  1,  1,  0, 
		 2, -1,  0,  0,  1,  0,  0, -1,  0,  0, 
		 0,  0,  1,  0,  0, -1,  0, -1, -1,  0, 
		 0,  0,  0, -1, -2, -1, -1, -1,  1,  0, 
		 0,  1,  0,  1, -1, -1,  0,  0,  0,  1, 

		 0, -1,  1,  1,  1,  0, -1,  0,  0, -1, 
		 0,  1, -1,  1, -2,  0,  1,  1, -1,  0, 
		 1, -1, -2,  0,  0, -1,  0,  0,  1,  0, 
		 0,  0,  1, -1,  1, -2,  0,  0, -1,  1, 
		 0,  0, -1, -1,  0, -1,  0,  0,  0,  0, 
		-1,  0,  1, -1,  1,  0, -1,  1,  0,  1, 
		 1,  0,  0, -1,  0,  1,  1,  0, -1,  1, 
		 1,  1,  2,  0,  0,  1,  0,  1,  0,  0, 
		-1, -1,  0, -2,  0, -1,  0,  0, -1,  1, 
		-1, -2,  0,  2,  0, -1,  2,  1,  0,  1, 

		 1,  1,  1,  0, -1,  1, -1,  1,  1, -1, 
		 0,  1,  1,  1,  0,  0,  0,  0,  1,  0, 
		-2,  0,  1,  1,  0, -1, -1,  1,  0,  1, 
		-2,  1,  1, -1,  1,  0,  0,  1, -1, -1, 
		 1,  0,  1,  1,  1, -1,  0, -1,  0,  0, 
		 0,  0,  1,  0,  0, -1,  0,  0,  0,  0, 
		 1, -1,  2, -1,  1,  0,  0,  1,  0,  0, 
		 0, -1, -1,  2,  1,  1,  0, -1,  0, -1, 
		 0,  0,  0,  0,  0,  0,  0, -1, -1,  0, 
		 0,  0,  0, -1,  0,  1,  1,  1, -1,  0, 

		-1,  1,  0,  1,  0,  0,  0,  1,  0, -1, 
		 0,  0,  1, -2,  0,  0,  0,  0, -1,  1, 
		 0,  1,  0,  0,  0, -1,  0,  1,  0, -1, 
		 0,  1, -1,  0,  0,  1,  0,  0,  0,  0, 
		 1, -1,  0, -2,  0,  0,  2,  0, -1, -1, 
		-1,  1,  1,  0,  1, -1,  1,  2, -1,  1, 
		-1,  0,  1, -2,  0,  0, -1,  2, -1,  0, 
		-1,  0, -1,  0,  1, -2,  0,  2,  0,  0, 
		 1, -1,  1, -1,  1,  0,  1,  1, -1,  0, 
		 0,  0, -1, -1,  0,  0,  0, -1, -2,  0, 

		 0,  0,  1, -2,  0,  0,  1,  1,  0, -1, 
		 0,  0,  0,  0,  0,  1,  0,  0,  0,  0, 
		 0,  0, -1,  0,  0,  0,  1,  0,  0,  0, 
		 1,  2,  0, -1,  0,  0,  1,  0,  0,  0, 
		-1,  0,  0,  1, -1,  0, -1,  0,  0, -1, 
		-1, -1,  2,  0,  0,  0, -1,  0,  2,  0, 
		-1,  0, -1,  0, -1,  1,  0,  0,  0,  0, 
		-1,  2,  0,  1,  0,  0, -1,  0,  0,  0, 
		 1, -1, -1,  0,  0, -1,  0, -1,  1, -1, 
		 1,  0, -1, -1,  1,  1,  0,  0,  0,  0, 

		 0,  0,  0,  1, -1,  0,  0,  0,  0,  0, 
		 0,  0 
	};
int16_t g[] = {
		 0,  2,  1,  0, -1,  1,  1,  1, -1, -1, 
		 1,  2,  0,  0,  0, -1,  0, -1,  1,  0, 
		 1, -1,  1,  0,  0,  0, -1, -1,  1,  0, 
		-1,  1,  0,  0,  0,  0,  0,  0,  0,  0, 
		 0,  1, -1, -1, -1, -1,  0,  0,  0,  0, 
		 0, -1,  0, -1, -2,  0,  0,  1,  0, -1, 
		-1, -1, -1, -1,  2,  1, -1,  0, -1,  0, 
		 0,  1,  1,  0,  1,  0,  0,  0, -1,  1, 
		 0,  1,  0,  0,  0,  1,  0,  0,  0,  0, 
		 0, -1,  0,  0,  0, -1,  0,  0,  0,  0, 

		 0,  1, -2,  1,  1, -1,  1,  1,  0,  1, 
		 0,  0,  1,  0,  0,  0, -1,  0,  0,  0, 
		 0,  0,  1,  0,  1, -1,  0,  0,  0,  1, 
		 1,  1,  0,  0,  1,  0,  0,  1,  1,  1, 
		 0,  0,  0, -1,  0, -1, -2,  1,  0,  1, 
		 0, -1, -2,  1,  0,  0, -1,  0,  0,  0, 
		 0,  0,  1,  0,  1, -1,  1,  1, -1,  0, 
		 0,  0,  1, -1,  1,  1, -2, -1,  1,  0, 
		-2,  0,  0,  0,  1,  1,  2,  0,  2,  1, 
		 1,  0,  1,  0, -1,  1,  0,  0,  0, -1, 

		-1, -1,  0,  0, -1,  1,  0,  1,  0, -1, 
		 0,  0,  2,  1,  0,  0,  1, -2, -1,  0, 
		 1,  0, -1,  1, -1,  0,  1, -1, -1,  1, 
		 0,  0, -1, -1, -1,  0,  0,  1, -2, -1, 
		 0, -1,  1, -1,  1, -1,  0, -1, -1,  1, 
		 0,  1, -1,  0,  2,  1, -1,  0, -2,  0, 
		-1,  0,  0,  1,  0, -1,  1,  1,  0,  0, 
		 0, -1, -2,  1,  0,  0,  2,  0, -1,  0, 
		 1,  1,  0, -1,  0,  0, -1, -1, -1,  0, 
		 0, -1,  0,  0,  0,  0,  1,  0, -1, -1, 

		 1, -1,  0,  0,  1,  0, -1,  1,  0,  1, 
		 0,  1,  1,  1, -1,  0,  0,  1,  0, -1, 
		 0, -1,  0,  0,  0, -1, -1,  0,  0,  0, 
		-1, -1,  0,  1,  0,  0,  0,  1,  0,  0, 
		 1,  1, -1,  0,  0,  0, -1,  0,  1,  1, 
		 0,  1,  0,  1,  0, -1, -1,  0,  0,  0, 
		 2, -1,  0,  0, -1,  1, -1, -2, -1,  0, 
		 0,  1,  0,  1,  1,  0,  0,  0, -1,  2, 
		 0, -1,  0,  0,  0, -1, -1, -1,  0,  1, 
		-2,  0,  0,  1, -1,  0,  0,  0,  1,  1, 

		 1,  1,  0, -1,  0,  0,  0,  0,  0,  0, 
		 0,  0,  0,  0,  0,  0, -2,  0,  0,  0, 
		 2,  1,  0,  0,  0,  0,  1,  0,  0, -1, 
		 1, -2,  0,  0,  1,  1,  1,  0, -2,  0, 
		-1,  0,  1,  2,  1,  0,  0, -2,  0, -1, 
		-1,  0,  1,  0,  1,  0,  1,  0, -1, -1, 
		 2,  0,  1, -1,  0,  1,  0,  0,  0, -1, 
		 1,  0,  1, -1,  0,  0,  0,  0,  0, -1, 
		 0,  0,  1, -1,  0,  0,  1,  1,  0,  0, 
		 0,  1, -1,  0, -1, -2, -1,  0,  0, -2, 

		 0, -1,  0,  0,  0, -1,  1,  0,  1,  1, 
		-1,  0
	};

	while (TRUE)
	{
		switch (va_arg(args, builder_part_t))
		{
			case BUILD_KEY_SIZE:
				key_size = va_arg(args, u_int);
				continue;
			case BUILD_END:
				break;
			default:
				return NULL;
		}
		break;
	}

	/* Only BLISS-I and BLISS-IV are supported */
	set = bliss_param_set_get_by_id(key_size);
	if (!set)
	{
		DBG1(DBG_LIB, "BLISS parameter set %u not supported");
		return NULL;
	}

	/* Some shortcuts for often used variables */
	n = set->n;
	q = set->q;

	if (set->fft_params->n != n || set->fft_params->q != q)
	{
		DBG1(DBG_LIB, "FFT parameters do not match BLISS parameters");
		return NULL;
	}
	this = bliss_private_key_create_empty();
	this->set = set;

	/* We derive the public key from the private key using the FFT */
	fft = bliss_fft_create(set->fft_params);

	/* Compute 2g + 1 */
	for (i = 0; i < n; i++)
	{
		g[i] *= 2;
	}
	g[0] += 1;

	l2_norm = wrapped_product(f, f, n, 0) + wrapped_product(g, g, n, 0);
	nks = nks_norm(f, g, n, set->kappa);
	DBG2(DBG_LIB, "L2 norm of s1||s2: %d, Nk(S): %u (%u max)",
		 l2_norm, nks, set->nks_max);

	F = malloc(n * sizeof(uint32_t));
	G = malloc(n * sizeof(uint32_t));
	A = malloc(n * sizeof(uint32_t));
	a = malloc(n * sizeof(uint32_t));

	/* Convert signed arrays to unsigned arrays before FFT */
	for (i = 0; i < n; i++)
	{
		F[i] = (f[i] < 0) ? f[i] + q : f[i];
		G[i] = (g[i] < 0) ? g[i] + q : g[i];
	}
	fft->transform(fft, F, F, FALSE);
	fft->transform(fft, G, G, FALSE);

	for (i = 0; i < n; i++)
	{
		if (F[i] == 0)
		{
			DBG1(DBG_LIB, "F[%d] is zero", i);
		}
		A[i] = invert(F[i], q);
		A[i] = (G[i] * A[i]) % q;
	}
	fft->transform(fft, A, a, TRUE);

	DBG4(DBG_LIB, "   i   f   g     a     F     G     A");
	for (i = 0; i < n; i++)
	{
		DBG4(DBG_LIB, "%4d %3d %3d %5u %5u %5u %5u",
			 i, f[i], g[i], a[i], F[i], G[i], A[i]);
	}

	/* Cleanup */
	fft->destroy(fft);
	free(a);
	free(A);
	free(F);
	free(G);

	return &this->public;
}

/**
 * See header.
 */
bliss_private_key_t *bliss_private_key_load(key_type_t type, va_list args)
{
	private_bliss_private_key_t *this;

	while (TRUE)
	{
		switch (va_arg(args, builder_part_t))
		{
			default:
				return NULL;
		}
		break;
	}

	this = bliss_private_key_create_empty();

	return &this->public;
}

