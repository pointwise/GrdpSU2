/****************************************************************************
*
* SU2 Grid Import Plugin (GRDP)
*
* Copyright (c) 2012-2018 Pointwise, Inc.
* All rights reserved.
*
* This sample Pointwise plugin is not supported by Pointwise, Inc.
* It is provided freely for demonstration purposes only.
* SEE THE WARRANTY DISCLAIMER AT THE BOTTOM OF THIS FILE.
*
***************************************************************************/

#ifndef _RTGRDPINITITEMS_H_
#define _RTGRDPINITITEMS_H_

/*.................................................
    initialize grdpRtItem[0]
*/
#define ID_GrdpSU2  390
{
    //== GRDP_FORMATINFO FormatInfo
    {   PWP_SITE_GROUPNAME,     // const char *group
        "SU2",                  // const char *name
        MAKEGUID(ID_GrdpSU2),   // PWP_UINT32 id
        PWP_FILEDEST_FILENAME,  // PWP_ENUM_FILEDEST fileDest
        "su2"                   // const char *exts
    },

    &pwpRtItem[1],  // PWU_RTITEM*

    0,  // FILE *fp

    // PWU_UNFDATA UnfData
    {   0,          // PWP_UINT32 status
        0,          // FILE *fp
        0,          // sysFILEPOS fPos
        PWP_FALSE,  // PWP_BOOL hadError
        PWP_FALSE,  // PWP_BOOL inRec
        0,          // PWP_UINT32 recBytes
        0,          // PWP_UINT32 totRecBytes
        0    },     // PWP_UINT32 recCnt

    PWGM_HGRIDMODEL_INIT,  // PWGM_HGRIDMODEL model

    0,    // const GRDP_READINFO * pReadInfo;

    0,    // PWP_UINT32 progTotal
    0,    // PWP_UINT32 progComplete
    {0},  // clock_t clocks[GRDP_CLKS_SIZE];
    0,    // PWP_BOOL opAborted

    // If you added any custom data in rtCaepInstanceData.h,
    // you need to initialize it here. The init below matches the 
    // example MY_GRDP_DATA struct given in rtCaepInstanceData.h */
    //{   0,
    //    0,
    //    0.0,
    //    "string" },
},

#endif /* _RTGRDPINITITEMS_H_ */


/****************************************************************************
*
* DISCLAIMER:
* TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, POINTWISE DISCLAIMS
* ALL WARRANTIES, EITHER EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED
* TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
* PURPOSE, WITH REGARD TO THIS SCRIPT. TO THE MAXIMUM EXTENT PERMITTED
* BY APPLICABLE LAW, IN NO EVENT SHALL POINTWISE BE LIABLE TO ANY PARTY
* FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES
* WHATSOEVER (INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF
* BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS) ARISING OUT OF THE
* USE OF OR INABILITY TO USE THIS SCRIPT EVEN IF POINTWISE HAS BEEN
* ADVISED OF THE POSSIBILITY OF SUCH DAMAGES AND REGARDLESS OF THE
* FAULT OR NEGLIGENCE OF POINTWISE.
*
***************************************************************************/
