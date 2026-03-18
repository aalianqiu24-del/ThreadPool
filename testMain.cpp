#include"FixedThreadPool2.hpp"
#include"SyncQueue2.hpp"
#include<iostream>
#include<vector>
#include<algorithm>
#include<chrono>
using namespace std;
using namespace std::chrono;

void my_rand(std::vector<int>& vec)
{
	int n = vec.capacity();
	for (int i = 0; i < n; i++)
	{
		vec.push_back(rand() % 1000000);
	}
}
void my_sort(std::vector<int>& vec)
{

	std::sort(vec.begin(), vec.end());
}
int main()//看用固定线程池进行排序的时间是多少
{
	FixedThreadPool mypool(20,8);
	const int n = 1000;
	const int elemnum = 10000;
	std::vector<std::vector<int>>ivec;
	ivec.resize(n);
	time_point<high_resolution_clock>begin;
	begin = high_resolution_clock::now();
	{
		FixedThreadPool mypool(20, 8);
		for (int i = 0; i < n; i++)
		{
			ivec[i].reserve(elemnum);
			//my_rand(ivec[i]);
			mypool.AddTask(std::bind(my_rand, std::ref(ivec[i])));
		}
	}
	{
		FixedThreadPool mypool(2000, 8);
		for (int i = 0; i < n; i++)
		{
			//my_sort(ivec[i]);
			mypool.AddTask(std::bind(my_sort, std::ref(ivec[i])));
		}
	}
	std::int64_t diff = duration_cast<microseconds>(high_resolution_clock::now() - begin).count();
	cout << "diff " << diff << "ms" << endl;
}



#if 0
int main()
{
	const int n = 1000;
	const int element = 10000;
	std::vector<std::vector<int>>ivec;
	ivec.resize(n);
	time_point<high_resolution_clock>begin;
	begin = high_resolution_clock::now();
	for (int i = 0; i < n; i++)
	{
		ivec[i].reserve(element);
		my_rand(ivec[i]);
	}
	for (int i = 0; i < n; i++)
	{
		my_sort(ivec[i]);
	}
	std::int64_t diff = duration_cast<microseconds>(high_resolution_clock::now() - begin).count();
	cout << "diff " << diff << "ms" << endl;
}
#endif
#if 0
void func(int x)
{
	static int num = 0;
	cout << "func_" << x << "num: " << ++num << endl;
}
class Object
{
private:
	int val;
public:
	Object() :val(0) {}
	void Print()
	{
		cout << "Object::Print num:" << ++val << endl;
	}
};

int main()
{
	FixedThreadPool mypool(1000,4);
	Object obj;
	int n = 100;
	for (int i = 0; i < n; i++)
	{
		mypool.AddTask(std::bind(&Object::Print, &obj));
	}
	//std::this_thread::sleep_for(std::chrono::seconds(2));
	obj.Print();
	return 0;
}
#endif
