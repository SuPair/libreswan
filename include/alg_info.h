/* Algorithm info parsing and creation functions
 *
 * Author: JuanJo Ciarlante <jjo-ipsec@mendoza.gov.ar>
 * Copyright (C) 2007 Michael Richardson <mcr@xelerance.com>
 * Copyright (C) 2012-2013 Paul Wouters <paul@libreswan.org>
 * Copyright (C) 2013 D. Hugh Redelmeier <hugh@mimosa.com>
 * Copyright (C) 2013 Paul Wouters <pwouters@redhat.com>
 * Copyright (C) 2015-2017 Andrew Cagney
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

#ifndef ALG_INFO_H
#define ALG_INFO_H

#include "constants.h"
#include "ike_alg.h"

struct parser_context;
struct alg_info;

/*
 * Place holder so that it is possible to clearly differentiate
 * between an unspecified rather than 'null' integrity algorithm.
 *
 * NOTE:
 *
 * The callback .alg_info_add() is passed NULL to identify an
 * unspecified algorithm (in keeping with the C tradition of 'no
 * value'), and either 'alg_info_integ_null' or 'ike_alg_encrypt_null'
 * to identify an explicitly specified 'null' algorithm
 *
 * Only when the algorithm is NULL (unspecified or missing) should
 * .alg_info_add() consider adding in defaults.  For instance: in
 * esp=aes, neither the integrity nor DH algorithm were specified so
 * both would be NULL; however in esp=aes_gcm-null, integrity was
 * specified as 'null' so 'ike_alg_integ_null' is used, but DH would
 * still be NULL.
 *
 * 'struct state', on the other hand, uses NULL for 'null' integrity,
 * and 'ike_alg_encrypt_null' for 'null' encryption.  .alg_info_add()
 * must deal with this (for the moment, 'ike_alg_integ_null' should
 * never escape the parser).
 *
 * Why not use NULL for 'null' and something else for unspecifed in
 * the parser?  Several reasons: as noted above, NULL is C's universal
 * identifier of 'no value'; having having lots of ike_alg_XXX_missing
 * structs quickly gets more messy.
 */
extern const struct integ_desc alg_info_integ_null;

/*
 * Parameters to tune the parser.
 */

struct parser_policy {
	bool ikev1;
	bool ikev2;
};

/*
 * Parameters to set the parser's basic behaviour - ESP/AH/IKE.
 */

struct parser_param {
	const char *protocol;
	enum ike_alg_key ikev1_alg_id;
	bool (*alg_is_implemented)(const struct ike_alg *alg);

	/*
	 * XXX: Is the proto-id needed?  Parser should be protocol
	 * agnostic.
	 */
	unsigned protoid;
	/*
	 * If things go wrong, return a non-null error string
	 * (possibly snprintf'd into ERR_BUF).
	 */
	const char *(*alg_info_add)(const struct parser_policy *const policy,
				    struct alg_info *alg_info,
				    const struct encrypt_desc *encrypt, int ek_bits,
				    const struct prf_desc *prf,
				    const struct integ_desc *integ,
				    const struct oakley_group_desc *dh_group,
				    char *err_buf, size_t err_buf_len);

	/*
	 * This lookup functions must set err and return null if NAME
	 * isn't valid.
	 */
	const struct ike_alg *(*encrypt_alg_byname)(const struct parser_param *param,
						    const struct parser_policy *const policy,
						    char *err_buf, size_t err_buf_len,
						    const char *name, size_t bit_length);
	const struct ike_alg *(*prf_alg_byname)(const struct parser_param *param,
						const struct parser_policy *const policy,
						char *err_buf, size_t err_buf_len,
						const char *name, size_t key_bit_length);
	const struct ike_alg *(*integ_alg_byname)(const struct parser_param *param,
						  const struct parser_policy *const policy,
						  char *err_buf, size_t err_buf_len,
						  const char *name, size_t key_bit_length);
	const struct ike_alg *(*dh_alg_byname)(const struct parser_param *param,
					       const struct parser_policy *const policy,
					       char *err_buf, size_t err_buf_len,
					       const char *name, size_t key_bit_length);
};

/*
 * A proposal as decoded by the parser.
 */

