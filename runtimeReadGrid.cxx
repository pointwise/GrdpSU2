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

#include <algorithm>
#include <cctype>
#include <cerrno>
#include <cstdlib> 
#include <functional> 
#include <sstream>
#include <vector>

#include "apiGRDP.h"
#include "apiGridModel.h"
#include "apiPWP.h"
#include "PwpFile.h"
#include "runtimeReadGrid.h"


typedef std::vector<std::string>    StringArray1;

static const PWGM_HVERTEXLIST   BadVertList = PWGM_HVERTEXLIST_INIT;
static const PWGM_ELEMCOUNTS    ZeroCounts = { {0} };

enum SU2ElemType {
    SU2Tri      = 5,
    SU2Quad     = 9,
    SU2Tet      = 10,
    SU2Pyramid  = 14,
    SU2Wedge    = 13,
    SU2Hex      = 12
};

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

class SU2GridReader {
public:
   
    SU2GridReader(GRDP_RTITEM *pRti) :
        pRti_(pRti),
        in_(),
        line_(),
        posNELEMData_(),
        posNPOINData_(),
        gridIs3D_(false),
        nPoints_(0),
        nElems_(0),
        nElemTypes_(ZeroCounts),
        hVL_(BadVertList)
    {}

    ~SU2GridReader() {}


    PWP_BOOL read()
    {
        const PWP_UINT32 NumMajorSteps = 4;
        return grdpProgressEnd(pRti_, grdpProgressInit(pRti_, NumMajorSteps) &&
            init() && readVertices() && loadCells());
    }


private:

