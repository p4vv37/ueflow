#include "pch.h"
#include "CppUnitTest.h"
#include "TFLibrary.h"
using namespace Microsoft::VisualStudio::CppUnitTestFramework;
namespace MyTest
{
    TEST_CLASS(MyTests)
    {
    public:
        TEST_METHOD(basicTests)
        {
            TFNetwork x;
            int a, b;
            bool result = x.Initialize(a, b, "D:\\git\\ueflow\\model\\model.pb");
            Assert::IsTrue(result);
            Assert::AreEqual(a, 3);
            Assert::AreEqual(b, 4);
        }
    };
}