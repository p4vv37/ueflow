// TFLibrary.h

#include <sys/stat.h>
#include <string>
#include <fstream>
#include <queue>
#include <deque>
#include <math.h>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <assert.h>  

#pragma once

#ifdef TFLIBRARY_EXPORTS
#include "include/Model.h"
#include "include/Tensor.h"
#define PI 3.14159265f
#define TFLIBRARY_API __declspec(dllexport)  

#else  
#define TFLIBRARY_API __declspec(dllimport)   
#endif  // TFLIBRARY_EXPORTS

class SamplesCache {
private:
    unsigned __int64 m_numberOfSamples{ 0 };
    unsigned __int64 m_numberOfElements{ 0 };
    float* m_data{ nullptr };
    int m_headIndex{ 0 };

    void UpdateSize();
public:
    SamplesCache() {};
    ~SamplesCache();
    SamplesCache(const SamplesCache& other);
    SamplesCache(SamplesCache&& other);
    SamplesCache& operator=(const SamplesCache& other);
    SamplesCache& operator=(SamplesCache&& other);

    void SetNumberOfSamples(const int numberOfSamples);
    void SetNumberOfElements(const int numberOfElements);
    void addSample(const float* sample);
};

class Tensor;
class Model;

TFLIBRARY_API int ExecuteExample();

class TFLIBRARY_API TFNetwork
{
private:
	Model* m_model;

	Tensor* m_input;
	Tensor* m_result;

    SamplesCache m_positions;
    SamplesCache m_orientations;

    int m_numberOfFrames{ 0 };
    int m_numberOfBlocks{ 0 };
    char* m_blocksModels;
    float* m_initialBlocksPositions;
    float* m_initialBlocksOrientations;

public:

    TFNetwork() { };
    ~TFNetwork();

	bool Initialize(int& numberOfFrames, int& numberOfBlocks, const char* modelPath = "model.pb");

    const char* getBlocksModels();
    void getInitialPositions(float* positions);
    void getInitialOrientations(float* orientations);

	bool AddSample(const float* positions, const float* orientations);
	bool Predict(float* positions, float* orientations);
};
