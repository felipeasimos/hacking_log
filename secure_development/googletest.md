# Google test

sources:
https://stackoverflow.com/questions/13513905/how-to-set-up-googletest-as-a-shared-library-on-linux \
http://www.yolinux.com/TUTORIALS/Cpp-GoogleTest.html
https://github.com/nordlow/gtest-tutorial
http://cheezyworld.com/wp-content/uploads/2010/12/PlainGoogleQuickTestReferenceGuide1.pdf

## Installation

Due to certain C++ stuff (One-definition Rule), google itself recommends to compile the framework instead
of installing pre-compiled binaries. Let's do it.

Install source code and compile:

```
cd /opt
sudo wget https://github.com/google/googletest/archive/release-1.10.0.tar.gz -O gtest-1.10.0.tar.gz
sudo tar -xvf gtest-1.10.0.tar.gz
cd googletest-release-1.10.0/
sudo cmake -DBUILD_SHARED_LIBS=ON
sudo make
```

Install headers and libs on system:

```
sudo cp -ar googletest/include/gtest /usr/local/include
sudo cp -ar googlemock/include/gmock /usr/local/include
sudo cp -a lib/*.so /usr/local/lib
```

You can check if the GNU linker can find the libs with:

```
sudo ldconfig -v 2>/dev/null | grep -E "g(test|mock)"
```

This should return:

```
	libgtest_main.so -> libgtest_main.so
	libgtest.so -> libgtest.so
	libgmock_main.so -> libgmock_main.so
	libgmock.so -> libgmock.so
```

## Compiling

If using `gtest`, add `#include <gtest/gtest.h>` and if
using `gmock` add `#include<gmock/gmock.h>`.

To link against the libraries just use `-l<lib_name>`,
which can be `gtest`, `gtest_main`, `gmock` or `gmock_main`.

### Difference between gtest and gtest\_main

If you are using a `main` function in your test, link to
`-lgtest`, otherwise link to `-lgtest_main`. This way you
just don't write a `main` function if you don't need it.

## Testing

### Assertions and Expectations

Inside the body of a test case you will have `ASSERT`
and `EXPECT` macros. Both do some kind of check,
that you can choose based on its suffix and inform
you about it (in greater detail if it fails).

The difference between them is that if the
check at the `ASSERT` macro fails it aborts
the test case and `EXPECT` just inform you
of the failure.

There are many times of assertions and expectations,
both have a counterpart of the same type in the other
(like `ASSERT_EQ` and `EXPECT_EQ`).

All asserts are written as `ASSERT_*` and expectations
are written as `EXPECT_*`. What is in the place of
the `*` defines the type of check that will be done.

There are some types of checks: between values,
between strings and other advanced types.

Here are some examples (ther are many more, check here):

Assertion | Expectation | Verifies |
----------|-------------|----------|
`ASSERT_EQ(val1, val2)` | `EXPECT_EQ(val1, val2)` | `val1==val2` |
`ASSERT_STREQ(str1,str2)` | `EXPECT_STREQ(str1, str2)` | `the two C strings have the same contents`

### Test Cases

#### Normal Test Cases

Normal tests use the `TEST` macro. The first
argument is the test suite name and the second
is the test case name:

```
TEST(FactorialTest, HandlesZeroInput) {
	EXPECT_EQ(Factorial(0), 1);
}
```

#### Test Cases With Fixtures

To have some code be done after and before we run test
cases, we declare a class inherited from `::Testing::Test`
with virtual `SetUp()` and `TearDown()` functions. Like
so:

```
#include <gtest/gtest.h>
#include "cache_info.hpp"

class CacheInfoTest : public ::testing::Test {

	protected:
		virtual void SetUp(){

			cache1 = CacheInfo();
		}
		virtual void TearDown(){
		
		}

	CacheInfo cache1;
};

TEST_F(CacheInfoTest, block_size){

	ASSERT_EQ(cache1.block_size(), 64);
}
```

As you can see, the test that uses fixtures is define by
the `TEST_F` macro. The first argument is the fixture
class (conceptually it is the test suite name)
and second is the name of the test case.