struct proposal_info {
	/*
	 * The encryption algorithm and key length.
	 *
	 * Because struct encrypt_desc still specifies multiple key
	 * lengths, ENCKEYLEN is still required.
	 */
	const struct encrypt_desc *encrypt;
	size_t enckeylen;    /* keylength for ESP transform (bits) */
	u_int8_t ikev1esp_transid;       /* enum ipsec_cipher_algo: ESP transform (AES, 3DES, etc.)*/
	/*
	 * The integrity and PRF algorithms.
	 */
	const struct prf_desc *prf;
	const struct integ_desc *integ;
	u_int16_t ikev1esp_auth;         /* enum ikev1_auth_attribute: AUTH */
	/*
	 * PFS/DH negotiation.
	 */
	const struct oakley_group_desc *dh;
};

/* common prefix of struct alg_info_esp and struct alg_info_ike */
struct alg_info {
	int alg_info_cnt;
	int ref_cnt;
	unsigned alg_info_protoid;
	struct proposal_info proposals[128];
};

struct alg_info_esp {
	struct alg_info ai;	/* common prefix */
	const struct oakley_group_desc *esp_pfsgroup;
};

struct alg_info_ike {
	struct alg_info ai;	/* common prefix */
};

extern void alg_info_free(struct alg_info *alg_info);
extern void alg_info_addref(struct alg_info *alg_info);
extern void alg_info_delref(struct alg_info *alg_info);

extern struct alg_info_ike *alg_info_ike_create_from_str(const struct parser_policy *policy,
							 const char *alg_str,
							 char *err_buf, size_t err_buf_len);

extern struct alg_info_esp *alg_info_esp_create_from_str(const struct parser_policy *policy,
							 const char *alg_str,
							 char *err_buf, size_t err_buf_len);

extern struct alg_info_esp *alg_info_ah_create_from_str(const struct parser_policy *policy,
							const char *alg_str,
							char *err_buf, size_t err_buf_len);

void alg_info_ike_snprint(char *buf, size_t buflen,
			  const struct alg_info_ike *alg_info_ike);
void alg_info_esp_snprint(char *buf, size_t buflen,
			  const struct alg_info_esp *alg_info_esp);

void alg_info_snprint_ike(char *buf, size_t buflen,
			  struct alg_info_ike *alg_info);

void alg_info_snprint_ike_info(char *buf, size_t buflen,
			       const struct proposal_info *ike_info);
void alg_info_snprint_esp_info(char *buf, size_t buflen,
			       const struct proposal_info *esp_info);
void alg_info_snprint_phase2(char *buf, size_t buflen,
			     struct alg_info_esp *alg_info);

/*
 * Iterate through the elements of an ESP or IKE table.
 *
 * Use __typeof__ instead of const to get around ALG_INFO some times
 * being const and sometimes not.
 *
 * XXX: yes, they are the same!
 */

#define FOR_EACH_PROPOSAL_INFO(ALG_INFO, PROPOSAL_INFO)			\
	for (__typeof__((ALG_INFO)->proposals[0]) *(PROPOSAL_INFO) = (ALG_INFO)->proposals; \
	     (PROPOSAL_INFO) < (ALG_INFO)->proposals + (ALG_INFO)->alg_info_cnt; \
	     (PROPOSAL_INFO)++)

#define FOR_EACH_ESP_INFO(ALG_INFO, ESP_INFO)		\
	FOR_EACH_PROPOSAL_INFO(&((ALG_INFO)->ai), ESP_INFO)

#define FOR_EACH_IKE_INFO(ALG_INFO, IKE_INFO)		\
	FOR_EACH_PROPOSAL_INFO(&((ALG_INFO)->ai), IKE_INFO)

/*
 * on success: returns alg_info
 * on failure: pfree(alg_info) and return NULL;
 *
 * POLICY should be used to guard algorithm supported checks.  For
 * instance: if POLICY=IKEV1, then IKEv1 support is required (IKEv2 is
 * don't care); and if POLICY=IKEV1|IKEV2, then both IKEv1 and IKEv2
 * support is required.
 *
 * Parsing with POLICY=IKEV1, but then proposing the result using
 * IKEv2 is a program error.  The IKEv2 should complain loudly and
 * hopefully not crash.
 *
 * Parsing with POLICY='0' is allowed. It will accept the algorithms
 * unconditionally (spi.c seems to need this).
 */
struct alg_info *alg_info_parse_str(const struct parser_policy *policy,
				    struct alg_info *alg_info,
				    const char *alg_str,
				    char *err_buf, size_t err_buf_len,
				    const struct parser_param *param);

#endif /* ALG_INFO_H */
