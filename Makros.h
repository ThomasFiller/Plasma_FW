//######################################################################################|
//                                                                                      |
//      Bit manipulation macros                                                         |
//                                                                                      |
//######## begin      ##################################################################|

#ifndef __MACROS_H_included
#define __MACROS_H_included

#define __BIT_MASK(A) (1<<(A))
#define SetBitMask( SFR, MASK ) 		((SFR)	|=	(MASK))
#define ClrBitMASK( SFR, MASK ) 		((SFR)	&=	~(MASK))
#define CheckBit1( X, MASK ) 		((X) 		& 	(MASK))
#define ToggleBit( SFR, MASK )	((SFR)	^=	(MASK))
//########   end    Bit manipulation macros       ######################################|
#endif