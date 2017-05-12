/************************************************************
The idea was NOT invented by myself, but inspired by some people/article online.
The implementation was largely done by myself with some help from internet.
*******************************************
Due to poor support for nameless recursive lambda in C++ 11, I created this one.
Referenced a lot of ideas and reading materials online.
**********************************************/
#include <functional>
#include <iostream>
using namespace std;
template<class ArgType, class RetType>
function<RetType(ArgType)> YCombinator(function<RetType(function<RetType(ArgType)>, ArgType)> pseudo_recur_func) //Input F
{
  return bind(pseudo_recur_func, bind(&YCombinator<ArgType, RetType>, pseudo_recur_func), placeholders::_1);//output F(Y(F))
}
//when using it, input a F to Y, i.e. we build a Y(F). Y(F) outputs F(Y(F)). So when we use it,  e.g. using the Y(F)(5) to solve a problem.
//Y(F)(5) = F(Y(F))(5). By the definition of F, F(Y(F))(5) = Y(F)(5), so Y(F)(5) = Y(F)(5), so Y(F)(5) is recursive.

int main()
{
	//pass a pseudo-recursive lambda function (in the sense that the 'self' argument is actually not itself)
	//to Y Combinator, which returns an equivalent real-recusive function of lambda.
	//test 0~9
	for(int i = 0; i < 10; ++i)
		cout<<YCombinator<int, int>([](function<int(int)> self , int n){return n <= 1 ? 1 : self(n - 2) + self(n - 1);})(i)<<ends;
	return 0;
}

