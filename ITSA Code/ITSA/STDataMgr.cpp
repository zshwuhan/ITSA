////////////////////////////////////////////////////////////////////////////////
//Disclaimer:
//
//The source code is shared to the research community as copyrighted freeware 
//for research purpose only. The authors own the copyright of the source code. 
//The freeware is provided as-is with no warranty or support. We do not take 
//any responsibility for any damage, loss of income, or any problems you might 
//experience from using our software. If you have questions, you are encouraged 
//to consult the paper and the source code. If you find our software useful, 
//please cite our paper.
////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "STDataMgr.h"

CSTDataMgr::CSTDataMgr(LPCTSTR rawPathFile, 
                       LPCTSTR anonyPathFile, 
                       LPCTSTR configFile,
                       LPCTSTR moleFile)
    : m_rawPathFile(rawPathFile),
      m_anonyPathFile(anonyPathFile),
      m_configFile(configFile),
      m_moleFile(moleFile),
      m_locIdx(-1),
      m_timeIdx(-1),
      m_senIdx(-1),
      m_nTotalInstances(0)
{
}

CSTDataMgr::~CSTDataMgr()
{
    m_attributes.cleanup();
    m_allRecords.cleanup();
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
bool CSTDataMgr::initialize()
{
    CBFFileHelper::removeFile(m_anonyPathFile);
    return true;
}

//---------------------------------------------------------------------------
// Assume the config files looks like:
// location:	| Location name may have space in between, e.g., {British Columbia}
// {abcdefgh {{ab {a} {b}} {c}} {defgh {def {d} {e} {f}} {gh {g} {h}}}}
// time:		| Assume time is always an integer.
// {1-8 {1-4 {1-2 {1} {2}} {3-4 {3} {4}}} {5-8 {5-6 {5} {6}} {7-8 {7} {8}}}}
// sensitive:	| List all possible sensitive values, but confidence bound requirement is applied to the ones with "*"
// {{HIV*} {Flu} {Diabetes*} {Heart Disease}}
//---------------------------------------------------------------------------
bool CSTDataMgr::readConfigFile()
{
    std::cout << _T("Reading configuration from file: ") << m_configFile << std::endl;
    m_locIdx = -1;
    m_timeIdx = -1;
    m_senIdx = -1;
    m_attributes.cleanup();
    try {
        CStdioFile configFile;
        if (!configFile.Open(m_configFile, CFile::modeRead)) {
            cerr << _T("CSTDataMgr: Failed to open file ") << m_configFile << endl;
            return false;
        }

        // Parse each line.        
        CString lineStr, attribName, attribType, attribValuesStr;
        int commentCharPos = -1, semiColonPos = -1;
        while (configFile.ReadString(lineStr)) {
            CBFStrHelper::trim(lineStr);
            if (lineStr.IsEmpty())
                continue;

            // Remove comments.
            commentCharPos = lineStr.Find(RF_CONHCHY_COMMENT);
            if (commentCharPos != -1) {
                lineStr = lineStr.Left(commentCharPos);
                CBFStrHelper::trim(lineStr);
                if (lineStr.IsEmpty())
                    continue;
            }

            // Find semicolon.
            semiColonPos = lineStr.Find(TCHAR(':'));
            if (semiColonPos == -1) {
                cerr << _T("CSTDataMgr: Unknown line: ") << lineStr << endl;
                ASSERT(false);
                return false;
            }

            // Extract attribute name.
            attribName = lineStr.Left(semiColonPos);
            CBFStrHelper::trim(attribName);
            if (attribName.IsEmpty()) {
                cerr << _T("CSTDataMgr: Invalid attribute: ") << lineStr << endl;
                ASSERT(false);
                return false;
            }
        
            // Read the next line which contains the hierarchy.
            if (!configFile.ReadString(attribValuesStr)) {
                cerr << _T("CSTDataMgr: Invalid attribute: ") << attribName << endl;
                ASSERT(false);
                return false;
            }
            CBFStrHelper::trim(attribValuesStr);
            if (attribValuesStr.IsEmpty()) {
                cerr << _T("CSTDataMgr: Invalid attribute: ") << attribName << endl;
                ASSERT(false);
                return false;
            }

            // Remove comments.
            commentCharPos = attribValuesStr.Find(RF_CONHCHY_COMMENT);
            if (commentCharPos != -1) {
                attribValuesStr = attribValuesStr.Left(commentCharPos);
                CBFStrHelper::trim(attribValuesStr);
                if (attribValuesStr.IsEmpty()) {
                    cerr << _T("CSTDataMgr: Invalid attribute: ") << attribName << endl;
                    ASSERT(false);
                    return false;
                }
            }

            // Create an attribute.
            CSTAttrib* pNewAttribute =  new CSTAttrib(attribName);
            if (!pNewAttribute) {
                ASSERT(false);
                return false;
            }

            // Identify which attribute is the sensitive attribute.
            if (attribName.CompareNoCase(RF_SENSITIVE_ATTRIB) == 0)
                pNewAttribute->m_bSensitive = true;

            // Build hierarchy
            if (!pNewAttribute->initHierarchy(attribValuesStr)) {
                cerr << _T("CSTDataMgr: Failed to build hierarchy for ") << attribName << endl;
                return false;
            }

            // Add the attribute to the array of attributes.
            pNewAttribute->m_attribIdx = (int) m_attributes.Add(pNewAttribute);
            
            if (attribName.CompareNoCase(RF_LOCATION_ATTRIB) == 0)
                // Identify which attribute is the location attribute.
                m_locIdx = pNewAttribute->m_attribIdx;
            else if (attribName.CompareNoCase(RF_TIME_ATTRIB) == 0)
                // Identify which attribute is the time attribute.
                m_timeIdx = pNewAttribute->m_attribIdx;
            else if (attribName.CompareNoCase(RF_SENSITIVE_ATTRIB) == 0)
                // Identify which attribute is the sensitive attribute.
                m_senIdx = pNewAttribute->m_attribIdx;
        }

        if (m_locIdx == -1 || m_timeIdx == -1 || m_senIdx == -1) {
            cerr << _T("Failed to read location, time, or sensitive attribute.") << endl;
            ASSERT(false);
            return false;
        }
        configFile.Close();
    }
    catch (CFileException&) {
        cerr << _T("Failed to read configuration file: ") << m_configFile << endl;
        ASSERT(false);
        return false;
    }
    std::cout << _T("Reading configuration succeeded.") << std::endl;
    return true;
}

//---------------------------------------------------------------------------
// Format: <CPair><CPair>...<CPair>:<Sensitive>:<QIDAttirb><QIDAttirb>...<QIDAttirb>
//---------------------------------------------------------------------------
bool CSTDataMgr::readRawData()
{
    std::cout << _T("Reading raw data from file: ") << m_rawPathFile << std::endl;    
    m_nTotalInstances = 0;
    m_allRecords.cleanup();
    try {
        CStdioFile rawFile;
        if (!rawFile.Open(m_rawPathFile, CFile::modeRead | CFile::shareDenyWrite)) {
            std::cout << _T("CSTDataMgr: Failed to open file ") << m_rawPathFile << std::endl;
            return false;
        }

        // Parse each line.
        int commentCharPos = -1, colonCharPos = -1;
        CString lineStr;
        while (rawFile.ReadString(lineStr)) {
            CBFStrHelper::trim(lineStr);
            if (lineStr.IsEmpty())
                continue;

            // Remove comments.
            commentCharPos = lineStr.Find(RF_PATHDATA_COMMENT);
            if (commentCharPos != -1) {
                lineStr = lineStr.Left(commentCharPos);
                CBFStrHelper::trim(lineStr);
                if (lineStr.IsEmpty())
                    continue;
            }

            // Separate sensitive value.
            CString senStr;
            colonCharPos = lineStr.Find(RF_PATHDATA_SVAL_SEPARATOR);
            if (colonCharPos != -1) {
                senStr = lineStr.Mid(colonCharPos + 1);
                CBFStrHelper::trim(senStr);
                if (senStr.IsEmpty()) {
                    cerr << _T("CSTDataMgr: Error: missing sensitive value") << lineStr << endl;
                    ASSERT(false);
                    return false;
                }

                lineStr = lineStr.Left(colonCharPos);
                CBFStrHelper::trim(lineStr);
                if (lineStr.IsEmpty())
                    continue;

                colonCharPos = senStr.Find(RF_PATHDATA_SVAL_SEPARATOR);
                if (colonCharPos != -1) {
                    senStr = senStr.Left(colonCharPos);
                }
            }
            
            CString pairStr, locStr, timeStr;
            CSTRecord* pNewRecord = new CSTRecord();
            CBFStrParser strParser(lineStr, RF_PATHDATA_VAL_DELIMETER);
            while (strParser.getNext(pairStr)) {
                // Check unknown value
                CBFStrHelper::trim(pairStr);
                if (pairStr.IsEmpty()) {
                    cerr << _T("CSTDataMgr: Empty value string in record: ") << lineStr << std::endl;
                    delete pNewRecord;
                    pNewRecord = NULL;
                    ASSERT(false);
                    return false;
                }
                
                if (!CSTPair::parsePair(pairStr, locStr, timeStr)) {
                    cerr << _T("CSTDataMgr: Empty value string in record: ") << lineStr << std::endl;
                    delete pNewRecord;
                    pNewRecord = NULL;
                    ASSERT(false);
                    return false;
                }

                // Location & Time
                CSTConcept* pLoc = getLocAttrib()->findConceptByStr(locStr);
                CSTConcept* pTime = getTimeAttrib()->findConceptByStr(timeStr);
                if (!pLoc || !pTime) {
                    cerr << _T("CSTDataMgr: Failed to find location or time in configuration file: ") << locStr << _T(" ") << timeStr << std::endl;
                    delete pNewRecord;
                    pNewRecord = NULL;
                    ASSERT(false);
                    return false;
                }

                if (!pNewRecord->addPair(pLoc, pTime)) {
                    delete pNewRecord;
                    pNewRecord = NULL;
                    ASSERT(false);
                    return false;
                }
                ++m_nTotalInstances;
            }

            if (pNewRecord->getNumPairs() == 0) {
                delete pNewRecord;
                pNewRecord = NULL;
                continue;
            }

            // Sensitive 
            CSTConcept* pSen = getSenAttrib()->findConceptByStr(senStr); 															
            if (!pSen) {
                cerr << _T("CSTDataMgr: Failed to find sensitive value in configuration file: ") << senStr << std::endl;
                delete pNewRecord;
                pNewRecord = NULL;
                ASSERT(false);
                return false;
            }
            pNewRecord->setSensitiveConcept(pSen);

            // Finally, add this record to array of records.
            if (!m_allRecords.addRecord(pNewRecord)) {
                ASSERT(false);
                return false;
            }
        }
        rawFile.Close();

        if (m_allRecords.GetSize() == 0) {
            cerr << _T("CSTDataMgr: No records.") << std::endl;
            return false;
        }
        else {
            cout << _T("Number of records = ") << m_allRecords.GetSize() << std::endl;
        }
    }
    catch (CFileException*) {
        std::cout << _T("Failed to read raw data file: ") << m_rawPathFile << std::endl;
        ASSERT(false);
        return false;
    }
    std::cout << _T("Reading raw data succeeded.") << std::endl;
    return true;
}

//---------------------------------------------------------------------------
// Write the anonymized data
//---------------------------------------------------------------------------
bool CSTDataMgr::writeAnonymousData()
{
    std::cout << _T("Writing anonymous data to file: ") << m_anonyPathFile << std::endl;
    /*int nRecords = 0;
    try {
        CStdioFile anonyFile(m_anonyPathFile, CFile::modeCreate | CFile::modeWrite);
        CString tempStr;
        for (int t = 0; t < m_allPaths.GetSize(); ++t) {
            tempStr = m_allRecords.GetAt(t)->toSupString();
            if (!tempStr.IsEmpty()) {
                anonyFile.WriteString(tempStr + _T("\n"));
                ++nRecords;
            }
        }
        anonyFile.Close();
    }
    catch(CFileException*) {
        std::cout << _T("Failed to write anonymous data: ") << m_anonyPathFile << std::endl;
        ASSERT(false);
        return false;
    }
    std::cout << _T("Number of records written = ") << (int) nRecords << std::endl;
    std::cout << _T("Writing anonymous data succeeded.") << std::endl;*/
    return true;
}

//---------------------------------------------------------------------------
// Write the minimal moles to file.
//---------------------------------------------------------------------------
bool CSTDataMgr::writeMinMoles(const CSTPairsCollection& minMoles, const int windowNum) const
{
	std::cout << _T("Window# ") << windowNum << _T(": Writing minimal moles to file: ") << m_moleFile << std::endl;
    try {
        CStdioFile moleFile(m_moleFile, CFile::modeCreate | CFile::modeWrite);
        CString tempStr;
        for (int m = 0; m < minMoles.GetSize(); ++m)
            moleFile.WriteString(minMoles.GetAt(m)->toString() + _T("\n"));
        moleFile.Close();
    }
    catch(CFileException*) {
        std::cout << _T("Window# ") << windowNum << _T(": Failed to write minimal moles to file: ") << m_moleFile << std::endl;
        ASSERT(false);
        return false;
    }
	std::cout << _T("Window# ") << windowNum << _T(": Number of moles written = ") << minMoles.GetSize() << std::endl;
    std::cout << _T("Window# ") << windowNum << _T(": Writing minimal moles succeeded.") << std::endl;
    return true;
}