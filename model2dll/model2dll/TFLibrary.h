// TFLibrary.h

#include <sys/stat.h>
#include <string>
#include <fstream>
#include <queue>
#include <deque>

#pragma once
#ifdef TFLIBRARY_EXPORTS
#define TFLIBRARY_API __declspec(dllexport)  

#include "include/Model.h"
#include "include/Tensor.h"


class SamplesCache {
private:
    unsigned __int64 m_numberOfSamples{ 0 };
    unsigned __int64 m_numberOfElements{ 0 };
    std::vector<std::vector<float>> m_data;
    int m_headIndex{ 0 };

    void UpdateSize()
    {
        m_data = std::vector<std::vector<float> >(m_numberOfSamples, std::vector<float>(m_numberOfElements));
        m_headIndex = 0;
    }
public:
    void SetNumberOfSamples(const int numberOfSamples)
    {
        m_numberOfSamples = numberOfSamples;
        UpdateSize();
    }
    void SetNumberOfElements(const int numberOfElements)
    {
        m_numberOfElements = numberOfElements;
        UpdateSize();
    }
    void addSample(const float* sample)
    {
        m_headIndex++;
        if (m_headIndex >= m_numberOfSamples)
        {
            m_headIndex = 0;
        }
        memcpy(&m_data[m_headIndex].begin(), sample, sizeof(float) * m_numberOfElements);
    }

    std::vector<float> flatten()
    {
        std::vector<float> result(m_numberOfElements * m_numberOfSamples);
        int head = m_headIndex;
        auto last = std::begin(result);
        for (int num = 0; num < m_numberOfSamples; num++)
        {
            head += num;
            if (head >= m_numberOfSamples)
            {
                head = 0;
            }
            last = std::copy(std::begin(m_data[head]), std::end(m_data[head]), last);
        }
        return result;
    }
};

#else  
#define TFLIBRARY_API __declspec(dllimport)   
#endif  // TFLIBRARY_EXPORTS


TFLIBRARY_API int ExecuteExample();

class TFLIBRARY_API TFNetwork
{
private:

#ifdef TFLIBRARY_EXPORTS
	std::unique_ptr<Model> m_model;

	std::unique_ptr<Tensor> m_input;
	std::unique_ptr<Tensor> m_result;

    SamplesCache m_positions;
    SamplesCache m_orientations;
#endif  // TFLIBRARY_EXPORTS

    int m_numberOfFrames{0};
	int m_numberOfBlocks{0};
public:
	// Predicting positions and orientations of blocks.
	bool Initialize(const int& numberOfFrames, const int& numberOfBlocks, const char* modelPath = "model.pb" );
	bool AddSample(const float* positions, const float* orientations);
	bool Predict(float* positions, float* orientations);
};
