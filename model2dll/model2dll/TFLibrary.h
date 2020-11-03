//Calculation.h
#pragma once
#ifdef TFLIBRARY_EXPORTS
#define TFLIBRARY_API __declspec(dllexport)   
#else  
#define TFLIBRARY_API __declspec(dllimport)   
#endif  

TFLIBRARY_API int ExecuteExample();

class TFLIBRARY_API TFNetwork 
{

private:
	std::string m_modelPath{ "model.pb" };
public:
	bool Initialize();
};
