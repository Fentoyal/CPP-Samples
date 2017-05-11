/******************************************************************************
The idea was NOT invented by myself, but inspired by some people online.
The implementation was done by myself nevertheless.
This array builder uses a class to mimic the behavior of a dynamically allocated multidimensional array, so the object is not a plain pointer.
It has sequential memory and supports N dim index to N-1 dim index linear translation: like int foo[3][5]; You can access foo[1][2] by (*foo)[1 * 5 + 2]
The advantage to my other IdealArrayBuilder is:
It is faster: it does not do multiple dereference of higher level pointers (e.g. *****ptr = 5 requires 4 times dereference)
instead, it calculates the position of the element directly.
The disadvantage is:
It is a class mimicking a plain array, but it is not a plain pointer, so it won't be compatible with C or old code.

USAGE SEE MAIN FUNCTION.
******************************************************************************/
#include <iostream>
#include <vector>
#include <cassert>
#include <memory.h>

struct Foo
{
    int i, j;
};

template<class Type, size_t Dimension>
class MultiArray
{
    size_t _size;
    Type * _buffer;
    size_t _sizes[Dimension];

    template<typename ClientType, size_t CurDim>
    struct _Proxy
    {
        typedef _Proxy<ClientType, CurDim - 1> DerefType; //This is they type operator[] returns.
        size_t _base_pos; //This is the position provided from higher dim.
        size_t _cur_size;
        ClientType * _client_ptr;
        const static size_t DIMENSION = CurDim;
        //Just pass the argument level by level to the highest base
        _Proxy(size_t pos, ClientType * outter_ptr, size_t cur_size):_base_pos(pos), _cur_size(cur_size), _client_ptr(outter_ptr) {}
        DerefType operator[](size_t idx) const
        {
            size_t dim_size = _client_ptr->_sizes[Dimension - CurDim];
            //Like VC implementation of STL containers,
            //I do not check boundaries in Release version for better performance
            size_t pos = dim_size * _base_pos + idx;//To get the next position, we do this calculation
            size_t next_size = dim_size * _cur_size;//maintain current total size by this dimension to check access violation in Debug mode.
            assert(0 <= pos && pos < next_size);
            return DerefType(pos, _client_ptr, next_size);
        }
        inline DerefType operator * ()
        {
            return operator[](0);
        }
        operator MultiArray<Type, CurDim> ()//Conversion operator. The 3 of them (see below) are all the same essentially.
        {
            size_t sizes_beg = ClientType::DIMENSION - DIMENSION;
            MultiArray<Type, CurDim> arr_to_conv(_client_ptr->_sizes + sizes_beg);
            size_t pos = arr_to_conv.size() * _base_pos;
            memcpy(arr_to_conv.buffer(), _client_ptr->buffer() + pos ,  arr_to_conv.size() * sizeof (Type));
            return arr_to_conv;//it will invoke the move semantic, no painful copying and the resouce is transferred to outside object.
        }
    };
    template<typename ClientType>
    struct _Proxy<ClientType, 1>
    {
        typedef Type & DerefType;
        size_t _base_pos;
        size_t _cur_size;
        ClientType * _client_ptr;
        _Proxy(size_t pos, ClientType * outter_ptr, size_t cur_size):_base_pos(pos), _cur_size(cur_size), _client_ptr(outter_ptr) {}
        DerefType operator[](size_t idx) const
        {
            size_t dim_size = _client_ptr->_sizes[Dimension - 1];
            size_t pos = dim_size * _base_pos + idx;
            size_t total_size = dim_size * _cur_size;
            assert(0 <= pos && pos < total_size);
            return _client_ptr->_buffer[pos];
        }
        inline DerefType operator * () const
        {
            return operator[](0);
        }
        operator MultiArray<Type, 1> ()
        {
            size_t sizes_beg = _client_ptr->DIMENSION - 1;
            MultiArray<Type, 1> arr_to_conv(_client_ptr->_sizes + sizes_beg);
            size_t pos = arr_to_conv.size() * _base_pos;
            memcpy(arr_to_conv.buffer(), _client_ptr->buffer() + pos ,  arr_to_conv.size() * sizeof (Type));
            return arr_to_conv;
        }

    };
    template<typename ClientType>
    struct _Proxy<const ClientType, 1>
    {
        typedef Type DerefType;
        size_t _base_pos;
        size_t _cur_size;
        const ClientType * _client_ptr;
        _Proxy(size_t pos, const ClientType * outter_ptr, size_t cur_size):_base_pos(pos),  _cur_size(cur_size),_client_ptr(outter_ptr) {}
        DerefType operator[](size_t idx) const
        {
            size_t dim_size = _client_ptr->_sizes[Dimension - 1];
            size_t pos = dim_size * _base_pos + idx;
            size_t total_size = dim_size * _cur_size; //
            assert(0 <= pos && pos < total_size);
            return _client_ptr->_buffer[pos];
        }
        inline DerefType operator * () const
        {
            return operator[](0);
        }
        operator MultiArray<Type, 1> ()
        {
            size_t sizes_beg = _client_ptr->DIMENSION - 1;
            MultiArray<Type, 1> arr_to_conv(_client_ptr->_sizes + sizes_beg);
            size_t pos = arr_to_conv.size() * _base_pos;
            memcpy(arr_to_conv.buffer(), _client_ptr->buffer() + pos ,  arr_to_conv.size() * sizeof (Type));
            return arr_to_conv;
        }

    };



public:
    const static size_t DIMENSION = Dimension;
    template <size_t Dim>
    using Accessor = _Proxy<MultiArray, Dim>;
    template <size_t Dim>
    using Const_Accessor = _Proxy<const MultiArray, Dim>;

