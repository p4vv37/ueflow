//Calculation.h
#pragma once
#ifdef MATHLIBRARY_EXPORTS   
#define MATHLIBRARY_API __declspec(dllexport)   
#else  
#define MATHLIBRARY_API __declspec(dllimport)   
#endif  
MATHLIBRARY_API int Addition();
