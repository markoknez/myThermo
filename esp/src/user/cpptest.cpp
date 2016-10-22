
#include "cpptest.h"

class test {
private :
	int x;
public :
	int get() {
		return x;
	}
	void set(int val) {
		x = val;
	}
} testObj;

extern "C" int cppTest() {
	testObj.set(15);
	return testObj.get();
}