    // trim leading whitespace
    static inline std::string &
    ltrim(std::string &s)
    {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(),
            std::not1(std::ptr_fun<int, int>(std::isspace))));
        return s;
    }


    // trim trailing whitespace
    static inline std::string &
    rtrim(std::string &s)
    {
        s.erase(std::find_if(s.rbegin(), s.rend(),
            std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
        return s;
    }


    // trim leading and trailing whitespace
    static inline std::string &
    trim(std::string &s)
    {
        return ltrim(rtrim(s));
    }


    // Convert a char* of specified base to an integer value of type T.
    template<typename T>
    static inline bool
    toInt(const char *str, T &val, const int base = 10)
    {
        char* endptr = 0;
        if (str && ('\0' != *str)) {
            errno = 0;
            val = static_cast<T>(strtol(str, &endptr, base));
            if (0 != errno) {
                endptr = 0;
            }
        }
        return endptr && ('\0' == *endptr);
    }


    // Convert a std::string of specified base to an integer value of type T.
    template<typename T>
    static inline bool
    toInt(const std::string &str, T &val, const int base = 10)
    {
        return toInt(str.c_str(), val, base);
    }


    // Convert a char* to a floating point value of type T.
    template<typename T>
    static inline bool
    toDbl(const char *str, T &val)
    {
        char* endptr = 0;
        if (str && ('\0' != *str)) {
            errno = 0;
            val = strtod(str, &endptr);
            if (0 != errno) {
                endptr = 0;
            }
        }
        return endptr && ('\0' == *endptr);
    }


    // Convert a std::string to a floating point value of type T.
    template<typename T>
    static inline bool
    toDbl(const std::string &str, T &val)
    {
        return toDbl(str.c_str(), val);
    }


    // Populate a PWGM_ELEMDATA from an array of std::string values. The array
    // values must be in "Type Vertex1 ... VertexN Index" order.
    static inline bool
    toksToElem(const StringArray1 &toks, PWGM_ENUM_ELEMTYPE type,
        const PWP_UINT32 cnt, PWGM_ELEMDATA &elem)
    {
        // toks MUST be in "Type Vertex1 ... VertexN Index" order.
        bool ret = (toks.size() == (cnt + 2));
        if (ret) {
            elem.type = type;
            elem.vertCnt = cnt;
            for (PWP_UINT32 ii = 0; ii < cnt; ++ii) {
                // Use ii+1 to skip past the "Type" value in toks.at(0)
                if (!toInt(toks.at(ii + 1), elem.index[ii])) {
                    ret = false;
                    break;
                }
            }
        }
        return ret;
    }


    // Reads the next non-empty, non-comment line into line_. Returns true if
    // line_ contains data. Returns false if EOF.
    bool
    readLine()
    {
        static const size_t BufSz = 1024;
        char buf[BufSz];
        bool ret = false;
        while (fgets(buf, BufSz, in_.fp())) {
            line_ = buf;
            if (trim(line_).empty() || ('%' == line_.at(0))) {
                // line_ is empty or comment - skip and get next
                continue;
            }
            ret = true;
            break;
        }
        return ret;
    }


    // Reads the next line and splits it into tokens (space delimited).
    // Returns true if toks contains data. Returns false if EOF.
    bool
    readLineTokens(StringArray1 &toks)
    {
        bool ret = readLine();
        if (ret) {
            toks.clear();
            std::string lineBuf;
            std::stringstream ss(line_);
            while (ss >> lineBuf) {
                // capture whitespace separated tokens
                toks.push_back(lineBuf);
            }
        }
        return ret;
    }


    // Attempts to parse line as a "key=value" string into a key and value.
    // Returns false if parsing fails. Returns true if key and value are set.
    // The key and value are trimmed of all leading and trailing whitespoace.
    static inline bool
    splitKeyVal(const std::string &line, std::string &key, std::string &val)
    {
        const size_t n = line.find('=');
        bool ret = (std::string::npos != n);
        if (ret) {
            key = line.substr(0, n);
            trim(key);
            val = line.substr(n + 1);
            trim(val);
        }
        return ret && !key.empty();
    }


    // Send an error message to the application with optional detail text.
    // The message is of the form: "msg" or "msg: 'detail'"
    void
    reportError(const char *msg, const std::string &detail)
    {
        std::ostringstream oss;
        oss << msg;
        if (!detail.empty()) {
            oss << ": '" << detail << "'";
        }
        grdpSendErrorMsg(pRti_, oss.str().c_str(), 0);
    }


    // Send an error message to the application with the current line_ as the
    // detail text.
    void
    reportError(const char *msg)
    {
        reportError(msg, line_);
    }


    // Before importing the grid data, scan file looking for certain "key=value"
    // pairs and cache the file positions for the cell and vertex data.
    bool
    init()
    {
        bool ret = false;
        // IMPORTANT! MUST use pwpBinary when opening file to prevent platform
        // EOL differences from breaking file position handling.
        if (!in_.open(pRti_->pReadInfo->fileDest, pwpRead | pwpBinary)) {
            reportError("Could not open file", pRti_->pReadInfo->fileDest);
        }
        else if (grdpProgressBeginStep(pRti_, 3)) {
            bool foundNDIME = false;
            bool foundNELEM = false;
            bool foundNPOIN = false;
            std::string key;
            std::string val;
            while (readLine()) {
                if (!splitKeyVal(line_, key, val)) {
                    // not a "key=value" pair
                    continue;
                }
                if ("NDIME" == key) {
                    if (foundNDIME) {
                        reportError("Duplicate NDIME value");
                        break;
                    }
                    if (!parseNDIMEVal(val)) {
                        reportError("Invalid NDIME value");
                        break;
                    }
                    foundNDIME = true;
                }
                else if ("NELEM" == key) {
                    if (foundNELEM) {
                        reportError("Duplicate NELEM value");
                        break;
                    }
                    if (!parseNELEMVal(val)) {
                        reportError("Invalid NELEM value");
                        break;
                    }
                    foundNELEM = true;
                }
                else if ("NPOIN" == key) {
                    if (foundNPOIN) {
                        reportError("Duplicate NPOIN value");
                        break;
                    }
                    if (!parseNPOIN(val)) {
                        reportError("Invalid NPOIN value");
                        break;
                    }
                    foundNPOIN = true;
                }
                else {
                    // Not anything we are interested in - read next line
                    continue;
                }
                // If here, we found a value
                if (foundNDIME && foundNELEM && foundNPOIN) {
                    // We have everything we need - we can stop scanning.
                    // Caveat: Will not detect if values are erroneously duped
                    //         later in file.
                    ret = true;
                    break;
                }
                if (!grdpProgressIncr(pRti_)) {
                    break;
                }
            }
        }
        return grdpProgressEndStep(pRti_) && ret &&
            (gridIs3D_ ? getCellCounts3() : getCellCounts2());
    }


    // Extract the grid's dimensionality from str.
    bool
    parseNDIMEVal(const std::string &str)
    {
        PWP_UINT32 dimty;
        return (toInt(str, dimty) && ((2 == dimty) || (3 == dimty))) ?
            ((gridIs3D_ = (3 == dimty)), true) : false;
    }


    // Extract the number of grid elements from str and cache the current file
    // position.
    bool
    parseNELEMVal(const std::string &str)
    {
        return toInt(str, nElems_) && in_.getPos(posNELEMData_);
    }


    // Extract the number of grid points from str and cache the current file
    // position.
    bool
    parseNPOIN(const std::string &str)
    {
        return toInt(str, nPoints_) && in_.getPos(posNPOINData_);
    }


    // Extract the element counts from a 2D grid file.
    bool
    getCellCounts2()
    {
        PWP_UINT32 cellCount = 0;
        line_.clear();
        // Set file position to beginning of element data
        bool ret = in_.setPos(posNELEMData_);
        if (grdpProgressBeginStep(pRti_, nElems_) && ret) {
            StringArray1 toks;
            PWP_UINT32 elemType;
            while (ret && (cellCount++ < nElems_)) {
                // For each line, expecting "Type Vertex1 ... VertexN Index"
                if (!readLineTokens(toks)) {
                    reportError("Unexpected EOF while reading 2D counts");
                    ret = false;
                    break;
                }

                if (!toInt(toks.at(0), elemType)) {
                    reportError("Could not read 2D count type");
                    ret = false;
                    break;
                }

                switch (elemType) {
                case SU2Tri:
                    ++PWGM_ECNT_Tri(nElemTypes_);
                    break;
                case SU2Quad:
                    ++PWGM_ECNT_Quad(nElemTypes_);
                    break;
                default:
                    reportError("Unexpected 2D count type");
                    ret = false;
                    break;
                }
                if (!grdpProgressIncr(pRti_)) {
                    ret = false;
                }
            }
        }
        return grdpProgressEndStep(pRti_) && ret;
    }


    // Extract the element counts from a 3D grid file.
    bool
    getCellCounts3()
    {
        PWP_UINT32 cellCount = 0;
        line_.clear();
        bool ret = in_.setPos(posNELEMData_);
        if (grdpProgressBeginStep(pRti_, nElems_) && ret) {
            StringArray1 toks;
            PWP_UINT32 elemType;
            while (ret && (cellCount++ < nElems_)) {
                // For each line, expecting "Type Vertex1 ... VertexN Index"
                if (!readLineTokens(toks)) {
                    reportError("Unexpected EOF while reading 3D counts");
                    ret = false;
                    break;
                }

                if (!toInt(toks.at(0), elemType)) {
                    reportError("Could not read 3D count type");
                    ret = false;
                    break;
                }

                switch (elemType) {
                case SU2Tet:
                    ++PWGM_ECNT_Tet(nElemTypes_);
                    break;
                case SU2Pyramid:
                    ++PWGM_ECNT_Pyramid(nElemTypes_);
                    break;
                case SU2Wedge:
                    ++PWGM_ECNT_Wedge(nElemTypes_);
                    break;
                case SU2Hex:
                    ++PWGM_ECNT_Hex(nElemTypes_);
                    break;
                default:
                    reportError("Unexpected 3D count type");
                    ret = false;
                    break;
                }
                if (!grdpProgressIncr(pRti_)) {
                    ret = false;
                }
            }
        }
        return grdpProgressEndStep(pRti_) && ret;
    }


    // Extract grid point data from the file and populate an uns vertex list.
    bool
    readVertices()
    {
        line_.clear();
        // Create the vertex list
        hVL_ = PwModCreateUnsVertexList(pRti_->model);
        // Allocate room for the nPoints_ vertices and set the file's position
        // to the begining of the vertex data.
        bool ret = PWGM_HVERTEXLIST_ISVALID(hVL_) &&
            PwVlstAllocate(hVL_, nPoints_) && in_.setPos(posNPOINData_);
        if (ret) {
            const size_t TokCnt = (gridIs3D_ ? 4 : 3);
            PWGM_VERTDATA vert = { 0.0 };
            StringArray1 toks;
            PWP_UINT32 vertCount = 0;
            PWP_UINT32 ndx = 0;
            while (vertCount < nPoints_) {
                if (!readLineTokens(toks)) {
                    reportError("Unexpected EOF while reading point");
                    ret = false;
                    break;
                }

                if (TokCnt != toks.size()) {
                    reportError("Unexpected number of point tokens");
                    ret = false;
                    break;
                }

                if (gridIs3D_) {
                    // Expecting "x y z index"
                    ret = toDbl(toks[0], vert.x) && toDbl(toks[1], vert.y) &&
                        toDbl(toks[2], vert.z) && toInt(toks[3], ndx);
                }
                else {
                    // Expecting "x y index"
                    ret = toDbl(toks[0], vert.x) && toDbl(toks[1], vert.y) &&
                        toInt(toks[2], ndx);
                }

                if (!ret) {
                    reportError("Could not read point");
                    break;
                }
                else if (!PwVlstSetXYZData(hVL_, vertCount++, vert)) {
                    reportError("Could set vertex list data");
                    ret = false;
                    break;
                }
            }
        }
        else {
            reportError("Could create vertex list");
        }
        return ret;
    }


    // Load all elements in the grid file
    bool
    loadCells()
    {
        line_.clear();
        return gridIs3D_ ? loadCells3() : loadCells2();
    }


    // Load all elements in the 2D grid file
    bool
    loadCells2()
    {
        // Create an empty domain using hVL_
        const PWGM_HDOMAIN hDom = PwVlstCreateUnsDomain(hVL_);
        // Allocate room for the nElemTypes_ domain elements and set the file's
        // position to the begining of the element data.
        bool ret = PWGM_HDOMAIN_ISVALID(hDom) &&
            grdpProgressBeginStep(pRti_, nElems_) &&
            PwUnsDomAllocateElementCounts(hDom, nElemTypes_) &&
            in_.setPos(posNELEMData_);
        if (ret) {
            StringArray1 toks;
            PWGM_ELEMDATA elem;
            PWP_UINT32 ndx = 0;
            PWP_UINT32 elemType;
            while (ret && (ndx < nElems_)) {
                // For each line, expecting "Type Vertex1 ... VertexN Index"
                if (!readLineTokens(toks)) {
                    reportError("Unexpected EOF while reading 2D element");
                    ret = false;
                    break;
                }

                if (!toInt(toks.at(0), elemType)) {
                    reportError("Could not read 2D element type");
                    ret = false;
                    break;
                }

                switch (elemType) {
                case SU2Tri:
                    if (!toksToElem(toks, PWGM_ELEMTYPE_TRI, 3, elem)) {
                        reportError("Invalid tri element connectivity");
                        ret = false;
                    }
                    break;
                case SU2Quad:
                    if (!toksToElem(toks, PWGM_ELEMTYPE_QUAD, 4, elem)) {
                        reportError("Invalid quad element connectivity");
                        ret = false;
                    }
                    break;
                default:
                    reportError("Unexpected 2D element type");
                    elem.type = PWGM_ELEMTYPE_SIZE;
                    elem.vertCnt = 0;
                    ret = false;
                    break;
                }

                if (ret && !PwUnsDomSetElement(hDom, ndx++, &elem)) {
                    reportError("Could not set 2D element data");
                    ret = false;
                    break;
                }

                if (ret && !grdpProgressIncr(pRti_)) {
                    ret = false;
                    break;
                }
            }
        }
        else {
            reportError("Could create domain entity");
        }
        return grdpProgressEndStep(pRti_) && ret;
    }


    // Load all elements in the 3D grid file
    bool
    loadCells3()
    {
        // Create an empty block using hVL_
        PWGM_HBLOCK hBlk = PwVlstCreateUnsBlock(hVL_);
        // Allocate room for the nElemTypes_ block elements and set the file's
        // position to the begining of the element data.
        bool ret = PWGM_HBLOCK_ISVALID(hBlk) &&
            grdpProgressBeginStep(pRti_, nElems_) &&
            PwUnsBlkAllocateElementCounts(hBlk, nElemTypes_) &&
            in_.setPos(posNELEMData_);
        if (ret) {
            StringArray1 toks;
            PWGM_ELEMDATA elem;
            PWP_UINT32 ndx = 0;
            PWP_UINT32 elemType;
            while (ret && (ndx < nElems_)) {
                // For each line, expecting "Type Vertex1 ... VertexN Index"
                if (!readLineTokens(toks)) {
                    reportError("Unexpected EOF while reading 3D element");
                    ret = false;
                    break;
                }

                if (!toInt(toks.at(0), elemType)) {
                    reportError("Could not read 3D element type");
                    ret = false;
                    break;
                }

                switch (elemType) {
                case SU2Tet:
                    if (!toksToElem(toks, PWGM_ELEMTYPE_TET, 4, elem)) {
                        reportError("Invalid tet element connectivity");
                        ret = false;
                    }
                    break;
                case SU2Pyramid:
                    if (!toksToElem(toks, PWGM_ELEMTYPE_PYRAMID, 5, elem)) {
                        reportError("Invalid pyramid element connectivity");
                        ret = false;
                    }
                    break;
                case SU2Wedge:
                    if (!toksToElem(toks, PWGM_ELEMTYPE_WEDGE, 6, elem)) {
                        reportError("Invalid prism element connectivity");
                        ret = false;
                    }
                    break;
                case SU2Hex:
                    if (!toksToElem(toks, PWGM_ELEMTYPE_HEX, 8, elem)) {
                        reportError("Invalid hex element connectivity");
                        ret = false;
                    }
                    break;
                default:
                    reportError("Unexpected 3D element type");
                    elem.type = PWGM_ELEMTYPE_SIZE;
                    elem.vertCnt = 0;
                    ret = false;
                    break;
                }

                if (ret && !PwUnsBlkSetElement(hBlk, ndx++, &elem)) {
                    reportError("Could not set 3D element data");
                    ret = false;
                    break;
                }

                if (ret && !grdpProgressIncr(pRti_)) {
                    ret = false;
                    break;
                }
            }
        }
        else {
            reportError("Could create block entity");
        }
        return grdpProgressEndStep(pRti_) && ret;
    }

    // hide copy constructor
    SU2GridReader(const SU2GridReader&) {}

    // hide assignment operator
    const SU2GridReader&  operator=(const SU2GridReader&) {
                        return *this; }


private:
    GRDP_RTITEM *       pRti_;          // the rti
    PwpFile             in_;            // input file
    std::string         line_;          // Current file line being processed
    sysFILEPOS          posNELEMData_;  // cached file pos of element data
    sysFILEPOS          posNPOINData_;  // cached file pos of coord data
    bool                gridIs3D_;      // true if grid dimensionality is 3D
    PWP_UINT32          nPoints_;       // total number of uns vertices
    PWP_UINT32          nElems_;        // total number of elements
    PWGM_ELEMCOUNTS     nElemTypes_;    // number of elements by type
    PWGM_HVERTEXLIST    hVL_;           // the grid's uns vertex list
};


/** runtimeReadGrid() - (API function)
*
*  This is the grid import API entry function.
*
*/
PWP_BOOL
runtimeReadGrid(GRDP_RTITEM *pRti)
{
    SU2GridReader g(pRti);
    return g.read();
}


PWP_BOOL 
assignValueEnum(const char name[], const char value[], bool createIfNotExists)
{
    return PwuAssignValueEnum(GRDP_INFO_GROUP, name, value, createIfNotExists);
}


PWP_BOOL
runtimeReadGridCreate(GRDP_RTITEM * /*pRti*/)
{
    PWP_BOOL ret = PWP_TRUE;
    // Publish the element types supported by this importer
    const char *etypes = "Bar|Tri|Quad|Tet|Pyramid|Wedge|Hex";
    ret = ret && assignValueEnum("ValidElements", etypes, true);
    return ret;
}


PWP_VOID
runtimeReadGridDestroy(GRDP_RTITEM * /*pRti*/)
{
}


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
