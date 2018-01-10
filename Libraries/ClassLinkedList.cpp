//CLASS Double Linked List************************************************************************************
//************************************************************************************************************

//Constructors


//Destructor
template<class AnyVar>
TypeLinkedList<AnyVar>::~TypeLinkedList () {Clear();}

//PRIVATE Methods----------------------------------------------------------------------------------
template<class AnyVar>
void TypeLinkedList<AnyVar>::Initialize()
{
    Size=0;
	Beginning=nullptr;
	Ending=nullptr;
	Bookmark.Index=0; Bookmark.ThisNode=nullptr;
}


template<class AnyVar>
void TypeLinkedList<AnyVar>::Attach (TypeNode<AnyVar,2>* ThisNode, unsigned AtIndex) //ThisNode better have been created with 'new'
{
    if (Size==0) AtIndex=0; else if (Size<AtIndex) AtIndex=Size;

	if (Size>0)
	{
		if (AtIndex==Size) //Implicitly request for adding AFTER the end of the list
		{
			Ending->Link[1]=ThisNode;		        //Link the formerly last node to the new one
			ThisNode->Link[0]=Ending;	            //Link the new node to the formerly last one
			Ending=ThisNode;    		            //Shift the ending pointer to the new node
			ThisNode->Link[1]=nullptr;
		}
		else if (AtIndex==0) //Request for making the new node the beginning of the list
		{
			ThisNode->Link[1]=Beginning;       	    //Link the new node to the former beginning
			Beginning=ThisNode;			            //Set the beginning pointer to the new node
			Beginning->Link[1]->Link[0]=ThisNode;   //Link baqckwards the former beginning node to the new node
			ThisNode->Link[0]=nullptr;
		}
		else //There are more than one nodes and client requests insertion somewhere in between
		{
			ThisNode->Link[1]=NodeAt(AtIndex);            //Link NewNode to the existing node at this index
			ThisNode->Link[0]=ThisNode->Link[1]->Link[0]; //Link the NewNode to the preceding node from this index
			ThisNode->Link[1]->Link[0]=ThisNode;          //Link backwords the existing node at index to the NewNode
			ThisNode->Link[0]->Link[1]=ThisNode;          //Link forward the preceding node to NewNode; thus completnig the insertion
		}

	}
	else //List is empty
	{
		Beginning=ThisNode;
		Ending=ThisNode;
		Bookmark.Index=0;
		ThisNode->Link[1]=nullptr; //Next
		ThisNode->Link[0]=nullptr; //Previous
	}

    Bookmark.Index=AtIndex; Bookmark.ThisNode = ThisNode; Size++;
}

//Carefull, memory is not deleted with this function
template<class AnyVar>
TypeNode<AnyVar,2>* TypeLinkedList<AnyVar>::Detach (unsigned AtIndex)
{
    if (Size==0) return nullptr;
	if (AtIndex > (Size-1)) AtIndex=Size-1;

	TypeNode<AnyVar,2>* Victim=NodeAt(AtIndex);

	//LinkThe previous and next nodes to each other
	if (Victim->Link[0] != nullptr && Victim->Link[1] != nullptr) // This node is in between two others
	{
		Victim->Link[1]->Link[0] = Victim->Link[0];
		Victim->Link[0]->Link[1] = Victim->Link[1];
		Bookmark.Index=AtIndex; Bookmark.ThisNode=Victim->Link[1];
	}
	else if (Victim->Link[0] != nullptr && Victim->Link[1] == nullptr) // It is the end node
	{
		Victim->Link[0]->Link[1] = nullptr;
		Ending=Victim->Link[0];
		Bookmark.Index=AtIndex-1; Bookmark.ThisNode=Ending;
	}
	else if (Victim->Link[0] == nullptr && Victim->Link[1] != nullptr) // It is the beginning node
	{
		Victim->Link[1]->Link[0] = nullptr;
		Beginning=Victim->Link[1];
		Bookmark.Index=0; Bookmark.ThisNode=Beginning;
	}
	else // It is the last remaining node
	{
		Beginning=Ending=nullptr;
		Bookmark.Index=0; Bookmark.ThisNode=nullptr;
	}
	Size--;
	return Victim;
}

// Return location of node at the corresponding index
template<class AnyVar>
TypeNode<AnyVar,2>* TypeLinkedList<AnyVar>::NodeAt (unsigned ThisIndex) const
{
    if (ThisIndex>=Size) return nullptr;
    MoveBookmarkTo (ThisIndex);
    return Bookmark.ThisNode;
}

