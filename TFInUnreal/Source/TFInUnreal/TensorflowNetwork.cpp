// Fill out your copyright notice in the Description page of Project Settings.


#include "TensorFlowNetwork.h"

// Sets default values
UTensorFlowNetwork::UTensorFlowNetwork()
{
}


// Called every frame
bool UTensorFlowNetwork::InitializeModel()
{
	bool result{ false };

	// Initialization of model itself
	auto projectDir = FPaths::ProjectDir();
	FString completePath = FPaths::Combine(projectDir, ModelPath);
	result = m_network.Initialize(m_numberOfFrames, m_numberOfBlocks, TCHAR_TO_UTF8(*completePath));
	if (!result)
	{
		return false;
	}
	const char* blockModels = m_network.getBlocksModels();

	// Preparation of scene
	AStaticMeshActor* tmpReference;
	for (int i = 0; i < m_numberOfBlocks; i++)
	{

		tmpReference = GetWorld()->SpawnActor<AStaticMeshActor>();
		m_elementsList.Add(tmpReference);
	}
	return true;
}


// Called every frame
void UTensorFlowNetwork::UpdateScene()
{
	//m_network.Predict();
}

