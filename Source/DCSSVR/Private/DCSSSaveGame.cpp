// Fill out your copyright notice in the Description page of Project Settings.

#include "DCSSSaveGame.h"

UDCSSSaveGame::UDCSSSaveGame() {
    inventoryLetterToName = TMap<FString, FString>();
	inventoryLocToLetter = TArray<FString>();
	hotbarInfos = TArray<FString>();
	inventoryRelLoc = FVector(0.0f, 150.0f, 200.0f);
	inventoryRelRot = FRotator(0.0f, -90.0f, 0.0f);
	musicVolume = 1.0f;
	trackInd = 0;
	snapDegrees = 30;
	serverAddress = "";
}