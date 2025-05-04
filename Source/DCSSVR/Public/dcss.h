// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/TextRenderActor.h"
#include "Runtime/Engine/Classes/Components/TextRenderComponent.h"
#include "CoreMinimal.h"
#include "DCSSSaveGame.h"
#include "GameFramework/Actor.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Components/WidgetComponent.h"
#include "Components/WidgetSwitcher.h"
#include "Components/ProgressBar.h"
#include "Blueprint/UserWidget.h"
#include "Sound/AmbientSound.h"
#include "Components/AudioComponent.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/Button.h"
#include "Components/Border.h"
#include "Runtime/HeadMountedDisplay/Public/IHeadMountedDisplay.h"
#include "IXRTrackingSystem.h"
#include "MotionControllerComponent.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "HttpModule.h"
#include "Kismet/GameplayStatics.h"
#include "RHIFeatureLevel.h"
#include "dcss.generated.h"

UCLASS()
class DCSSVR_API Adcss : public AActor
{
	GENERATED_BODY()
	
public:

	// Sets default values for this actor's properties
	Adcss();

	// The reference to the ui object
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	AActor* refToUIActor;

	// The reference to the menu panels
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	AActor* refToMainMenuActor;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	AActor* refToSpeciesActor;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	AActor* refToBackgroundActor;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	AActor* refToNameActor;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	AActor* refToSaveActor;

	// The references to the music
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<USoundWave*> musicList;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	AAmbientSound* refToMusicActor;

	// The reference to the description panel
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	AActor* refToDescriptionActor;

	// The reference to the keyboard panel
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	AActor* refToKeyboardActor;

	// The reference to the inventory panel
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	AActor* refToInventoryActor;

	// The reference to the death panel
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	AActor* refToDeathActor;

	// The reference to the altar panel
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	AActor* refToAltarActor;

	// The reference to the shop panel
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	AActor* refToShopActor;

	// The reference to the settings panel
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	AActor* refToSettingsActor;

	// The reference to the choice panel
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	AActor* refToChoiceActor;

	// The reference to the text object
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ATextRenderActor* refToTextActor;

	// The references to the floor planes
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	AActor* floorTemplate;

	// The references to the wall planes
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	AActor* wallTemplate;

	// The references to the effect planes
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	AActor* effectTemplate;

	// The references to the enemies/plants etc.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	AActor* enemyTemplate;

	// The references to any items
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	AActor* itemTemplate;

	// The references to texutres
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FString, UTexture2D*> textures;

	// Whether to generate a new level
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool generateNew = false;

	// Debug angles
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector debugAngles;

	// Keep track of the current active hand
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString activeHand = "right";

	// The function called when a key is pressed
	UFUNCTION(BlueprintCallable, Category = "DCSSVR")
	void keyPressed(FString key, FVector2D delta);

	// The function called when a button is hovered/unhovered
	UFUNCTION(BlueprintCallable, Category = "DCSSVR")
	void buttonHovered(FString buttonName, bool hovered);

	// Put here to stop nullptr errors
	UPROPERTY(VisibleAnywhere, Category = Basic)
	UDCSSSaveGame* saveGame;
	UPROPERTY(VisibleAnywhere, Category = Basic)
	UDCSSSaveGame* saveGameGlobal;

	// Various functions
	void updateLevel();
	void writeCommand(FString input);
	void writeCommandQueued(FString input);
	UTexture2D* getTexture(FString name);
	FString itemNameToTextureName(FString name);
	FString enemyNameToTextureName(FString name);
	void loadEverything();
	void saveEverything();
	void init();
	void clearThings();
	void shiftLetters();
	void submitBug(FString message);

protected:

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called when the game ends
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

};

