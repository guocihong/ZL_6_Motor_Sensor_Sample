/*H**************************************************************************
* NAME:   compiler.h         
*----------------------------------------------------------------------------
* Copyright (c) 2008.
*----------------------------------------------------------------------------
* RELEASE:   2008.11.14
* REVISION:  1.0     
*----------------------------------------------------------------------------
* PURPOSE:   for 51 serial MCU and KEIL C compiler.
*****************************************************************************/

#ifndef _COMPILER_H_
#define _COMPILER_H_


/*_____ M A C R O S ________________________________________________________*/
#define BIT0  0x01
#define BIT1  0x02
#define BIT2  0x04
#define BIT3  0x08
#define BIT4  0x10
#define BIT5  0x20
#define BIT6  0x40
#define BIT7  0x80


/* Type definition */
typedef unsigned char      Byte;
typedef unsigned char      byte;
typedef unsigned char      Uchar;
typedef unsigned char      uchar;   
typedef unsigned char      Uint8;
typedef unsigned int       Uint16;
typedef unsigned int       uint;
typedef int                Int16;
typedef float              Float16;
typedef unsigned long int  Uint32;
typedef unsigned long      ulong;
typedef long int           Int32;

typedef unsigned char      INT8U;
typedef unsigned int       INT16U;


typedef union 
{
  Uint32 l;
  Uint16 w[2];
  Byte   b[4];
} Union32;


typedef union 
{
  Uint16 w;
  Byte   b[2];
} Union16;


/***  General purpose defines ***/
#define FALSE   0
#define TRUE    1
#define KO      0
#define OK      1
#define OFF     0
#define ON      1
#define CLR     0
#define SET     1
#define	NO      0
#define	YES     1

#define LOW(U16)   ((Byte)U16)
#define HIGH(U16)  ((Byte)(U16>>8))

/* C51 use Big endian by default */
/* little endian conversion */
#define LE16(b) (((b & 0xFF) << 8) | ((b & 0xFF00) >> 8))            // b 为16位字
#define LE32(b) (((b & 0xFF) << 24) | ((b & 0xFF00) << 8) | \        
                 ((b & 0xFF0000) >> 8) | ((b & 0xFF000000) >> 24))   // b 为32位字

#define TST_BIT_X(addrx,mask)       (*addrx & mask)               //test a bit
#define SET_BIT_X(addrx,mask)       (*addrx = (*addrx | mask))    //set a bit
#define CLR_BIT_X(addrx,mask)       (*addrx = (*addrx & ~mask))   //clear a bit
#define OUT_X(addrx,value)          (*addrx = value)
#define IN_X(addrx)                 (*addrx)

/***********************************************************
 SET_SFR_BIT macro 
  parameters 
    sfr_reg : defined value in include file for sfr register 
    bit_pos : defined value B_XX in include file for particular
              bit of sfr register, scope is 0 ~ 7
    bit_val : CLR / SET 
************************************************************/
#define SET_SFR_BIT(sfr_reg, bit_pos, bit_val) {sfr_reg &= ~(1<<(bit_pos)); sfr_reg |= ((bit_val)<<(bit_pos));}
/***********************************************************
 TST_SFR_BIT macro 
  parameters 
    sfr_reg : defined value in include file for sfr register 
    bit_pos : defined value B_XX in include file for particular
              bit of sfr register 
************************************************************/
#define TST_SFR_BIT(sfr_reg, bit_pos) ((sfr_reg & (1<<(bit_pos)))>>(bit_pos))

#define Reentrant(x)      x reentrant
#define Sfr(x,y)          sfr x = y
#define Sfr16(x,y)        sfr16 x = y
#define Sbit(x,y,z)       sbit x = y ^ z
#define Interrupt(x,y)    x interrupt y
#define At(x)             _at_ x


#endif /* _COMPILER_H_ */

