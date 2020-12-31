// Fill out your copyright notice in the Description page of Project Settings.


#include "TensorflowFunctionsLibrary.h"
// #include <TFLibrary.h>


bool UTensorflowFunctionsLibrary::LoadAndRunNetwork()
{
    // Load model with a path to the .pb file. 
    // An optional std::vector<uint8_t> parameter can be used to supply Tensorflow with
    // session options. The vector must represent a serialized ConfigProto which can be 
    // generated manually in python. See create_config_options.py.
    // Example:
    // const std::vector<uint8_t> ModelConfigOptions = { 0x32, 0xb, 0x9, 0x9a, 0x99, 0x99, 0x99, 0x99, 0x99, 0xb9, 0x3f, 0x20, 0x1 };
    // Model model("../model.pb", ModelConfigOptions);
    int x = 1;

	UE_LOG(LogTemp, Warning, TEXT("Your message"));
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Blocking Hit =:"));
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("X: x: %d"), x));

    return true;
}