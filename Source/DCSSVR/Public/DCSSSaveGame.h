// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "DCSSSaveGame.generated.h"

UCLASS()
class DCSSVR_API UDCSSSaveGame : public USaveGame {
	GENERATED_BODY()

public:

	UPROPERTY(VisibleAnywhere, Category = Basic)
	TMap<FString, FString> inventoryLetterToName;

	UPROPERTY(VisibleAnywhere, Category = Basic)
	TArray<FString> inventoryLocToLetter;

	UPROPERTY(VisibleAnywhere, Category = Basic)
	TArray<FString> hotbarInfos;

	UPROPERTY(VisibleAnywhere, Category = Basic)
	float musicVolume;

	UPROPERTY(VisibleAnywhere, Category = Basic)
	int trackInd;

	UDCSSSaveGame();
	
};
