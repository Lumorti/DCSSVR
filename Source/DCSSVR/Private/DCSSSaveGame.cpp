// Fill out your copyright notice in the Description page of Project Settings.

#include "DCSSSaveGame.h"

UDCSSSaveGame::UDCSSSaveGame() {
    inventoryLetterToName = TMap<FString, FString>();
	inventoryLocToLetter = TArray<FString>();
	hotbarInfos = TArray<FString>();
	musicVolume = 1.0f;
	trackInd = 0;
}