/******************************************************
The idea was not mine, but the implementation was done by myself.
**********************************/
#include <iostream>
#include <thread>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <algorithm>
#include <memory>

/*due to some complier issue at the moment some details may not be satisfactory,
*e.g. noexcept specification for move constructor,
*constexpr for capacity() method, member function reference qualifier for operator=(), etc.
*/
template<size_t N> class ImplCircularQueue;
template<size_t N> class CircularQueue;
template<size_t N>
void swap(CircularQueue<N> & lhs, CircularQueue<N> & rhs)//noexcept
{
    using std::swap;
	//std::lock garantees no deadlocks could happen
	std::lock(lhs._mutex, rhs._mutex );
    swap(lhs._pimpl, rhs._pimpl);
	lhs._mutex.unlock();
	rhs._mutex.unlock();
	//Probably the storage status has changed, notify them.
	lhs._enq_cv.notify_one();
	lhs._deq_cv.notify_one();
	rhs._enq_cv.notify_one();
	rhs._deq_cv.notify_one();
}
//suppose this is not a header, use "using" for simplicity
using namespace  std;
//CircularQueue is a concurrent wrapper of ImplCircularQueue
template<size_t N>
class CircularQueue
{
protected:
	unique_ptr<ImplCircularQueue<N>> _pimpl;
	mutable mutex _mutex;
	condition_variable _enq_cv;
	condition_variable _deq_cv;
	template<size_t M>
	friend void swap(CircularQueue<M> & lhs, CircularQueue<M> & rhs);
public:
	CircularQueue();
	CircularQueue(const CircularQueue & to_copy );
    CircularQueue(CircularQueue && to_move );
	~CircularQueue(){};
	CircularQueue & operator=(CircularQueue); //using copy-and-swap

	size_t  capacity()	const {return ImplCircularQueue<N>::CAPACITY;}
	//lock gurad uses RAII to lock and unlock
	size_t	size()	const {lock_guard<mutex> lock(_mutex); return _pimpl->size();}
	//Suppose by default a non-blocking enqueue and dequeue were requried; return false if it is empty;
	bool	enqueue(int val);
	bool	dequeue();
	//I also provided the blocking versions of them.
	void	wait_and_enqueue (int val);
	int		wait_and_dequeue ();
	int		front() const {lock_guard<mutex> lock(_mutex); return _pimpl->front();}
	int		back()	const {lock_guard<mutex> lock(_mutex); return _pimpl->back();}
	bool	empty() const {lock_guard<mutex> lock(_mutex); return _pimpl->empty();}
	bool	full()	const {lock_guard<mutex> lock(_mutex); return _pimpl->full();}
};

template<size_t N>
CircularQueue<N>::CircularQueue()
	:_pimpl(new ImplCircularQueue<N>)
{
}
template<size_t N>
CircularQueue<N>::CircularQueue(const CircularQueue &to_copy )
	:_pimpl(new ImplCircularQueue<N>)
{
    if (this == &to_copy)
        return;
	lock_guard<mutex> lock(to_copy._mutex);
    *_pimpl = *to_copy._pimpl;
}
template<size_t N>
CircularQueue<N>::CircularQueue(CircularQueue &&to_move ) //noexcept
{
    if (this == &to_move)
        return;
	lock_guard<mutex> lock(to_move._mutex);
    _pimpl = std::move(to_move._pimpl);
	//After move, to_move can do nothing but be destroyed!

}
template<size_t N>
auto CircularQueue<N>::operator=(CircularQueue to_assign)->CircularQueue&
{
	//copy-and-swap
    swap(*this, to_assign);
    return *this;
}

template<size_t N>
bool CircularQueue<N>::enqueue(int val)
{
	//unique_lock by default locks mutex on creation.
	//Somehow it is still RAII
	//but it can be changed by passing arugments.
	unique_lock<mutex> lock(_mutex);
	if (_pimpl->enqueue(val))
	{
		_deq_cv.notify_one();
		return true;
	}
	return false;
}
template<size_t N>
bool CircularQueue<N>::dequeue()
{
	unique_lock<mutex> lock(_mutex);
	if (_pimpl->dequeue())
	{
		_enq_cv.notify_one();
		return true;
	}
	return false;
}

