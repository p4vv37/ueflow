// Fill out your copyright notice in the Description page of Project Settings.


#include "TensorFlowNetwork.h"

// Sets default values
UTensorFlowNetwork::UTensorFlowNetwork()
{
}


// Called every frame
bool UTensorFlowNetwork::InitializeModel()
{
	return m_network.Initialize();
}


// Called every frame
void UTensorFlowNetwork::UpdateScene()
{
}