    typedef  typename _Proxy<MultiArray, Dimension>::DerefType DerefType;
    typedef typename _Proxy<const MultiArray, Dimension>::DerefType CDerefType;
    inline Type& at(size_t idx)
    {
        assert(0 <= idx && idx < _size);
        return _buffer[idx];
    }
    inline Type at(size_t idx) const
    {
        assert(0 <= idx && idx < _size);
        return _buffer[idx];
    }
    DerefType  operator[](size_t idx)
    {
        return _Proxy<MultiArray, Dimension>(0, this, 1)[idx];
    }

    CDerefType operator[](size_t idx) const
    {
        return _Proxy<const MultiArray, Dimension>(0, this, 1)[idx];
    }
    DerefType operator *()
    {
        return operator[](0);
    }
    CDerefType operator * () const
    {
        return operator[](0);
    }
    template<typename ForwardIter>
    explicit MultiArray(ForwardIter sizes_beg = ForwardIter(), Type * buffer = nullptr): _size(0), _buffer(nullptr)
    {
        memset(_sizes, 0, Dimension * sizeof (size_t));
        reallocate(sizes_beg, buffer);
    }
    //no sizes info means we keep the current size but re-initialize the elements
    //Note instead of making it quite like a container, I also like to make it like a RAII recource wrapper.
    template<typename ForwardIter>
    void reallocate(ForwardIter sizes_iter = ForwardIter(), Type * buffer = nullptr)
    {
        delete [] _buffer;
        morph(sizes_iter);

        if (_size == 0)
        {
            _buffer = nullptr;
            return;
        }
        if (buffer == nullptr)
            _buffer = new Type[_size](); //I do not handle exceptions
        else
            _buffer = buffer;
    }
    //It is not yet a qualified smart pointer, but it behaves similar so it can simplfy my work.
    void reset(Type * buffer = nullptr)
    {
        if (buffer == nullptr)
        {
            clear();
            return ;
        }
        delete [] _buffer;
        _buffer = buffer;
    }

    //change the size of each dimension, but the buffer is not affected.
    template<typename ForwardIter>
    void morph(ForwardIter sizes_iter = ForwardIter())
    {
        if (sizes_iter == ForwardIter())
        {
            //we leave it peacefully if the iterator is invalid.
            return ;
        }
        _size = 1;
        for (size_t i = 0; i < Dimension ; ++i)
        {
            _sizes[i] = *(sizes_iter ++);
            _size *= _sizes[i];
        }

    }

    void clear()
    {
        memset(_sizes, 0, Dimension * sizeof (size_t));
        delete [] _buffer;
        _buffer = nullptr;
        _size = 0;
    }

    MultiArray(const MultiArray &to_copy): _size(0), _buffer(nullptr)
    {
        *this = to_copy;
    }
    MultiArray & operator = (const MultiArray &to_assign)
    {
        if (this == &to_assign)
            return *this;

        _size = to_assign._size;

        memcpy(_sizes, to_assign._sizes, Dimension * sizeof(size_t));

        delete [] _buffer;
        _buffer = new Type[_size](); //I do not handle exceptions
        memcpy(_buffer, to_assign._buffer, _size * sizeof(Type));
        return *this;
    }
    MultiArray(MultiArray && to_move):  _size(0), _buffer(nullptr)
    {
        *this = std::move(to_move);
    }
    MultiArray & operator = (MultiArray && to_move_assign)
    {
        if (this == &to_move_assign)
            return *this;
        clear();
        _size = to_move_assign._size;
        memcpy(_sizes, to_move_assign._sizes, Dimension * sizeof(size_t));
        memset(to_move_assign._sizes, 0, Dimension * sizeof (size_t));
        _buffer = to_move_assign._buffer;
        to_move_assign._buffer = nullptr;
        to_move_assign._size = 0;

        return *this;
    }

