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

class CSTConcept;

typedef CTypedPtrArray<CPtrArray, CSTConcept*> CSTConceptArray;
class CSTConcepts : public CSTConceptArray
{
public:
    CSTConcepts();
    virtual ~CSTConcepts();
    void cleanup();
};

class CSTAttrib;

class CSTConcept
{
public:
    CSTConcept(CSTAttrib* pAttrib);
    virtual ~CSTConcept();
    
    bool addChildConcept(CSTConcept* pConceptNode);
    int getNumChildConcepts() const;
    CSTConcept* getChildConcept(int idx) const;
    CSTConcept* getParentConcept();
    inline CSTAttrib* getAttrib() { return m_pAttrib; };

    bool initHierarchy(LPCTSTR conceptStr, int depth);
    static bool parseConceptValue(LPCTSTR str, CString& conceptVal, CString& restStr);
    static bool parseFirstConcept(CString& firstConcept, CString& restStr);

// attributes
    CString m_conceptValue;          // Actual value in string format.
    int m_timestamp;                 // This value is used for time concept value.

    bool    m_bSensitive;            // Is it sensitive?
    int     m_depth;                 // Depth of this concept.
    int     m_childIdx;              // Child index in concept hierarchy.
    int     m_flattenIdx;            // Flattened index in concept hierarchy.    

protected:
    CSTAttrib*     m_pAttrib;		 // Pointer to this attribute.
    CSTConcept*    m_pParentConcept;
    CSTConcepts    m_childConcepts;
};
