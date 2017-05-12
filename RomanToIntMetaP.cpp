/******************************************************
The idea was initially completely invented by myself. This idea was not even inspired by others' work.
The implementation was completely done by myself too.
*****************************************************
This program can translate a Romain number to an Arabic number in COMPILE time.
*******************************************/

#include <iostream>

class R2I {
	const static int DIG;
public:
	constexpr static char IN[] = "MCDXLVI"; ///This is the INPUT number;
	const static int OUT;
};

template <char C> struct ID {};
template <> struct ID <'I'> { static const int I = 0; static const int V = 1; };
template <> struct ID <'V'> { static const int I = 1; static const int V = 5; };
template <> struct ID <'X'> { static const int I = 2; static const int V = 10; };
template <> struct ID <'L'> { static const int I = 3; static const int V = 50; };
template <> struct ID <'C'> { static const int I = 4; static const int V = 100; };
template <> struct ID <'D'> { static const int I = 5; static const int V = 500; };
template <> struct ID <'M'> { static const int I = 6; static const int V = 1000; };
template <bool A, bool B, int I0, int I1>
struct DiffHelper { static const int D = -9; }; //-9 means it hits the end of R2I::IN. It's a valid state.
template<int I0, int I1>
struct DiffHelper <true, true, I0, I1> {
	static const int VAL0 = ID<R2I::IN[I0]>::I;
	static const int VAL1 = ID<R2I::IN[I1]>::I;
	//The only possible Diff between Pos0 and Pos1 can be -2(IX), -1(IV), 0(II), 1(VI), 2+ (MI), -3 - is not possible
	//To reduce the states, I combined 1 and 2+, becasue 1 is only used to transfer to state 6, so I did a calc at state 5 for it.
	static const int D = (VAL0 - VAL1 <= -3) ? -3 : ((VAL0 - VAL1 >= 1) ? 1 : (VAL0 - VAL1));
};
//Diff Caculate the Difference of the indices.
template <int I0, int I1> struct Diff { static const int D = DiffHelper<(I0 < sizeof(R2I::IN) - 1), (I1 < sizeof(R2I::IN) - 1), I0, I1>::D; };
///////////// General ones
template <int N, int D, int P> struct State { const static int DIG = -4000; };//Not accepted;  The largest in Roman is 3999.
template <int P, int N>    struct State<N, -9, P> { const static int DIG = ID<R2I::IN[P]>::V; }; //End of R2I::IN, return what it is.
																								 //////////  State 9: IX XC CM
template <int P>    struct State<9, 1, P> { const static int DIG = ID<R2I::IN[P]>::V * 4 / 5 + State<(ID<R2I::IN[P]>::I - ID<R2I::IN[P + 1]>::I) % 2, Diff<P + 1, P + 2>::D, P + 1>::DIG; };
template <int P>    struct State<9, -9, P> { const static int DIG = ID<R2I::IN[P]>::V * 4 / 5; };
//////////// State 6: VI LX DC This state can be merged with II
template <int P>    struct State<6, 1, P> { const static int DIG = ID<R2I::IN[P]>::V + State<(ID<R2I::IN[P]>::I - ID<R2I::IN[P + 1]>::I) % 2, Diff<P + 1, P + 2>::D, P + 1>::DIG; };
template <int P>    struct State<6, 0, P> { const static int DIG = ID<R2I::IN[P]>::V + State<2, Diff<P + 1, P + 2>::D, P + 1>::DIG; };
//////////// State 4: IV XL CD, Note it is different from 9 as it ends with V. So the odd/even is opposite for Diff
template <int P>    struct State<4, 1, P> { const static int DIG = ID<R2I::IN[P]>::V * 3 / 5 + State<(ID<R2I::IN[P]>::I - ID<R2I::IN[P + 1]>::I + 1) % 2, Diff<P + 1, P + 2>::D, P + 1>::DIG; };
template <int P>    struct State<4, -9, P> { const static int DIG = ID<R2I::IN[P]>::V * 3 / 5; };
//////// State 3: III XXX CCC MMM
template <int P>    struct State<3, 1, P> { const static int DIG = ID<R2I::IN[P]>::V + State<(ID<R2I::IN[P]>::I - ID<R2I::IN[P + 1]>::I) % 2, Diff<P + 1, P + 2>::D, P + 1>::DIG; };
/////// State 2: II XX CC MM
template <int P>    struct State<2, 1, P> { const static int DIG = ID<R2I::IN[P]>::V + State<(ID<R2I::IN[P]>::I - ID<R2I::IN[P + 1]>::I) % 2, Diff<P + 1, P + 2>::D, P + 1>::DIG; };
template <int P>    struct State<2, 0, P> { const static int DIG = ID<R2I::IN[P]>::V + State<3, Diff<P + 1, P + 2>::D, P + 1>::DIG; };
////////////// State 5: V L D
template <int P>    struct State<1, 1, P> {
	const static int DIG = ID<R2I::IN[P]>::V +
		State<(ID<R2I::IN[P]>::I - ID<R2I::IN[P + 1]>::I == 1) * 6 + (ID<R2I::IN[P]>::I - ID<R2I::IN[P + 1]>::I + 1) % 2, Diff<P + 1, P + 2>::D, P + 1>::DIG;
};
///////////////State 1: I X C M ; Look into  Diff<P+1, P+2> to calculate what Difference the next stage is facing.
template <int P>    struct State<0, 1, P> { const static int DIG = ID<R2I::IN[P]>::V + State<(ID<R2I::IN[P]>::I - ID<R2I::IN[P + 1]>::I) % 2, Diff<P + 1, P + 2>::D, P + 1>::DIG; };
template <int P>    struct State<0, 0, P> { const static int DIG = ID<R2I::IN[P]>::V + State<2, Diff<P + 1, P + 2>::D, P + 1>::DIG; };
template <int P>    struct State<0, -1, P> { const static int DIG = ID<R2I::IN[P]>::V + State<4, Diff<P, P + 2>::D, P + 1>::DIG; }; //Use Diff<P, P+2> on purpose for situation XL;
template <int P>    struct State<0, -2, P> { const static int DIG = ID<R2I::IN[P]>::V + State<9, Diff<P, P + 2>::D, P + 1>::DIG; };
///////////////////////
const int R2I::DIG = State<ID<R2I::IN[0]>::I % 2, Diff<0, 1>::D, 0>::DIG;
const int R2I::OUT = DIG > 0 ? DIG : 0;
using namespace std;
int main()
{
	char carr[R2I::OUT];
	cout << sizeof(carr) << endl; ///Compile time constant.
	cout << R2I::OUT << endl; ///The OUTPUT of the INPUT R2I::IN
}


