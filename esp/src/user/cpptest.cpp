
extern "C" {
	int cppTest();
}

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