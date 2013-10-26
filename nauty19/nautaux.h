/*****************************************************************************
*                                                                            *
* This is the header file for versions 1.9 of nautaux.c.                     *
*                                                                            *
*   Copyright (1984-1993) Brendan McKay.  All rights reserved.               *
*   Subject to the waivers and disclaimers in nauty.h.                       *
*                                                                            *
*   CHANGE HISTORY                                                           *
*       26-Apr-89 : initial creation for version 1.5.                        *
*       14-Oct-90 : renamed as version 1.6 (no changes to this file)         *
*        5-Jun-93 : renamed as version 1.7+ (no changes to this file)        *
*       18-Aug-93 : renamed as version 1.8 (no changes to this file)         *
*       17-Sep-93 : renamed as version 1.9 (no changes to this file)         *
*       19-Apr-95 : added prototype wrapper for C++                          *
*                                                                            *
*****************************************************************************/

#include "nauty.h"           /* which includes stdio.h */

#ifdef __cplusplus
extern "C" {
#endif

EXTPROC(boolean equitable,(graph*,nvector*,nvector*,int,int,int))
EXTPROC(long ptncode,(graph*,nvector*,nvector*,int,int,int))
EXTPROC(int component,(graph*,int,set*,int,int))

#ifdef __cplusplus
}
#endif