template<class AnyVar>
bool TypeLinkedList<AnyVar>::MoveBookmarkTo (unsigned ThisIndex) const
{
    if (ThisIndex>=Size) return false;             //check validity of request
    if (ThisIndex==Bookmark.Index) return true;    //Trivial easy case

    TypeNode<AnyVar,2>* Runner;
    unsigned Counter;
    unsigned fromMark=ThisIndex-Bookmark.Index;
    unsigned fromEnd=Size-1-ThisIndex;
    short dir; if (fromMark<0) {dir=-1; fromMark=-fromMark;} else {dir=1;}

    if (ThisIndex<=fromMark) {Runner=Beginning; Counter=0; dir=1;}      //Start at the beginning
    else if (fromEnd<fromMark) {Runner=Ending; Counter=Size-1; dir=-1;} //Start at the end. Go backwards
    else {Runner=Bookmark.ThisNode; Counter=Bookmark.Index;}            //Start at the bookmark. Either backwards or forward

	while (Counter*dir < ThisIndex*dir)
	{
		if (dir>0) {Runner=Runner->Link[1];} else {Runner=Runner->Link[0];}
		Counter=Counter+dir;
		//std::cout<<".";
	}
    Bookmark.Index=ThisIndex; Bookmark.ThisNode=Runner;
    return true;
}

//PUBLIC methods---------------------------------------------------------------------------------------
template<class AnyVar>
void TypeLinkedList<AnyVar>::operator= (TypeLinkedList<AnyVar>& OtherList)
{
    //Delete everything
    Clear();

    //Copy Everything
    Insert(OtherList,0);
}

//Method to Report the contents of the list
template<class AnyVar>
std::string TypeLinkedList<AnyVar>::Print () const
{
	TypeNode<AnyVar,2>* Runner=Beginning; //A pointer parsing the entire list of nodes sequencially
	std::stringstream ss;	    //C++ can't even convert int to string without jumping hoops (I thought std::to_string was supported by C++11, but couldn't compile)

	while (Runner != nullptr)
	{
		ss<<Runner->Data;
		Runner=Runner->Link[1];
		if (Runner != nullptr ) ss<<", "; //Add a space only if not at the end
	}
	return ss.str();
}

//Append, append_data, insert, insert_data all in one
template<class AnyVar>
void TypeLinkedList<AnyVar>::Insert (const AnyVar& Value, unsigned AtIndex)
{
	if (AtIndex>Size) AtIndex=Size;

	TypeNode<AnyVar,2>* NewNode = new TypeNode<AnyVar,2>;
	NewNode->key=0; NewNode->Data=Value; NewNode->Link[0]=nullptr; NewNode->Link[1]=nullptr;
    Attach (NewNode, AtIndex);
}

template<class AnyVar>
void TypeLinkedList<AnyVar>::Insert (const TypeLinkedList<AnyVar>& OtherList, unsigned AtIndex)
{
    const unsigned Len=OtherList.ListSize();
    if (Len==0) return;


    for (unsigned i=0;i<Len;i++) {Insert(OtherList[i]->Data,AtIndex++);}
}

template<class AnyVar>
void TypeLinkedList<AnyVar>::Transplant(unsigned AtIndex,unsigned sourceIdx, TypeLinkedList<AnyVar>& sourceList)
{
    TypeNode<AnyVar,2>* subject=sourceList.Detach(sourceIdx);
    if (subject==nullptr) return;
    else Attach(subject,AtIndex);
}

//Delete a node at the corresponding index
template<class AnyVar>
AnyVar TypeLinkedList<AnyVar>::Pop (unsigned AtIndex)
{
    AnyVar result;
    TypeNode<AnyVar,2>* Victim=Detach(AtIndex);
    if (Victim==nullptr) return result;
    else {result=Victim->Data; delete Victim; return result;}
}

template<class AnyVar>
void TypeLinkedList<AnyVar>::Clear ()
{
    if (Size<=0) return;
     TypeNode<AnyVar,2>* Next = Beginning->Link[1];
     delete Beginning; Size--; Beginning=Next; Clear();
}

template<class AnyVar>
bool TypeLinkedList<AnyVar>::DeleteValue (AnyVar DataValue)
{
    if (Size==0) return false;
    long long FoundIndex=GetIndexOf(DataValue);
    if (FoundIndex==-1) return false;
    Pop(FoundIndex);
    return true;
}

//Search the list for a specific vlaue and if found return the index of the node where it is at
template<class AnyVar>
long long TypeLinkedList<AnyVar>::GetIndexOf (AnyVar Value)
{
	TypeNode<AnyVar,2>* Runner=Beginning;
	unsigned Counter=0;

	while (Runner != nullptr)
	{
		if (Runner->Data == Value) return Counter;
		Runner=Runner->Link[1];
		Counter++;
	}

	return -1; //If the value is not found in the etire list return -1;
}

//CLASS ************************************************************************************
//************************************************************************************************************
