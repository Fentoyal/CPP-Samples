/******************************************************
The idea was inpired by other work.
The implementation was completely done by myself independently
***********************************/
#include <iostream>
#include <typeinfo>
#include <cstdio>
using namespace std;
//How we resolve a function's signature.
template<typename T>
struct Func
{
};
template<typename Ret, typename ... Args >
struct Func<Ret (Args...)>
{
    using FuncType =  Ret(Args ...) ;
    using RetType =  Ret;
    //Note, the * is unavoidable
    //That's because int(int, int) is a function type, we can't pass this type.
    //int (*) (int, int) is what we can past.
    //In fact, we can even declare (not define ) a function like this:
    //using ThreeI = int(int, int);
    //ThreeI f; //f is a function, not a pointer to function.
    //So if we miss the *, it would turn into a member function declaration.
    FuncType *func;
    Func(FuncType f): func(f)
    {
    }
    Ret operator()(Args ... args)
    {
        return func(args...);
    }
};
//Note, in the template parameter list, the type is T but in the function's, the type is T *.
//Same reason as we mentioned above.
template <typename T, typename ... Types >
//To define paramter pack type, use ... , to expand it, use ... too, but following the type
//just like to define a pointer, we use * and to dereference a pointer, we also use *.
//To define a paramter pack, use Types ... args.
auto call(T* func, Types ... args) -> typename Func<T>::RetType
{

    // args is a parameter pack. use ... to make it expanded.
    // so it basically turns into this form: arg0, arg1, arg2 (suppose there are only 3), just like a macro expansion.
    printf("%d %d %d\n", args... );
    // Intrestingly, we can also use "map" strategy on args:
    // op(args...) <=> op(arg0), op(arg1), op(arg2)
    printf("%s%s%s\n", typeid(args).name()...);
    // Note, sizeof... is a special operator for the number of arguments in a parameter pack:
    printf("%d \n", sizeof...(args) );
    printf("%d %d %d\n", sizeof(args)... );//this is a normal expansion
    Func<T> ff(func);
    return ff(args...);
}
//similarly, we can resolve an array type:
template<typename T>
struct Arr
{
};
template <typename T, size_t N>
struct Arr<T [N]>
{
    enum {S = N};
    T (&arr) [N];
    Arr(T (&ar)[N]):arr(ar)
    {
        cout<<S<<endl;
        for (auto v : arr)
        {
            cout<<v<<' ';
        }
        cout<<endl;
    }
};
int foo(int i, int j, int k)
{
   return i*j * k;
}


int main()
{
    int arr[] = {1, 2 ,3 ,4 , 5};
    Arr<decltype(arr)> my_arr(arr);
    Func<decltype(foo)> func(foo);
    cout<<func(10, 3 , 2)<<endl;;
    cout<<call(foo, 10, 3, 2)<<endl;
}
//    int arr[] = {1, 2, 3};
//    K(arr);
//     //std::string ss[] = {"a", "bb", "ccc"};
//     std::initializer_list<std::string> ss = {"a", "bb", "ccc"};
//     //std::move_iterator<std::string *> rpointer_s = std::make_move_iterator(const_cast<std::string *>(begin(ss)));
//     //std::move_iterator<std::string *> rpointer_e = std::make_move_iterator(const_cast<std::string *>(end(ss)));
//     //std::vector<std::string> vs(rpointer_s, rpointer_e);
//     std::vector<std::string> vs(std::move(ss));
//     for(auto str : vs)
//             std::cout << "\"" << str << "\" ";
//     std::cout << '\n';
//     for(auto str : ss)
//             std::cout << "\"" << str << "\" ";