    inline bool empty() const
    {
        return _size == 0;
    }
    inline size_t size() const
    {
        return _size;
    }
    inline Type * buffer()
    {
        return _buffer;
    }
    inline const size_t * sizes() const
    {
        return _sizes;
    }
    inline size_t dimension() const
    {
        return DIMENSION;
    }
    ~MultiArray()
    {
        clear();
    }


};




using namespace std;

int main()
{
    size_t sizes[] = {4, 5, 6, 2};//4 is the highest dimesion
    MultiArray<Foo, 4> arr(sizes);
    vector<size_t> sizes_vec = {1, 2, 3 ,4};
    MultiArray<Foo, 4> brr(sizes_vec.begin());// works too
    MultiArray<Foo, 1> arr1d(sizes);
    arr1d[2].i = 3;

    cout<< MultiArray<Foo, 4>::DIMENSION<<endl;
    cout<<arr1d[2].i<<endl;

    (****arr).i = 5;
    arr[2][3][0][0].i = 3;
    arr[2][3][0][1].j = 4;

    MultiArray<Foo, 4> crr( arr); //copy
    //naturally, these works too, but they are copying.
    MultiArray<Foo, 2> copy2d = arr[2][3];
    MultiArray<Foo, 1> copy1d = copy2d[0];
    cout<<copy2d.size()<<endl;
    cout<<copy2d[0][1].j<<endl;
    cout<<copy1d[1].j<<endl<<endl;

    //use accessor to reference the intermediate object so that no need to copy.
    //Note, the const qualifier is modifying Accessor not the array.
    MultiArray<Foo, 4>::Accessor<2> const & acc_ref = arr[2][3];//auto const & acc_ref works too
    acc_ref[3][1].i = 5;
    cout<<acc_ref[3][1].i<<endl<<endl;

    //copy and move semantic:
    auto arr_cp = arr;
    cout<<arr_cp[2][3][0][1].j<<endl;
    auto arr_mv = std::move(arr_cp);
    cout<<arr_cp.size()<<endl;//arr_cp is no longer holding any resource.
    cout<<arr_mv[2][3][0][1].j<<endl;
    cout<<endl;

    //the subsripting ability is demonstrated below.
    const auto &arrref = arr;
    cout<<arr[2][3][0][0].i<<endl;
    cout<<arrref[2][3][0][1].j<<endl;
    cout<<arrref[0][0][0][0].i<<endl; //arrref[0][0][0][0] <=>****arrref
    cout<<(****arrref).i<<endl;
    cout<<endl;
    cout<<arrref[2][3][0][1].j<<endl;
    cout<<(***arrref)[157].j<<endl; //157 = 2*5*6*2 + 3*6*2 + 0*2 + 1
    cout<<(**arrref)[78][1].j<<endl; // 78 = 2*5*6 + 3*6
    cout<<(*arrref)[13][0][1].j<<endl; // 13 = 2*5 + 3
    cout<<(*arr[2][3])[1].j<<endl;//*arr[2][3] <=> arr[2][3][0]
    cout<<(*(*arr)[13])[1].j<<endl;//(*arr)[13] <=> arr[2][3]
    cout<<arrref.at(157).j<<endl;

    arr[2][3][0][3].i = 3; //valid, this is allowed because of the memory is allocated in a continuous memory block.

    //cout<<(*(*arr)[20])[1].i<<endl;//debug assertion failed because for the 2nd dimension, the largest idx allowed is 4*5 = 20

    //arr.at(240).i = 7;//debug assertion failed because of access violation
    //cout<<(***arrref)[250].i<<endl; //debug assertion failed because of access violation
    //cout<<(**arrref)[130][1].i<<endl; //debug assertion failed because for the 3rd dimesion, the largest idx allowed is 4*5*6 = 120
    size_t sizes2[]= {12, 2, 2, 5};
    arr.morph(sizes2);
    cout<<arr[7][1][1][2].j<<endl;
    arr.reallocate(sizes2);
    cout<<arr[7][1][1][2].j<<endl;
    return 0;
}
