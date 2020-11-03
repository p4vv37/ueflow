// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../ThirdParty/include/TFLibrary.h"
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "TensorflowFunctionsLibrary.generated.h"

/**
 * 
 */
UCLASS()
class TFINUNREAL_API UTensorflowFunctionsLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "TensorFlow")
	static bool LoadAndRunNetwork();
};
