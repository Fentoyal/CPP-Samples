/******************************************************
The idea was inspired by other work,
And the implementation was largely done by myself but also referenced some others work too.
***********************************
This is a class that can be two different types at the same time.
*************************/
#include <vector>
#include <algorithm>
#include <iostream>
#include <string>
#include <stdexcept>
#include <iterator>
using namespace std;

class Token
{
    public:
    enum{INT, STR} tag;
    operator string() const {if (tag != STR) throw (domain_error("NotContaining String")); return str;}
    operator int() const {if (tag != INT) throw (domain_error("NotContaining Int")); return i;}
    Token (const string & s):str(s), tag(STR){}
    Token (int i0):i(i0), tag(INT){}
    Token & operator= (int i0)
    {
        if (tag == STR)
            str.~string();
        i = i0;
        tag = INT;
        return *this;
    }
    Token & operator= (const string &s)
    {
        if (tag == STR)
            str = s;
        else{
            tag = STR;
            new (&str) string (s);
        }
        return *this;

    }
    ~Token (){
        if (tag == STR)
            str.~string();
    }
    Token (const Token & to_copy):tag (to_copy.tag)
    {
        if (tag == STR)
        {
            new (&str) string(to_copy.str);
        }else{
            i = to_copy.i;
        }
    }
    Token (Token && to_move):tag (to_move.tag)
    {
        if (tag == STR)
        {
            new (&str) string(std::move(to_move.str));
        }else{
            i = to_move.i;
        }
    }
    Token & operator= (Token to_assign)
    {
        swap(*this, to_assign);
    }
    friend ostream & operator<<(ostream & os, const Token &tok)
    {
        if (tok.tag == INT)
            return os<<tok.i;
        return os<<tok.str;
    }
    friend void swap(Token &lhs, Token &rhs)
    {
        if (lhs.tag == STR && rhs.tag == STR)
        {
            swap(lhs.str, rhs.str);

        }else if (lhs.tag == STR && rhs.tag != STR)
        {
            int temp = rhs.i;
            new (&rhs.str) string (std::move(lhs.str));//placement new guarantee noexcept
            lhs.str.~string();
            lhs.i = temp;
        }else if (lhs.tag != STR && rhs.tag == STR)
        {
            int temp = lhs.i;
            new (&lhs.str) string (std::move(rhs.str));//placement new guarantee noexcept
            rhs.str.~string();
            rhs.i = temp;
        }else{
            swap(lhs.i, rhs.i);

        }
        swap(lhs.tag, rhs.tag);
    }
private:
    union{
        string str;
        int     i;
    };



};
typedef vector<Token>::iterator Iter;

//  template<typename _Container>
//    inline back_insert_iterator<_Container>
//    back_inserter(_Container& __x)
//    { return back_insert_iterator<_Container>(__x); }

int main()
{
    Token a(20);
    Token b("ss");
    swap(a, b);
    b = std::move(a);
    cout<<a<<b<<endl;

    //input string mixing with integers, sort them separately. The relative orders between string and integer cannot change.
    vector<Token> vec = {Token(10), Token("bb"), Token("a"), Token(100)};
    vector<string> str_vec;
    vector<int>     int_vec;
    copy_if(vec.cbegin(), vec.cend(), back_inserter(str_vec), [](const Token & tok){return tok.tag == Token::STR;});
    copy_if(vec.cbegin(), vec.cend(), back_inserter(int_vec), [](const Token & tok){return tok.tag == Token::INT;});

    sort(str_vec.begin(), str_vec.end());
    sort(int_vec.begin(), int_vec.end());
    auto str_iter = str_vec.cbegin();
    auto int_iter = int_vec.cbegin();
    for (auto & t : vec)
    {
        if (t.tag == Token::STR)
            t = *str_iter ++;
        else
            t = *int_iter ++;

    }
    copy(vec.cbegin(), vec.cend(), ostream_iterator<Token>(cout, " "));

}
