/*
 * memory_organization.h
 *
 *  Created on: 26 gru 2015
 *      Author: Konrad
 */

#ifndef LIBRARIES_MEMORY_ORGANIZATION_H_
#define LIBRARIES_MEMORY_ORGANIZATION_H_

#include "stdint-gcc.h"


#define	MEM_ORG_KEY_AREA_START_ADDRESS				(uint32_t)0x28800
#define MEM_ORG_KEY_AREA_END_ADDRESS				(uint32_t)0x29000

#define MEM_ORG_DATA_AREA_START_ADDRESS				MEM_ORG_KEY_AREA_END_ADDRESS
#define MEM_ORG_DATA_AREA_END_ADDRESS				(uint32_t)0x3F800

#endif /* LIBRARIES_MEMORY_ORGANIZATION_H_ */
