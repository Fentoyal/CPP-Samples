/****************************************
The idea was from Boost, after 3 hours digging of the code.
But the implementation was done by myself alone.

This is a lightweight bind function that can only bind 3 arguments.
I created this so I don't need to depend on Boost for my project.
**********************************/

#include <tuple>
#include <type_traits>
#include <iostream>
using namespace std;

template<int i> //use int for identifying which placeholder I am using.
struct PH
{

 };
//Dectect if it is a placeholder and if the answer is yes, tell me which (value) I am using.
template<typename _Tp>
struct is_placeholder : public integral_constant<int, 0>
{ };
template<int i>
struct is_placeholder<PH<i>>  : public integral_constant<int, i>
{ };

 //ArgMap is what I referred as "mapping rule" in my previous post.
//It takes a value of type T, and returns this value if T is not a placeholder;
// otherwise it returns the argument at the corresponding position in the argument list.
template<int idx, class T, class ... Args>
struct ArgMap{
      static auto arg_at(T val, tuple<Args...>  arg_list) -> decltype(get<idx - 1>(arg_list))
      {
          return get<idx-1>(arg_list); //idx indicating which argument I should retrieve. e.g. idx = 2, means it should return the 2nd argument of arg_list
      }

 };
template<class T, class ... Args>
struct ArgMap<0, T, Args...>{ //0 indicating not a placeholder
      static T arg_at(T val, tuple<Args...>  arg_list)
      {
          return val;
      }

 };
template<class R,class F, class T1,class T2,class T3>
struct my_bind_3{
      T1 _t1;
      T2 _t2;
      T3 _t3;
      F   _pf;
      my_bind_3(F pf,T1 t1, T2 t2, T3 t3)
          : _pf(pf),
            _t1(t1),
            _t2(t2),
            _t3(t3)
      {}
      //the code above of this class are all yours, I only chaneged the operator();
      template<class ...ArgList>
      R operator()(ArgList ... args)
      {
         tuple<ArgList...> arg_list (args...); //As I mentioned, we need to store the argument list.
         // this is what I meant in my previous post:
         //return  F(arg_list [bind_list[0]]，arg_list [bind_list[1]]， arg_list [bind_list[2]], …… );
         //note that,  _t1, _t2, _t3 are, as a matter of fact now, bind_list[0], bind_list[1], bind_list[2];

          return (*_pf)( ArgMap<is_placeholder<T1>::value, T1, ArgList... >::arg_at(_t1, arg_list),
                     ArgMap<is_placeholder<T2>::value, T2, ArgList...  >::arg_at(_t2, arg_list),
                     ArgMap<is_placeholder<T3>::value, T3, ArgList...  >::arg_at(_t3, arg_list));
      }
};



 template<class R, class F, class T1,class T2,class T3>
my_bind_3<R,F,T1,T2,T3> bind_3(F fn, T1 arg1, T2 arg2, T3 arg3 ) //这里这么写
{

      return my_bind_3<R,F,T1,T2,T3>(fn, arg1, arg2, arg3); //这里这么写
}

 void show(int i, int j, int k)
{
     cout<<i<<j<<k<<endl;

 }
PH<1> _1;
PH<2> _2;
 int main()
{
     bind_3<void>(show, 5, _1, _2)(6, 7);
     bind_3<void>(show, 5, _2, _1)(6, 7);
     bind_3<void>(show, 2, 3, 4)();
 }
//output:
//567
//576
//534
