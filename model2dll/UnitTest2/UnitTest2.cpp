#include "pch.h"
#include "CppUnitTest.h"
#include "..\model2dll\TFLibrary.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTest2
{
	TEST_CLASS(UnitTest2)
	{
	public:
		
		TEST_METHOD(TestMethod1)
		{
            TFNetwork network;

            // Test initialization
            int numberOfFrames, numberOfBlocks;
            bool result = network.Initialize(numberOfFrames, numberOfBlocks, "../../model/saved_model");
            Assert::IsTrue(result);
            Assert::AreEqual(numberOfFrames, 3);
            Assert::AreEqual(numberOfBlocks, 4);
            // Test add samplle
            std::vector<float> positions(numberOfBlocks * 3, 0);
            std::vector<float> orientations(numberOfBlocks * 3, 0);
            network.AddSample(positions.data(), orientations.data(), 0, 0);
            network.AddSample(positions.data(), orientations.data(), 0, 0);
            network.AddSample(positions.data(), orientations.data(), 0, 0);
            network.AddSample(positions.data(), orientations.data(), 0, 0);
            network.AddSample(positions.data(), orientations.data(), 0, 0);
            network.AddSample(positions.data(), orientations.data(), 0, 0);
            network.AddSample(positions.data(), orientations.data(), 0, 0);
            network.AddSample(positions.data(), orientations.data(), 0, 0);
            network.AddSample(positions.data(), orientations.data(), 0, 0);
            network.AddSample(positions.data(), orientations.data(), 0, 0);

            // test initial 
            network.getInitialPositions(positions.data());
            network.getInitialOrientations(orientations.data());

            // Test predict
            network.Predict(positions.data(), orientations.data());
            Assert::IsTrue(fabs(positions[0]) > 0.00001);
            Assert::IsTrue(fabs(orientations[0]) > 0.00001);
		}
	};
}
