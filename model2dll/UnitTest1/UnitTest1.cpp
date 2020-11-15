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
            TFNetwork network;

            // Test initialization
            int numberOfFrames, numberOfBlocks;
            bool result = network.Initialize(numberOfFrames, numberOfBlocks, "D:\\git\\ueflow\\model\\model.pb");
            Assert::IsTrue(result);
            Assert::AreEqual(numberOfFrames, 3);
            Assert::AreEqual(numberOfBlocks, 4);
            // Test add samplle
            std::vector<float> positions(numberOfBlocks, 0);
            std::vector<float> orientations(numberOfBlocks, 0);
            network.AddSample(positions.data(), orientations.data());
            network.AddSample(positions.data(), orientations.data());
            network.AddSample(positions.data(), orientations.data());
            network.AddSample(positions.data(), orientations.data());
            network.AddSample(positions.data(), orientations.data());
            network.AddSample(positions.data(), orientations.data());
            network.AddSample(positions.data(), orientations.data());
            network.AddSample(positions.data(), orientations.data());
            network.AddSample(positions.data(), orientations.data());
            network.AddSample(positions.data(), orientations.data());

            // Test predict
            network.Predict(positions.data(), orientations.data());
            Assert::IsTrue(fabs(positions[0]) > 0.00001);
            Assert::IsTrue(fabs(orientations[0]) > 0.00001);
        }
    };
}