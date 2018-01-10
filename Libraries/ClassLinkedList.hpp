/*2015 Thomas Androxman
-------------------------------------------------------------------------------------------
This library handles:
-A link-list object (made from scratch)
The advantage of this object is that its nodes are static in memory and it is safe to link to them via pointers.
As opposed to using the C++ STL vector, which  keeps reallocating memory every so often during push_back()
The linked-list object uses a bookmark facility making sequential retrieval an O(1) operation
-------------------------------------------------------------------------------------------
*/

#pragma once
#include <string>
#include <iostream>
#include <sstream>
#include <vector>

template<class AnyVar, unsigned LinkCount> struct TypeNode
{
    double    key=0;
    AnyVar    Data;

    TypeNode<AnyVar, LinkCount>* Link[LinkCount];
};

template<class AnyVar> class TypeLinkedList //class declaration
{
private:

    //The bookmark is an internal facility that does not affect the data of the list, so logically it doesn't count as a change (therefore declared 'mutable')
    mutable struct {unsigned Index; TypeNode<AnyVar,2>* ThisNode;} Bookmark;

    TypeNode<AnyVar,2>* Beginning; //Will be set to NULL by the constructor
    TypeNode<AnyVar,2>* Ending;    //Will be set to NULL by the constructor
    unsigned              Size;    //Keeps truck of number of elements currently in the list (could give incorrect value if size of list exceeds MAX_Limit of long long)

//Private Methods
    TypeNode<AnyVar,2>* NodeAt        (unsigned AtIndex) const;
    TypeNode<AnyVar,2>* Detach        (unsigned AtIndex); //Careful, the node is not deleted
    AnyVar              ValueAt       (unsigned AtIndex)  {return AtIndex>(Size-1)? NodeAt (AtIndex)->Data:0;}
    void                Initialize    ();
    void                Attach        (TypeNode<AnyVar,2>* ThisNode, unsigned AtIndex=0);
    bool                MoveBookmarkTo(unsigned ThisIndex) const;

public:
//Constructor
    TypeLinkedList () {Initialize();}                                                            //Default constructor
    TypeLinkedList (const TypeLinkedList<AnyVar>& OtherList) {Initialize();Insert(OtherList,0);} //Copy constructor
//Destructor
    ~TypeLinkedList();                //Destructor (take care of memory leaks; delete all nodes from memory)


//Public Methods
    void SetValue (AnyVar Value, unsigned AtIndex)        {if (AtIndex<Size) NodeAt(AtIndex)->Data = Value;}
    void Clear    ();

    AnyVar               Pop          (unsigned AtIndex);  //Detach node at specified index, Return a pointer to the detached node;
    AnyVar               PopBack      ()                  {return Pop(Size);}
    AnyVar               PopFront     ()                  {return Pop(0);}
    TypeNode<AnyVar,2>*  Back         ()                  {return Ending;}
    TypeNode<AnyVar,2>*  Front        ()                  {return Beginning;}
    void                 Transplant   (unsigned AtIndex, unsigned sourceIdx, TypeLinkedList<AnyVar>& sourceList); //transfer a node from one list to another without deletion or data copy
    void                 Insert       (const AnyVar& Value, unsigned AtIndex=0); //(append, append_data, insert, insert_data all in one)
    void                 Insert       (const TypeLinkedList<AnyVar>& OtherList, unsigned AtIndex);
    void                 PushBack     (AnyVar Value)      {Insert(Value,Size);}
    void                 PushFront    (AnyVar Value)      {Insert(Value,0);}
    bool                 DeleteValue  (AnyVar DataValue);
    long long            GetIndexOf   (AnyVar Value);	    //Search the list for the specified value and return the index where it is at, or -1 if not found
    unsigned             ListSize     () const            {return Size;}

    std::string          Print        () const;	//Return a space delimited string of all list data in order.

    void                 operator=    (TypeLinkedList<AnyVar>& OtherList);
    TypeNode<AnyVar,2>*  operator[]   (unsigned idx)      {if (idx<Size) return NodeAt(idx); else if (Size==0) return nullptr; else return NodeAt(Size-1);}
    TypeNode<AnyVar,2>*  operator[]   (unsigned idx) const{if (idx<Size) return NodeAt(idx); else if (Size==0) return nullptr; else return NodeAt(Size-1);}
};
//The rest of this is at
#include "ClassLinkedList.cpp"
