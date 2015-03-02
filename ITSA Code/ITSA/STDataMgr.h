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

#pragma once

#include "STAttrib.h"
#include "STRecord.h"

class CSTDataMgr
{
public:
	CSTDataMgr(LPCTSTR rawPathFile, 
               LPCTSTR anonyPathFile, 
               LPCTSTR configFile,
               LPCTSTR moleFile);
    virtual ~CSTDataMgr();

    bool initialize();
    bool readConfigFile();
    bool readRawData();
    bool writeAnonymousData();
    bool writeMinMoles(const CSTPairsCollection& minMoles, const int windowNum) const;
    inline CSTAttrib* getLocAttrib() const { return m_attributes.GetAt(m_locIdx); }
    inline CSTAttrib* getTimeAttrib() const { return m_attributes.GetAt(m_timeIdx); }
    inline CSTAttrib* getSenAttrib() const { return m_attributes.GetAt(m_senIdx); }
    inline CSTAttribs* getAttribs() { return &m_attributes; };
    inline CSTRecords* getRecords() { return &m_allRecords; };
    inline int getTotalNumInstances() const { return m_nTotalInstances; };
    
protected:
// attributes
    CString m_rawPathFile;
    CString m_anonyPathFile;
    CString m_configFile;
    CString m_moleFile;

    CSTAttribs m_attributes;  // all attributes
    CSTRecords m_allRecords;  // data manager is owner of all records. Responsible to deallocate it.

    int m_nTotalInstances;    // Total number of pairs in the database.
    int m_locIdx;             // index number of the location attribute. 
	int m_timeIdx;            // index number of the time attribute. 
	int m_senIdx;             // index number of the sensitive attribute.    
};
