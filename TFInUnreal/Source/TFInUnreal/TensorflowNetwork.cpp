// Fill out your copyright notice in the Description page of Project Settings.


#include "TensorFlowNetwork.h"

// Sets default values
ATensorFlowNetwork::ATensorFlowNetwork()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh> StaticMeshFinder(TEXT("StaticMesh'/Game/Meshes/box.box'"));
	mMeshes.Emplace("box0", StaticMeshFinder.Object);
}


bool ATensorFlowNetwork::InitializeModel()
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

	std::vector<float> initialPositions(m_numberOfBlocks * 3, 0);
	std::vector<float> initialOrientations(m_numberOfBlocks * 3, 0);

	m_network.getInitialPositions(initialPositions.data());
	m_network.getInitialOrientations(initialOrientations.data());
	const char* blocksModelsRaw = m_network.getBlocksModels();
	FString blocksModels(blocksModelsRaw);
	TArray < FString > blocksModelsArray;
	int numOfModels = blocksModels.ParseIntoArray(blocksModelsArray, TEXT(";"), true);
	if (numOfModels != m_numberOfBlocks)
	{
		return false;
	}

	// Preparation of scene
	float unitsScale{ 100.0f };
	for (int i = 0; i < m_numberOfBlocks; i++)
	{

		FTransform transform;
		FVector vec{ unitsScale * initialPositions[3 * i] + 100,   unitsScale * initialPositions[3 * i + 2], unitsScale * initialPositions[3 * i + 1] };
		FVector vecRotation{ initialOrientations[3*i],  initialOrientations[3 * i + 2], initialOrientations[3 * i + 1] };
		FRotator rotation;
		rotation.MakeFromEuler(vecRotation);
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.NameMode = FActorSpawnParameters::ESpawnActorNameMode::Required_Fatal;
		SpawnParams.bHideFromSceneOutliner = 0;
		SpawnParams.Instigator = GetInstigator();
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AStaticMeshActor* NewElement = GetWorld()->SpawnActor<AStaticMeshActor>(vec, rotation, SpawnParams);
		NewElement->GetStaticMeshComponent()->SetStaticMesh(mMeshes["box0"]);
		NewElement->SetActorHiddenInGame(false);
		NewElement->SetMobility(EComponentMobility::Movable);

		if (!i)
		{
			NewElement->Tags.Add(FName("bottomElement"));
		}

		if (disablePhysics)
		{
			NewElement->DisableComponentsSimulatePhysics();
			NewElement->SetActorEnableCollision(false);
			NewElement->GetStaticMeshComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
		else
		{
			NewElement->GetStaticMeshComponent()->SetSimulatePhysics(true);
			NewElement->GetStaticMeshComponent()->RegisterComponent();
		}

		m_elementsList.Add(NewElement);
	}
	return true;
}

void ATensorFlowNetwork::ApplyForce()
{
	mForceFramesLeft = 3;
}

// Called every frame
void ATensorFlowNetwork::UpdateScene()
{
	if (GetWorld()->GetTimeSeconds() - mSeconds > 1.0f / 30.0f)
	{
		mSeconds = GetWorld()->GetTimeSeconds();
		std::vector<float> positions(m_elementsList.Num() * 3, 0);
		std::vector<float> orientations(m_elementsList.Num() * 3, 0);
		FVector loc{};
		FVector rot{};

		int i = 0;
		for (AStaticMeshActor* elem : m_elementsList)
		{
			loc = elem->GetActorLocation();
			positions[i * 3] = loc.X / 100.0;
			positions[i * 3 + 1] = loc.Z / 100.0;
			positions[i * 3 + 2] = loc.Y / 100.0;
			i++;
		}

		m_network.AddSample(positions.data(), orientations.data(), mForceFramesLeft > 0, forceAngle);
		mForceFramesLeft--;

		i = 0;
		m_network.Predict(positions.data(), orientations.data());
		FQuat rotQuat;
		for (AStaticMeshActor* elem : m_elementsList)
		{
			loc.X = positions[i * 3] * 100.0;
			loc.Z = positions[i * 3 + 1] * 100.0;
			loc.Y = positions[i * 3 + 2] * 100.0;
			elem->SetActorLocation(loc);

			rot.X = positions[i * 3];
			rot.Z = positions[i * 3 + 1];
			rot.Y = positions[i * 3 + 2];
			rotQuat.MakeFromEuler(rot);
			elem->SetActorRotation(rotQuat);
			i++;
		}
	}
}

