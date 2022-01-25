// Fill out your copyright notice in the Description page of Project Settings.


#include "TensorFlowNetwork.h"

#define MULTIPLIER 100.0f
#define SLOWDOWN 1.0f

// Sets default values
ATensorFlowNetwork::ATensorFlowNetwork()
{
}


bool ATensorFlowNetwork::InitializeModel()
{
    if (!FPaths::DirectoryExists(ModelPath))
    {
        return false;
    }

    std::string stringPath = std::string(TCHAR_TO_UTF8(*ModelPath));
    cppflow::model model (stringPath);
    // m_model = MakeUnique<cppflow::model>(std::string(TCHAR_TO_UTF8(*ModelPath)));
	return true;
}

void ATensorFlowNetwork::ApplyForce()
{
    mForceFramesLeft = 1;
}

// #pragma optimize( "", off )
// Called every frame
void ATensorFlowNetwork::UpdateScene()
{
}

#pragma optimize( "", on ) 

