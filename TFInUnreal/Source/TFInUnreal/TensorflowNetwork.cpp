// Fill out your copyright notice in the Description page of Project Settings.


#include "TensorFlowNetwork.h"

#define MULTIPLIER 100.0f
#define SLOWDOWN 1.0f

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>

// Sets default values
ATensorFlowNetwork::ATensorFlowNetwork()
{
    static ConstructorHelpers::FObjectFinder<UStaticMesh> StaticMeshFinder(TEXT("StaticMesh'/Game/Meshes/waterSurface.waterSurface'"));
    mMesh = StaticMeshFinder.Object;
}


bool ATensorFlowNetwork::InitializeModel()
{
    if (!FPaths::DirectoryExists(ModelPath))
    {
        return false;
    }


    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.NameMode = FActorSpawnParameters::ESpawnActorNameMode::Required_Fatal;
    SpawnParams.bHideFromSceneOutliner = 0;
    SpawnParams.Instigator = GetInstigator();
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    AStaticMeshActor* NewElement = GetWorld()->SpawnActor<AStaticMeshActor>(SpawnParams);
    NewElement->GetStaticMeshComponent()->SetStaticMesh(mMesh);
    NewElement->SetActorHiddenInGame(false);
    NewElement->SetMobility(EComponentMobility::Movable);

    std::string stringPath = std::string(TCHAR_TO_UTF8(*ModelPath));
    cppflow::model model (stringPath);
    m_model = MakeUnique<cppflow::model>(std::string(TCHAR_TO_UTF8(*ModelPath)));

    std::fstream in("D:/dnn/data/0_0.158275115632_500__0.txt");
    std::string line;
    std::vector<float> inWater;

    while (std::getline(in, line))
    {
        float value;
        std::stringstream ss(line);

        ss >> value;
        inWater.push_back(value);
    }
    std::cout << inWater.size();
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

