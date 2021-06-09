/****************************************************************************
 *
 * (C) 2021 Cadence Design Systems, Inc. All rights reserved worldwide.
 *
 * This sample source code is not supported by Cadence Design Systems, Inc.
 * It is provided freely for demonstration purposes only.
 * SEE THE WARRANTY DISCLAIMER AT THE BOTTOM OF THIS FILE.
 *
 ***************************************************************************/
/****************************************************************************
*
* SU2 Grid Import Plugin (GRDP)
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
 * This file is licensed under the Cadence Public License Version 1.0 (the
 * "License"), a copy of which is found in the included file named "LICENSE",
 * and is distributed "AS IS." TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE
 * LAW, CADENCE DISCLAIMS ALL WARRANTIES AND IN NO EVENT SHALL BE LIABLE TO
 * ANY PARTY FOR ANY DAMAGES ARISING OUT OF OR RELATING TO USE OF THIS FILE.
 * Please see the License for the full text of applicable terms.
 *
 ****************************************************************************/
