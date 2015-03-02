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

#include "STConcept.h"

class CSTAttrib
{
public:
    CSTAttrib(LPCTSTR attribName);
    virtual ~CSTAttrib();

// operations
    bool initHierarchy(LPCTSTR conceptStr);
    bool flattenHierarchy();    
    CSTConcept* findConceptByStr(LPCTSTR str);
    inline CSTConcept* getConceptRoot() { return m_pConceptRoot; };
    inline CSTConcepts* getFlattenConcepts() { return &m_flattenConcepts; };

// attributes
    CString     m_attribName;       // Attribute name.
    int         m_attribIdx;        // Attribute Index 
    bool        m_bSensitive;       // Is it a sensitive attribute?

protected:    
    CSTConcept* m_pConceptRoot;     // Root of concept hierarchy.
    CSTConcepts m_flattenConcepts;  
};

typedef CTypedPtrArray<CPtrArray, CSTAttrib*> CSTAttribArray;
class CSTAttribs : public CSTAttribArray
{
public:
    CSTAttribs();
    virtual ~CSTAttribs();
    void cleanup();
    friend ostream& operator<<(ostream& os, const CSTAttribs& attribs);
};