template<size_t N>
void CircularQueue<N>::wait_and_enqueue(int val)
{
	unique_lock<mutex> lock(_mutex);
	//because not every notification is made exactly when the condition is satisfied
	//e.g. the ones in swap statement. while loop is needed.
	while(_pimpl->full())
    {
		//Wait blocks the thread and will automatically release mutex.
		//when notified, the mutex will be reacquired
		//i.e. lock will be tried to execute on _mutex.
        _enq_cv.wait(lock);
    }
	_pimpl->enqueue(val);
	//its OK we notify before unlock mutex. Just let others w8 a little bit.
	_deq_cv.notify_one();
}
template<size_t N>
int CircularQueue<N>::wait_and_dequeue()
{
	unique_lock<mutex> lock(_mutex);
	while(_pimpl->empty())
    {
        _deq_cv.wait(lock);
    }
	int val = _pimpl->front();
	_pimpl->dequeue();
	_enq_cv.notify_one();
	return val;
}

//Implemenation of a plan circular queue
template<size_t N>
class ImplCircularQueue
{
protected:
	//This class is used as dynamically allocated on heap,
	//so I do not dynamically allocate space for this array.
	//There is no special things to do in initialize() method too.
	int _arr[N];
	size_t _front, _size;
public:
	ImplCircularQueue():_front(0), _size(0){}
	const static size_t CAPACITY = N;
	size_t  capacity() const {return CAPACITY;}
	size_t	size() const {return _size;}
	//I would like to also define enqueue(T && val) and emplace_enqueue(Args&&...)
	//if it is designed for general types. But for int, it is not necessary.
	//bool indicates if the operation succeeds. We could throw exceptions instead depending on the requirements.
	bool	enqueue(int val);
	bool	dequeue();
	//calling front() when it is empty is an undefiend behavior; which is consistent with STL convention
	int		front() const {return _arr[_front];}
	//calling back() when it is empty is an undefiend behavior; which is consistent with STL convention
	int		back() const {return _arr[(_front + _size - 1) % CAPACITY];}
	bool	empty() const {return _size == 0;}
	bool	full() const {return _size == CAPACITY;}
};
template<size_t N>
bool ImplCircularQueue<N>::enqueue(int val)
{
	if (full())
		return false;
	_arr[(_front + _size) % CAPACITY] = val;
	++ _size;
	return true;
}
template<size_t N>
bool ImplCircularQueue<N>::dequeue()
{
	if (empty())
		return false;
	_front = (_front + 1) % CAPACITY;
	--_size;
	return true;
}
void test1(CircularQueue<10> & que)
{

	que.wait_and_enqueue(1);
	que.wait_and_enqueue(2);
	que.wait_and_enqueue(3);
	que.wait_and_enqueue(4);
	que.wait_and_enqueue(5);

	que.wait_and_enqueue(6);
	que.wait_and_enqueue(7);
	que.wait_and_enqueue(8);
	que.wait_and_enqueue(9);
	que.wait_and_enqueue(10);
	cout<<"haha"<<endl;
	que.wait_and_enqueue(11);
	cout<<"haha"<<endl;
	que.wait_and_enqueue(12);
	que.wait_and_enqueue(13);
	cout<<que.size()<<endl;



}
void test2(CircularQueue<10> & que)
{
	std::chrono::milliseconds dura( 4000 );
    std::this_thread::sleep_for( dura );
	auto qq = que;
	int val = qq.wait_and_dequeue();
	cout<<val<<endl;
	val = qq.wait_and_dequeue();
	cout<<val<<endl;
	val = qq.wait_and_dequeue();
	cout<<val<<endl;
	cout<<"HH"<<endl;
	std::this_thread::sleep_for( dura );
	que = std::move(qq);



}
void test(CircularQueue<10> & que)
{

	cout<<que.size()<<que.empty()<<endl;
	que.enqueue(5);
	cout<<que.size()<<que.front()<<que.back()<<endl;
	que.enqueue(6);
	cout<<que.size()<<que.front()<<que.back()<<endl;
	que.enqueue(7);
	que.enqueue(8);
	que.enqueue(7);
	que.enqueue(8);
	que.enqueue(7);
	que.enqueue(8);
	que.enqueue(7);
	que.enqueue(7);
	que.enqueue(7);
	que.enqueue(8);
	que.enqueue(8);
	que.enqueue(8);
	cout<<que.size()<<que.empty()<<que.full()<<endl;
	que.dequeue();
	que.dequeue();
	que.dequeue();
	que.enqueue(9);
	que.enqueue(10);
	cout<<que.size()<<que.front()<<que.back()<<endl;

	auto qqq = que;
	cout<<qqq.size()<<qqq.front()<<qqq.back()<<endl;
	auto qq = std::move(qqq);


	cout<<qq.size()<<qq.front()<<qq.back()<<qq.empty()<<endl;
}
int main()
{
	CircularQueue<10> que;
	std::thread t1( test1, std::ref(que));
   std::thread t2( test2, std::ref(que));
    t1.join();
    t2.join();

   cout<<que.empty()<<endl;
	while(!que.empty())
	{
		cout<<que.front()<<endl;
		que.dequeue();
    }

}














