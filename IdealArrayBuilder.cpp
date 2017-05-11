/******************************************************
The idea was initially completely invented by myself during my large processing of volume images. This idea was not even inspired by others' work.
The implementation was completely done by myself too.

This is a real dynamic multi-array that mimicks a traditional stack allocated array.
Features:
1.	It's not simulated by or wrapped in a Class. The generated array's name is just a plain pointer. So it is compatible with C or old code.
2.	The array can have arbitrary dimension - e.g. 10000 dimension (NOT SIZE) is OK as long as you have enough memory.
3.	Unlike an array of pointers, this entire array is laid out sequentially in memory.
    So it supporst N dim index to N-1 dim index linear translation: like int foo[3][5]; You can access foo[1][2] by (*foo)[1 * 5 + 2]
	This is what an array of pointers created by new statement can't do because their space isn't continous.
4.	Dynamic allocation on heap.

In short:
  It behaves like  MyClass foo[2][3][4][5] on stack, but it is on heap.
  It achieved exactly the same "desired" function as this intuitive psuedo code:
  MyClass **** my_ptr  = new MyClass[2][3][4][5];//This  makes no sense in C++;

/***********************************************
USAGE
size_t sizes[] = {2,3,3,3}; //2 is the highest dimension.
MyClass **** my_classes = IdealArrBuilder<MyClass, 4>::inew(sizes) ;
IdealArrBuilder<MyClass, 4>::idelete(my_classes);
**********************************************
MONSTEROUS USAGE To allocate a 10000-dimensional array, the size of each dimension is the N.O. of current dimension. O(10^40000)
size_t sizes[10000];
for (int i = 0; i < 10000; ++i)
sizes[i] = 10000 - i;
IdealArrBuilder<MyClass, 10000>::FinalType my_classes = IdealArrBuilder<MyClass, 10000>::inew(sizes) ;
IdealArrBuilder<MyClass, 10000>::idelete(my_classes);
***********************************************
SEE MAIN FUNCTION FOR HOW TO ACCESS ELEMENTS IN IT.
**********************************************/


#include <iostream>
#include <vector>
using namespace std;
#define SUCCESS 1
#define PTRERR 0
template<class ElemType, size_t N_Dim>
class IdealArrBuilder : public IdealArrBuilder<ElemType*, N_Dim-1> {
public:
	typedef typename IdealArrBuilder<ElemType*, N_Dim-1>::FinalType FinalType;
protected:
	template<class RandomAccessIter>
	static FinalType _bldCurDim(size_t sizec, RandomAccessIter size_beg, ElemType lower_dim) {
		RandomAccessIter cur_iter = size_beg;
		size_t dim_size = 1;
		for (size_t i = 0; i < sizec; ++i)
			dim_size *= *(cur_iter++);
		ElemType* higher_dim = new ElemType[dim_size];
		higher_dim[0] = lower_dim;
		for (size_t i = 1; i < dim_size; ++i)
			higher_dim[i] = higher_dim[i - 1] + *(size_beg+sizec);
		return IdealArrBuilder <ElemType*, N_Dim - 1>::_bldCurDim(sizec - 1, size_beg, higher_dim);
	}
	static ElemType _freeCurDim(FinalType ptr){
		ElemType * cur_ptr = IdealArrBuilder <ElemType*, N_Dim - 1>::_freeCurDim(ptr);
		if (cur_ptr == NULL) return NULL;
		ElemType lower_ptr = *cur_ptr;
		delete [] cur_ptr;
		return lower_ptr;
	}

public:
	template<class RandomAccessIter>
	static FinalType ialloc(RandomAccessIter size_beg){
		RandomAccessIter cur_iter = size_beg;
		size_t dim_size = 1;
		for (size_t i = 0; i < N_Dim; ++i)
			dim_size *= *(cur_iter++);
		ElemType* base = reinterpret_cast <ElemType*> (operator new (dim_size*sizeof(ElemType)));
		return IdealArrBuilder <ElemType*, N_Dim - 1>::_bldCurDim(N_Dim - 1, size_beg, base);
	}
	template<class RandomAccessIter>
	static FinalType inew(RandomAccessIter size_beg)    {
		RandomAccessIter cur_iter = size_beg;
		size_t dim_size = 1;
		for (size_t i = 0; i < N_Dim; ++i)
			dim_size *= *(cur_iter++);
		ElemType* base = new ElemType[dim_size];
		return IdealArrBuilder <ElemType*, N_Dim - 1>::_bldCurDim(N_Dim - 1, size_beg, base);
	}
	static int ifree(FinalType ptr)     {
		if (ptr == NULL) return PTRERR;
		ElemType * cur_ptr = IdealArrBuilder <ElemType*, N_Dim - 1>::_freeCurDim(ptr);
		if (cur_ptr == NULL) return PTRERR;
		operator delete (cur_ptr);
		return SUCCESS;
	}
	static int idelete(FinalType ptr)    {
		if (ptr == NULL) return PTRERR;
		ElemType * cur_ptr = IdealArrBuilder <ElemType*, N_Dim - 1>::_freeCurDim(ptr);
		if (cur_ptr == NULL) return PTRERR;
		delete [] cur_ptr;
		return SUCCESS;
	}
};
template <class ElemType>
class IdealArrBuilder< ElemType, 0> {

protected:
	typedef ElemType FinalType;
	template<class RandomAccessIter>
	static FinalType _bldCurDim (size_t sizec, RandomAccessIter size_beg, ElemType lower_base) {
		return lower_base;
	}
	static ElemType _freeCurDim(FinalType ptr){
		return ptr;
	}
};


//Example:
struct Foo
{
	int i, j;
};
int main(){
	/*It works too:*/
	//size_t sizes[] = {4, 5, 6, 2};//4 is the highest dimesion
	//Foo ****arr = IdealArrBuilder<Foo, 4>::inew(sizes);
	vector<int> sizevec;
	sizevec.size();
	sizevec.push_back(4);
	sizevec.push_back(5);
	sizevec.push_back(6);
	sizevec.push_back(2);
	//Foo ****arr = IdealArrBuilder<Foo, 4>::inew(sizevec.begin());
	IdealArrBuilder<Foo, 4>::FinalType arr = IdealArrBuilder<Foo, 4>::inew(sizevec.begin()); //Or this way
	//*/
	(****arr).i = 5;
	(***arr)->j = 6;
	arr[2][3][0][0].i = 3;
	arr[2][3][0][1].i = 4;
	const auto &arrref = arr;
	cout<<(****arrref).j<<endl;
	cout<<arrref[2][3][0]->i<<endl;
	cout<<arrref[2][3][0][1].i<<endl;
	cout<<(***arrref)[157].i<<endl; //because it sequentially laid out in memory, you can do index dim reduction: 157 = 2*5*6*2 + 3*6*2 + 0*2 + 1
	cout<<(**arrref)[78][1].i<<endl; // 78 = 2*5*6 + 3*6
	cout<<(*arrref)[13][0][1].i<<endl; // 13 = 2*5 + 3
	cout<<(*arr[2][3])[1].i<<endl;//*arr[2][3] <=> arr[2][3][0]
	cout<<(*(*arr)[13])[1].i<<endl;//(*arr)[13] <=> arr[2][3]
}
