/*--------------------------------------------------------------------------
 * File Contents:
 *   lib_crc.c
 *    The file lib_crc.c contains the private and public functions used for
 *    the calculation of CRC-16, CRC-CCITT and CRC-32 cyclic redundancy values.
 *  Project:
 *	UartUpdateTool
 *---------------------------------------------------------------------------
 */

#define LIB_CRC_C

#include "lib_crc.h"


   /*********************************************************************
    *                                                                   *
    *   Library         : lib_crc                                       *
    *   File            : lib_crc.c                                     *
    *   Author          : Lammert Bies  1999-2005                       *
    *   E-mail          : info@lammertbies.nl                           *
    *   Language        : ANSI C                                        *
    *                                                                   *
    *                                                                   *
    *   Description                                                     *
    *   ===========                                                     *
    *                                                                   *
    *   The file lib_crc.c contains the private  and  public  func-     *
    *   tions  used  for  the  calculation of CRC-16, CRC-CCITT and     *
    *   CRC-32 cyclic redundancy values.                                *
    *                                                                   *
    *                                                                   *
    *   Dependencies                                                    *
    *   ============                                                    *
    *                                                                   *
    *   lib_crc.h       CRC definitions and prototypes                  *
    *                                                                   *
    *                                                                   *
    *   Modification history                                            *
    *   ====================                                            *
    *                                                                   *
    *   Date        Version Comment                                     *
    *                                                                   *
    *   2005-05-14  1.12    Added CRC-CCITT with start value 0          *
    *                                                                   *
    *   2005-02-05  1.11    Fixed bug in CRC-DNP routine                *
    *                                                                   *
    *   2005-02-04  1.10    Added CRC-DNP routines                      *
    *                                                                   *
    *   1999-02-21  1.01    Added FALSE and TRUE mnemonics              *
    *                                                                   *
    *   1999-01-22  1.00    Initial source                              *
    *                                                                   *
    *********************************************************************
    */



   /*********************************************************************
    *                                                                   *
    *   #define P_xxxx                                                  *
    *                                                                   *
    *   The CRC's are computed using polynomials. The  coefficients     *
    *   for the algorithms are defined by the following constants.      *
    *                                                                   *
    *********************************************************************
    */


#ifdef CRC_16
#define                 P_16        0xA001
#endif /* CRC_16 */

#ifdef CRC_32
#define                 P_32        0xEDB88320L
#endif /* CRC_32 */

#ifdef CRC_CCITT
#define                 P_CCITT     0x1021
#endif /* CRC_CCITT */

#ifdef CRC_DNP
#define                 P_DNP       0xA6BC
#endif /* CRC_DNP */



   /*********************************************************************
    *                                                                   *
    *   static void crc...tab_val();                                    *
    *                                                                   *
    *   Three local functions are used  to  initialize  the  tables     *
    *   with values for the algorithm.                                  *
    *                                                                   *
    *********************************************************************
    */
#ifdef CRC_16
static unsigned short   crc16_tab_val(unsigned short);
#endif /* CRC_16 */

#ifdef CRC_32
static unsigned long    crc32_tab_val(unsigned long);
#endif /* CRC_32 */

#ifdef CRC_CCITT
static unsigned short   crcccitt_tab_val(unsigned short);
#endif /* CRC_CCITT */

#ifdef CRC_DNP
static unsigned short   crcdnp_tab_val(unsigned short);
#endif /* CRC_DNP */


   /*********************************************************************
    *                                                                   *
    *   unsigned short update_crc( unsigned long crc, char c );         *
    *                                                                   *
    *   The function update_crc calculates  a  new  CRC-CCITT           *
    *   value  based  on the previous value of the CRC and the next     *
    *   byte of the data to be checked.                                 *
    *                                                                   *
    *********************************************************************
    */
#ifdef CRC_CCITT
unsigned short update_crc(unsigned short crc, char c)
{
	unsigned short tmp, short_c;

	short_c  = 0x00ff & (unsigned short) c;

	tmp = (crc >> 8) ^ short_c;
	crc = (crc << 8) ^ crcccitt_tab_val(tmp);

	return crc;

}  /* update_crc */
#endif /* CRC_CCITT */


   /*********************************************************************
    *                                                                   *
    *   unsigned short update_crc( unsigned long crc, char c );         *
    *                                                                   *
    *   The function update_crc calculates a  new  CRC-16  value        *
    *   based  on  the  previous value of the CRC and the next byte     *
    *   of the data to be checked.                                      *
    *                                                                   *
    *********************************************************************
    */
