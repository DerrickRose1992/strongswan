/*
 * Copyright (C) 2010 Andreas Steffen
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

/**
 * @defgroup pb_pa_msg pb_pa_msg
 * @{ @ingroup tnccs_20
 */

#ifndef PB_PA_MSG_H_
#define PB_PA_MSG_H_

typedef struct pb_pa_msg_t pb_pa_msg_t;

#include "pb_tnc_msg.h"

/**
 * Class representing the PB-PA message type.
 */
struct pb_pa_msg_t {

	/**
	 * PB-TNC Message interface
	 */
	pb_tnc_msg_t pb_interface;

	/**
	 * Get PA Message Vendor ID and Subtype
	 *
	 * @param subtype		PA Subtype
	 * @return				PA Message Vendor ID
	 */
	u_int32_t (*get_vendor_id)(pb_pa_msg_t *this, u_int32_t *subtype);

	/**
	 * Get Posture Collector ID
	 *
	 * @return				Posture Collector ID
	 */
	u_int16_t (*get_collector_id)(pb_pa_msg_t *this);

	/**
	 * Get Posture Validator ID
	 *
	 * @return				Posture Validator ID
	 */
	u_int16_t (*get_validator_id)(pb_pa_msg_t *this);

	/**
	 * Get the PA Message Body
	 *
	 * @return				PA Message Body
	 */
	chunk_t (*get_body)(pb_pa_msg_t *this);

	/**
	 * Get the exclusive flag
	 *
	 * @return				exclusive flag
	 */
	bool (*get_exclusive_flag)(pb_pa_msg_t *this);

	/**
	 * Set the exclusive flag
	 *
	 * @param excl			vexclusive flag
	 */
	void (*set_exclusive_flag)(pb_pa_msg_t *this, bool excl);
};

/**
 * Create a PB-PA message from parameters
 *
 * @param vendor_id			PA Message Vendor ID
 * @param subtype			PA Subtype		
 * @param collector_id		Posture Collector ID
 * @param validator_id		Posture Validator ID
 * @param msg_body		 	PA Message Body
 */
pb_tnc_msg_t *pb_pa_msg_create(u_int32_t vendor_id, u_int32_t subtype,
							   u_int16_t collector_id, u_int16_t validator_id,
							   chunk_t msg_body);

/**
 * Create an unprocessed PB-PA message from raw data
 *
  * @param data		PB-PA message data
 */
pb_tnc_msg_t* pb_pa_msg_create_from_data(chunk_t data);

#endif /** PB_PA_MSG_H_ @}*/
