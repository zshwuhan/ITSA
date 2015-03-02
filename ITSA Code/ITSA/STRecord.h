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

#include "STPair.h"

class CSTRecord
{
public:
    CSTRecord();
    virtual ~CSTRecord();
    bool addPair(CSTConcept* pLoc, CSTConcept* pTime);    
    CString toString() const;
    inline void setSensitiveConcept(CSTConcept* pSenConcept) { m_pSenConcept = pSenConcept; }
    inline CSTConcept* getSensitiveConcept() const { return m_pSenConcept; }
    inline CSTPairs* getPath() { return &m_path; }
    inline int getNumPairs() const { return (int) m_path.GetSize(); }

protected:
    CSTPairs m_path;
    CSTConcept* m_pSenConcept;
};

typedef CTypedPtrArray<CPtrArray, CSTRecord*> CSTRecordArray;
class CSTRecords : public CSTRecordArray
{
public:
    CSTRecords();
    virtual ~CSTRecords();    
    void cleanup();
    bool addRecord(CSTRecord* pRec);
};