#ifdef CRC_16
unsigned short update_crc16(unsigned short crc, char c)
{
	unsigned short tmp, short_c;

	short_c = 0x00ff & (unsigned short) c;
	tmp =  crc       ^ short_c;
	crc = (crc >> 8) ^ crc16_tab_val(tmp & 0xff);

	return crc;

}  /* update_crc */
#endif /* CRC_16 */


   /*********************************************************************
    *                                                                   *
    *   unsigned short update_crc( unsigned long crc, char c );         *
    *                                                                   *
    *   The function update_crc calculates a new CRC-DNP  value         *
    *   based  on  the  previous value of the CRC and the next byte     *
    *   of the data to be checked.                                      *
    *                                                                   *
    *********************************************************************
    */
#ifdef CRC_DNP
unsigned short update_crc(unsigned short crc, char c)
{
	unsigned short tmp, short_c;

	short_c = 0x00ff & (unsigned short) c;

	tmp =  crc       ^ short_c;
	crc = (crc >> 8) ^ crcdnp_tab_val(tmp & 0xff);

	return crc;

}  /* update_crc */
#endif /* CRC_DNP */


   /*********************************************************************
    *                                                                   *
    *   unsigned long update_crc( unsigned long crc, char c );          *
    *                                                                   *
    *   The function update_crc calculates a  new  CRC-32  value        *
    *   based  on  the  previous value of the CRC and the next byte     *
    *   of the data to be checked.                                      *
    *                                                                   *
    *********************************************************************
    */
#ifdef CRC_32
unsigned long update_crc32(unsigned long crc, char c)
{

	unsigned long tmp, long_c;

	long_c = 0x000000ffL & (unsigned long) c;

	tmp = crc ^ long_c;
	crc = (crc >> 8) ^ crc32_tab_val(tmp & 0xff);

	return crc;

}  /* update_crc */
#endif /* CRC_32 */


   /*********************************************************************
    *                                                                   *
    *   static void crc16_tab_val( unsigned short );                    *
    *                                                                   *
    *   The function crc16_tab_val() is used  to  fill  the  array      *
    *   for calculation of the CRC-16 with values.                      *
    *                                                                   *
    *********************************************************************
    */
#ifdef CRC_16
static unsigned short crc16_tab_val(unsigned short c)
{

	int j;
	unsigned short crc;

	crc = 0;

	for (j = 0; j < 8; j++) {
		if ((crc ^ c) & 0x0001)
			crc = (crc >> 1) ^ P_16;
		else
			crc =   crc >> 1;

		c = c >> 1;
	}

	return crc;

}  /* crc16_tab_val */
#endif /* CRC_16 */


   /*********************************************************************
    *                                                                   *
    *   static unsigned short crcdnp_tab_val( unsigned short c );       *
    *                                                                   *
    *   The function crcdnp_tab_val() is used  to  fill  the  array     *
    *   for calculation of the CRC-DNP with values.                     *
    *                                                                   *
    *********************************************************************
    */
#ifdef CRC_DNP
static unsigned short crcdnp_tab_val(unsigned short c)
{
	int j;
	unsigned short crc;

	crc = 0;

	for (j = 0; j < 8; j++) {

		if ((crc ^ c) & 0x0001)
			crc = (crc >> 1) ^ P_DNP;
		else
			crc = crc >> 1;

		c = c >> 1;
	}

	return crc;

}  /* crcdnp_tab_val */
#endif /* CRC_DNP */


   /*********************************************************************
    *                                                                   *
    *   static unsigned long init_crc32_tab( unsigned long );           *
    *                                                                   *
    *   The function init_crc32_tab() is used  to  fill  the  array     *
    *   for calculation of the CRC-32 with values.                      *
    *                                                                   *
    *********************************************************************
    */
#ifdef CRC_32
static unsigned long crc32_tab_val(unsigned long c)
{

	int  j;
	unsigned long crc;

	crc = (unsigned long) c;

	for (j = 0; j < 8; j++) {
		if (crc & 0x00000001L)
			crc = (crc >> 1) ^ P_32;
		else
			crc = crc >> 1;
	}

	return crc;

}  /* crc32_tab_val */
#endif /* CRC_32 */


   /*********************************************************************
    *                                                                   *
    *   static unsigned short crcccitt_tab_val( unsigned short );       *
    *                                                                   *
    *   The function crcccitt_tab_val() is used to fill the  array      *
    *   for calculation of the CRC-CCITT with values.                   *
    *                                                                   *
    *********************************************************************
    */
#ifdef CRC_CCITT
static unsigned short crcccitt_tab_val(unsigned short c)
{

	int j;
	unsigned short crc;


	crc = 0;
	c   = ((unsigned short) c) << 8;

	for (j = 0; j < 8; j++) {
		if ((crc ^ c) & 0x8000)
			crc = (crc << 1) ^ P_CCITT;
		else
			crc = crc << 1;

		c = c << 1;
	}

	return crc;

}  /* crcccitt_tab_val */
#endif /* CRC_CCITT */


#undef LIB_CRC_C
