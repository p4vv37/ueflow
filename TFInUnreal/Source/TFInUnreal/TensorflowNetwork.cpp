// Fill out your copyright notice in the Description page of Project Settings.


#include "TensorFlowNetwork.h"

// Sets default values
UTensorFlowNetwork::UTensorFlowNetwork()
{
}


// Called every frame
bool UTensorFlowNetwork::InitializeModel()
{
	// Preparation of scene
	AStaticMeshActor* tmpReference;
	for (int i = 0; i < 4; i++)
	{

		tmpReference = GetWorld()->SpawnActor<AStaticMeshActor>();
		m_elementsList.Add(tmpReference);
	}

	// Initialization of model itself
	auto projectDir = FPaths::ProjectDir();
	FString completePath = FPaths::Combine(projectDir, ModelPath);
	return m_network.Initialize(1, 1, TCHAR_TO_UTF8(*completePath));
}


// Called every frame
void UTensorFlowNetwork::UpdateScene()
{
	//m_network.Predict();
}

