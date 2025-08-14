#include "dcss.h"
FString version = TEXT("0.1");

// 0.1 Initial Release
// - spear evoke
// - item evoke
// - all uniques
// - BUG sometimes saving on exit doesn't work

// From https://hashnode.com/post/case-sensitive-tmaplessfstring-int32greater-in-unreal-engine-4-in-c-ckvc1jse20qf645s14e3d6ntd
// Needed because FString == FString is case-insensitive, which is literally insane
// "A" == "a"? Are you kidding me?
template<typename TValueType>
struct FCaseSensitiveLookupKeyFuncs : BaseKeyFuncs<TValueType, FString>
{
    static FORCEINLINE const FString& GetSetKey(const TPair<FString, TValueType>& Element)
    {
        return Element.Key;
    }
    static FORCEINLINE bool Matches(const FString& A, const FString& B)
    {
        return A.Equals(B, ESearchCase::CaseSensitive);
    }
    static FORCEINLINE uint32 GetKeyHash(const FString& Key)
    {
        return FCrc::StrCrc32<TCHAR>(*Key);
    }
};

// Game params/constants
int LOS;
int gridWidth;
int maxEnemies;
int maxEffects;
int maxItems;
float wallScaling;
float diagWallScaling;
float halfDiagWallScaling;
float smallWallScaling;
FString thingsThatCountAsWalls;
FString thingsThatCountAsDoors;
FString thingsThatCountAsItems;
float epsilon;
int maxLogShown;
int numHotbarSlots;
TMap<FString, FString, FDefaultSetAllocator, FCaseSensitiveLookupKeyFuncs<FString>> skillDescriptions;
FString menuOutput;

// Stuff for the process
FString exePath;
FString args;
void* StdOutReadHandle = nullptr;
void* StdOutWriteHandle = nullptr;
void* StdInReadHandle = nullptr;
void* StdInWriteHandle = nullptr;
FProcHandle ProcHandle;
TArray<FString> commandQueue;
double oldTime;
int numProcessed;
int prevProcessed;
bool crawlHasStarted;
bool vrEnabled;
FString outText;
TArray<FTimerHandle> timerHandles;
TArray<TSharedRef<IHttpRequest>> httpRequests;
bool isMenu;
int nextCommand;
TArray<FString> lastInputs;
TArray<FString> lastOutputs;

// Server stuff
bool serverConnected;
bool needNewRequest;
FString serverAddress;
TArray<FString> serverAddressesToTry;
int currentRequests;

// Tile info type
struct TileInfo {
	FString currentChar;
	FString floorChar;
	TArray<FString> items;
	TArray<FString> itemHotkeys;
	FString effect;
	FString enemy;
	FString enemyHotkey;
};

// For storing the current output (2d array of chars)
FString outputBuffer = TEXT("");
TArray<FString> charArray;
TArray<TArray<FString>> levelAscii;
TArray<TArray<FString>> levelAsciiPrev;
TArray<TArray<TileInfo>> levelInfo;
bool isChecking;
bool hasLoaded;
FString prevOutput;
float lastCommandTime;
bool shiftOn;
bool rmbOn;
bool needMenu;
bool hasBeenWelcomed;

// For storing game objects
TArray<TArray<TArray<AActor*>>> wallArray;
TArray<TArray<AActor*>> floorArray;
TArray<AActor*> enemyArray;
TArray<AActor*> itemArray;
TArray<AActor*> effectArray;
float floorWidth;
float floorHeight;
float wallWidth;
int enemyUseCount;
int effectUseCount;
int itemUseCount;
TMap<FString, FString, FDefaultSetAllocator, FCaseSensitiveLookupKeyFuncs<FString>> enemyList;
TArray<TTuple<int, int>> checkLocs;
int currentHP;
int maxHP;
int currentMP;
int maxMP;
bool inMainMenu;
FString fullName;
UWorld* worldRef;
FString leftText;
FString rightText;
FString statusText;
FString whiteLine;
FString currentDescription;
FString currentUsage;
TArray<FString> logText;
UTextRenderComponent* refToLogRender;
FString currentUI;
FString defaultSkillDescription;
FString overviewText;
FString religionText;
bool shouldRedrawOverview;
bool shouldRedrawReligion;
bool settingsOpen;
FString saveFile;
TMap<int, TArray<FVector>> itemLocs;
FString currentType;
bool justUsedAScroll;
FString currentBranch;
bool skipNextFullDescription;
TArray<FString> gateList;
TArray<TArray<FString>> mapToDraw;
TArray<TArray<FString>> mapToDrawRotated;

// Main menu stuff
FString currentBackground;
FString currentSpecies;
FString currentName;
FString currentSeed;
TMap<FString, FString, FDefaultSetAllocator, FCaseSensitiveLookupKeyFuncs<FString>> speciesDescriptions;
TMap<FString, FString, FDefaultSetAllocator, FCaseSensitiveLookupKeyFuncs<FString>> backgroundDescriptions;
FString defaultBackgroundDescription;
FString defaultSpeciesDescription;
TMap<FString, FString, FDefaultSetAllocator, FCaseSensitiveLookupKeyFuncs<FString>> backgroundToLetter;
TMap<FString, FString, FDefaultSetAllocator, FCaseSensitiveLookupKeyFuncs<FString>> backgroundToLetterNoGods;
TMap<FString, FString, FDefaultSetAllocator, FCaseSensitiveLookupKeyFuncs<FString>> speciesToLetter;
TArray<FString> saveNames;
int savesPage;
TArray<int> saveLocToIndex;
int buttonConfirming;

// Inventory stuff
bool draggingInventory;
float inventoryGrabDistance;
FVector inventoryGrabPoint;
TMap<FString, FString, FDefaultSetAllocator, FCaseSensitiveLookupKeyFuncs<FString>> inventoryLetterToName;
TArray<TArray<FString>> inventoryLocToLetter;
bool inventoryOpen;
bool shouldRedrawInventory;
FIntVector2 inventoryNextSpot;
TMap<FString, FString, FDefaultSetAllocator, FCaseSensitiveLookupKeyFuncs<FString>> inventoryLetterToNamePrev;
int gold;
FIntVector2 locForBlink;
FVector inventoryRelLoc;
FRotator inventoryRelRot;

// Keyboard stuff
bool isKeyboardOpen;
bool isShifted;
FString editting;
FString defaultNameText;
FString defaultSeedText;

// For choices
TArray<FString> choiceNames;
TArray<FString> choiceLetters;
bool isChoiceOpen;
FString choiceType;

// Bug reporting
FString currentBug;
FString defaultBugText;
float lastBugSubmitted;

// Music/audio stuff
int trackInd;
TArray<FString> musicListNames;

// For spells
struct SpellInfo {
	FString name;
	FString school;
	int level;
	int failure;
};
TMap<FString, SpellInfo, FDefaultSetAllocator, FCaseSensitiveLookupKeyFuncs<SpellInfo>> spellLetterToInfo;
TArray<FString> spellLetters;
bool shouldRedrawSpells;
int spellPage;
bool memorizing;
int spellLevels;
int targetingRange;

// For abilities
struct AbilityInfo {
	FString name;
	FString cost;
	int failure;
};
TMap<FString, AbilityInfo, FDefaultSetAllocator, FCaseSensitiveLookupKeyFuncs<AbilityInfo>> abilityLetterToInfo;
TArray<FString> abilityLetters;
bool shouldRedrawAbilities;
int abilityPage;

// For skills
struct SkillInfo {
	FString letter;
	float level;
	int focussed;
	int train;
	int apt;
};
TMap<FString, SkillInfo, FDefaultSetAllocator, FCaseSensitiveLookupKeyFuncs<SkillInfo>> skillNameToInfo;
bool shouldRedrawSkills;

// For passives
TArray<FString> passives;
int passivePage;
bool shouldRedrawPassives;

// For the hotbar
struct HotbarInfo {
	FString name;
	FString type;
	FString letter;
};
TArray<HotbarInfo> hotbarInfos;
HotbarInfo equippedInfo;
bool shouldRedrawHotbar;

// Altar stuff
FString altarOverview;
FString altarPowers;
FString altarWrath;
bool showingAltar;
int nextAltar;
bool showNextAltar;

// Shop stuff
struct ShopItem {
	FString name;
	FString letter;
	int price;
	bool selected;
};
TArray<ShopItem> shopItems;
FString shopTitle;
bool showingShop;
bool showNextShop;

// For selecting things
struct SelectedThing {
	int x = -1;
	int y = -1;
	FString thingIs = TEXT("");
	FString letter = TEXT("");
	int thingIndex = -1;
	SelectedThing() {}
	SelectedThing(int x, int y, FString thingIs, int thingIndex, FString letter) {
		this->x = x;
		this->y = y;
		this->thingIs = thingIs;
		this->thingIndex = thingIndex;
		this->letter = letter;
	}
};
SelectedThing selected;
TMap<FString, SelectedThing, FDefaultSetAllocator, FCaseSensitiveLookupKeyFuncs<SelectedThing>> meshNameToThing;
SelectedThing thingBeingDragged;

// For debugging
UTextRenderComponent* refToTextRender;
TSet<FString> listOfMissingThings;

// Get a texture with a given name
UTexture2D* Adcss::getTexture(FString name) {
	if (!textures.Contains(name)) {
		if (!listOfMissingThings.Contains(name)) {
			listOfMissingThings.Add(name);
			FString allMissingTextures = TEXT("");
			for (auto& Elem : listOfMissingThings) {
				allMissingTextures += Elem + TEXT(", ");
			}
			UE_LOG(LogTemp, Warning, TEXT("Missing textures: %s"), *allMissingTextures);
		}
		return textures[FString("Unknown")];
	}
	return textures[name];
}

// Sets default values
Adcss::Adcss() {
	PrimaryActorTick.bCanEverTick = true;
}

// Write a command to the process
void Adcss::writeCommand(FString input) {
	lastInputs.Add(input);
	while (lastInputs.Num() > 50) {
		lastInputs.RemoveAt(0);
	}
	if (!useServer) {
		FPlatformProcess::WritePipe(StdInWriteHandle, input);
	} else if (serverAddress.Len() > 0 && serverConnected) {
		TSharedRef<IHttpRequest> req = (&FHttpModule::Get())->CreateRequest();
		req->SetVerb("GET");
		input = input.Replace(TEXT("-"), TEXT(""));
		FString url = serverAddress + "put/" + input + FString::FromInt(nextCommand);
		nextCommand++;
		req->SetURL(url);
		req->SetTimeout(1.0f);
		req->ProcessRequest();
		httpRequests.Add(req);
		UE_LOG(LogTemp, Display, TEXT("Sending command to server: %s"), *url);
	}
}

// Write a command to the process queue
void Adcss::writeCommandQueued(FString input) {
	UE_LOG(LogTemp, Display, TEXT("Adding command to queue: %s"), *input);
	commandQueue.Add(input);
}

// Toggle the inventory open/closed
void Adcss::toggleInventory() {

	// Toggle it
	inventoryOpen = !inventoryOpen;
	if (refToInventoryActor != nullptr) {
		refToInventoryActor->SetActorHiddenInGame(!inventoryOpen);
		refToInventoryActor->SetActorEnableCollision(inventoryOpen);
	}

	// Depending on the current tab
	if (inventoryOpen) {
		UE_LOG(LogTemp, Display, TEXT("INPUT - Inventory opened, current UI: %s"), *currentUI);
		if (currentUI == "inventory") {
			selected.thingIs = "ButtonInventory";
			keyPressed("lmb", FVector2D(0.0f, 0.0f));
			selected.thingIs = "OpenButton";
		} else if (currentUI == "spells") {
			selected.thingIs = "ButtonSpells";
			keyPressed("lmb", FVector2D(0.0f, 0.0f));
			selected.thingIs = "OpenButton";
		} else if (currentUI == "abilities") {
			selected.thingIs = "ButtonAbilities";
			keyPressed("lmb", FVector2D(0.0f, 0.0f));
			selected.thingIs = "OpenButton";
		} else if (currentUI == "skills") {
			selected.thingIs = "ButtonSkills";
			keyPressed("lmb", FVector2D(0.0f, 0.0f));
			selected.thingIs = "OpenButton";
		} else if (currentUI == "character") {
			selected.thingIs = "ButtonCharacter";
			keyPressed("lmb", FVector2D(0.0f, 0.0f));
			selected.thingIs = "OpenButton";
		} else if (currentUI == "map") {
			selected.thingIs = "ButtonMap";
			keyPressed("lmb", FVector2D(0.0f, 0.0f));
			selected.thingIs = "OpenButton";
		} else if (currentUI == "menu") {
			selected.thingIs = "ButtonMenu";
			keyPressed("lmb", FVector2D(0.0f, 0.0f));
			selected.thingIs = "OpenButton";
		}
	}
}

// From https://dev.epicgames.com/community/learning/tutorials/R6rv/unreal-engine-upload-an-image-using-http-post-request-c
TArray<uint8> FStringToUint8(const FString& InString)
{
	TArray<uint8> OutBytes;
 
	// Handle empty strings
	if (InString.Len() > 0)
	{
		FTCHARToUTF8 Converted(*InString); // Convert to UTF8
		OutBytes.Append(reinterpret_cast<const uint8*>(Converted.Get()), Converted.Length());
	}
 
	return OutBytes;
}

// Submit a bug report with a message
void Adcss::submitBug(FString message) {

	// How much to indent all of the non-title lines
	FString indent = TEXT("    ");

	// If it's the default message, clear it
	if (message == defaultBugText || message == TEXT("")) {
		message = TEXT("no message");
	}

	// Generate the data dump
	FString dump = TEXT("");

	// Also put the message at the top
	dump += FString::Printf(TEXT("Bug report message:\n%s%s\n"), *indent, *message);

	// The whole inventory
	dump += TEXT("\nInventory letter to name:\n");
	for (auto& Elem : inventoryLetterToName) {
		dump += indent + Elem.Key + TEXT(": ") + Elem.Value + TEXT("\n");
	}

	// The inventory locations
	dump += TEXT("\nInventory loc to letter: \n");
	for (int i = 0; i < inventoryLocToLetter.Num(); i++) {
		for (int j = 0; j < inventoryLocToLetter[i].Num(); j++) {
			if (inventoryLocToLetter[i][j] != TEXT("")) {
				dump += indent + FString::Printf(TEXT("%d %d %s "), i, j, *inventoryLocToLetter[i][j]) + TEXT("\n");
			}
		}
	}

	// The hotbar info
	dump += TEXT("\nHotbar info:\n");
	for (int i = 0; i < hotbarInfos.Num(); i++) {
		dump += indent + FString::Printf(TEXT("%d %s %s %s "), i, *hotbarInfos[i].letter, *hotbarInfos[i].name, *hotbarInfos[i].type) + TEXT("\n");
	}

	// Status stuff
	dump += FString::Printf(TEXT("\nLeft text:\n%s%s\n"), *indent, *leftText);
	dump += FString::Printf(TEXT("\nRight text:\n%s%s\n"), *indent, *rightText);
	dump += FString::Printf(TEXT("\nStatus text:\n%s%s\n"), *indent, *statusText);
	dump += FString::Printf(TEXT("\nGold:\n%s%s\n"), *indent, *FString::FromInt(gold));

	// Command queue
	dump += TEXT("\nCommand queue:\n");
	for (int i = 0; i < commandQueue.Num(); i++) {
		dump += indent + FString::Printf(TEXT("%d %s\n"), i, *commandQueue[i]);
	}

	// Passives list
	dump += TEXT("\nPassives:\n");
	for (int i = 0; i < passives.Num(); i++) {
		dump += indent + passives[i] + TEXT("\n");
	}

	// Abilities list
	dump += TEXT("\nAbilities:\n");
	for (auto& Elem : abilityLetterToInfo) {
		dump += indent + Elem.Key + TEXT(": ") + Elem.Value.name + TEXT(" ") + FString::FromInt(Elem.Value.failure) + TEXT("\n");
	}

	// Spells list
	dump += TEXT("\nSpells:\n");
	for (auto& Elem : spellLetterToInfo) {
		dump += indent + Elem.Key + TEXT(": ") + Elem.Value.name + TEXT(" ") + FString::FromInt(Elem.Value.failure) + TEXT("\n");
	}

	// Level ascii
	dump += TEXT("\nLevel ascii:\n");
	for (int i = 0; i < levelAscii.Num(); i++) {
		dump += indent;
		for (int j = 0; j < levelAscii[i].Num(); j++) {
			dump += levelAscii[i][j] + TEXT(" ");
		}
		dump += TEXT("\n");
	}

	// Level info
	dump += TEXT("\nCurrent chars:\n");
	for (int i = 0; i < levelInfo.Num(); i++) {
		dump += indent;
		for (int j = 0; j < levelInfo[i].Num(); j++) {
			dump += levelInfo[i][j].currentChar + TEXT(" ");
		}
		dump += TEXT("\n");
	}
	dump += TEXT("\nFloor chars:\n");
	for (int i = 0; i < levelInfo.Num(); i++) {
		dump += indent;
		for (int j = 0; j < levelInfo[i].Num(); j++) {
			dump += levelInfo[i][j].floorChar + TEXT(" ");
		}
		dump += TEXT("\n");
	}
	dump += TEXT("\nItems:\n");
	for (int i = 0; i < levelInfo.Num(); i++) {
		for (int j = 0; j < levelInfo[i].Num(); j++) {
			for (int k = 0; k < levelInfo[i][j].items.Num(); k++) {
				if (levelInfo[i][j].items[k] != TEXT("")) {
					dump += indent + FString::Printf(TEXT("%d %d %s %s "), i, j, *levelInfo[i][j].items[k], *levelInfo[i][j].itemHotkeys[k]) + TEXT("\n");
				}
			}
		}
	}
	dump += TEXT("\nEffects:\n");
	for (int i = 0; i < levelInfo.Num(); i++) {
		for (int j = 0; j < levelInfo[i].Num(); j++) {
			if (levelInfo[i][j].effect != TEXT("")) {
				dump += indent + FString::Printf(TEXT("%d %d %s "), i, j, *levelInfo[i][j].effect) + TEXT("\n");
			}
		}
	}
	dump += TEXT("\nEnemies:\n");
	for (int i = 0; i < levelInfo.Num(); i++) {
		for (int j = 0; j < levelInfo[i].Num(); j++) {
			if (levelInfo[i][j].enemy != TEXT("")) {
				dump += indent + FString::Printf(TEXT("%d %d %s "), i, j, *levelInfo[i][j].enemy) + TEXT("\n");
			}
		}
	}

	// Current description
	dump += TEXT("\nCurrent description:\n");
	TArray<FString> descriptionLines;
	currentDescription.ParseIntoArray(descriptionLines, TEXT("\n"), true);
	for (int i = 0; i < descriptionLines.Num(); i++) {
		dump += indent + descriptionLines[i] + TEXT("\n");
	}
	dump += TEXT("\nCurrent usage:\n");
	TArray<FString> usageLines;
	currentUsage.ParseIntoArray(usageLines, TEXT("\n"), true);
	for (int i = 0; i < usageLines.Num(); i++) {
		dump += indent + usageLines[i] + TEXT("\n");
	}
	dump += TEXT("\nCurrent type:\n") + indent + currentType + TEXT("\n");

	// Most recent overview text
	dump += TEXT("\nMost recent overview text:\n");
	TArray<FString> overviewLines;
	overviewText.ParseIntoArray(overviewLines, TEXT("\n"), true);
	for (int i = 0; i < overviewLines.Num(); i++) {
		dump += indent + overviewLines[i] + TEXT("\n");
	}

	// Currently selected
	dump += TEXT("\nCurrent UI:\n") + indent + currentUI + TEXT("\n");
	dump += FString::Printf(TEXT("\nCurrently selected:\n%s %d %d %s %d\n"), *indent, selected.x, selected.y, *selected.thingIs, selected.thingIndex);
	dump += FString::Printf(TEXT("\nCurrently dragging:\n%s %d %d %s %d\n"), *indent, thingBeingDragged.x, thingBeingDragged.y, *thingBeingDragged.thingIs, thingBeingDragged.thingIndex);

	// Skills
	dump += TEXT("\nSkills: \n");
	for (auto& Elem : skillNameToInfo) {
		dump += indent + Elem.Key + TEXT(": ") + FString::Printf(TEXT("%f %d %d %d\n"), Elem.Value.level, Elem.Value.focussed, Elem.Value.train, Elem.Value.apt);
	}

	// Log text
	dump += TEXT("\nLog text: \n");
	for (int i = 0; i < logText.Num(); i++) {
		dump += indent + FString::Printf(TEXT("%d %s\n"), i, *logText[i]);
	}

	// Last so many outputs
	dump += TEXT("\nLast outputs:\n\n");
	for (int i = 0; i < lastOutputs.Num(); i++) {
		TArray<FString> outputLines;
		lastOutputs[i].ParseIntoArray(outputLines, TEXT("\n"), true);
		for (int j = 0; j < outputLines.Num(); j++) {
			dump += indent + outputLines[j] + TEXT("\n");
		}
		if (i < lastOutputs.Num() - 1) {
			dump += TEXT("\n");
		}
	}

	// Last so many commands sent
	dump += TEXT("\nLast commands sent:\n");
	dump += indent;
	for (int i = 0; i < lastInputs.Num(); i++) {
		dump += FString::Printf(TEXT("%s"), *lastInputs[i]);
		if (i < lastInputs.Num() - 1) {
			dump += TEXT(", ");
		}
		if (i % 10 == 9 || i == lastInputs.Num() - 1) {
			dump += TEXT("\n") + indent;
		}
	}

	// List of missing things
	dump += TEXT("\nList of missing things:\n") + indent;
	for (auto& Elem : listOfMissingThings) {
		if (Elem.Len() > 0) {
			dump += Elem + TEXT(", ");
		}
	}
	dump += TEXT("\n");

	// Metadata
	FString app = TEXT("DCSSVR");
	FString label = TEXT("bug report");

	// Debugging output
	UE_LOG(LogTemp, Display, TEXT("Bug report message: %s"), *message);
	UE_LOG(LogTemp, Display, TEXT("Bug report dump: %s"), *dump);

	// Build the response
	FString boundary = TEXT("AaB03x");
	FString messageContent = TEXT("");

	// The user's message
	messageContent += TEXT("--") + boundary + TEXT("\r\n");
	messageContent += TEXT("Content-Disposition: form-data; name=\"text\"\r\n");
	messageContent += TEXT("Content-Type: text/plain\r\n");
	messageContent += "\r\n";
	messageContent += message + TEXT("\r\n");

	// Metadata
	messageContent += TEXT("--") + boundary + TEXT("\r\n");
	messageContent += TEXT("Content-Disposition: form-data; name=\"app\"\r\n");
	messageContent += TEXT("Content-Type: text/plain\r\n");
	messageContent += "\r\n";
	messageContent += app + TEXT("\r\n");
	messageContent += TEXT("--") + boundary + TEXT("\r\n");
	messageContent += TEXT("Content-Disposition: form-data; name=\"version\"\r\n");
	messageContent += TEXT("Content-Type: text/plain\r\n");
	messageContent += "\r\n";
	messageContent += version + TEXT("\r\n");
	messageContent += TEXT("--") + boundary + TEXT("\r\n");
	messageContent += TEXT("Content-Disposition: form-data; name=\"label\"\r\n");
	messageContent += TEXT("Content-Type: text/plain\r\n");
	messageContent += "\r\n";
	messageContent += label + TEXT("\r\n");

	// The dump
	messageContent += TEXT("--") + boundary + TEXT("\r\n");
	messageContent += TEXT("Content-Disposition: form-data; name=\"log\"; filename=\"dump.txt\"\r\n");
	messageContent += TEXT("Content-Type: text/plain\r\n");
	messageContent += "\r\n";
	messageContent += dump + TEXT("\r\n");

	// The end of the message
	messageContent += TEXT("--") + boundary + TEXT("--\r\n");

	// Send it (hard-coded URL because it's just a shitty little VPS with nothing)
	UE_LOG(LogTemp, Display, TEXT("Sending request to server: %s"), *messageContent);
	TArray<uint8> messageEncoded = FStringToUint8(messageContent);
	TSharedRef<IHttpRequest> FileUploadRequest = (&FHttpModule::Get())->CreateRequest();
	FileUploadRequest->SetVerb("POST");
	FileUploadRequest->SetHeader("Content-Type", "multipart/form-data; boundary=" + boundary);
	FileUploadRequest->SetContent(messageEncoded);
	FileUploadRequest->SetURL("http://146.59.228.169:9110/api/submit");
	FileUploadRequest->OnProcessRequestComplete().BindLambda(
		[this](
			FHttpRequestPtr pRequest,
			FHttpResponsePtr pResponse,
			bool connectedSuccessfully) mutable {
				if (connectedSuccessfully) {
					UE_LOG(LogTemp, Display, TEXT("Request completed successfully."));
					UE_LOG(LogTemp, Display, TEXT("Response code: %d"), pResponse->GetResponseCode());
					UE_LOG(LogTemp, Display, TEXT("Response content: %s"), *pResponse->GetContentAsString());
				} else {
					UE_LOG(LogTemp, Error, TEXT("Request failed."));
				}
		});
	FileUploadRequest->ProcessRequest();
	httpRequests.Add(FileUploadRequest);
	UE_LOG(LogTemp, Display, TEXT("Request sent"));

}

// Clear all items and enemies in info
void Adcss::clearThings() { 
	UE_LOG(LogTemp, Display, TEXT("Clearing things"));
	for (int i = 0; i < gridWidth; i++) {
		for (int j = 0; j < gridWidth; j++) {
			levelInfo[i][j].items.Empty();
			levelInfo[i][j].itemHotkeys.Empty();
			levelInfo[i][j].enemy = TEXT("");
			levelInfo[i][j].effect = TEXT("");
		}
	}
}

// Save all the data
void Adcss::saveEverything() {
	UE_LOG(LogTemp, Display, TEXT("Saving everything"));

	// Create a save if it doesn't exist
	if (saveGame == nullptr) {
		saveGame = Cast<UDCSSSaveGame>(UGameplayStatics::CreateSaveGameObject(UDCSSSaveGame::StaticClass()));
	}
	if (saveGame == nullptr) {
		UE_LOG(LogTemp, Warning, TEXT("Save game is null"));
		return;
	}
	if (saveFile.IsEmpty()) {
		UE_LOG(LogTemp, Warning, TEXT("Save file is not set"));
		return;
	}

	// Same for the global
	if (saveGameGlobal == nullptr) {
		saveGameGlobal = Cast<UDCSSSaveGame>(UGameplayStatics::CreateSaveGameObject(UDCSSSaveGame::StaticClass()));
	}
	if (saveGameGlobal == nullptr) {
		UE_LOG(LogTemp, Warning, TEXT("Save game global is null"));
		return;
	}

	// The inventory letters
	for (auto& Elem : inventoryLetterToName) {
		saveGame->inventoryLetterToName.Add(Elem.Key, Elem.Value);
	}

	// The inventory locations
	saveGame->inventoryLocToLetter.Empty();
	for (int i = 0; i < inventoryLocToLetter.Num(); i++) {
		for (int j = 0; j < inventoryLocToLetter[i].Num(); j++) {
			saveGame->inventoryLocToLetter.Add(inventoryLocToLetter[i][j]);
		}
	}
	
	// The hotbar info
	saveGame->hotbarInfos.Empty();
	for (int i = 0; i < hotbarInfos.Num(); i++) {
		saveGame->hotbarInfos.Add(hotbarInfos[i].letter);
		saveGame->hotbarInfos.Add(hotbarInfos[i].name);
		saveGame->hotbarInfos.Add(hotbarInfos[i].type);
	}

	// The inventory grab location
	saveGame->inventoryRelLoc = inventoryRelLoc;
	saveGame->inventoryRelRot = inventoryRelRot;

	// The music volume
	UAudioComponent* musicComponent = refToMusicActor->GetAudioComponent();
	if (musicComponent != nullptr) {
		saveGameGlobal->musicVolume = musicComponent->VolumeMultiplier;
	}

	// The music track
	saveGameGlobal->trackInd = trackInd;

	// The snap degrees
	saveGameGlobal->snapDegrees = snapDegrees;

	// The server address
	if (serverAddress.Len() > 0) {
		saveGameGlobal->serverAddress = serverAddress;
	}

	// Actually save the files
	UGameplayStatics::SaveGameToSlot(saveGame, saveFile, 0);
	UGameplayStatics::SaveGameToSlot(saveGameGlobal, TEXT("globalsavefile"), 0);

}

// Load all the data
void Adcss::loadEverything() {
	UE_LOG(LogTemp, Display, TEXT("Loading everything"));

	// Make sure the save file is set
	if (saveGame != nullptr) {

		// Load the inventory letters
		for (auto& Elem : saveGame->inventoryLetterToName) {
			inventoryLetterToName.Add(Elem.Key, Elem.Value);
		}

		// Load the inventory locations
		inventoryLocToLetter.Empty();
		if (saveGame->inventoryLocToLetter.Num() == 6 * 9) {
			for (int i = 0; i < 6; i++) {
				TArray<FString> row;
				for (int j = 0; j < 9; j++) {
					row.Add(saveGame->inventoryLocToLetter[i * 9 + j]);
				}
				inventoryLocToLetter.Add(row);
			}
		} else {
			inventoryLocToLetter.SetNum(6);
			for (int i = 0; i < inventoryLocToLetter.Num(); i++) {
				inventoryLocToLetter[i].SetNum(9);
			}
		}

		// Load the hotbar info
		hotbarInfos.Empty();
		if (saveGame->hotbarInfos.Num() == numHotbarSlots * 3) {
			for (int i = 0; i < numHotbarSlots; i++) {
				HotbarInfo info;
				info.letter = saveGame->hotbarInfos[i * 3];
				info.name = saveGame->hotbarInfos[i * 3 + 1];
				info.type = saveGame->hotbarInfos[i * 3 + 2];
				hotbarInfos.Add(info);
			}
		} else {
			for (int i = 0; i < numHotbarSlots; i++) {
				hotbarInfos.Add(HotbarInfo());
			}
		}

	}

	// Make sure the global file is set (it should always be, but just in case)
	if (saveGameGlobal != nullptr) {

		// Load the music volume
		UAudioComponent* musicComponent = refToMusicActor->GetAudioComponent();
		if (musicComponent != nullptr) {
			float currentVolume = saveGameGlobal->musicVolume;
			currentVolume = FMath::Clamp(currentVolume, 0.0f, 1.0f);
			musicComponent->SetVolumeMultiplier(currentVolume);
			UE_LOG(LogTemp, Display, TEXT("Volume set to %f"), currentVolume);
			UWidgetComponent* WidgetComponent = Cast<UWidgetComponent>(refToSettingsActor->GetComponentByClass(UWidgetComponent::StaticClass()));
			if (WidgetComponent != nullptr) {
				UUserWidget* UserWidget = WidgetComponent->GetUserWidgetObject();
				if (UserWidget != nullptr) {
					UTextBlock* VolumeText = Cast<UTextBlock>(UserWidget->GetWidgetFromName(TEXT("TextMusicVolume")));
					if (VolumeText != nullptr) {
						int asPercent = FMath::RoundToInt(currentVolume*100.0);
						VolumeText->SetText(FText::FromString("Music: " + FString::FromInt(asPercent) + "%"));
					}
				}
			}
		}

		// Load the track index
		trackInd = saveGameGlobal->trackInd;
		UWidgetComponent* WidgetComponent = Cast<UWidgetComponent>(refToSettingsActor->GetComponentByClass(UWidgetComponent::StaticClass()));
		if (WidgetComponent != nullptr) {
			UUserWidget* UserWidget = WidgetComponent->GetUserWidgetObject();
			if (UserWidget != nullptr) {
				UTextBlock* TrackText = Cast<UTextBlock>(UserWidget->GetWidgetFromName(TEXT("TextTrack")));
				if (TrackText != nullptr) {
					TrackText->SetText(FText::FromString("Track: " + musicListNames[trackInd]));
				}
			}
		}
		trackInd = FMath::Clamp(trackInd, 0, musicList.Num() - 1);
		if (musicComponent != nullptr) {
			musicComponent->SetSound(musicList[trackInd]);
		}

		// Load the snap degrees
		snapDegrees = saveGameGlobal->snapDegrees;
		if (refToSettingsActor != nullptr) {
			UWidgetComponent* WidgetComponent2 = Cast<UWidgetComponent>(refToSettingsActor->GetComponentByClass(UWidgetComponent::StaticClass()));
			if (WidgetComponent2 != nullptr) {
				UUserWidget* UserWidget = WidgetComponent2->GetUserWidgetObject();
				if (UserWidget != nullptr) {
					UTextBlock* SnapText = Cast<UTextBlock>(UserWidget->GetWidgetFromName(TEXT("TextSnapAngle")));
					if (SnapText != nullptr) {
						SnapText->SetText(FText::FromString("Snap: " + FString::FromInt(snapDegrees) + " deg"));
					}
				}
			}
		}

		// Load the server address
		if (saveGameGlobal->serverAddress.Len() > 0 && serverAddress.Len() == 0) {
			serverAddressesToTry.Add(saveGameGlobal->serverAddress);
			UE_LOG(LogTemp, Display, TEXT("Server address loaded: %s"), *serverAddress);
		}

	}

	// Output the sizes
	UE_LOG(LogTemp, Display, TEXT("Inventory letter to name size: %d"), inventoryLetterToName.Num());
	UE_LOG(LogTemp, Display, TEXT("Inventory loc to letter size: %d"), inventoryLocToLetter.Num());
	UE_LOG(LogTemp, Display, TEXT("Hotbar infos size: %d"), hotbarInfos.Num());

}

// Go from item name to texture name
FString Adcss::itemNameToTextureName(FString name) {

	// If it's a spell book
	if (name.Contains("Almanac") 
	|| name.Contains("Codex") 
	|| name.Contains("Lessons") 
	|| name.Contains("Annotations") 
	|| name.Contains("Opusculum") 
	|| name.Contains("Information") 
	|| name.Contains("Compilation") 
	|| name.Contains("Catalogue") 
	|| name.Contains("Compendium") 
	|| name.Contains("Grimoire") 
	|| name.Contains("Encyclopedia") 
	|| name.Contains("Papyrus") 
	|| name.Contains("Incunabulum") 
	|| name.Contains("Precepts") 
	|| name.Contains("Mastery of") 
	|| name.Contains("Vademecum") 
	|| name.Contains("Anthology") 
	|| name.Contains("Excursus") 
	|| name.Contains("Cyclopedia") 
	|| name.Contains("Meditations") 
	|| name.Contains("Prolegomenon") 
	|| name.Contains("Commentary") 
	|| name.Contains("Elucidation") 
	|| name.Contains("Quarto") 
	|| name.Contains("Memoranda") 
	|| name.Contains("Vol.") 
	|| name.Contains("Analects") 
	|| name.Contains("Handbook") 
	|| name.Contains("Spellbook") 
	|| name.Contains("Necronomicon") 
	|| name.Contains("Folio") 
	|| name.Contains("Treatise") 
	|| name.Contains("Volume") 
	|| (name.Contains("Wrath") && name.Contains("Trog")) 
	|| (name.Contains("Sojourn") && name.Contains("Swampland")) 
	|| (name.Contains("Collected") && name.Contains("Works")) 
	|| name.Contains("Book")) {
		return "Book";
	}

	// Determine the actual name of the item
	FString itemName = name;
	itemName = itemName.Replace(TEXT("the "), TEXT(""));
	int32 labelledIndex = itemName.Find(TEXT("labelled"));
	if (labelledIndex != INDEX_NONE) {
		itemName = itemName.Left(labelledIndex);
	}
	itemName = itemName.Replace(TEXT("+0 "), TEXT(""));
	itemName = itemName.Replace(TEXT("+1 "), TEXT(""));
	itemName = itemName.Replace(TEXT("+2 "), TEXT(""));
	itemName = itemName.Replace(TEXT("+3 "), TEXT(""));
	itemName = itemName.Replace(TEXT("+4 "), TEXT(""));
	itemName = itemName.Replace(TEXT("+5 "), TEXT(""));
	itemName = itemName.Replace(TEXT("+6 "), TEXT(""));
	itemName = itemName.Replace(TEXT("+7 "), TEXT(""));
	itemName = itemName.Replace(TEXT("+8 "), TEXT(""));
	itemName = itemName.Replace(TEXT("+9 "), TEXT(""));
	int32 commaIndex = itemName.Find(TEXT(","));
	if (commaIndex != INDEX_NONE) {
		itemName = itemName.Left(commaIndex);
	}
	itemName = itemName.Replace(TEXT("-"), TEXT(" "));
	itemName = itemName.Replace(TEXT("glowing"), TEXT(""));
	itemName = itemName.Replace(TEXT("runed"), TEXT(""));
	itemName = itemName.Replace(TEXT("heavily"), TEXT(""));
	itemName = itemName.Replace(TEXT("masterwork"), TEXT(""));
	itemName = itemName.Replace(TEXT("embroidered"), TEXT(""));
	itemName = itemName.Replace(TEXT("vampiric"), TEXT(""));
	itemName = itemName.Replace(TEXT("heavy"), TEXT(""));
	itemName = itemName.Replace(TEXT("Antimagic"), TEXT(""));
	itemName = itemName.Replace(TEXT("polished"), TEXT(""));
	itemName = itemName.Replace(TEXT("shiny"), TEXT(""));
	itemName = itemName.Replace(TEXT("pair of "), TEXT(""));
	itemName = itemName.Replace(TEXT("not visited"), TEXT(""));
	itemName = itemName.Replace(TEXT("not visit"), TEXT(""));
	itemName = itemName.Replace(TEXT("smoky"), TEXT(""));
	itemName = itemName.Replace(TEXT("cursed"), TEXT(""));
	itemName = itemName.Replace(TEXT("small"), TEXT(""));

	// Remove any text in brackets using Regex
	FRegexPattern pattern(TEXT("\\(.*?\\)"));
	FRegexMatcher matcher(pattern, itemName);
	while (matcher.FindNext()) {
		itemName = itemName.Replace(*matcher.GetCaptureGroup(0), TEXT(""));
		matcher = FRegexMatcher(pattern, itemName);
	}

	// Deal with brands e.g. dagger of venom
	int32 ofIndex = itemName.Find(TEXT(" of "));
	if (ofIndex != INDEX_NONE && !itemName.Contains(TEXT("statue"))) {
		itemName = itemName.Left(ofIndex);
	}

	// If we have a name i.e. the +1 hat "Nutis" {+Inv rF+++ Dex-5 Stlth-}
	int32 quoteIndex = itemName.Find(TEXT("\""));
	if (quoteIndex != INDEX_NONE) {
		itemName = itemName.Left(quoteIndex);
	}

	// If we have something in braces i.e. the +1 hat "Nutis" {+Inv rF+++ Dex-5 Stlth-}
	int32 braceIndex = itemName.Find(TEXT("{"));
	if (braceIndex != INDEX_NONE) {
		itemName = itemName.Left(braceIndex);
	}

	// If it's a fountain
	if (itemName.Contains(TEXT("fountain"))) {
		return "Fountain";
	}

	// If it's a shop
	if (itemName.Contains(TEXT(" shop")) 
		|| itemName.Contains(TEXT(" store")) 
		|| itemName.Contains(TEXT(" boutique")) 
		|| itemName.Contains(TEXT(" antiques")) 
		|| itemName.Contains(TEXT(" emporium")) 
		|| itemName.Contains(TEXT(" bazaar"))) {
		return "Shop";
	}

	// Capitalize the item name
	TArray<FString> words;
	itemName.ParseIntoArray(words, TEXT(" "), true);
	for (int l = 0; l < words.Num(); l++) {
		words[l] = words[l].Mid(0, 1).ToUpper() + words[l].Mid(1);
	}
	itemName = TEXT("");
	for (int l = 0; l < words.Num(); l++) {
		if (!words[l].IsNumeric() && words[l] != TEXT("a") && words[l] != TEXT("an") && !words[l].Contains(TEXT("'s"))) {
			itemName += words[l];
		}
	}

	// If it's gold
	if (itemName.Contains(TEXT("gold")) && !itemName.Contains(TEXT("golden"))) {
		return "Gold";
	}

	// If it's a corpse
	if (itemName.Contains(TEXT("corpse")) || itemName.Contains(TEXT("skeleton"))) {
		itemName = itemName.Replace(TEXT("corpse"), TEXT(""));
		itemName = itemName.Replace(TEXT("skeleton"), TEXT(""));
		itemName = "Monster" + itemName;
	}

	// If it ends with s
	if (!textures.Contains(itemName) && itemName.EndsWith(TEXT("s"))) {
		itemName = itemName.Left(itemName.Len() - 1);
	}

	// If it's a statue
	if (!textures.Contains(itemName) && itemName.Contains(TEXT("statue"))) {
		getTexture(itemName);
		return "Statue";
	}

	// If it's a ring (need to be careful with things like "shimmering")
	int32 ringIndex = itemName.Find(TEXT("ring"));
	if (ringIndex != INDEX_NONE && !textures.Contains(itemName)) {
		if (ringIndex == 0 || itemName[ringIndex] == 'R') {
			getTexture(itemName);
			return "Ring";
		}
	}

	// If it's a talisman
	if (itemName.Contains(TEXT("talisman")) && !textures.Contains(itemName)) {
		getTexture(itemName);
		return "Talisman";
	}

	// If it's a potion
	if (itemName.Contains(TEXT("potion")) && !textures.Contains(itemName)) {
		getTexture(itemName);
		return "Potion";
	}

	// If it's an arch
	if (itemName.Contains(TEXT("arch")) && !textures.Contains(itemName)) {
		getTexture(itemName);
		return "Arch";
	}

	// If it's an idol
	if (itemName.Contains(TEXT("idol")) && !textures.Contains(itemName)) {
		getTexture(itemName);
		return "Idol";
	}

	// If it's a portal/gateway
	if (gateList.Contains(name) && !textures.Contains(itemName)) {
		getTexture(itemName);
		return "Portal";
	}

	// Return the texture name
	return itemName;

}

// Go from enemy name to texture name
FString Adcss::enemyNameToTextureName(FString name) {

	// If it contains the player's name
	if (name.Contains(fullName)  || name.Contains("the ancestor")) {
		return "Player";
	}

	// Remove various things
	FString enemyName = name;
	enemyName = enemyName.Replace(TEXT("(here)"), TEXT(""));
	int32 openBracketIndex = enemyName.Find(TEXT("("));
	if (openBracketIndex != INDEX_NONE) {
		enemyName = enemyName.Left(openBracketIndex);
	}
	int32 commaIndex = enemyName.Find(TEXT(","));
	if (commaIndex != INDEX_NONE) {
		enemyName = enemyName.Left(commaIndex);
	}

	// Capitalize the enemy name
	TArray<FString> words;
	enemyName.ParseIntoArray(words, TEXT(" "), true);
	for (int k = 0; k < words.Num(); k++) {
		words[k] = words[k].Mid(0, 1).ToUpper() + words[k].Mid(1);
	}
	enemyName = TEXT("");
	for (int k = 0; k < words.Num(); k++) {
		if (!words[k].IsNumeric() && words[k] != TEXT("a") && words[k] != TEXT("an")) {
			enemyName += words[k];
		}
	}

	// Simulacrum/zombie/skeleton are just copies of other monsters
	enemyName = enemyName.Replace(TEXT("Spectral"), TEXT(""));
	if (enemyName.EndsWith(TEXT("Simulacrum"))) {
		enemyName = enemyName.Replace(TEXT("Simulacrum"), TEXT(""));
	}
	if (enemyName.EndsWith(TEXT("Zombie"))) {
		enemyName = enemyName.Replace(TEXT("Zombie"), TEXT(""));
	}
	if (enemyName.EndsWith(TEXT("Skeleton"))) {
		enemyName = enemyName.Replace(TEXT("Skeleton"), TEXT(""));
	}

	// Convert to material name
	FString materialName = "Monster" + enemyName;

	// Check for plurals
	if (!textures.Contains(materialName) && materialName.EndsWith(TEXT("s"))) {
		materialName = materialName.Left(materialName.Len() - 1);
	}

	// In case it's a monster version of a weapon
	FString itemName = itemNameToTextureName(name);
	if (!textures.Contains(materialName) && textures.Contains(itemName)) {
		materialName = itemName;
	}

	return materialName;

}

// Called when the actor is destroyed or deleted
void Adcss::EndPlay(const EEndPlayReason::Type EndPlayReason) {
	UE_LOG(LogTemp, Display, TEXT("Closing process"));
	if (!useServer) {
		writeCommandQueued("escape");
		writeCommandQueued("escape");
		writeCommandQueued("enter");
		writeCommandQueued("enter");
		writeCommandQueued("exit");
		writeCommandQueued("escape");
		FPlatformProcess::Sleep(2.0f);
		FPlatformProcess::TerminateProc(ProcHandle, true);
		FPlatformProcess::ClosePipe(StdInReadHandle, StdInWriteHandle);
		FPlatformProcess::ClosePipe(StdOutReadHandle, StdOutWriteHandle);
	} else {
		for (FTimerHandle& TimerHandle : timerHandles) {
			worldRef->GetTimerManager().ClearTimer(TimerHandle);
		}
	}
	for (TSharedRef<IHttpRequest>& Request : httpRequests) {
		Request->CancelRequest();
	}
}

// Depending on whether the shift key is pressed, shift the letters
void Adcss::shiftLetters() {
	UE_LOG(LogTemp, Display, TEXT("Shifting letters, isShifted: %d"), isShifted);

	// Check the reference to the keyboard
	if (refToKeyboardActor != nullptr) {
		UWidgetComponent* WidgetComponent = Cast<UWidgetComponent>(refToKeyboardActor->GetComponentByClass(UWidgetComponent::StaticClass()));
		if (WidgetComponent != nullptr) {
			UUserWidget* UserWidget = WidgetComponent->GetUserWidgetObject();
			if (UserWidget != nullptr) {

				// For each letter key (e.g. ButtonKeyboardQ)
				for (int i = 0; i < 26; i++) {
					FString letter = FString::Chr(i + 65);
					FString textName = "TextKeyboard" + letter;
					UTextBlock* TextBlock = Cast<UTextBlock>(UserWidget->GetWidgetFromName(*textName));
					if (TextBlock != nullptr) {
						FString text = TextBlock->GetText().ToString();
						if (isShifted) {
							text = text.ToUpper();
						} else {
							text = text.ToLower();
						}
						TextBlock->SetText(FText::FromString(text));
					}
				}
				
			}
		}
	}
	
}

// Wrapped function
void Adcss::init(bool firstTime) {

	// Make sure all the templates are not null
	if (floorTemplate == nullptr || wallTemplate == nullptr || enemyTemplate == nullptr || itemTemplate == nullptr) {
		UE_LOG(LogTemp, Error, TEXT("Templates not set"));
		return;
	}

	// Get the platform name
	FString platformName = UGameplayStatics::GetPlatformName();
	UE_LOG(LogTemp, Display, TEXT("Platform name: %s"), *platformName);

	// Get the world
	worldRef = GetWorld();

	// Whether to use the server or not
	if (firstTime) {
		serverConnected = false;
		needNewRequest = true;
		serverAddress = "";
		nextCommand = 0;
		serverAddressesToTry.Empty();
		currentRequests = 0;
	}

	// Set params
	FString binaryPath = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir() + TEXT("\\Content\\DCSS\\"));
	exePath = binaryPath + TEXT("crawl.exe");
	args = TEXT(" -extra-opt-first monster_item_view_coordinates=true ");
	args += TEXT(" -extra-opt-first bad_item_prompt=false ");
	args += TEXT(" -extra-opt-first monster_item_view_features+=cloud ");
	args += TEXT(" -extra-opt-first monster_item_view_features+=(here) ");
	args += TEXT(" -extra-opt-first monster_item_view_features+=trap ");
	args += TEXT(" -extra-opt-first monster_item_view_features+=arch ");
	args += TEXT(" -extra-opt-first monster_item_view_features+=idol ");
	args += TEXT(" -extra-opt-first monster_item_view_features+=statue ");
	args += TEXT(" -extra-opt-first monster_item_view_features+=fountain ");
	args += TEXT(" -extra-opt-first monster_item_view_features+=translucent ");
	args += TEXT(" -extra-opt-first monster_item_view_features+=abandoned ");
	args += TEXT(" -extra-opt-first monster_item_view_features+=door ");
	args += TEXT(" -extra-opt-first monster_item_view_features+=gate ");
	args += TEXT(" -extra-opt-first prompt_menu=true ");
	args += TEXT(" -extra-opt-first reduce_animations=true ");
	args += TEXT(" -extra-opt-first force_spell_targeter=all ");
	args += TEXT(" -extra-opt-first force_ability_targeter=all ");
	args += TEXT(" -extra-opt-first default_autopickup=false ");
	args += TEXT(" -extra-opt-first fail_severity_to_confirm=0 ");
	args += TEXT(" -extra-opt-first wiz_mode=yes ");
	args += TEXT(" -extra-opt-first char_set=ascii ");
	args += TEXT(" -extra-opt-first crawl_dir=\"") + binaryPath + TEXT("\" ");
	UE_LOG(LogTemp, Display, TEXT("Executable loc: %s"), *exePath);
	UE_LOG(LogTemp, Display, TEXT("Args: %s"), *args);
	StdOutReadHandle = nullptr;
	StdOutWriteHandle = nullptr;
	StdInReadHandle = nullptr;
	StdInWriteHandle = nullptr;
	hasLoaded = false;
	LOS = 8;
	maxLogShown = 6;
	enemyUseCount = 0;
	effectUseCount = 0;
	itemUseCount = 0;
	gridWidth = 2 * LOS + 1;
	maxEnemies = 100;
	fullName = TEXT("");
	maxEffects = 100;
	maxItems = 100;
	skipNextFullDescription = false;
	wallScaling = 4.0f;
	crawlHasStarted = !firstTime;
	hasBeenWelcomed = false;
	isMenu = true;
	inventoryRelLoc = FVector(0.0f, 150.0f, 200.0f);
	inventoryRelRot = FRotator(0.0f, -90.0f, 0.0f);
	diagWallScaling = sqrt(2 * wallScaling * wallScaling);
	halfDiagWallScaling = sqrt(0.5 * wallScaling * wallScaling);
	inMainMenu = true;
	thingsThatCountAsWalls = "#+' ";
	thingsThatCountAsDoors = "+'";
	thingsThatCountAsItems = "()|%[?O:/}!=\"";
	currentType = TEXT("Monsters");
	shiftOn = false;
	numProcessed = 0;
	listOfMissingThings.Empty();
	locForBlink = FIntVector2(-1, -1);
	prevProcessed = 0;
	isChecking = false;
	epsilon = 10;
	currentBranch = "Dungeon";
	justUsedAScroll = false;
	saveNames.Empty();
	draggingInventory = false;
	smallWallScaling = wallScaling / 2.0f;
	prevOutput = TEXT("");
	altarOverview = TEXT("");
	altarPowers = TEXT("");
	altarWrath = TEXT("");
	showingAltar = false;
	showNextAltar = true;
	showNextShop = true;
	isKeyboardOpen = false;
	isShifted = false;
	editting = TEXT("");
	defaultNameText = TEXT("(random name)");
	defaultSeedText = TEXT("(random seed)");
	defaultBugText = TEXT("(click to type)");
	currentBug = defaultBugText;
 	lastBugSubmitted = -100000;
	shopItems.Empty();
	shopTitle = TEXT("");
	showingShop = false;
	nextAltar = 0;
	selected = SelectedThing();
	numHotbarSlots = 5;
	thingBeingDragged = selected;
	buttonConfirming = -1;
	gold = -1;
	spellLevels = 0;
	currentDescription = TEXT("");
	currentUsage = TEXT("");
	memorizing = false;
	rmbOn = false;
	logText.Empty();
	spellPage = 0;
	abilityPage = 0;
	passivePage = 0;
	savesPage = 0;
	inventoryGrabDistance = 0.0f;
	currentName = defaultNameText;
	currentSeed = defaultSeedText;
	currentSpecies = "Minotaur";
	currentBackground = "Fighter";
	inventoryGrabPoint = FVector(0.0f, 0.0f, 0.0f);
	inventoryLetterToName = TMap<FString, FString, FDefaultSetAllocator, FCaseSensitiveLookupKeyFuncs<FString>>();
	inventoryLocToLetter = TArray<TArray<FString>>();
	inventoryLetterToNamePrev = TMap<FString, FString, FDefaultSetAllocator, FCaseSensitiveLookupKeyFuncs<FString>>();
	inventoryLocToLetter.SetNum(6);
	for (int i = 0; i < inventoryLocToLetter.Num(); i++) {
		inventoryLocToLetter[i].SetNum(9);
	}
	targetingRange = -1;
	inventoryNextSpot = FIntVector2(-1, -1);
	shouldRedrawInventory = true;
	currentUI = "inventory";
	if (firstTime) {
		commandQueue.Empty();
	}
	lastCommandTime = 0.0;
	inventoryOpen = true;
	choiceNames.Empty();
	choiceLetters.Empty();
	isChoiceOpen = false;
	choiceType = "default";
	refToChoiceActor->SetActorEnableCollision(isChoiceOpen);
	spellLetters.Empty();
	abilityLetters.Empty();
	settingsOpen = false;
	spellLetterToInfo.Empty();
	abilityLetterToInfo.Empty();
	passives.Empty();
	shouldRedrawAbilities = true;
	shouldRedrawOverview = true;
	shouldRedrawSpells = true;
	shouldRedrawReligion = true;
	shouldRedrawSkills = true;
	shouldRedrawHotbar = true;
	religionText = TEXT("You are not religious.");
	overviewText = TEXT("");
	skillNameToInfo.Empty();
	defaultSkillDescription = "Hover over a skill to see a description, interact to change priority.\nThe training percent is auto-adjusted based on the skills you use most.";
	menuOutput = ""
"===START==="
"                                                                                \n"
"                                     Fiqeka Puinn the Sneak                     \n"
"                                     Vampire                                    \n"
"                                     Health: 13/13     ======================== \n"
"                                     Magic:  3/3       ======================== \n"
"                                     AC:  2            Str: 7                   \n"
"             ###+###                 EV: 11            Int: 17                  \n"
"             #.....#                 SH:  0            Dex: 14                  \n"
"             #.....#                 XL:  1 Next:  0%  Place: Dungeon:1         \n"
"             #..@..#                 Noise: ---------  Time: 0.0 (0.0)          \n"
"             #.....#                 -) Nothing wielded                         \n"
"             #.....#                 Cast: Nothing quivered                     \n"
"             #######                 ===MENU===                                 \n"
"                                                                                \n"
"                                                                                \n"
"                                                                                \n"
"                                                                                \n"
"                                                                                \n"
"                                                                                \n"
"===END==="
"===READY===";

	// The list of things considered gates
	gateList.Empty();
	gateList.Add(TEXT("a phantasmal passage"));
	gateList.Add(TEXT("a sand-covered staircase"));
	gateList.Add(TEXT("a glowing drain"));
	gateList.Add(TEXT("a flagged portal"));
	gateList.Add(TEXT("a gauntlet entrance"));
	gateList.Add(TEXT("a frozen archway"));
	gateList.Add(TEXT("a dark tunnel"));
	gateList.Add(TEXT("a ruined gateway"));
	gateList.Add(TEXT("a magic portal"));
	gateList.Add(TEXT("a one-way gateway to a ziggurat"));
	gateList.Add(TEXT("a gateway to a ziggurat"));
	gateList.Add(TEXT("a gateway to a bazaar"));
	gateList.Add(TEXT("a portal to a secret trove of treasure"));

	// Set the skill descriptions
	skillDescriptions.Empty();
	skillDescriptions.Add(TEXT("Fighting"), TEXT("Fighting skill increases your accuracy and damage in physical combat, and also increases your maximum health."));
	skillDescriptions.Add(TEXT("Maces"), TEXT("Being skilled with Maces & Flails will allow you to attack more quickly with all weapons of this type, as well as increasing your accuracy and damage with them.\nCross-trains with Axes and Staves."));
	skillDescriptions.Add(TEXT("Axes"), TEXT("Being skilled with Axes will allow you to attack more quickly with all weapons of this type, as well as increasing your accuracy and damage with them.\nCross-trains with Maces / Flails and Polearms."));
	skillDescriptions.Add(TEXT("Polearms"), TEXT("Being skilled with Polearms will allow you to attack more quickly with all weapons of this type, as well as increasing your accuracy and damage with them.\nCross-trains with Axes and Staves."));
	skillDescriptions.Add(TEXT("Staves"), TEXT("Being skilled with Staves will allow you to attack more quickly with all weapons of this type, as well as increasing your accuracy and damage with them.\nCross-trains witg Maces / Flails and Polearms."));
	skillDescriptions.Add(TEXT("Unarmed"), TEXT("Being skilled in Unarmed Combat increases your accuracy, damage and attack speed in melee combat when barehanded."));
	skillDescriptions.Add(TEXT("Throwing"), TEXT("Training Throwing will make thrown weapons (such as javelins) more accurate and damaging. It also makes certain magical darts more likely to have an effect."));
	skillDescriptions.Add(TEXT("Short"), TEXT("Being skilled with Short Blades will allow you to attack more quickly with all weapons of this type, as well as increasing your accuracy and damage with them.\nCross-trains with Long Blades."));
	skillDescriptions.Add(TEXT("Long"), TEXT("Being skilled with Long Blades will allow you to attack more quickly with all weapons of this type, as well as increasing your accuracy and damage with them.\nCross-trains with Short Blades."));
	skillDescriptions.Add(TEXT("Ranged"), TEXT("Being skilled with Ranged Weapons will let you attack more quickly with all bows, crossbows and slings, as well as increasing your accuracy and damage with them."));
	skillDescriptions.Add(TEXT("Armour"), TEXT("Armour skill multiplies the AC gained from wearing armour of all kinds, both from body armour and cloaks, gloves, etc. It also slightly lessens penalties to spellcasting and ranged weapon speed from heavy armour."));
	skillDescriptions.Add(TEXT("Dodging"), TEXT("The Dodging skill will affect your chance of dodging an attack, be it melee, ranged or magical. Certain types of attack cannot be dodged, however, such as enchantments (which must be resisted with willpower) and explosions."));
	skillDescriptions.Add(TEXT("Shields"), TEXT("A high Shields skill helps you block melee attacks or projectiles (both magical and non-magical) with your equipped shield, and reduces their penalty to your evasion, attack speed and spellcasting success."));
	skillDescriptions.Add(TEXT("Stealth"), TEXT("By training Stealth, you can make it less likely that monsters will notice you if they are asleep or otherwise unalert, and increase your chances of monsters losing track of you after you leave their vision."));
	skillDescriptions.Add(TEXT("Spellcasting"), TEXT("Being skilled in Spellcasting slightly increases the success rate and spell power of all spells, along with increasing your magical reserves. Training Spellcasting also gives you more spell slots to spend on new spells."));
	skillDescriptions.Add(TEXT("Conjurations"), TEXT("Being skilled in Conjurations increases the success rate and spell power\nof spells in this school. Spells with multiple schools depend on the average\nskill in these schools."));
	skillDescriptions.Add(TEXT("Hexes"), TEXT("Being skilled in Hexes increases the success rate and spell power\nof spells in this school. Spells with multiple schools depend on the average\nskill in these schools."));
	skillDescriptions.Add(TEXT("Summonings"), TEXT("Being skilled in Summoning increases the success rate and spell power\nof spells in this school. Spells with multiple schools depend on the average\nskill in these schools."));
	skillDescriptions.Add(TEXT("Necromancy"), TEXT("Being skilled in Necromancy increases the success rate and spell power\nof spells in this school. Spells with multiple schools depend on the average\nskill in these schools."));
	skillDescriptions.Add(TEXT("Translocations"), TEXT("Being skilled in Translocations increases the success rate and spell power\nof spells in this school. Spells with multiple schools depend on the average\nskill in these schools."));
	skillDescriptions.Add(TEXT("Alchemy"), TEXT("Being skilled in Alchemy increases the success rate and spell power\nof spells in this school. Spells with multiple schools depend on the average\nskill in these schools."));
	skillDescriptions.Add(TEXT("Fire"), TEXT("Being skilled in Fire Magic increases the success rate and spell power\nof spells in this school. Spells with multiple schools depend on the average\nskill in these schools."));
	skillDescriptions.Add(TEXT("Ice"), TEXT("Being skilled in Ice Magic increases the success rate and spell power\nof spells in this school. Spells with multiple schools depend on the average\nskill in these schools."));
	skillDescriptions.Add(TEXT("Air"), TEXT("Being skilled in Air Magic increases the success rate and spell power\nof spells in this school. Spells with multiple schools depend on the average\nskill in these schools."));
	skillDescriptions.Add(TEXT("Earth"), TEXT("Being skilled in Earth Magic increases the success rate and spell power\nof spells in this school. Spells with multiple schools depend on the average\nskill in these schools."));
	skillDescriptions.Add(TEXT("Invocations"), TEXT("Your Invocations skill affects the likelihood that an attempt to use divine abilities with certain gods will be successful, and often the power of such abilities."));
	skillDescriptions.Add(TEXT("Evocations"), TEXT("Evocations skill affects your ability to use magical items like wands and other uncommon objects, increasing the power or success rate of such items."));
	skillDescriptions.Add(TEXT("Shapeshifting"), TEXT("Being skilled in Shapeshifting allows one to use more powerful forms without gravely damaging one's health, as well as making most forms somewhat stronger, in ways which vary between forms."));
	
	// Skill description default based on the current text value
	UWidgetComponent* WidgetComponentSkill = Cast<UWidgetComponent>(refToInventoryActor->GetComponentByClass(UWidgetComponent::StaticClass()));
	if (WidgetComponentSkill != nullptr) {
		UUserWidget* UserWidget = WidgetComponentSkill->GetUserWidgetObject();
		if (UserWidget != nullptr) {
			UTextBlock* Description = Cast<UTextBlock>(UserWidget->GetWidgetFromName(TEXT("TextSkillDescription")));
			if (Description != nullptr) {
				defaultSkillDescription = Description->GetText().ToString();
			}
		}
	}

	// Set the background descriptions
	backgroundDescriptions.Empty();
	backgroundDescriptions.Add(TEXT("Fighter"), TEXT("Fighters are equipped with armour and shield, as well as a weapon of their choice and a potion of might."));
	backgroundDescriptions.Add(TEXT("Gladiator"), TEXT("Gladiators are ready for the arena with light armour, a weapon of their choice, and a few throwing weapons and nets."));
	backgroundDescriptions.Add(TEXT("Monk"), TEXT("Monks start with a simple weapon of their choice, an orb of light, and a potion of divine ambrosia, and gain additional piety with the first god they worship."));
	backgroundDescriptions.Add(TEXT("Hunter"), TEXT("Hunters carry a shortbow, light armour and a scroll of butterflies."));
	backgroundDescriptions.Add(TEXT("Brigand"), TEXT("Brigands carry poisoned darts and a small number of nasty curare darts."));
	backgroundDescriptions.Add(TEXT("Berserker"), TEXT("Berserkers believe in Trog, the magic-hating god of frenzy. Not much can stop a raging berserker early on, apart from hubris."));
	backgroundDescriptions.Add(TEXT("CinderAcolyte"), TEXT("Cinder Acolytes worship Ignis, drawing upon the flame of a dying god until there is nothing left."));
	backgroundDescriptions.Add(TEXT("ChaosKnight "), TEXT("Chaos Knights are playthings of the unpredictable Xom, and are subject to the god's constantly changing moods."));
	backgroundDescriptions.Add(TEXT("Artificer"), TEXT("Artificers start with a few magic wands and a club. They'll need to find better weapons and armour before their wands run out."));
	backgroundDescriptions.Add(TEXT("Shapeshifter"), TEXT("Shapeshifters use talismans to shift their form, making them powerful melee fighters (especially when unarmed) at the cost of armour."));
	backgroundDescriptions.Add(TEXT("Wanderer"), TEXT("Wanderers start with random equipment and skills."));
	backgroundDescriptions.Add(TEXT("Delver"), TEXT("Delvers start their quest five levels below the surface. A set of powerful magic items helps them survive the dangerous journey upward."));
	backgroundDescriptions.Add(TEXT("Warper"), TEXT("Warpers start with a weapon of their choice, a scroll of blinking, darts of dispersal, and some Translocation spells in their library."));
	backgroundDescriptions.Add(TEXT("Hexslinger"), TEXT("Hexslingers carry a sling, and use hex magic to make their foes more vulnerable to attacks from afar."));
	backgroundDescriptions.Add(TEXT("Enchanter"), TEXT("Enchanters know a variety of Hexes that can incapacitate their foes, and are equipped with an enchanted dagger and potions of invisibility."));
	backgroundDescriptions.Add(TEXT("Reaver"), TEXT("Reavers start with a weapon of their choice and deadly magic to supplement it when in need. They start with the Kiss of Death spell."));
	backgroundDescriptions.Add(TEXT("HedgeWizard"), TEXT("Hedge Wizards learn many minor magics, letting them backstab or escape their bewildered foes."));
	backgroundDescriptions.Add(TEXT("Conjurer"), TEXT("Conjurers confront problems with damaging spells."));
	backgroundDescriptions.Add(TEXT("Summoner"), TEXT("Summoners are able to cast Summon Small Mammal from the start, and begin with other Summonings spells in their library."));
	backgroundDescriptions.Add(TEXT("Necromancer"), TEXT("Necromancers are wizards specialising in the practice of death magic. They start out with the Soul Splinter spell."));
	backgroundDescriptions.Add(TEXT("FireElementalist"), TEXT("Fire Elementalists have learned the Foxfire spell. Their other starting spells are likewise focused on fiery destruction."));
	backgroundDescriptions.Add(TEXT("IceElementalist"), TEXT("Ice Elementalists begin with the Freeze spell. The spells in their starting library are quite versatile."));
	backgroundDescriptions.Add(TEXT("AirElementalist"), TEXT("Air Elementalists have learned the Shock spell. Later on, their selection of Air Magic spells provides them with the flexibility to survive."));
	backgroundDescriptions.Add(TEXT("EarthElementalist"), TEXT("Earth Elementalists know the Sandblast spell, and can later learn more earthen spells suitable for subtle stabbing or brute-force blasting."));
	backgroundDescriptions.Add(TEXT("Alchemist"), TEXT("Alchemists start with the Sting spell, and their magic is very effective against foes susceptible to poison."));

	// Background name to letter
	backgroundToLetter.Empty();
	backgroundToLetter.Add(TEXT("Fighter"), TEXT("a"));
	backgroundToLetter.Add(TEXT("Gladiator"), TEXT("b"));
	backgroundToLetter.Add(TEXT("Monk"), TEXT("c"));
	backgroundToLetter.Add(TEXT("Hunter"), TEXT("d"));
	backgroundToLetter.Add(TEXT("Brigand"), TEXT("e"));
	backgroundToLetter.Add(TEXT("Berserker"), TEXT("f"));
	backgroundToLetter.Add(TEXT("CinderAcolyte"), TEXT("g"));
	backgroundToLetter.Add(TEXT("ChaosKnight "), TEXT("h"));
	backgroundToLetter.Add(TEXT("Artificer"), TEXT("i"));
	backgroundToLetter.Add(TEXT("Shapeshifter"), TEXT("j"));
	backgroundToLetter.Add(TEXT("Wanderer"), TEXT("k"));
	backgroundToLetter.Add(TEXT("Delver"), TEXT("l"));
	backgroundToLetter.Add(TEXT("Warper"), TEXT("m"));
	backgroundToLetter.Add(TEXT("Hexslinger"), TEXT("n"));
	backgroundToLetter.Add(TEXT("Enchanter"), TEXT("o"));
	backgroundToLetter.Add(TEXT("Reaver"), TEXT("p"));
	backgroundToLetter.Add(TEXT("HedgeWizard"), TEXT("q"));
	backgroundToLetter.Add(TEXT("Conjurer"), TEXT("r"));
	backgroundToLetter.Add(TEXT("Summoner"), TEXT("s"));
	backgroundToLetter.Add(TEXT("Necromancer"), TEXT("t"));
	backgroundToLetter.Add(TEXT("FireElementalist"), TEXT("u"));
	backgroundToLetter.Add(TEXT("IceElementalist"), TEXT("v"));
	backgroundToLetter.Add(TEXT("AirElementalist"), TEXT("w"));
	backgroundToLetter.Add(TEXT("EarthElementalist"), TEXT("x"));
	backgroundToLetter.Add(TEXT("Alchemist"), TEXT("y"));
	backgroundToLetterNoGods.Empty();
	backgroundToLetterNoGods.Add(TEXT("Fighter"), TEXT("a"));
	backgroundToLetterNoGods.Add(TEXT("Gladiator"), TEXT("b"));
	backgroundToLetterNoGods.Add(TEXT("Hunter"), TEXT("c"));
	backgroundToLetterNoGods.Add(TEXT("Brigand"), TEXT("d"));
	backgroundToLetterNoGods.Add(TEXT("Artificer"), TEXT("e"));
	backgroundToLetterNoGods.Add(TEXT("Shapeshifter"), TEXT("f"));
	backgroundToLetterNoGods.Add(TEXT("Wanderer"), TEXT("g"));
	backgroundToLetterNoGods.Add(TEXT("Delver"), TEXT("h"));
	backgroundToLetterNoGods.Add(TEXT("Warper"), TEXT("i"));
	backgroundToLetterNoGods.Add(TEXT("Hexslinger"), TEXT("j"));
	backgroundToLetterNoGods.Add(TEXT("Enchanter"), TEXT("k"));
	backgroundToLetterNoGods.Add(TEXT("Reaver"), TEXT("l"));
	backgroundToLetterNoGods.Add(TEXT("HedgeWizard"), TEXT("m"));
	backgroundToLetterNoGods.Add(TEXT("Conjurer"), TEXT("n"));
	backgroundToLetterNoGods.Add(TEXT("Summoner"), TEXT("o"));
	backgroundToLetterNoGods.Add(TEXT("Necromancer"), TEXT("p"));
	backgroundToLetterNoGods.Add(TEXT("FireElementalist"), TEXT("q"));
	backgroundToLetterNoGods.Add(TEXT("IceElementalist"), TEXT("r"));
	backgroundToLetterNoGods.Add(TEXT("AirElementalist"), TEXT("s"));
	backgroundToLetterNoGods.Add(TEXT("EarthElementalist"), TEXT("t"));
	backgroundToLetterNoGods.Add(TEXT("Alchemist"), TEXT("u"));

	// Species description default based on the current text value
	UWidgetComponent* WidgetComponentSpecies = Cast<UWidgetComponent>(refToSpeciesActor->GetComponentByClass(UWidgetComponent::StaticClass()));
	if (WidgetComponentSpecies != nullptr) {
		UUserWidget* UserWidget = WidgetComponentSpecies->GetUserWidgetObject();
		if (UserWidget != nullptr) {
			UTextBlock* Description = Cast<UTextBlock>(UserWidget->GetWidgetFromName(TEXT("TextSpecies")));
			if (Description != nullptr) {
				defaultSpeciesDescription = Description->GetText().ToString();
			}
		}
	}

	// Set the species descriptions
	speciesDescriptions.Empty();
	speciesDescriptions.Add(TEXT("MountainDwarf"), TEXT("Mountain Dwarves are strong melee fighters and capable heavy-armoured casters. They can use scrolls to enchant even some artefacts."));
	speciesDescriptions.Add(TEXT("Minotaur"), TEXT("Minotaurs are large muscular humans with bovine heads. They excel at all forms of close and ranged combat."));
	speciesDescriptions.Add(TEXT("Merfolk"), TEXT("Half fish, half human, Merfolk are citizens of both water and land. They are strong combatants, and adept at using magic to assist in battle."));
	speciesDescriptions.Add(TEXT("Gargoyle"), TEXT("Made of living stone, Gargoyles have a preternatural affinity for the earth. They have low health, but their stone bodies are incredibly tough."));
	speciesDescriptions.Add(TEXT("Draconian"), TEXT("Draconians are versatile hybrids. They mature when reaching level 7 and develop a colour."));
	speciesDescriptions.Add(TEXT("Troll"), TEXT("Trolls are monstrous creatures with powerful claws. They regenerate health rapidly."));
	speciesDescriptions.Add(TEXT("DeepElf"), TEXT("Deep Elves have a strong affinity for all kinds of magic, but are very fragile. They regenerate magical power rapidly."));
	speciesDescriptions.Add(TEXT("Armataur"), TEXT("Scaled mammals the size of horses, Armataurs are fast-rolling and tenacious when pursuing enemies, but lack dexterity."));
	speciesDescriptions.Add(TEXT("Gnoll"), TEXT("Gnolls are canine-like humanoids. They are unable to specialise in any single skill, but instead gradually become proficient in all skills at once."));
	speciesDescriptions.Add(TEXT("Human"), TEXT("Humans are natural explorers. Discovering new places refreshes and invigorates them. They are otherwise completely average."));
	speciesDescriptions.Add(TEXT("Kobold"), TEXT("Kobolds are attuned to the shadows and can only see or be seen at a close range. They are small and a little fragile, and are talented in most skills."));
	speciesDescriptions.Add(TEXT("Demonspawn"), TEXT("Demonspawn are half human, half demon. They acquire strong demonic mutations as they grow."));
	speciesDescriptions.Add(TEXT("Djinni"), TEXT("Djinn are ever-burning spirits. They learn spells innately, without books, and cast by drawing down their own fires."));
	speciesDescriptions.Add(TEXT("Spriggan"), TEXT("Very small, very frail and very fast, Spriggans are highly competent stabbers and spellcasters."));
	speciesDescriptions.Add(TEXT("Ghoul"), TEXT("Ghouls are undead creatures, damned to rot eternally. They have sharp claws, and they heal from the death of the living."));
	speciesDescriptions.Add(TEXT("Tengu"), TEXT("Tengu are bird-people who love to fight, both with weapons and spells. They are fragile but agile, and eventually learn to fly."));
	speciesDescriptions.Add(TEXT("Oni"), TEXT("Large, robust, and extremely fond of drinking, Oni make capable brawlers and spellcasters and gain extra benefits from healing potions."));
	speciesDescriptions.Add(TEXT("Barachi"), TEXT("Barachim are slow-moving, amphibious humanoids who can hop long distances. Their extended range of vision also allows them to be seen from further away."));
	speciesDescriptions.Add(TEXT("Coglin"), TEXT("Coglins ride enchanted exoskeletons able to wield weapons in both hands, wrapping slow-revving metal around their puny goblinoid frames."));
	speciesDescriptions.Add(TEXT("VineStalker"), TEXT("Frail symbionts, Vine Stalkers cannot regain health from potions. Their magical reserves absorb damage, and they pack a magic-restoring bite."));
	speciesDescriptions.Add(TEXT("Vampire"), TEXT("Shifting between the states of life and undeath at will, Vampires are accomplished stabbers and casters."));
	speciesDescriptions.Add(TEXT("Demigod"), TEXT("Demigods refuse to worship any god, but make up for this with divine attributes and high reserves of health and magic."));
	speciesDescriptions.Add(TEXT("Formicid"), TEXT("Formicids are humanoid ants adept at digging. Their limbs are exceptionally strong and they live in a state of permanent stasis."));
	speciesDescriptions.Add(TEXT("Naga"), TEXT("Nagas are hybrids; human from the waist up, with a snake-like tail instead of legs. They move very slowly but are naturally resilient."));
	speciesDescriptions.Add(TEXT("Octopode"), TEXT("Octopodes are a species of amphibious cephalopods. They can wear eight rings, but almost no armour fits them."));
	speciesDescriptions.Add(TEXT("Felid"), TEXT("Felids are many-lived sentient cats. They cannot use armour or weapons."));
	speciesDescriptions.Add(TEXT("Mummy"), TEXT("Compelled to walk by an ancient curse, Mummies have innate faith and are adept at Necromancy, but learn other skills very slowly. They cannot drink potions."));

	// Species name to letter
	speciesToLetter.Empty();
	speciesToLetter.Add(TEXT("MountainDwarf"), TEXT("a"));
	speciesToLetter.Add(TEXT("Minotaur"), TEXT("b"));
	speciesToLetter.Add(TEXT("Merfolk"), TEXT("c"));
	speciesToLetter.Add(TEXT("Gargoyle"), TEXT("d"));
	speciesToLetter.Add(TEXT("Draconian"), TEXT("e"));
	speciesToLetter.Add(TEXT("Troll"), TEXT("f"));
	speciesToLetter.Add(TEXT("DeepElf"), TEXT("g"));
	speciesToLetter.Add(TEXT("Armataur"), TEXT("h"));
	speciesToLetter.Add(TEXT("Gnoll"), TEXT("i"));
	speciesToLetter.Add(TEXT("Human"), TEXT("j"));
	speciesToLetter.Add(TEXT("Kobold"), TEXT("k"));
	speciesToLetter.Add(TEXT("Demonspawn"), TEXT("l"));
	speciesToLetter.Add(TEXT("Djinni"), TEXT("m"));
	speciesToLetter.Add(TEXT("Spriggan"), TEXT("n"));
	speciesToLetter.Add(TEXT("Ghoul"), TEXT("o"));
	speciesToLetter.Add(TEXT("Tengu"), TEXT("p"));
	speciesToLetter.Add(TEXT("Oni"), TEXT("q"));
	speciesToLetter.Add(TEXT("Barachi"), TEXT("r"));
	speciesToLetter.Add(TEXT("Coglin"), TEXT("s"));
	speciesToLetter.Add(TEXT("VineStalker"), TEXT("t"));
	speciesToLetter.Add(TEXT("Vampire"), TEXT("u"));
	speciesToLetter.Add(TEXT("Demigod"), TEXT("v"));
	speciesToLetter.Add(TEXT("Formicid"), TEXT("w"));
	speciesToLetter.Add(TEXT("Naga"), TEXT("x"));
	speciesToLetter.Add(TEXT("Octopode"), TEXT("y"));
	speciesToLetter.Add(TEXT("Felid"), TEXT("z"));
	speciesToLetter.Add(TEXT("Mummy"), TEXT("A"));

	// Background description default based on the current text value
	UWidgetComponent* WidgetComponentBackground = Cast<UWidgetComponent>(refToBackgroundActor->GetComponentByClass(UWidgetComponent::StaticClass()));
	if (WidgetComponentBackground != nullptr) {
		UUserWidget* UserWidget = WidgetComponentBackground->GetUserWidgetObject();
		if (UserWidget != nullptr) {
			UTextBlock* Description = Cast<UTextBlock>(UserWidget->GetWidgetFromName(TEXT("TextBackground")));
			if (Description != nullptr) {
				defaultBackgroundDescription = Description->GetText().ToString();
			}
		}
	}

	// The names of the songs
	musicListNames.Empty();
	musicListNames.Add(TEXT("Dark Fantasy Ambience - DeusLower"));
	musicListNames.Add(TEXT("Dark Fantasy Atmosphere - DeusLower"));
	musicListNames.Add(TEXT("The Time Is Upon Us - Elias Weber"));
	musicListNames.Add(TEXT("8-bit Dungeon - Kaden_Cook"));
	trackInd = 0;
	UWidgetComponent* WidgetComponent = Cast<UWidgetComponent>(refToSettingsActor->GetComponentByClass(UWidgetComponent::StaticClass()));
	if (WidgetComponent != nullptr) {
		UUserWidget* UserWidget = WidgetComponent->GetUserWidgetObject();
		if (UserWidget != nullptr) {
			UTextBlock* TrackText = Cast<UTextBlock>(UserWidget->GetWidgetFromName(TEXT("TextTrack")));
			if (TrackText != nullptr) {
				TrackText->SetText(FText::FromString("Track: " + musicListNames[trackInd]));
			}
		}
	}

	// Make sure we have the default texture
	if (!textures.Contains(TEXT("Unknown"))) {
		UE_LOG(LogTemp, Error, TEXT("Unknown texture not found"));
		textures.Add(TEXT("Unknown"), nullptr);
	}

	// Cache a blank line
	whiteLine = "";
	for (int j = 0; j < 80; j++) {
		whiteLine += TEXT(" ");
	}
	whiteLine += TEXT("\n");

	// Set the initial play button text 
	if (refToMainMenuActor != nullptr) {
		UWidgetComponent* WidgetComponentMainMenu = Cast<UWidgetComponent>(refToMainMenuActor->GetComponentByClass(UWidgetComponent::StaticClass()));
		if (WidgetComponentMainMenu != nullptr) {
			UUserWidget* UserWidgetMainMenu = WidgetComponentMainMenu->GetUserWidgetObject();
			if (UserWidgetMainMenu != nullptr) {
				UTextBlock* PlayButtonText = Cast<UTextBlock>(UserWidgetMainMenu->GetWidgetFromName(TEXT("TextMainPlay")));
				if (PlayButtonText != nullptr) {
					if (!firstTime) {
						PlayButtonText->SetText(FText::FromString("Play"));
					} else if (useServer) {
						PlayButtonText->SetText(FText::FromString("Waiting for server..."));
					} else {
						PlayButtonText->SetText(FText::FromString("Waiting for process..."));
					}
				}
			}
		}
	}

    // If we're not using the server, launch the process
	if (!useServer) {	
		UE_LOG(LogTemp, Display, TEXT("Launching pipes..."));
		FPlatformProcess::CreatePipe(StdOutReadHandle, StdOutWriteHandle);
		FPlatformProcess::CreatePipe(StdInReadHandle, StdInWriteHandle, true);
		UE_LOG(LogTemp, Display, TEXT("Launching process..."));
		ProcHandle = FPlatformProcess::CreateProc(*exePath, *args, true, true, true, nullptr, 0, nullptr, StdOutWriteHandle, StdInReadHandle);
	}
	
	// Search the saves list
	// Extra ups are needed because it starts with the most recent save selected
	int maxSaves = 20;
	for (int i=0; i<maxSaves+10; i++) {
		writeCommandQueued("down");
	}
	for (int i = 0; i < maxSaves+10; i++) {
		writeCommandQueued("up");
	}
	needMenu = true;

	// Setup the map arrays
	mapToDraw.Empty();
	mapToDrawRotated.Empty();
	mapToDraw.SetNum(gridWidth);
	mapToDrawRotated.SetNum(gridWidth);
	for (int i = 0; i < gridWidth; i++) {
		mapToDraw[i].SetNum(gridWidth);
		mapToDrawRotated[i].SetNum(gridWidth);
	}

	// Setup array sizes
	UE_LOG(LogTemp, Display, TEXT("Setting up arrays..."));
	wallArray.Empty();
	wallArray.SetNum(gridWidth);
	for (int i = 0; i < gridWidth; i++) {
		wallArray[i].SetNum(gridWidth);
		for (int j = 0; j < gridWidth; j++) {
			wallArray[i][j].SetNum(4);
		}
	}
	floorArray.Empty();
	floorArray.SetNum(gridWidth);
	for (int i = 0; i < gridWidth; i++) {
		floorArray[i].SetNum(gridWidth);
	}
	levelAscii.Empty();
	levelAscii.SetNum(gridWidth);
	for (int i = 0; i < gridWidth; i++) {
		levelAscii[i].SetNum(gridWidth);
	}
	levelAsciiPrev.Empty();
	levelAsciiPrev.SetNum(gridWidth);
	for (int i = 0; i < gridWidth; i++) {
		levelAsciiPrev[i].SetNum(gridWidth);
		for (int j = 0; j < gridWidth; j++) {
			levelAsciiPrev[i][j] = TEXT(" ");
		}
	}
	levelInfo.Empty();
	levelInfo.SetNum(gridWidth);
	for (int i = 0; i < gridWidth; i++) {
		levelInfo[i].SetNum(gridWidth);
		for (int j = 0; j < gridWidth; j++) {
			levelInfo[i][j].items.SetNum(0);
			levelInfo[i][j].itemHotkeys.SetNum(0);
		}
	}
	hotbarInfos.Empty();
	hotbarInfos.SetNum(numHotbarSlots);
	for (int i = 0; i < numHotbarSlots; i++) {
		hotbarInfos[i] = HotbarInfo();
	}

	// Reset level info
	UE_LOG(LogTemp, Display, TEXT("Resetting level info..."));
	for (int i = 0; i < gridWidth; i++) {
		for (int j = 0; j < gridWidth; j++) {
			levelInfo[i][j].currentChar = TEXT(" ");
			levelInfo[i][j].floorChar = TEXT(".");
			levelInfo[i][j].items.Empty();
			levelInfo[i][j].itemHotkeys.Empty();
			levelInfo[i][j].effect = TEXT("");
			levelInfo[i][j].enemy = TEXT("");
			levelInfo[i][j].enemyHotkey = TEXT("");
		}
	}

	// Spawn the floor planes
	UE_LOG(LogTemp, Display, TEXT("Spawning objects..."));
	UStaticMeshComponent* floorMesh = floorTemplate->FindComponentByClass<UStaticMeshComponent>();
	floorWidth = floorMesh->GetStaticMesh()->GetBounds().GetBox().GetSize().X * floorTemplate->GetActorScale3D().X;
	floorHeight = floorMesh->GetStaticMesh()->GetBounds().GetBox().GetSize().Y * floorTemplate->GetActorScale3D().Y;
	for (int i = -LOS; i < LOS + 1; i++) {
		for (int j = -LOS; j < LOS + 1; j++) {
			FVector Location(floorWidth * i, floorHeight * j, 0.0f);
			FActorSpawnParameters SpawnInfo;
			SpawnInfo.Template = floorTemplate;
			AActor* floor = worldRef->SpawnActor<AActor>(floorTemplate->GetClass(), SpawnInfo);
			floor->SetActorLocation(Location);
			floor->SetActorHiddenInGame(false); 
			floorArray[i + LOS][j + LOS] = floor;
			UStaticMeshComponent* mesh = floor->FindComponentByClass<UStaticMeshComponent>();
			UMaterialInstanceDynamic* dynamicMaterial = UMaterialInstanceDynamic::Create(mesh->GetMaterial(0), this);
			mesh->SetMaterial(0, dynamicMaterial);
		}
	}

	// Spawn the wall planes
	UStaticMeshComponent* wallMesh = wallTemplate->FindComponentByClass<UStaticMeshComponent>();
	wallWidth = wallMesh->GetStaticMesh()->GetBounds().GetBox().GetSize().X * wallTemplate->GetActorScale3D().X;
	UE_LOG(LogTemp, Display, TEXT("wallWidth: %f"), wallWidth);
	for (int i = -LOS; i < LOS + 1; i++) {
		for (int j = -LOS; j < LOS + 1; j++) {

			// Array locations
			int xInd = i + LOS;
			int yInd = j + LOS;
			float xLoc = -floorWidth * i;
			float yLoc = floorHeight * j;
			float zLoc = wallWidth / 2.0f;
			
			// The north wall for the tile
			FVector Location(xLoc + wallWidth/2, yLoc, zLoc);
			FRotator Rotation(0.0f, -90.0f, 0.0f);
			FActorSpawnParameters SpawnInfo;
			SpawnInfo.Template = wallTemplate;
			AActor* wall = worldRef->SpawnActor<AActor>(wallTemplate->GetClass(), Location, Rotation, SpawnInfo);
			wall->SetActorLocation(Location);
			wall->SetActorHiddenInGame(true);
			wallArray[xInd][yInd][0] = wall;
			UStaticMeshComponent* mesh = wall->FindComponentByClass<UStaticMeshComponent>();
			UMaterialInstanceDynamic* dynamicMaterial = UMaterialInstanceDynamic::Create(mesh->GetMaterial(0), this);
			mesh->SetMaterial(0, dynamicMaterial);

			// The south wall for the tile
			Location = FVector(xLoc - wallWidth / 2, yLoc, zLoc);
			Rotation = FRotator(0.0f, 90.0f, 0.0f);
			wall = worldRef->SpawnActor<AActor>(wallTemplate->GetClass(), Location, Rotation, SpawnInfo);
			wall->SetActorLocation(Location);
			wall->SetActorHiddenInGame(true);
			wallArray[xInd][yInd][1] = wall;
			mesh = wall->FindComponentByClass<UStaticMeshComponent>();
			dynamicMaterial = UMaterialInstanceDynamic::Create(mesh->GetMaterial(0), this);
			mesh->SetMaterial(0, dynamicMaterial);

			// The east wall for the tile
			Location = FVector(xLoc, yLoc + wallWidth / 2, zLoc);
			Rotation = FRotator(0.0f, 0.0f, 0.0f);
			wall = worldRef->SpawnActor<AActor>(wallTemplate->GetClass(), Location, Rotation, SpawnInfo);
			wall->SetActorLocation(Location);
			wall->SetActorHiddenInGame(true);
			wallArray[xInd][yInd][2] = wall;
			mesh = wall->FindComponentByClass<UStaticMeshComponent>();
			dynamicMaterial = UMaterialInstanceDynamic::Create(mesh->GetMaterial(0), this);
			mesh->SetMaterial(0, dynamicMaterial);

			// The west wall for the tile
			Location = FVector(xLoc, yLoc - wallWidth / 2, zLoc);
			Rotation = FRotator(0.0f, 180.0f, 0.0f);
			wall = worldRef->SpawnActor<AActor>(wallTemplate->GetClass(), Location, Rotation, SpawnInfo);
			wall->SetActorLocation(Location);
			wall->SetActorHiddenInGame(true);
			wallArray[xInd][yInd][3] = wall;
			mesh = wall->FindComponentByClass<UStaticMeshComponent>();
			dynamicMaterial = UMaterialInstanceDynamic::Create(mesh->GetMaterial(0), this);
			mesh->SetMaterial(0, dynamicMaterial);

		}
	}
	
	// Spawn the enemy planes
	enemyArray.Empty();
	enemyArray.SetNum(maxEnemies);
	for (int i = 0; i < maxEnemies; i++) {
		FActorSpawnParameters SpawnInfo;
		SpawnInfo.Template = enemyTemplate;
		AActor* wall = worldRef->SpawnActor<AActor>(enemyTemplate->GetClass(), SpawnInfo);
		wall->SetActorScale3D(FVector(0.66f*wallScaling, 0.66f*wallScaling, 1.0f));
		wall->SetActorHiddenInGame(true);
		enemyArray[i] = wall;
		UStaticMeshComponent* mesh = wall->FindComponentByClass<UStaticMeshComponent>();
		UMaterialInstanceDynamic* dynamicMaterial = UMaterialInstanceDynamic::Create(mesh->GetMaterial(0), this);
		mesh->SetMaterial(0, dynamicMaterial);
	}

	// Spawn the item planes
	itemArray.Empty();
	itemArray.SetNum(maxItems);
	for (int i = 0; i < maxItems; i++) {
		FActorSpawnParameters SpawnInfo;
		SpawnInfo.Template = itemTemplate;
		AActor* wall = worldRef->SpawnActor<AActor>(itemTemplate->GetClass(), SpawnInfo);
		wall->SetActorScale3D(FVector(0.3f*wallScaling, 0.3f*wallScaling, 1.0f));
		wall->SetActorHiddenInGame(true);
		itemArray[i] = wall;
		UStaticMeshComponent* mesh = wall->FindComponentByClass<UStaticMeshComponent>();
		UMaterialInstanceDynamic* dynamicMaterial = UMaterialInstanceDynamic::Create(mesh->GetMaterial(0), this);
		mesh->SetMaterial(0, dynamicMaterial);
	}

	// Spawn the effect planes
	effectArray.Empty();
	effectArray.SetNum(maxEffects);
	for (int i = 0; i < maxEffects; i++) {
		FActorSpawnParameters SpawnInfo;
		SpawnInfo.Template = effectTemplate;
		AActor* wall = worldRef->SpawnActor<AActor>(effectTemplate->GetClass(), SpawnInfo);
		wall->SetActorScale3D(FVector(0.66f*wallScaling, 0.66f*wallScaling, 1.0f));
		wall->SetActorHiddenInGame(true);
		effectArray[i] = wall;
		UStaticMeshComponent* mesh = wall->FindComponentByClass<UStaticMeshComponent>();
		UMaterialInstanceDynamic* dynamicMaterial = UMaterialInstanceDynamic::Create(mesh->GetMaterial(0), this);
		mesh->SetMaterial(0, dynamicMaterial);
	}

	// Clear the check locations
	checkLocs.Empty();
	UE_LOG(LogTemp, Display, TEXT("Num check locs: %d"), checkLocs.Num());

	// Start with the inventory closed
	inventoryOpen = false;
	if (refToInventoryActor != nullptr) {
		refToInventoryActor->SetActorHiddenInGame(!inventoryOpen);
		refToInventoryActor->SetActorEnableCollision(inventoryOpen);
	}

	// Set the initial menus
	if (refToSpeciesActor != nullptr) {
		refToSpeciesActor->SetActorHiddenInGame(true);
		refToSpeciesActor->SetActorEnableCollision(false);
	}
	if (refToBackgroundActor != nullptr) {
		refToBackgroundActor->SetActorHiddenInGame(true);
		refToBackgroundActor->SetActorEnableCollision(false);
	}
	if (refToNameActor != nullptr) {
		refToNameActor->SetActorHiddenInGame(true);
		refToNameActor->SetActorEnableCollision(false);
	}
	if (refToSaveActor != nullptr) {
		refToSaveActor->SetActorHiddenInGame(true);
		refToSaveActor->SetActorEnableCollision(false);
	}
	if (refToDeathActor != nullptr) {
		refToDeathActor->SetActorHiddenInGame(true);
		refToDeathActor->SetActorEnableCollision(false);
	}
	if (refToAltarActor != nullptr) {
		refToAltarActor->SetActorHiddenInGame(true);
		refToAltarActor->SetActorEnableCollision(false);
	}
	if (refToShopActor != nullptr) {
		refToShopActor->SetActorHiddenInGame(true);
		refToShopActor->SetActorEnableCollision(false);
	}
	if (refToSettingsActor != nullptr) {
		refToSettingsActor->SetActorHiddenInGame(true);
		refToSettingsActor->SetActorEnableCollision(false);
	}
	if (refToKeyboardActor != nullptr) {
		refToKeyboardActor->SetActorHiddenInGame(true);
		refToKeyboardActor->SetActorEnableCollision(false);
	}
	if (refToTutorialActor != nullptr) {
		refToTutorialActor->SetActorHiddenInGame(true);
		refToTutorialActor->SetActorEnableCollision(false);
	}

	// Close the keyboard
	isKeyboardOpen = false;
	if (refToKeyboardActor != nullptr) {
		refToKeyboardActor->SetActorHiddenInGame(true);
		refToKeyboardActor->SetActorEnableCollision(false);
	}

	// Load the global save game
	if (firstTime) {
		saveGameGlobal = Cast<UDCSSSaveGame>(UGameplayStatics::LoadGameFromSlot("globalsavefile", 0));
		if (saveGameGlobal == nullptr) {
			UE_LOG(LogTemp, Display, TEXT("Global save file not found, creating"));
			saveGameGlobal = Cast<UDCSSSaveGame>(UGameplayStatics::CreateSaveGameObject(UDCSSSaveGame::StaticClass()));
			saveEverything();
		}
		loadEverything();
	}

	// Items locs depending on the number of items
	float startingHeight = 0.25f * floorWidth;
	float extraHeightPer = 0.25f * floorWidth;
	itemLocs.Empty();
	itemLocs.Add(1, { 
		FVector(0.0f, 0.0f, startingHeight) 
	});
	itemLocs.Add(2, {
		FVector(-0.25f * floorWidth, 0.0f, startingHeight),
		FVector(0.25f * floorWidth, 0.0f, startingHeight)
	});
	itemLocs.Add(3, {
		FVector(-0.25f * floorWidth, 0.25f * floorWidth, startingHeight),
		FVector(0.25f * floorWidth, 0.25f * floorWidth, startingHeight),
		FVector(0.0f, -0.25f * floorWidth, startingHeight)
	});
	itemLocs.Add(4, {
		FVector(-0.25f * floorWidth, 0.25f * floorWidth, startingHeight),
		FVector(0.25f * floorWidth, 0.25f * floorWidth, startingHeight),
		FVector(-0.25f * floorWidth, -0.25f * floorWidth, startingHeight),
		FVector(0.25f * floorWidth, -0.25f * floorWidth, startingHeight)
	});
	itemLocs.Add(5, {
		FVector(-0.25f * floorWidth, 0.25f * floorWidth, startingHeight),
		FVector(0.25f * floorWidth, 0.25f * floorWidth, startingHeight),
		FVector(-0.25f * floorWidth, -0.25f * floorWidth, startingHeight),
		FVector(0.25f * floorWidth, -0.25f * floorWidth, startingHeight),
		FVector(0.0f, 0.0f, startingHeight)
	});
	itemLocs.Add(6, {
		FVector(-0.25f * floorWidth, 0.25f * floorWidth, startingHeight),
		FVector(0.25f * floorWidth, 0.25f * floorWidth, startingHeight),
		FVector(-0.25f * floorWidth, -0.25f * floorWidth, startingHeight),
		FVector(0.25f * floorWidth, -0.25f * floorWidth, startingHeight),
		FVector(-0.25f * floorWidth, 0.0f, startingHeight),
		FVector(0.25f * floorWidth, 0.0f, startingHeight)
	});

	// Then for each number after that, stack
	for (int k = 7; k < maxItems; k++) {
		TArray<FVector> newLocs = itemLocs[6];
		int remaining = k - 6;
		int levelCount = 1;
		while (remaining > 0) {
			int thisLevel = FMath::Min(remaining, 6);
			newLocs.Append(itemLocs[thisLevel]);
			remaining -= thisLevel;
			for (int l = newLocs.Num() - thisLevel; l < newLocs.Num(); l++) {
				newLocs[l].Z += extraHeightPer * levelCount;
			}
			levelCount++;
		}
		itemLocs.Add(k, newLocs);
	}

	UE_LOG(LogTemp, Display, TEXT("Finished initialization"));

}

// Called when the game starts or when spawned
void Adcss::BeginPlay() {
	Super::BeginPlay();
	init(true);
}

// Called when the level ascii has changed
void Adcss::updateLevel() {

	// Reset some things
	enemyUseCount = 0;
	for (int i = 0; i < maxEnemies; i++) {
		enemyArray[i]->SetActorHiddenInGame(true);
		enemyArray[i]->SetActorLocation(FVector(-1000.0f, -1000.0f, -1000.0f));
	}
	itemUseCount = 0;
	for (int i = 0; i < maxItems; i++) {
		itemArray[i]->SetActorHiddenInGame(true);
		itemArray[i]->SetActorLocation(FVector(-1000.0f, -1000.0f, -1000.0f));
	}
	effectUseCount = 0;
	for (int i = 0; i < maxEffects; i++) {
		effectArray[i]->SetActorHiddenInGame(true);
		effectArray[i]->SetActorLocation(FVector(-1000.0f, -1000.0f, -1000.0f));
	}

	// For debugging, output the whole level info
	UE_LOG(LogTemp, Display, TEXT("Current chars:"));
	for (int i = 0; i < gridWidth; i++) {
		FString row = TEXT("");
		for (int j = 0; j < gridWidth; j++) {
			row += levelInfo[i][j].currentChar;
		}
		UE_LOG(LogTemp, Display, TEXT("%s"), *row);
	}
	UE_LOG(LogTemp, Display, TEXT("Floor chars:"));
	for (int i = 0; i < gridWidth; i++) {
		FString row = TEXT("");
		for (int j = 0; j < gridWidth; j++) {
			row += levelInfo[i][j].floorChar;
		}
		UE_LOG(LogTemp, Display, TEXT("%s"), *row);
	}
	UE_LOG(LogTemp, Display, TEXT("Enemies:"));
	for (int i = 0; i < gridWidth; i++) {
		for (int j = 0; j < gridWidth; j++) {
			if (levelInfo[i][j].enemy.Len() > 0) {
				UE_LOG(LogTemp, Display, TEXT("(%d, %d): %s"), i, j, *levelInfo[i][j].enemy);
			}
		}
	}
	UE_LOG(LogTemp, Display, TEXT("Items:"));
	for (int i = 0; i < gridWidth; i++) {
		for (int j = 0; j < gridWidth; j++) {
			for (int k = 0; k < levelInfo[i][j].items.Num(); k++) {
				if (levelInfo[i][j].items[k].Len() > 0) {
					UE_LOG(LogTemp, Display, TEXT("(%d, %d): %s"), i, j, *levelInfo[i][j].items[k]);
				}
			}
		}
	}
	UE_LOG(LogTemp, Display, TEXT("Effects:"));
	for (int i = 0; i < gridWidth; i++) {
		for (int j = 0; j < gridWidth; j++) {
			if (levelInfo[i][j].effect.Len() > 0) {
				UE_LOG(LogTemp, Display, TEXT("(%d, %d): %s"), i, j, *levelInfo[i][j].effect);
			}
		}
	}

	// For each tile in the level
	for (int i = 0; i < levelInfo.Num(); i++) {
		for (int j = 0; j < levelInfo[i].Num(); j++) {

			// The ascii character at the tile
			FString ascii = levelInfo[i][j].currentChar;

			// The wall tiles at the location
			AActor* wallNorth = wallArray[i][j][0];
			AActor* wallSouth = wallArray[i][j][1];
			AActor* wallEast = wallArray[i][j][2];
			AActor* wallWest = wallArray[i][j][3];
			wallNorth->SetActorHiddenInGame(true);
			wallSouth->SetActorHiddenInGame(true);
			wallEast->SetActorHiddenInGame(true);
			wallWest->SetActorHiddenInGame(true);
			wallNorth->SetActorEnableCollision(false);
			wallSouth->SetActorEnableCollision(false);
			wallEast->SetActorEnableCollision(false);
			wallWest->SetActorEnableCollision(false);
			meshNameToThing.Add(wallNorth->GetName(), SelectedThing(j, i, "Wall", 0, ""));
			meshNameToThing.Add(wallSouth->GetName(), SelectedThing(j, i, "Wall", 1, ""));
			meshNameToThing.Add(wallEast->GetName(), SelectedThing(j, i, "Wall", 2, ""));
			meshNameToThing.Add(wallWest->GetName(), SelectedThing(j, i, "Wall", 3, ""));

			// The floor
			AActor* floor = floorArray[i][j];
			floor->SetActorLocation(FVector(-floorWidth * (i - LOS), floorHeight * (j - LOS), 0.0f));
			UStaticMeshComponent* mesh = floor->FindComponentByClass<UStaticMeshComponent>();
			UTexture2D* texture = getTexture("Floor" + currentBranch);
			if (currentBranch == "Abyss") {
				int randNum = FMath::Abs((i + j) % 6);
				texture = getTexture("WallAbyss" + FString::FromInt(randNum));
			}
			if (levelInfo[i][j].floorChar == TEXT("H")) {
				texture = getTexture("WaterDeep");
			} else if (levelInfo[i][j].floorChar == TEXT("~")) {
				texture = getTexture("WaterDeep");
			}
			UMaterialInstanceDynamic* material = (UMaterialInstanceDynamic*)mesh->GetMaterial(0);
			material->SetTextureParameterValue("TextureImage", texture);
			floor->SetActorHiddenInGame(false);
			meshNameToThing.Add(floor->GetName(), SelectedThing(j, i, "Floor", 0, ""));

			// If we have an effect
			if (levelInfo[i][j].effect.Len() > 0) {
			
				// Create the effect
				if (enemyUseCount < maxEffects) {

					// Set up the effect
					effectArray[effectUseCount]->SetActorHiddenInGame(false);
					FVector effectLocation = FVector(-floorWidth * (i - LOS), floorHeight * (j - LOS), floorWidth / 3.0f);
					effectArray[effectUseCount]->SetActorLocation(effectLocation);
					UStaticMeshComponent* effectMesh = effectArray[effectUseCount]->FindComponentByClass<UStaticMeshComponent>();
					meshNameToThing.Add(effectArray[effectUseCount]->GetName(), SelectedThing(j, i, "Effect", 0, ""));

					// Set the texture
					FString effect = levelInfo[i][j].effect;
					FString textureName = itemNameToTextureName(effect);
					UE_LOG(LogTemp, Display, TEXT("Effect: %s"), *effect);
					UE_LOG(LogTemp, Display, TEXT("Texture name: %s"), *textureName);
					UTexture2D* texture2 = getTexture(textureName);
					UMaterialInstanceDynamic* material2 = (UMaterialInstanceDynamic*)effectMesh->GetMaterial(0);
					material2->SetTextureParameterValue("TextureImage", texture2);
					material2->SetScalarParameterValue("Transparency", 0.3f);

					// No collision
					effectArray[effectUseCount]->SetActorEnableCollision(false);

					// Move slightly away from the player for easier selecting of other stuff
					FVector deltaLoc = effectLocation;
					deltaLoc.Normalize();
					deltaLoc *= 50.0f;
					deltaLoc.Z = 0.0f;
					FVector newLocation = effectLocation + deltaLoc;
					effectArray[effectUseCount]->SetActorLocation(newLocation);

					// Increment the effect use count
					effectUseCount++;

				} else {
					UE_LOG(LogTemp, Warning, TEXT("Effect count exceeded"));
				}

			}

			// If the ascii character is a wall
			if (ascii == TEXT("#")) {
				float xLoc = -floorWidth * (i - LOS);
				float yLoc = floorHeight * (j - LOS);
				float zLoc = wallWidth / 2.0f;
				wallNorth->SetActorHiddenInGame(false);
				wallSouth->SetActorHiddenInGame(false);
				wallEast->SetActorHiddenInGame(false);
				wallWest->SetActorHiddenInGame(false);
				wallNorth->SetActorEnableCollision(true);
				wallSouth->SetActorEnableCollision(true);
				wallEast->SetActorEnableCollision(true);
				wallWest->SetActorEnableCollision(true);
				wallNorth->SetActorLocation(FVector(xLoc + wallWidth / 2, yLoc, zLoc));
				wallSouth->SetActorLocation(FVector(xLoc - wallWidth / 2, yLoc, zLoc));
				wallEast->SetActorLocation(FVector(xLoc, yLoc + wallWidth / 2, zLoc));
				wallWest->SetActorLocation(FVector(xLoc, yLoc - wallWidth / 2, zLoc));
				wallNorth->SetActorScale3D(FVector(wallScaling, wallScaling, 1.0f));
				wallSouth->SetActorScale3D(FVector(wallScaling, wallScaling, 1.0f));
				wallEast->SetActorScale3D(FVector(wallScaling, wallScaling, 1.0f));
				wallWest->SetActorScale3D(FVector(wallScaling, wallScaling, 1.0f));
				wallNorth->SetActorRotation(FRotator(0.0f, 270.0f, 90.0f));
				wallSouth->SetActorRotation(FRotator(0.0f, 90.0f, 90.0f));
				wallEast->SetActorRotation(FRotator(0.0f, 0.0f, 90.0f));
				wallWest->SetActorRotation(FRotator(0.0f, 180.0f, 90.0f));
				UStaticMeshComponent* wallMeshNorth = wallNorth->FindComponentByClass<UStaticMeshComponent>();
				UStaticMeshComponent* wallMeshSouth = wallSouth->FindComponentByClass<UStaticMeshComponent>();
				UStaticMeshComponent* wallMeshEast = wallEast->FindComponentByClass<UStaticMeshComponent>();
				UStaticMeshComponent* wallMeshWest = wallWest->FindComponentByClass<UStaticMeshComponent>();
				UTexture2D* texture2 = getTexture("Wall" + currentBranch);
				if (currentBranch == "Abyss") {
					int randNum = FMath::Abs((i + j) % 6);
					texture2 = getTexture("WallAbyss" + FString::FromInt(randNum));
				}
				UMaterialInstanceDynamic* material2 = (UMaterialInstanceDynamic*)wallMeshNorth->GetMaterial(0);
				material2->SetTextureParameterValue("TextureImage", texture2);
				material2 = (UMaterialInstanceDynamic*)wallMeshSouth->GetMaterial(0);
				material2->SetTextureParameterValue("TextureImage", texture2);
				material2 = (UMaterialInstanceDynamic*)wallMeshEast->GetMaterial(0);
				material2->SetTextureParameterValue("TextureImage", texture2);
				material2 = (UMaterialInstanceDynamic*)wallMeshWest->GetMaterial(0);
				material2->SetTextureParameterValue("TextureImage", texture2);

				// Check for the adjacent walls
				bool northNotWall = i > 0 && (!thingsThatCountAsWalls.Contains(levelInfo[i - 1][j].currentChar, ESearchCase::CaseSensitive) || levelInfo[i - 1][j].currentChar == TEXT("@"));
				bool eastNotWall = j < gridWidth - 1 && (!thingsThatCountAsWalls.Contains(levelInfo[i][j + 1].currentChar, ESearchCase::CaseSensitive) || levelInfo[i - 1][j].currentChar == TEXT("@"));;
				bool southNotWall = i < gridWidth - 1 && (!thingsThatCountAsWalls.Contains(levelInfo[i + 1][j].currentChar, ESearchCase::CaseSensitive) || levelInfo[i - 1][j].currentChar == TEXT("@"));;
				bool westNotWall = j > 0 && (!thingsThatCountAsWalls.Contains(levelInfo[i][j - 1].currentChar, ESearchCase::CaseSensitive) || levelInfo[i - 1][j].currentChar == TEXT("@"));;

				//  .
				// .#.
				//  .
				if (northNotWall && eastNotWall && southNotWall && westNotWall) {
					wallNorth->SetActorScale3D(FVector(smallWallScaling, wallScaling, 1.0f));
					wallEast->SetActorScale3D(FVector(smallWallScaling, wallScaling, 1.0f));
					wallSouth->SetActorScale3D(FVector(smallWallScaling, wallScaling, 1.0f));
					wallWest->SetActorScale3D(FVector(smallWallScaling, wallScaling, 1.0f));
					wallNorth->SetActorLocation(FVector(xLoc + wallWidth / 4.0f, yLoc, zLoc));
					wallSouth->SetActorLocation(FVector(xLoc - wallWidth / 4.0f, yLoc, zLoc));
					wallEast->SetActorLocation(FVector(xLoc, yLoc + wallWidth / 4.0f, zLoc));
					wallWest->SetActorLocation(FVector(xLoc, yLoc - wallWidth / 4.0f, zLoc));

				//  .
				// .#.
				//  #  
				} else if (!southNotWall && westNotWall && eastNotWall && northNotWall) {
					wallEast->SetActorScale3D(FVector(halfDiagWallScaling, wallScaling, 1.0f));
					wallWest->SetActorScale3D(FVector(halfDiagWallScaling, wallScaling, 1.0f));
					wallWest->SetActorLocation(FVector(-(i - LOS) * floorWidth - wallWidth/4.0f, (j - LOS) * floorHeight + wallWidth/4.0f, wallWidth / 2.0f));
					wallEast->SetActorLocation(FVector(-(i - LOS) * floorWidth - wallWidth/4.0f, (j - LOS) * floorHeight - wallWidth/4.0f, wallWidth / 2.0f));
					wallWest->SetActorRotation(FRotator(0.0f, 315.0f, 90.0f));
					wallEast->SetActorRotation(FRotator(0.0f, 225.0f, 90.0f));
					wallNorth->SetActorHiddenInGame(true);

				//  .
				// .##
				//  .   
				} else if (southNotWall && westNotWall && !eastNotWall && northNotWall) {
					wallNorth->SetActorScale3D(FVector(halfDiagWallScaling, wallScaling, 1.0f));
					wallSouth->SetActorScale3D(FVector(halfDiagWallScaling, wallScaling, 1.0f));
					wallNorth->SetActorLocation(FVector(-(i - LOS) * floorWidth + wallWidth/4.0f, (j - LOS) * floorHeight + wallWidth/4.0f, wallWidth / 2.0f));
					wallSouth->SetActorLocation(FVector(-(i - LOS) * floorWidth - wallWidth/4.0f, (j - LOS) * floorHeight + wallWidth/4.0f, wallWidth / 2.0f));
					wallNorth->SetActorRotation(FRotator(0.0f, 225.0f, 90.0f));
					wallSouth->SetActorRotation(FRotator(0.0f, 135.0f, 90.0f));
					wallWest->SetActorHiddenInGame(true);

				//  #
				// .#.
				//  .  
				} else if (!northNotWall && westNotWall && eastNotWall && southNotWall) {
					wallEast->SetActorScale3D(FVector(halfDiagWallScaling, wallScaling, 1.0f));
					wallWest->SetActorScale3D(FVector(halfDiagWallScaling, wallScaling, 1.0f));
					wallWest->SetActorLocation(FVector(-(i - LOS) * floorWidth + wallWidth/4.0f, (j - LOS) * floorHeight - wallWidth/4.0f, wallWidth / 2.0f));
					wallEast->SetActorLocation(FVector(-(i - LOS) * floorWidth + wallWidth/4.0f, (j - LOS) * floorHeight + wallWidth/4.0f, wallWidth / 2.0f));
					wallWest->SetActorRotation(FRotator(0.0f, 135.0f, 90.0f));
					wallEast->SetActorRotation(FRotator(0.0f, 45.0f, 90.0f));
					wallSouth->SetActorHiddenInGame(true);

				//  .
				// ##.
				//  .  
				} else if (southNotWall && eastNotWall && !westNotWall && northNotWall) {
					wallNorth->SetActorScale3D(FVector(halfDiagWallScaling, wallScaling, 1.0f));
					wallSouth->SetActorScale3D(FVector(halfDiagWallScaling, wallScaling, 1.0f));
					wallNorth->SetActorLocation(FVector(-(i - LOS) * floorWidth - wallWidth/4.0f, (j - LOS) * floorHeight - wallWidth/4.0f, wallWidth / 2.0f));
					wallSouth->SetActorLocation(FVector(-(i - LOS) * floorWidth + wallWidth/4.0f, (j - LOS) * floorHeight - wallWidth/4.0f, wallWidth / 2.0f));
					wallNorth->SetActorRotation(FRotator(0.0f, 45.0f, 90.0f));
					wallSouth->SetActorRotation(FRotator(0.0f, 315.0f, 90.0f));
					wallEast->SetActorHiddenInGame(true);

				// . #
				//   .
				} else if (southNotWall && westNotWall) {
					wallSouth->SetActorScale3D(FVector(diagWallScaling, wallScaling, 1.0f));
					wallSouth->SetActorRotation(FRotator(0.0f, 135.0f, 90.0f));
					wallSouth->SetActorLocation(FVector(-(i - LOS) * floorWidth, (j - LOS) * floorHeight, wallWidth / 2.0f));
					wallSouth->SetActorHiddenInGame(false);
					wallWest->SetActorHiddenInGame(true);

				// # .
				// .  
				} else if (southNotWall && eastNotWall) {
					wallSouth->SetActorScale3D(FVector(diagWallScaling, wallScaling, 1.0f));
					wallSouth->SetActorRotation(FRotator(0.0f, 45.0f, 90.0f));
					wallSouth->SetActorLocation(FVector(-(i - LOS) * floorWidth, (j - LOS) * floorHeight, wallWidth / 2.0f));
					wallSouth->SetActorHiddenInGame(false);
					wallEast->SetActorHiddenInGame(true);

				// . 
				// # .
				} else if (northNotWall && eastNotWall) {
					wallNorth->SetActorScale3D(FVector(diagWallScaling, wallScaling, 1.0f));
					wallNorth->SetActorRotation(FRotator(0.0f, 315.0f, 90.0f));
					wallNorth->SetActorLocation(FVector(-(i - LOS) * floorWidth, (j - LOS) * floorHeight, wallWidth / 2.0f));
					wallNorth->SetActorHiddenInGame(false);
					wallEast->SetActorHiddenInGame(true);


				//   .
				// . #
				} else if (northNotWall && westNotWall) {
					wallNorth->SetActorScale3D(FVector(diagWallScaling, wallScaling, 1.0f));
					wallNorth->SetActorRotation(FRotator(0.0f, 225.0f, 90.0f));
					wallNorth->SetActorLocation(FVector(-(i - LOS) * floorWidth, (j - LOS) * floorHeight, wallWidth / 2.0f));
					wallWest->SetActorHiddenInGame(true);
				}

			// If it's a door
			} else if (ascii == TEXT("+") || ascii == TEXT("'")) {
				wallNorth->SetActorHiddenInGame(false);
				wallSouth->SetActorHiddenInGame(false);
				wallEast->SetActorHiddenInGame(true);
				wallWest->SetActorHiddenInGame(true);
				UE_LOG(LogTemp, Display, TEXT("Door at (%d, %d)"), i, j);
				wallNorth->SetActorLocation(FVector(-floorWidth * (i - LOS), floorHeight * (j - LOS), wallWidth / 2.0f));;
				wallSouth->SetActorLocation(FVector(-floorWidth * (i - LOS), floorHeight* (j - LOS), wallWidth / 2.0f));
				UStaticMeshComponent* wallMeshNorth = wallNorth->FindComponentByClass<UStaticMeshComponent>();
				UStaticMeshComponent* wallMeshSouth = wallSouth->FindComponentByClass<UStaticMeshComponent>();

				// Check for nearby doors
				bool hasDoorEast = j < gridWidth - 1 && thingsThatCountAsDoors.Contains(levelInfo[i][j + 1].currentChar, ESearchCase::CaseSensitive);
				bool hasDoorWest = j > 0 && thingsThatCountAsDoors.Contains(levelInfo[i][j - 1].currentChar, ESearchCase::CaseSensitive);
				bool hasDoorNorth = i > 0 && thingsThatCountAsDoors.Contains(levelInfo[i - 1][j].currentChar, ESearchCase::CaseSensitive);
				bool hasDoorSouth = i < gridWidth - 1 && thingsThatCountAsDoors.Contains(levelInfo[i + 1][j].currentChar, ESearchCase::CaseSensitive);

				// Set the rotation of the door based on the surrounding walls
				bool north = i > 0 && thingsThatCountAsWalls.Contains(levelInfo[i - 1][j].currentChar, ESearchCase::CaseSensitive);
                bool east = j < gridWidth - 1 && thingsThatCountAsWalls.Contains(levelInfo[i][j + 1].currentChar, ESearchCase::CaseSensitive);
                bool south = i < gridWidth - 1 && thingsThatCountAsWalls.Contains(levelInfo[i + 1][j].currentChar, ESearchCase::CaseSensitive);
                bool west = j > 0 && thingsThatCountAsWalls.Contains(levelInfo[i][j - 1].currentChar, ESearchCase::CaseSensitive);
				FString matName = "Door";
				if (north && south) {
					if (j > LOS) {
						wallNorth->SetActorRotation(FRotator(0.0f, 180.0f, 90.0f));
						wallSouth->SetActorRotation(FRotator(0.0f, 0.0f, 90.0f));
						if (hasDoorNorth && hasDoorSouth) {
							matName += "Middle";
						} else if (hasDoorNorth) {
							matName += "Left";
						} else if (hasDoorSouth) {
							matName += "Right";
						}
					} else {
						wallNorth->SetActorRotation(FRotator(0.0f, 0.0f, 90.0f));
						wallSouth->SetActorRotation(FRotator(0.0f, 180.0f, 90.0f));
						if (hasDoorNorth && hasDoorSouth) {
							matName += "Middle";
						} else if (hasDoorNorth) {
							matName += "Right";
						} else if (hasDoorSouth) {
							matName += "Left";
						}
					}
				} else if (east && west) {
					if (i > LOS) {
						wallNorth->SetActorRotation(FRotator(0.0f, 270.0f, 90.0f));
						wallSouth->SetActorRotation(FRotator(0.0f, 90.0f, 90.0f));
						if (hasDoorEast && hasDoorWest) {
							matName += "Middle";
						} else if (hasDoorEast) {
							matName += "Right";
						} else if (hasDoorWest) {
							matName += "Left";
						}
					} else {
						wallNorth->SetActorRotation(FRotator(0.0f, 90.0f, 90.0f));
						wallSouth->SetActorRotation(FRotator(0.0f, 270.0f, 90.0f));
						if (hasDoorEast && hasDoorWest) {
							matName += "Middle";
						} else if (hasDoorEast) {
							matName += "Left";
						} else if (hasDoorWest) {
							matName += "Right";
						}
					}
				}
				if (ascii == TEXT("+")) {
					matName += "Closed";
				} else {
					matName += "Open";
				}
				UTexture2D* texture2 = getTexture(matName);
				UMaterialInstanceDynamic* material2 = (UMaterialInstanceDynamic*)wallMeshNorth->GetMaterial(0);
				material2->SetTextureParameterValue("TextureImage", texture2);
				material2 = (UMaterialInstanceDynamic*)wallMeshSouth->GetMaterial(0);
				material2->SetTextureParameterValue("TextureImage", texture2);

			// If it's a plant
            } else if (levelInfo[i][j].enemy.Len() == 0 && (ascii == TEXT("P") || ascii == TEXT("7")  || ascii == TEXT("c") || ascii == TEXT("C"))) {

				// Add an enemy
				if (enemyUseCount < maxEnemies) {
					enemyArray[enemyUseCount]->SetActorHiddenInGame(false);
					enemyArray[enemyUseCount]->SetActorLocation(FVector(-floorWidth * (i - LOS) + 1, floorHeight * (j - LOS) + 1, floorWidth / 4.0f));
					UStaticMeshComponent* enemyMesh = enemyArray[enemyUseCount]->FindComponentByClass<UStaticMeshComponent>();
                    if (ascii == TEXT("P")) {
						UTexture2D* texture2 = getTexture("Plant");
						UMaterialInstanceDynamic* material2 = (UMaterialInstanceDynamic*)enemyMesh->GetMaterial(0);
						material2->SetTextureParameterValue("TextureImage", texture2);
                    } else if (ascii == TEXT("C") || ascii == TEXT("c") || ascii == TEXT("7")) {
                        UTexture2D* texture2 = getTexture("Tree");
						UMaterialInstanceDynamic* material2 = (UMaterialInstanceDynamic*)enemyMesh->GetMaterial(0);
						material2->SetTextureParameterValue("TextureImage", texture2);
                    }
					meshNameToThing.Add(enemyArray[enemyUseCount]->GetName(), SelectedThing(j, i, "Enemy", 0, ""));
					enemyUseCount++;
				} else {
					UE_LOG(LogTemp, Warning, TEXT("Enemy count exceeded"));
				}

			// If it's an staircase down
			} else if (ascii == TEXT(">")) {

				// Create an enemy to act as a ladder
				if (enemyUseCount < maxEnemies) {
					enemyArray[enemyUseCount]->SetActorHiddenInGame(false);
					enemyArray[enemyUseCount]->SetActorLocation(FVector(-floorWidth * (i - LOS), floorHeight * (j - LOS), floorWidth / 5.0f));
					UStaticMeshComponent* enemyMesh = enemyArray[enemyUseCount]->FindComponentByClass<UStaticMeshComponent>();
					UTexture2D* texture2 = getTexture("LadderDown");
					UMaterialInstanceDynamic* material2 = (UMaterialInstanceDynamic*)enemyMesh->GetMaterial(0);
					material2->SetTextureParameterValue("TextureImage", texture2);
					meshNameToThing.Add(enemyArray[enemyUseCount]->GetName(), SelectedThing(j, i, "LadderDown", 0, ""));
					enemyUseCount++;
				} else {
					UE_LOG(LogTemp, Warning, TEXT("Enemy count exceeded"));
				}

			// If it's an staircase up
			} else if (ascii == TEXT("<")) {

				// Create an enemy to act as a ladder
				if (enemyUseCount < maxEnemies) {
					enemyArray[enemyUseCount]->SetActorHiddenInGame(false);
					enemyArray[enemyUseCount]->SetActorLocation(FVector(-floorWidth * (i - LOS), floorHeight * (j - LOS), floorWidth / 1.0f));
					UStaticMeshComponent* enemyMesh = enemyArray[enemyUseCount]->FindComponentByClass<UStaticMeshComponent>();
					UTexture2D* texture2 = getTexture("LadderUp");
					UMaterialInstanceDynamic* material2 = (UMaterialInstanceDynamic*)enemyMesh->GetMaterial(0);
					material2->SetTextureParameterValue("TextureImage", texture2);
					meshNameToThing.Add(enemyArray[enemyUseCount]->GetName(), SelectedThing(j, i, "LadderUp", 0, ""));
					enemyUseCount++;
				} else {
					UE_LOG(LogTemp, Warning, TEXT("Enemy count exceeded"));
				}

			}

			// If it's an enemy
			if (levelInfo[i][j].enemy.Len() > 0) {

				// Add an enemy
				if (enemyUseCount < maxEnemies) {

					// Set up the enemy
					enemyArray[enemyUseCount]->SetActorHiddenInGame(false);
					enemyArray[enemyUseCount]->SetActorLocation(FVector(-floorWidth * (i - LOS) + 1, floorHeight * (j - LOS) + 1, floorWidth / 3.0f));
					UStaticMeshComponent* enemyMesh = enemyArray[enemyUseCount]->FindComponentByClass<UStaticMeshComponent>();
					meshNameToThing.Add(enemyArray[enemyUseCount]->GetName(), SelectedThing(j, i, "Enemy", 0, ""));

					// Set the texture
					FString enemyName = levelInfo[i][j].enemy;
					FString textureName = enemyNameToTextureName(enemyName);
					UTexture2D* texture2 = getTexture(textureName);
					UMaterialInstanceDynamic* material2 = (UMaterialInstanceDynamic*)enemyMesh->GetMaterial(0);
					material2->SetTextureParameterValue("TextureImage", texture2);
					enemyUseCount++;

				} else {
					UE_LOG(LogTemp, Warning, TEXT("Enemy count exceeded"));
				}

			}

			// If it's an item
			if (levelInfo[i][j].items.Num() > 0) {

				// Add each item
				for (int k = 0; k < levelInfo[i][j].items.Num(); k++) {
					if (itemUseCount < maxItems) {

						// Setup the item
						itemArray[itemUseCount]->SetActorHiddenInGame(false);
						FVector itemDelta = itemLocs[levelInfo[i][j].items.Num()][k];
						itemArray[itemUseCount]->SetActorLocation(FVector(-floorWidth * (i - LOS) + itemDelta.X, floorHeight * (j - LOS) + itemDelta.Y, itemDelta.Z));
						UStaticMeshComponent* itemMesh = itemArray[itemUseCount]->FindComponentByClass<UStaticMeshComponent>();
						FString typeName = "Item";
						if (levelInfo[i][j].items[k].Contains(TEXT("altar"))) {
							typeName = "Altar";
						} else if (levelInfo[i][j].items[k].Contains(TEXT("arch"))) {
							typeName = "Arch";
						} else if (gateList.Contains(levelInfo[i][j].items[k])) {
							typeName = "Gate";
						} else if (levelInfo[i][j].items[k].Contains(TEXT(" shop")) 
						|| levelInfo[i][j].items[k].Contains(TEXT(" bazaar")) 
						|| levelInfo[i][j].items[k].Contains(TEXT(" emporium")) 
						|| levelInfo[i][j].items[k].Contains(TEXT(" boutique")) 
						|| levelInfo[i][j].items[k].Contains(TEXT(" antiques")) 
						|| levelInfo[i][j].items[k].Contains(TEXT(" store"))) {
							typeName = "Shop";
						}
						meshNameToThing.Add(itemArray[itemUseCount]->GetName(), SelectedThing(j, i, typeName, k, ""));

						// Determine the actual name of the item
						FString itemName = levelInfo[i][j].items[k];
						FString textureName = itemNameToTextureName(itemName);
						UTexture2D* texture2 = getTexture(textureName);
						UMaterialInstanceDynamic* material2 = (UMaterialInstanceDynamic*)itemMesh->GetMaterial(0);
						material2->SetTextureParameterValue("TextureImage", texture2);

						// If it's non-standard
						bool isSpecial = false;
						if (itemName.Contains(TEXT("+"))) {
							isSpecial = true;
						}
						int32 ofIndex = itemName.Find(TEXT(" of "));
						if (ofIndex != INDEX_NONE) {
							isSpecial = true;
						}

						// If it's a corpse, different rotation, also bigger
						if (itemName.Contains(TEXT("Corpse")) || itemName.Contains(TEXT("Skeleton"))) {
							itemArray[itemUseCount]->SetActorRotation(FRotator(0.0f, 0.0f, 0.0f));
							itemArray[itemUseCount]->SetActorScale3D(FVector(0.66f*wallScaling, 0.66f*wallScaling, 1.0f));
							itemArray[itemUseCount]->SetActorLocation(FVector(-floorWidth * (i - LOS) + itemDelta.X, floorHeight * (j - LOS) + itemDelta.Y, 2*epsilon));
						
						// Some things are also bigger
						} else if (itemName.Contains(TEXT("arch")) || typeName == "Gate" || itemName.Contains(TEXT("idol")) || itemName.Contains(TEXT("statue")) || itemName.Contains(TEXT("fountain"))) {
							itemArray[itemUseCount]->SetActorRotation(FRotator(0.0f, 0.0f, 90.0f));
							itemArray[itemUseCount]->SetActorScale3D(FVector(0.66f*wallScaling, 0.66f*wallScaling, 1.0f));
							itemArray[itemUseCount]->SetActorLocation(FVector(-floorWidth * (i - LOS), floorHeight * (j - LOS), floorHeight*0.33f));
						
						// Altars and shop are a bit bigger
						} else if (typeName == "Altar" || typeName == "Shop") {
							itemArray[itemUseCount]->SetActorRotation(FRotator(0.0f, 0.0f, 90.0f));
							itemArray[itemUseCount]->SetActorScale3D(FVector(0.6f*wallScaling, 0.6f*wallScaling, 1.0f));
							itemArray[itemUseCount]->SetActorLocation(FVector(-floorWidth * (i - LOS), floorHeight * (j - LOS), floorHeight*0.3f));
					
						// Otherwise reset it
						} else {
							itemArray[itemUseCount]->SetActorRotation(FRotator(0.0f, 0.0f, 90.0f));
							itemArray[itemUseCount]->SetActorScale3D(FVector(0.3f*wallScaling, 0.3f*wallScaling, 1.0f));
						}

						// Update the item count
						itemUseCount++;

					} else {
						UE_LOG(LogTemp, Warning, TEXT("Item count exceeded"));
					}
				}

			}

		}
	}

}

// When a button in a menu is hovered or unhovered
void Adcss::buttonHovered(FString buttonName, bool hovered) {

	// Set the global var
	if (hovered) {
		selected.thingIs = buttonName;
	} else {
		if (selected.thingIs == buttonName) {
			selected = SelectedThing();
		}
	}

	// If it's inventory item e.g. "ItemButton_1_4"
	if (buttonName.Contains(TEXT("ItemButton"))) {

		// Get the row and column
		TArray<FString> parts;
		buttonName.ParseIntoArray(parts, TEXT("_"), true);
		int32 row = FCString::Atoi(*parts[1])-1;
		int32 col = FCString::Atoi(*parts[2])-1;

		// Get the overall widget
		UWidgetComponent* WidgetComponent = Cast<UWidgetComponent>(refToInventoryActor->GetComponentByClass(UWidgetComponent::StaticClass()));
		if (WidgetComponent != nullptr) {
			UUserWidget* UserWidget = WidgetComponent->GetUserWidgetObject();
			if (UserWidget != nullptr) {

				// Get the tooltip widget
				UTextBlock* Tooltip = Cast<UTextBlock>(UserWidget->GetWidgetFromName(TEXT("Tooltip")));
				if (Tooltip != nullptr) {

					// If hovered, show the item name
					if (hovered) {
						FString letter = inventoryLocToLetter[row][col];
						if (inventoryLetterToName.Contains(letter)) {
							Tooltip->SetText(FText::FromString("selected: " + inventoryLetterToName[letter]));
						}

					// Otherwise, reset the text
					} else if (thingBeingDragged.thingIs != "InventoryItem") {
						Tooltip->SetText(FText::FromString(TEXT("selected: none")));
					}

				}

				// Get the border widget
				FString borderName = buttonName.Replace(TEXT("ItemButton"), TEXT("ItemBorder"));
				UBorder* Border = Cast<UBorder>(UserWidget->GetWidgetFromName(*borderName));
				if (Border != nullptr) {
					if (hovered) {
						Border->SetBrushColor(FLinearColor(0.65f, 0.65f, 0.65f, 1.0f));
					} else {
						Border->SetBrushColor(FLinearColor(0.381326f, 0.381326f, 0.381326f, 1.0f));
					}
				} else {
					UE_LOG(LogTemp, Warning, TEXT("No border found for %s"), *borderName);
				}
			}
		}

	// If it's a background
	} else if (buttonName.Contains(TEXT("ButtonBackground"))) {

		// Get the background name
		FString backgroundName = buttonName.Replace(TEXT("ButtonBackground"), TEXT(""));
		UE_LOG(LogTemp, Display, TEXT("Background hovered: %s"), *backgroundName);
		if (backgroundDescriptions.Contains(backgroundName)) {

			// Get the description
			FString description = backgroundDescriptions[backgroundName];
			if (!hovered) {
				if (currentBackground == "none" || !backgroundDescriptions.Contains(currentBackground)) {
					description = defaultBackgroundDescription;
				} else {
					description = backgroundDescriptions[currentBackground];
				}
			}

			// Set the description text
			UWidgetComponent* WidgetComponent = Cast<UWidgetComponent>(refToBackgroundActor->GetComponentByClass(UWidgetComponent::StaticClass()));
			if (WidgetComponent != nullptr) {
				UUserWidget* UserWidget = WidgetComponent->GetUserWidgetObject();
				if (UserWidget != nullptr) {
					UTextBlock* Description = Cast<UTextBlock>(UserWidget->GetWidgetFromName(TEXT("TextBackground")));
					if (Description != nullptr) {
						Description->SetText(FText::FromString(description));
					}
				}
			}

		}

	// If it's a species
	} else if (buttonName.Contains(TEXT("ButtonSpecies"))) {

		// Get the species name
		FString speciesName = buttonName.Replace(TEXT("ButtonSpecies"), TEXT(""));
		UE_LOG(LogTemp, Display, TEXT("Species hovered: %s"), *speciesName);
		if (speciesDescriptions.Contains(speciesName)) {

			// Get the description
			FString description = speciesDescriptions[speciesName];
			if (!hovered) {
				if (currentSpecies == "none" || !speciesDescriptions.Contains(currentSpecies)) {
					description = defaultSpeciesDescription;
				} else {
					description = speciesDescriptions[currentSpecies];
				}
			}

			// Set the description text
			UWidgetComponent* WidgetComponent = Cast<UWidgetComponent>(refToSpeciesActor->GetComponentByClass(UWidgetComponent::StaticClass()));
			if (WidgetComponent != nullptr) {
				UUserWidget* UserWidget = WidgetComponent->GetUserWidgetObject();
				if (UserWidget != nullptr) {
					UTextBlock* Description = Cast<UTextBlock>(UserWidget->GetWidgetFromName(TEXT("TextSpecies")));
					if (Description != nullptr) {
						Description->SetText(FText::FromString(description));
					}
				}
			}

		}

	// If it's a skill
	} else if (buttonName.Contains(TEXT("ButtonSkill"))) {

		// Get the skill name
		FString skillName = buttonName.Replace(TEXT("ButtonSkill"), TEXT(""));
		if (skillDescriptions.Contains(skillName)) {

			// Get the description
			FString description = skillDescriptions[skillName];
			if (!hovered) {
				description = defaultSkillDescription;
			}

			// Set the description text
			UWidgetComponent* WidgetComponent = Cast<UWidgetComponent>(refToInventoryActor->GetComponentByClass(UWidgetComponent::StaticClass()));
			if (WidgetComponent != nullptr) {
				UUserWidget* UserWidget = WidgetComponent->GetUserWidgetObject();
				if (UserWidget != nullptr) {
					UTextBlock* Description = Cast<UTextBlock>(UserWidget->GetWidgetFromName(TEXT("TextSkillDescription")));
					if (Description != nullptr) {
						Description->SetText(FText::FromString(description));
					}
				}
			}

		}

	}

}

// When a DCSS key is pressed
void Adcss::keyPressed(FString key, FVector2D delta) {
	if (key == "move") {
		if (delta.X < 0) {
			writeCommandQueued("left");
		} else if (delta.X > 0) {
			writeCommandQueued("right");
		} else if (delta.Y < 0) {
			writeCommandQueued("up");
		} else if (delta.Y > 0) {
			writeCommandQueued("down");
		}
	} else if (key == "shifton") {
		shiftOn = true;
	} else if (key == "shiftoff") {
		shiftOn = false;
	} else if (key == "lmb") {
		UE_LOG(LogTemp, Display, TEXT("INPUT - Left mouse: (%d, %d) %s %d"), selected.x, selected.y, *selected.thingIs, selected.thingIndex);
		UE_LOG(LogTemp, Display, TEXT("INPUT- Thing being dragged: %s %d"), *thingBeingDragged.thingIs, thingBeingDragged.thingIndex);

		// Main menu button
		if (selected.thingIs == "ButtonMainMenu") {
			UE_LOG(LogTemp, Display, TEXT("INPUT - Main menu button clicked"));
			needMenu = true;
			refToUIActor->SetActorHiddenInGame(true);
			refToUIActor->SetActorEnableCollision(false);
			refToTutorialActor->SetActorHiddenInGame(true);
			refToTutorialActor->SetActorEnableCollision(false);
			refToInventoryActor->SetActorHiddenInGame(true);
			refToInventoryActor->SetActorEnableCollision(false);
			refToMainMenuActor->SetActorHiddenInGame(false);
			refToMainMenuActor->SetActorEnableCollision(true);
			refToMainInfoActor->SetActorHiddenInGame(false);
			refToMainInfoActor->SetActorEnableCollision(true);
			inventoryOpen = false;

			// If it's the process, close it
			if (!useServer) {
				UE_LOG(LogTemp, Display, TEXT("INPUT - Closing process"));
				writeCommandQueued("escape");
				writeCommandQueued("escape");
				writeCommandQueued("enter");
				writeCommandQueued("enter");
				writeCommandQueued("exit");
				writeCommandQueued("escape");
				FPlatformProcess::Sleep(2.0f);
				FPlatformProcess::TerminateProc(ProcHandle, true);
				FPlatformProcess::ClosePipe(StdInReadHandle, StdInWriteHandle);
				FPlatformProcess::ClosePipe(StdOutReadHandle, StdOutWriteHandle);

			// If it's the server, just go back to the menu
			} else {
				writeCommandQueued("escape");
				writeCommandQueued("escape");
				writeCommandQueued("enter");
				writeCommandQueued("enter");
				writeCommandQueued("exit");
			}

			// Remove all the actors
			for (int i = 0; i < maxEnemies; i++) {
				enemyArray[i]->Destroy();
			}
			for (int i = 0; i < maxItems; i++) {
				itemArray[i]->Destroy();
			}
			for (int i = 0; i < gridWidth; i++) {
				for (int j = 0; j < gridWidth; j++) {
					wallArray[i][j][0]->Destroy();
					wallArray[i][j][1]->Destroy();
					wallArray[i][j][2]->Destroy();
					wallArray[i][j][3]->Destroy();
					floorArray[i][j]->Destroy();
				}
			}

			// Start everything up again
			init(false);

		// Debug buttons
		} else if (selected.thingIs.Contains(TEXT("Debug"))) {
			UE_LOG(LogTemp, Display, TEXT("INPUT - Debug button clicked: %s"), *selected.thingIs);

			// Process the debug command
			if (selected.thingIs == "ButtonDebugUp") {
				writeCommandQueued("&");
				writeCommandQueued("u");
				writeCommandQueued("enter");
			} else if (selected.thingIs == "ButtonDebugDown") {
				writeCommandQueued("&");
				writeCommandQueued("d");
				writeCommandQueued("enter");
			} else if (selected.thingIs == "ButtonDebugIdentify") {
				writeCommandQueued("&");
				writeCommandQueued("i");
				writeCommandQueued("enter");
			} else if (selected.thingIs == "ButtonDebugForget") {
				writeCommandQueued("&");
				writeCommandQueued("I");
				writeCommandQueued("enter");
			} else if (selected.thingIs == "ButtonDebugZot") {
				writeCommandQueued("&");
				writeCommandQueued("o");
				writeCommandQueued("0");
				writeCommandQueued("enter");
			} else if (selected.thingIs == "ButtonDebugCycle") {
				TArray<TTuple<FString, FString>> branches = {
					{"Dungeon", "D"},
					{"Temple", "T"},
					{"Lair", "L"},
					{"Swamp", "S"},
					{"Shoals", "A"},
					{"SnakePit", "P"},
					{"SpiderNest", "N"},
					{"SlimePits", "M"},
					{"OrcishMines", "O"},
					{"ElvenHalls", "E"},
					{"Vaults", "V"},
					{"Crypt", "C"},
					{"Tomb", "W"},
					{"Depths", "U"},
					{"Hell", "H"},
					{"Dis", "I"},
					{"Gehenna", "G"},
					{"Cocytus", "X"},
					{"Tartarus", "Y"},
					{"Zot", "Z"},
					{"Abyss", "J"},
					{"Pandemonium", "R"},
					{"Ziggurat", "Q"},
					{"Shop", "1"},
					{"Trove", "2"},
					{"Sewer", "3"},
					{"Ossuary", "4"},
					{"Bailey", "5"},
					{"IceCave", "6"},
					{"Volcano", "7"},
					{"Wizlab", "8"},
					{"Desolation", "9"},
					{"Gauntlet", "!"},
					{"Arena", "\""},
					{"Crucible", "@"},
				};
				writeCommandQueued("&");
				writeCommandQueued("~");
				for (int i = 0; i < branches.Num(); i++) {
					if (currentBranch == branches[i].Key) {
						writeCommandQueued(branches[(i+1) % branches.Num()].Value);
						break;
					}
				}
				writeCommandQueued("enter");
			} else if (selected.thingIs == "ButtonDebugMol") {
				writeCommandQueued("&");
				writeCommandQueued("w");
			} else if (selected.thingIs == "ButtonDebugGodGift") {
				writeCommandQueued("&");
				writeCommandQueued("-");
			} else if (selected.thingIs == "ButtonDebugMaxPiety") {
				writeCommandQueued("&");
				writeCommandQueued("^");
				writeCommandQueued("2");
				writeCommandQueued("0");
				writeCommandQueued("0");
				writeCommandQueued("enter");
				writeCommandQueued("enter");
			} else if (selected.thingIs == "ButtonDebugMinPiety") {
				writeCommandQueued("&");
				writeCommandQueued("^");
				writeCommandQueued("1");
				writeCommandQueued("5");
				writeCommandQueued("enter");
				writeCommandQueued("enter");
			} else if (selected.thingIs == "ButtonDebugAcquire") {
				writeCommandQueued("&");
				writeCommandQueued("a");
			} else if (selected.thingIs == "ButtonDebugPotions") {
				TArray<FString> potionNames = {
					"ambrosia",
					"attraction",
					"berserk rage",
					"brilliance",
					"cancellation",
					"curing",
					"degeneration",
					"enlightenment",
					"experience",
					"haste",
					"heal wounds",
					"invisibility",
					"lignification",
					"magic",
					"might",
					"mutation",
					"resistance"
				};
				for (int i = 0; i < potionNames.Num(); i++) {
					writeCommandQueued("&");
					writeCommandQueued("o");
					writeCommandQueued("!");
					for (int j = 0; j < potionNames[i].Len(); j++) {
						writeCommandQueued(potionNames[i].Mid(j, 1));
					}
					writeCommandQueued("enter");
				}
			} else if (selected.thingIs == "ButtonDebugScrolls") {
				TArray<FString> scrollNames = {
					"acquirement",
					"amnesia",
					"blinking",
					"brand weapon",
					"butterflies",
					"enchant armour",
					"enchant weapon",
					"fear",
					"fog",
					"identify",
					"immolation",
					"noise",
					"poison",
					"revelation",
					"silence",
					"summoning",
					"teleportation",
					"torment",
					"vulnerability"
				};
				for (int i = 0; i < scrollNames.Num(); i++) {
					writeCommandQueued("&");
					writeCommandQueued("o");
					writeCommandQueued("?");
					for (int j = 0; j < scrollNames[i].Len(); j++) {
						writeCommandQueued(scrollNames[i].Mid(j, 1));
					}
					writeCommandQueued("enter");
				}
			} else if (selected.thingIs == "ButtonDebugGold0") {
				writeCommandQueued("&");
				writeCommandQueued("$");
				writeCommandQueued("0");
				writeCommandQueued("enter");
			} else if (selected.thingIs == "ButtonDebugGold1000") {
				writeCommandQueued("&");
				writeCommandQueued("$");
				writeCommandQueued("1");
				writeCommandQueued("0");
				writeCommandQueued("0");
				writeCommandQueued("0");
				writeCommandQueued("enter");
			} else if (selected.thingIs == "ButtonDebugDismiss") {
				writeCommandQueued("&");
				writeCommandQueued("G");
				writeCommandQueued("enter");
			} else if (selected.thingIs == "ButtonDebugShop") {
				writeCommandQueued("&");
				writeCommandQueued("\\");
				writeCommandQueued("*");
				writeCommandQueued("enter");
			} else if (selected.thingIs == "ButtonDebugRegenerate") {
				writeCommandQueued("&");
				writeCommandQueued("R");
				writeCommandQueued("enter");
			} else if (selected.thingIs == "ButtonDebugHeal") {
				writeCommandQueued("&");
				writeCommandQueued("h");
				writeCommandQueued("enter");
			} else if (selected.thingIs == "ButtonDebugGainLevel") {
				writeCommandQueued("&");
				writeCommandQueued("x");
				writeCommandQueued("enter");
			} else if (selected.thingIs == "ButtonDebugResetLevel") {
				writeCommandQueued("&");
				writeCommandQueued("l");
				writeCommandQueued("1");
				writeCommandQueued("enter");
			} else if (selected.thingIs == "ButtonDebugMonster") {
				writeCommandQueued("&");
				writeCommandQueued("m");
				writeCommandQueued("r");
				writeCommandQueued("a");
				writeCommandQueued("n");
				writeCommandQueued("d");
				writeCommandQueued("o");
				writeCommandQueued("m");
				writeCommandQueued("enter");
			} else if (selected.thingIs == "ButtonDebugMonsterMany") {
				int numMonsters = 20;
				for (int i = 0; i < numMonsters; i++) {
					writeCommandQueued("&");
					writeCommandQueued("m");
					writeCommandQueued("r");
					writeCommandQueued("a");
					writeCommandQueued("n");
					writeCommandQueued("d");
					writeCommandQueued("o");
					writeCommandQueued("m");
					writeCommandQueued("enter");
				}
			} else {
				UE_LOG(LogTemp, Warning, TEXT("Unknown debug button: %s"), *selected.thingIs);
			}

		// Choice cancel button
		} else if (selected.thingIs == "ButtonOptionCancel") {
			UE_LOG(LogTemp, Display, TEXT("INPUT - Choice cancel button clicked"));
			writeCommandQueued("escape");
			writeCommandQueued("escape");
			writeCommandQueued("escape");

		// Choice button
		} else if (selected.thingIs.Contains(TEXT("ButtonOption")) && isChoiceOpen) {
			UE_LOG(LogTemp, Display, TEXT("INPUT - Choice button clicked: %s"), *selected.thingIs);
			FString choiceName = selected.thingIs.Replace(TEXT("ButtonOption"), TEXT(""));
			int choiceIndex = FCString::Atoi(*choiceName)-1;
			if (choiceIndex >= 0 && choiceIndex < choiceLetters.Num() && choiceIndex < choiceNames.Num() && choiceNames[choiceIndex].Len() > 0) {
				UE_LOG(LogTemp, Display, TEXT("Writing choice: %s"), *choiceLetters[choiceIndex]);
				for (int i = 0; i < choiceLetters[choiceIndex].Len(); i++) {
					writeCommandQueued(choiceLetters[choiceIndex].Mid(i, 1));
				}
				if (choiceType == "acquirement") {
					writeCommandQueued("y");
					writeCommandQueued("CLEAR");
					writeCommandQueued("ctrl-X");
					writeCommandQueued(">");
					writeCommandQueued(">");
					writeCommandQueued(">");
					writeCommandQueued("escape");
				} else if (choiceType == "divine") {
					writeCommandQueued("r");
					int x = selected.x;
					int y = selected.y;
					if (locForBlink.X != -1 && locForBlink.Y != -1) {
						x = locForBlink.X;
						y = locForBlink.Y;
					}
					int currentX = LOS;
					int currentY = LOS;
					while (currentX != x) {
						if (currentX < x) {
							writeCommandQueued("l");
							currentX++;
						} else {
							writeCommandQueued("h");
							currentX--;
						}
					}
					while (currentY != y) {
						if (currentY < y) {
							writeCommandQueued("j");
							currentY++;
						} else {
							writeCommandQueued("k");
							currentY--;
						}
					}
					writeCommandQueued("enter");
				} else if (choiceType == "amnesia") {
					writeCommandQueued("Y");
				} else if (choiceType == "removal") {
					writeCommandQueued("enter");
					writeCommandQueued("enter");
					writeCommandQueued("CLEARINV");
					writeCommandQueued("i");
					writeCommandQueued(">");
					writeCommandQueued(">");
					writeCommandQueued("escape");
					writeCommandQueued("escape");
				} else if (choiceType == "gozag") {
					writeCommandQueued("enter");
					writeCommandQueued("enter");
					writeCommandQueued("enter");
					writeCommandQueued("enter");
					writeCommandQueued("enter");
					writeCommandQueued("enter");
					writeCommandQueued("enter");
					writeCommandQueued("enter");
				} else if (choiceType != "branding" && choiceType != "cards") {
					writeCommandQueued("enter");
					writeCommandQueued("enter");
				}
			}

		// Change page buttons
		} else if (selected.thingIs == "ButtonSaveRight") {
			savesPage++;
			int perPage = 6;
			int pagesNeeded = FMath::CeilToInt((float)saveNames.Num() / (float)perPage);
			savesPage = FMath::Clamp(savesPage, 0, pagesNeeded-1);
			int startIndex = savesPage * perPage;
			int endIndex = FMath::Min(saveNames.Num(), (savesPage+1) * perPage);
			pagesNeeded = FMath::Max(pagesNeeded, 1);
			UWidgetComponent* WidgetComponentSaves = Cast<UWidgetComponent>(refToSaveActor->GetComponentByClass(UWidgetComponent::StaticClass()));
			if (WidgetComponentSaves != nullptr) {
				UUserWidget* UserWidgetSaves = WidgetComponentSaves->GetUserWidgetObject();
				if (UserWidgetSaves != nullptr) {
					UTextBlock* PageText = Cast<UTextBlock>(UserWidgetSaves->GetWidgetFromName(TEXT("TextSavePage")));
					if (PageText != nullptr) {
						PageText->SetText(FText::FromString(FString::FromInt(savesPage+1) + "/" + FString::FromInt(pagesNeeded)));
					}
					for (int i = 0; i < perPage; i++) {
						FString textName = "TextSave" + FString::FromInt(i+1);
						UTextBlock* SaveText = Cast<UTextBlock>(UserWidgetSaves->GetWidgetFromName(*textName));
						if (SaveText != nullptr) {
							if (startIndex + i < saveNames.Num()) {
								SaveText->SetText(FText::FromString(saveNames[startIndex + i]));
							} else {
								SaveText->SetText(FText::FromString(TEXT("--------------------")));
							}
						}
					}
				}
			}
		
		// Clicking on the name input box
		} else if (selected.thingIs.Contains(TEXT("ButtonEditName")) || selected.thingIs.Contains(TEXT("ButtonEditSeed"))) {
			if (selected.thingIs.Contains(TEXT("ButtonEditName"))) {
				editting = "name";
			} else if (selected.thingIs.Contains(TEXT("ButtonEditSeed"))) {
				editting = "seed";
			}
			UE_LOG(LogTemp, Display, TEXT("INPUT - Edit button clicked: %s"), *selected.thingIs);

			// Initial shift if the current name is empty or the default
			if ((currentName == defaultNameText || currentName == TEXT("")) && editting == "name") {
				isShifted = true;
				shiftLetters();
			} else if ((currentSeed == defaultSeedText || currentSeed == TEXT("")) && editting == "seed") {
				isShifted = true;
				shiftLetters();
			}

			// Open the keyboard if it isn't already
			isKeyboardOpen = true;
			if (refToKeyboardActor != nullptr) {
				if (isKeyboardOpen) {
					UE_LOG(LogTemp, Display, TEXT("Showing keyboard actor"));
					refToKeyboardActor->SetActorHiddenInGame(false);
					refToKeyboardActor->SetActorEnableCollision(true);
				}
			}

		// Keyboard close
		} else if (selected.thingIs.Contains(TEXT("ButtonKeyboardClose"))) {
			UE_LOG(LogTemp, Display, TEXT("INPUT - Keyboard close button clicked"));
			isKeyboardOpen = false;
			if (refToKeyboardActor != nullptr) {
				refToKeyboardActor->SetActorHiddenInGame(true);
				refToKeyboardActor->SetActorEnableCollision(false);
			}

		// Keyboard shift
		} else if (selected.thingIs.Contains(TEXT("ButtonKeyboardShift"))) {
			UE_LOG(LogTemp, Display, TEXT("INPUT - Keyboard shift button clicked"));
			isShifted = !isShifted;
			shiftLetters();

		// Keyboard keys
		} else if (selected.thingIs.Contains(TEXT("ButtonKeyboard"))) {
			UE_LOG(LogTemp, Display, TEXT("INPUT - Keyboard key clicked: %s"), *selected.thingIs);

			// Get the letter
			FString letter = selected.thingIs.Replace(TEXT("ButtonKeyboard"), TEXT(""));
			if (letter == "Space") {
				letter = " ";
			}
			
			// Handle is shifted
			if (letter != "Backspace") {
				if (isShifted) {
					letter = letter.ToUpper();
				} else {
					letter = letter.ToLower();
				}
				if (isShifted) {
					isShifted = false;
					shiftLetters();
				}
			}

			// If it's the default text, for now empty it
			if (editting == "name") {
				currentName = currentName.Replace(*defaultNameText, TEXT(""));
			} else if (editting == "seed") {
				currentSeed = currentSeed.Replace(*defaultSeedText, TEXT(""));
			} else if (editting == "bug") {
				currentBug = currentBug.Replace(*defaultBugText, TEXT(""));
			}

			// Add this to the corresponding string
			if (letter == "Backspace") {
				if (editting == "name" && currentName.Len() > 0) {
					currentName = currentName.LeftChop(1);
				} else if (editting == "seed" && currentSeed.Len() > 0) {
					currentSeed = currentSeed.LeftChop(1);
				} else if (editting == "bug" && currentBug.Len() > 0) {
					currentBug = currentBug.LeftChop(1);
				}
			} else {
				if (editting == "name") {
					currentName += letter;
				} else if (editting == "seed") {
					currentSeed += letter;
				} else if (editting == "bug") {
					currentBug += letter;
				}
			}

			// If they're empty, reset to the hint text
			if (currentName.Len() == 0 && editting == "name") {
				currentName = defaultNameText;
				isShifted = true;
				shiftLetters();
			}
			if (currentSeed.Len() == 0 && editting == "seed") {
				currentSeed = defaultSeedText;
				isShifted = true;
				shiftLetters();
			}
			if (currentBug.Len() == 0 && editting == "bug") {
				currentBug = defaultBugText;
				isShifted = true;
				shiftLetters();
			}

			// Update the UI element
			if (refToNameActor != nullptr && (editting == "name" || editting == "seed")) { 
				UWidgetComponent* WidgetComponentName = Cast<UWidgetComponent>(refToNameActor->GetComponentByClass(UWidgetComponent::StaticClass()));
				if (WidgetComponentName != nullptr) {
					UUserWidget* UserWidgetName = WidgetComponentName->GetUserWidgetObject();
					if (UserWidgetName != nullptr) {
						if (editting == "name") {
							UTextBlock* NameText = Cast<UTextBlock>(UserWidgetName->GetWidgetFromName(TEXT("EditableName")));
							if (NameText != nullptr) {
								NameText->SetText(FText::FromString(currentName));
							}
						} else if (editting == "seed") {
							UTextBlock* SeedText = Cast<UTextBlock>(UserWidgetName->GetWidgetFromName(TEXT("EditableSeed")));
							if (SeedText != nullptr) {
								SeedText->SetText(FText::FromString(currentSeed));
							}
						}
					}
				}
			} else if (refToSettingsActor != nullptr && editting == "bug") {
				UWidgetComponent* WidgetComponentBug = Cast<UWidgetComponent>(refToSettingsActor->GetComponentByClass(UWidgetComponent::StaticClass()));
				if (WidgetComponentBug != nullptr) {
					UUserWidget* UserWidgetBug = WidgetComponentBug->GetUserWidgetObject();
					if (UserWidgetBug != nullptr) {
						UTextBlock* BugText = Cast<UTextBlock>(UserWidgetBug->GetWidgetFromName(TEXT("EditableBug")));
						if (BugText != nullptr) {
							BugText->SetText(FText::FromString(currentBug));
						}
					}
				}
			}

		// If it's the submit bug button
		} else if (selected.thingIs.Contains(TEXT("ButtonSettingsBug"))) {
			UE_LOG(LogTemp, Display, TEXT("INPUT - Submit bug button clicked"));

			// Make sure it's been long enough
			double currentTime = FPlatformTime::Seconds();
			if (currentTime - lastBugSubmitted > 30.0) {

				// Submit the bug
				submitBug(currentBug);

				// Reset the text
				currentBug = defaultBugText;
				if (refToSettingsActor != nullptr && editting == "bug") {
					UWidgetComponent* WidgetComponentBug = Cast<UWidgetComponent>(refToSettingsActor->GetComponentByClass(UWidgetComponent::StaticClass()));
					if (WidgetComponentBug != nullptr) {
						UUserWidget* UserWidgetBug = WidgetComponentBug->GetUserWidgetObject();
						if (UserWidgetBug != nullptr) {
							UTextBlock* BugText = Cast<UTextBlock>(UserWidgetBug->GetWidgetFromName(TEXT("EditableBug")));
							if (BugText != nullptr) {
								BugText->SetText(FText::FromString(currentBug));
							}
						}
					}
				}

				// Close the settings
				settingsOpen = false;
				if (refToSettingsActor != nullptr) {
					refToSettingsActor->SetActorHiddenInGame(!settingsOpen);
					refToSettingsActor->SetActorEnableCollision(settingsOpen);
				}

				// Close the keyboard
				isKeyboardOpen = false;
				if (refToKeyboardActor != nullptr) {
					refToKeyboardActor->SetActorHiddenInGame(true);
					refToKeyboardActor->SetActorEnableCollision(false);
				}

				// Update the last submitted time
				lastBugSubmitted = currentTime;

			// Otherwise, just log it
			} else {
				UE_LOG(LogTemp, Display, TEXT("INPUT - Bug already submitted recently: %.2f seconds ago"), currentTime - lastBugSubmitted);
			}

		// If it's the edit bug button
		} else if (selected.thingIs.Contains(TEXT("ButtonEditBug"))) {
			editting = "bug";
			UE_LOG(LogTemp, Display, TEXT("INPUT - Edit bug button clicked"));
			if (currentBug == defaultBugText || currentBug == TEXT("")) {
				isShifted = true;
				shiftLetters();
			}
			isKeyboardOpen = true;
			if (refToKeyboardActor != nullptr) {
				if (isKeyboardOpen) {
					UE_LOG(LogTemp, Display, TEXT("Showing keyboard actor"));
					refToKeyboardActor->SetActorHiddenInGame(false);
					refToKeyboardActor->SetActorEnableCollision(true);
				}
			}

		// If it's the altar cancel button
		} else if (selected.thingIs.Contains(TEXT("ButtonAltarCancel"))) {
			UE_LOG(LogTemp, Display, TEXT("INPUT - Altar cancel button clicked"));
			if (refToAltarActor != nullptr && showingAltar) {
				showingAltar = false;
				refToAltarActor->SetActorHiddenInGame(true);
				refToAltarActor->SetActorEnableCollision(false);
			}

		// If it's the altar pray button
		} else if (selected.thingIs.Contains(TEXT("ButtonAltarPray"))) {
			showNextAltar = false;
			UE_LOG(LogTemp, Display, TEXT("INPUT - Altar pray button clicked"));
			writeCommandQueued(">");
			writeCommandQueued("enter");
			writeCommandQueued("enter");
			writeCommandQueued("enter");
			if (refToAltarActor != nullptr && showingAltar) {
				showingAltar = false;
				refToAltarActor->SetActorHiddenInGame(true);
				refToAltarActor->SetActorEnableCollision(false);
			}

		// If it's an altar button
		} else if (selected.thingIs.Contains(TEXT("ButtonAltar"))) {
			UE_LOG(LogTemp, Display, TEXT("INPUT - Altar button clicked: %s"), *selected.thingIs);

			// Which text to show
			FString textToChangeTo = altarOverview;
			if (selected.thingIs.Contains(TEXT("Power"))) {
				textToChangeTo = altarPowers;
			} else if (selected.thingIs.Contains(TEXT("Wrath"))) {
				textToChangeTo = altarWrath;
			}

			// Show the altar actor and set the text
			showNextAltar = true;
			if (refToAltarActor != nullptr && showingAltar) {
				UWidgetComponent* WidgetComponentAltar = Cast<UWidgetComponent>(refToAltarActor->GetComponentByClass(UWidgetComponent::StaticClass()));
				if (WidgetComponentAltar != nullptr) {
					UUserWidget* UserWidgetAltar = WidgetComponentAltar->GetUserWidgetObject();
					if (UserWidgetAltar != nullptr) {
						UTextBlock* AltarText = Cast<UTextBlock>(UserWidgetAltar->GetWidgetFromName(TEXT("TextAltar")));
						if (AltarText != nullptr) {
							AltarText->SetText(FText::FromString(textToChangeTo));
						}
					}
				}
			}

		// If it's an altar
		} else if (thingBeingDragged.thingIs.Contains(TEXT("Altar")) || selected.thingIs.Contains(TEXT("Altar"))) {
			UE_LOG(LogTemp, Display, TEXT("INPUT - Altar clicked: (%d, %d)"), thingBeingDragged.x, thingBeingDragged.y);
			showNextAltar = true;
			if ((thingBeingDragged.x == LOS && thingBeingDragged.y == LOS && thingBeingDragged.thingIs.Contains(TEXT("Altar"))) ||
				(selected.x == LOS && selected.y == LOS && selected.thingIs.Contains(TEXT("Altar")))) {
				writeCommandQueued(">");
				writeCommandQueued("enter");
				writeCommandQueued("!");
				writeCommandQueued("!");
				writeCommandQueued("escape");
				nextAltar = 0;
			}

		// If it's a gate
		} else if (thingBeingDragged.thingIs.Contains(TEXT("Gate")) || selected.thingIs.Contains(TEXT("Gate"))) {
			UE_LOG(LogTemp, Display, TEXT("INPUT - Gate clicked: (%d, %d)"), thingBeingDragged.x, thingBeingDragged.y);
			if ((thingBeingDragged.x == LOS && thingBeingDragged.y == LOS && thingBeingDragged.thingIs.Contains(TEXT("Gate"))) ||
				(selected.x == LOS && selected.y == LOS && selected.thingIs.Contains(TEXT("Gate")))) {
				writeCommandQueued(">");
				writeCommandQueued("Y");
				writeCommandQueued("enter");
				writeCommandQueued("&");
				writeCommandQueued("{");
				writeCommandQueued("enter");
			}

		// Shop buy button
		} else if (selected.thingIs.Contains(TEXT("ButtonShopBuy"))) {
			UE_LOG(LogTemp, Display, TEXT("INPUT - Shop buy button clicked"));
			showNextShop = false;
			writeCommandQueued(">");
			for (int i = 0; i < shopItems.Num(); i++) {
				if (shopItems[i].selected) {
					writeCommandQueued(shopItems[i].letter);
				}
			}
			writeCommandQueued("enter");
			writeCommandQueued("y");
			writeCommandQueued("escape");
			writeCommandQueued("CLEARINV");
			writeCommandQueued("i");
			writeCommandQueued(">");
			writeCommandQueued(">");
			writeCommandQueued("escape");

			// Trigger the cancel button
			if (refToShopActor != nullptr && showingShop) {
				showingShop = false;
				refToShopActor->SetActorHiddenInGame(true);
				refToShopActor->SetActorEnableCollision(false);
			}
			FString meshName = TEXT("");
			for (auto& Elem : meshNameToThing) {
				if (Elem.Value.thingIs.Contains(TEXT("Shop")) && Elem.Value.x == LOS && Elem.Value.y == LOS) {
					meshName = Elem.Key;
					break;
				}
			}
			if (meshName.Len() > 0) {
				UE_LOG(LogTemp, Display, TEXT("Showing shop mesh %s"), *meshName);
				for (int i=0; i<itemArray.Num(); i++) {
					if (itemArray[i]->GetName() == meshName) {
						itemArray[i]->SetActorHiddenInGame(false);
						itemArray[i]->SetActorEnableCollision(true);
						break;
					}
				}
			}

		// Shop cancel button
		} else if (selected.thingIs.Contains(TEXT("ButtonShopCancel"))) {
			UE_LOG(LogTemp, Display, TEXT("INPUT - Shop cancel button clicked"));
			if (refToShopActor != nullptr && showingShop) {
				showingShop = false;
				refToShopActor->SetActorHiddenInGame(true);
				refToShopActor->SetActorEnableCollision(false);
			}
			FString meshName = TEXT("");
			for (auto& Elem : meshNameToThing) {
				if (Elem.Value.thingIs.Contains(TEXT("Shop")) && Elem.Value.x == LOS && Elem.Value.y == LOS) {
					meshName = Elem.Key;
					break;
				}
			}
			if (meshName.Len() > 0) {
				UE_LOG(LogTemp, Display, TEXT("Showing shop mesh %s"), *meshName);
				for (int i=0; i<itemArray.Num(); i++) {
					if (itemArray[i]->GetName() == meshName) {
						itemArray[i]->SetActorHiddenInGame(false);
						itemArray[i]->SetActorEnableCollision(true);
						break;
					}
				}
			}

		// Shop item button
		} else if (selected.thingIs.Contains(TEXT("ButtonShop"))) {
			UE_LOG(LogTemp, Display, TEXT("INPUT - Shop item clicked: %s"), *selected.thingIs);

			// Change item's selected state
			int itemIndex = FCString::Atoi(*selected.thingIs.Replace(TEXT("ButtonShop"), TEXT("")))-1;
			if (itemIndex >= 0 && itemIndex < shopItems.Num()) {
				shopItems[itemIndex].selected = !shopItems[itemIndex].selected;

				// Get the widget component
				UWidgetComponent* WidgetComponent = Cast<UWidgetComponent>(refToShopActor->GetComponentByClass(UWidgetComponent::StaticClass()));
				if (WidgetComponent != nullptr) {

					// If it's selected, the text should be green
					UUserWidget* UserWidget = WidgetComponent->GetUserWidgetObject();
					if (UserWidget != nullptr) {
						FString textName = "TextShop" + FString::FromInt(itemIndex+1);
						UTextBlock* ItemText = Cast<UTextBlock>(UserWidget->GetWidgetFromName(*textName));
						if (ItemText != nullptr) {
							if (shopItems[itemIndex].selected) {
								ItemText->SetColorAndOpacity(FLinearColor(0.0f, 1.0f, 0.0f, 1.0f));
							} else {
								ItemText->SetColorAndOpacity(FLinearColor(1.0f, 1.0f, 1.0f, 1.0f));
							}
						}
					}

					// Set the total cost
					int totalCost = 0;
					UTextBlock* ShopTotal = Cast<UTextBlock>(UserWidget->GetWidgetFromName(TEXT("TextShopCost")));
					if (ShopTotal != nullptr) {
						for (int i = 0; i < shopItems.Num(); i++) {
							if (shopItems[i].selected) {
								totalCost += shopItems[i].price;
							}
						}
						ShopTotal->SetText(FText::FromString(FString::Printf(TEXT("Total: %d gold"), totalCost)));
					}

				}

			}
		
		// If it's an shop
		} else if (thingBeingDragged.thingIs.Contains(TEXT("Shop")) || selected.thingIs.Contains(TEXT("Shop"))) {
			UE_LOG(LogTemp, Display, TEXT("INPUT - Shop clicked: (%d, %d)"), thingBeingDragged.x, thingBeingDragged.y);
			showNextShop = true;
			if (thingBeingDragged.x == LOS && thingBeingDragged.y == LOS) {
				writeCommandQueued(">");
				writeCommandQueued("escape");
			}

		// Volume buttons
		} else if (selected.thingIs.Contains(TEXT("ButtonMusicVolume"))) {
			UE_LOG(LogTemp, Display, TEXT("INPUT - Music volume button clicked: %s"), *selected.thingIs);

			// Make sure the audio component exists
			UAudioComponent* musicComponent = refToMusicActor->GetAudioComponent();
			if (musicComponent != nullptr) {

				// Get the current volume
				float currentVolume = musicComponent->VolumeMultiplier;
				if (selected.thingIs.Contains(TEXT("Up"))) {
					currentVolume += 0.1f;
				} else {
					currentVolume -= 0.1f;
				}
				currentVolume = FMath::Clamp(currentVolume, 0.0f, 1.0f);
				musicComponent->SetVolumeMultiplier(currentVolume);
				UE_LOG(LogTemp, Display, TEXT("INPUT - Volume set to %f"), currentVolume);

				// Change the text (TextVolume)
				UWidgetComponent* WidgetComponent = Cast<UWidgetComponent>(refToSettingsActor->GetComponentByClass(UWidgetComponent::StaticClass()));
				if (WidgetComponent != nullptr) {
					UUserWidget* UserWidget = WidgetComponent->GetUserWidgetObject();
					if (UserWidget != nullptr) {
						UTextBlock* VolumeText = Cast<UTextBlock>(UserWidget->GetWidgetFromName(TEXT("TextMusicVolume")));
						if (VolumeText != nullptr) {
							int asPercent = FMath::RoundToInt(currentVolume*100.0);
							VolumeText->SetText(FText::FromString("Music: " + FString::FromInt(asPercent) + "%"));
						}
					}
				}

				// Save everything
				saveEverything();
			
			}

		// Change snap buttons
		} else if (selected.thingIs.Contains(TEXT("ButtonSnap"))) {
			UE_LOG(LogTemp, Display, TEXT("INPUT - Snap button clicked: %s"), *selected.thingIs);

			// Adjust the value
			if (selected.thingIs.Contains(TEXT("Next"))) {
				snapDegrees += 5;
			} else {
				snapDegrees -= 5;
			}
			snapDegrees = FMath::Clamp(snapDegrees, 5, 180);

			// Set the text
			UWidgetComponent* WidgetComponent = Cast<UWidgetComponent>(refToSettingsActor->GetComponentByClass(UWidgetComponent::StaticClass()));
			if (WidgetComponent != nullptr) {
				UUserWidget* UserWidget = WidgetComponent->GetUserWidgetObject();
				if (UserWidget != nullptr) {
					UTextBlock* SnapText = Cast<UTextBlock>(UserWidget->GetWidgetFromName(TEXT("TextSnapAngle")));
					if (SnapText != nullptr) {
						SnapText->SetText(FText::FromString("Snap: " + FString::FromInt(snapDegrees) + " deg"));
					}
				}
			}

			// Save everything
			saveEverything();

		// Change track buttons
		} else if (selected.thingIs.Contains(TEXT("ButtonTrack"))) {
			UE_LOG(LogTemp, Display, TEXT("INPUT - Music track button clicked: %s"), *selected.thingIs);
			UAudioComponent* musicComponent = refToMusicActor->GetAudioComponent();

			// Adjust the current track index
			if (musicComponent != nullptr) {
				if (selected.thingIs.Contains(TEXT("Next"))) {
					trackInd++;
				} else {
					trackInd--;
				}
			}
			if (trackInd < 0) {
				trackInd = musicList.Num() - 1;
			} else if (trackInd >= musicList.Num()) {
				trackInd = 0;
			}

			// Update the audio and the text
			musicComponent->SetSound(musicList[trackInd]);
			UWidgetComponent* WidgetComponent = Cast<UWidgetComponent>(refToSettingsActor->GetComponentByClass(UWidgetComponent::StaticClass()));
			if (WidgetComponent != nullptr) {
				UUserWidget* UserWidget = WidgetComponent->GetUserWidgetObject();
				if (UserWidget != nullptr) {
					UTextBlock* TrackText = Cast<UTextBlock>(UserWidget->GetWidgetFromName(TEXT("TextTrack")));
					if (TrackText != nullptr) {
						TrackText->SetText(FText::FromString("Track: " + musicListNames[trackInd]));
					}
				}
			}

			// Save everything
			saveEverything();

		// A save page button
		} else if (selected.thingIs == "ButtonSaveLeft") {
			savesPage--;
			savesPage = FMath::Max(savesPage, 0);
			int perPage = 6;
			int startIndex = savesPage * perPage;
			int endIndex = FMath::Min(saveNames.Num(), (savesPage+1) * perPage);
			int pagesNeeded = FMath::CeilToInt((float)saveNames.Num() / (float)perPage);
			pagesNeeded = FMath::Max(pagesNeeded, 1);
			UWidgetComponent* WidgetComponentSaves = Cast<UWidgetComponent>(refToSaveActor->GetComponentByClass(UWidgetComponent::StaticClass()));
			if (WidgetComponentSaves != nullptr) {
				UUserWidget* UserWidgetSaves = WidgetComponentSaves->GetUserWidgetObject();
				if (UserWidgetSaves != nullptr) {
					UTextBlock* PageText = Cast<UTextBlock>(UserWidgetSaves->GetWidgetFromName(TEXT("TextSavePage")));
					if (PageText != nullptr) {
						PageText->SetText(FText::FromString(FString::FromInt(savesPage+1) + "/" + FString::FromInt(pagesNeeded)));
					}
					for (int i = 0; i < perPage; i++) {
						FString textName = "TextSave" + FString::FromInt(i+1);
						UTextBlock* SaveText = Cast<UTextBlock>(UserWidgetSaves->GetWidgetFromName(*textName));
						if (SaveText != nullptr) {
							if (startIndex + i < saveNames.Num()) {
								SaveText->SetText(FText::FromString(saveNames[startIndex + i]));
							} else {
								SaveText->SetText(FText::FromString(TEXT("--------------------")));
							}
						}
					}
				}
			}

		// Exit game button 
		} else if (selected.thingIs == "ButtonQuit" || selected.thingIs == "ButtonMainQuit") {

			// Send the save commands
			if (!useServer) {
				UE_LOG(LogTemp, Display, TEXT("INPUT - Closing process"));
				writeCommandQueued("escape");
				writeCommandQueued("escape");
				writeCommandQueued("enter");
				writeCommandQueued("enter");
				writeCommandQueued("exit");
				writeCommandQueued("escape");
				FPlatformProcess::Sleep(2.0f);
				FPlatformProcess::TerminateProc(ProcHandle, true);
				FPlatformProcess::ClosePipe(StdInReadHandle, StdInWriteHandle);
				FPlatformProcess::ClosePipe(StdOutReadHandle, StdOutWriteHandle);
			} else {
				writeCommand("enter");
				writeCommand("enter");
				writeCommand("exit");
			}

			// Quit
			TEnumAsByte<EQuitPreference::Type> QuitPreference = EQuitPreference::Quit;
			UKismetSystemLibrary::QuitGame(worldRef, UGameplayStatics::GetPlayerController(worldRef, 0), QuitPreference, true);		

		// If we're holding an item, equip or use it
		} else if (thingBeingDragged.thingIs == "InventoryItem"  && currentDescription.Len() > 0 && thingBeingDragged.x !=	-1  && thingBeingDragged.y != -1) {

			// Get the letter and item name
			FString letter = inventoryLocToLetter[thingBeingDragged.y][thingBeingDragged.x];
			if (letter == TEXT("") || !inventoryLetterToName.Contains(letter)) {
				UE_LOG(LogTemp, Display, TEXT("INPUT - Item not valid"));
				refToDescriptionActor->SetActorHiddenInGame(true);
				thingBeingDragged = SelectedThing();
				return;
			}
			FString itemName = inventoryLetterToName[letter];

			// Determine what type of item it is
			FString useType = "w";
			bool isRing = false;
			bool isScroll = false;
			if (currentDescription.Contains(TEXT("(worn)"))) {
				useType = "t";
			} else if (currentDescription.Contains(TEXT("(weapon)"))) {
				useType = "u";
			} else if (currentDescription.Contains(TEXT(" scroll ")) || currentDescription.Contains(TEXT(" scrolls "))) {
				useType = "r";
				isScroll = true;
			} else if (currentDescription.Contains(TEXT(" potion ")) || currentDescription.Contains(TEXT(" potions "))) {
				useType = "q";
			} else if (currentDescription.Contains(TEXT(" ring "))) {
				isRing = true;
				if (currentDescription.Contains(TEXT("(left hand)")) || currentDescription.Contains(TEXT("(right hand)"))) {
					useType = "r";
				} else {
					useType = "p";
				}
			} else if (currentDescription.Contains(TEXT(" amulet "))) {
				if (currentDescription.Contains(TEXT("(around neck)"))) {
					useType = "r";
				} else {
					useType = "p";
				}
			}
			UE_LOG(LogTemp, Display, TEXT("INPUT - Using item %s with command %s"), *inventoryLetterToName[letter], *useType);

			// If it's a ring, check if we already have two equipped
			FString leftFound = TEXT("");
			FString rightFound = TEXT("");
			if (isRing) {
				for (auto& Elem : inventoryLetterToName) {
					if (Elem.Value.Contains(TEXT(" ring ")) && Elem.Value.Contains(TEXT("(left hand)"))) {
						leftFound = Elem.Value.Replace(TEXT("(left hand)"), TEXT("")).TrimStartAndEnd();
					} else if (Elem.Value.Contains(TEXT(" ring ")) && Elem.Value.Contains(TEXT("(right hand)"))) {
						rightFound = Elem.Value.Replace(TEXT("(right hand)"), TEXT("")).TrimStartAndEnd();
					}
				}
			}
			bool twoUniqueRings = leftFound.Len() > 0 && rightFound.Len() > 0 && leftFound != rightFound;

			// Use the item
			writeCommandQueued("i");
			writeCommandQueued(letter);
			writeCommandQueued(useType);
			if (!isScroll && !(twoUniqueRings && isRing && useType == "p")) {
				writeCommandQueued("escape");
				writeCommandQueued("escape");
				writeCommandQueued("CLEARINV");
				writeCommandQueued("i");
				writeCommandQueued(">");
				writeCommandQueued(">");
				writeCommandQueued(letter);
				writeCommandQueued(">");
				writeCommandQueued(">");
				writeCommandQueued("escape");
				writeCommandQueued("escape");
			} else if (isScroll) {
				justUsedAScroll = true;
				locForBlink = FIntVector2(selected.x, selected.y);
				UE_LOG(LogTemp, Display, TEXT("INPUT - Just used a scroll"));
				UE_LOG(LogTemp, Display, TEXT("INPUT - Blink location: (%d, %d)"), locForBlink.X, locForBlink.Y);
			} else if (isRing) {
				UE_LOG(LogTemp, Display, TEXT("INPUT - Two rings already equipped"));
			}

   			// Reset some things if we used the item
			if (inventoryLocToLetter[thingBeingDragged.y][thingBeingDragged.x] == TEXT("") || useType == "q" || isScroll) {
				refToDescriptionActor->SetActorHiddenInGame(true);
				thingBeingDragged = SelectedThing();
			}

		// If memorizing a spell 
		} else if (thingBeingDragged.thingIs == "Memorizing" && memorizing && thingBeingDragged.thingIndex >= 0 && thingBeingDragged.thingIndex < spellLetters.Num()) {
			UE_LOG(LogTemp, Display, TEXT("INPUT - Memorizing spell: %s"), *selected.thingIs);

			// Get the index
			int spellNum = thingBeingDragged.thingIndex;
			UE_LOG(LogTemp, Display, TEXT("INPUT - Spell index: %d"), spellNum);

			// Check if there's something there
			if (spellNum >= 0 && spellNum < spellLetters.Num()) {
				FString letter = spellLetters[spellNum];
				if (spellLetterToInfo.Contains(letter)) {

					// If it's not on the first page
					int nextPagesNeeded = 0;
					if (letter.Len() > 1) {
						nextPagesNeeded = 2*(letter.Len() - 1);
						letter = letter.Left(1);
					}

					// Write the commands
					writeCommandQueued("M");
					for (int i = 0; i < nextPagesNeeded; i++) {
						writeCommandQueued(">");
					}
					writeCommandQueued(letter);
					writeCommandQueued("Y");

					// Refresh the spell list
					writeCommandQueued("CLEARSPELLS");
					writeCommandQueued("M");
					writeCommandQueued(">");
					writeCommandQueued(">");
					writeCommandQueued(">");
					writeCommandQueued(">");
					writeCommandQueued(">");
					writeCommandQueued("escape");

				}
			}

			// Reset some things
			refToDescriptionActor->SetActorHiddenInGame(true);
			thingBeingDragged = SelectedThing();

		// If we're holding a spell, use it
		} else if (thingBeingDragged.thingIs == "Spell" && (thingBeingDragged.letter.Len() > 0 || (thingBeingDragged.thingIndex >= 0 && thingBeingDragged.thingIndex < spellLetters.Num()))) {
			UE_LOG(LogTemp, Display, TEXT("INPUT - Using spell: %s"), *thingBeingDragged.thingIs);

			// Get the letter
			FString letter = "a";
			if (thingBeingDragged.letter.Len() > 0) {
				letter = thingBeingDragged.letter;
			} else if (thingBeingDragged.thingIndex >= 0 && thingBeingDragged.thingIndex < spellLetters.Num()) {
				letter = spellLetters[thingBeingDragged.thingIndex];
			} else {
				UE_LOG(LogTemp, Display, TEXT("INPUT - Invalid spell letter or index"));
				return;
			}
			UE_LOG(LogTemp, Display, TEXT("INPUT - Spell letter: %s"), *letter);

			// Make sure we're in range
			int x = selected.x;
			int y = selected.y;
			int dist = FMath::RoundToInt(FMath::Sqrt(FMath::Pow(float(x-LOS), 2) + FMath::Pow(float(y-LOS), 2)));
			UE_LOG(LogTemp, Display, TEXT("INPUT - targeting range: %d, distance: %d"), targetingRange, dist);
			if (dist <= targetingRange || targetingRange <= 0) {
				writeCommandQueued("Z");
				writeCommandQueued(letter);
				if (targetingRange >= 1) {
					writeCommandQueued("r");
					int currentX = LOS;
					int currentY = LOS;
					while (currentX != x) {
						if (currentX < x) {
							writeCommandQueued("l");
							currentX++;
						} else {
							writeCommandQueued("h");
							currentX--;
						}
					}
					while (currentY != y) {
						if (currentY < y) {
							writeCommandQueued("j");
							currentY++;
						} else {
							writeCommandQueued("k");
							currentY--;
						}
					}
				}
				writeCommandQueued("enter");
				writeCommandQueued("enter");
				writeCommandQueued("enter");
			}

		// If we're holding an ability, use it
		} else if (thingBeingDragged.thingIs == "Ability" && (thingBeingDragged.letter.Len() > 0 || (thingBeingDragged.thingIndex >= 0 && thingBeingDragged.thingIndex < abilityLetters.Num()))) {

			// Get the letter
			FString letter = "a";
			if (thingBeingDragged.letter.Len() > 0) {
				letter = thingBeingDragged.letter;
			} else if (thingBeingDragged.thingIndex >= 0 && thingBeingDragged.thingIndex < abilityLetters.Num()) {
				letter = abilityLetters[thingBeingDragged.thingIndex];
			} else {
				UE_LOG(LogTemp, Display, TEXT("INPUT - Invalid ability letter or index"));
				return;
			}
			UE_LOG(LogTemp, Display, TEXT("INPUT - Ability letter: %s"), *letter);

			// Some spells abilities have targeting
			if (currentDescription.Contains(TEXT("Divine Exegesis"))) {
				locForBlink = FIntVector2(selected.x, selected.y);
				UE_LOG(LogTemp, Display, TEXT("INPUT - Just used Divine Exegesis, setting blink location to (%d, %d)"), locForBlink.X, locForBlink.Y);
			}

			// In case we need to avoid enter
			bool noEnter = false;
			if (currentDescription.Contains(TEXT("Forget Spell"))
				|| currentDescription.Contains(TEXT("Divine Exegesis"))
				|| currentDescription.Contains(TEXT("Curse Item"))
				|| currentDescription.Contains(TEXT("Ancestor Life"))
				|| currentDescription.Contains(TEXT("Triple Draw"))
				|| currentDescription.Contains(TEXT("Deal Four"))
				|| currentDescription.Contains(TEXT("Stack Five"))
				|| currentDescription.Contains(TEXT("Receive Forbidden Knowledge"))
				|| currentDescription.Contains(TEXT("Sacrifice "))
				|| currentDescription.Contains(TEXT("Shatter the Chains"))
				|| currentDescription.Contains(TEXT("Renounce Religion"))
			) {
				UE_LOG(LogTemp, Display, TEXT("INPUT - Ignoring enter for ability"));
				noEnter = true;
			}

			// Make sure we're in range
			int x = selected.x;
			int y = selected.y;
			int dist = FMath::RoundToInt(FMath::Sqrt(FMath::Pow(float(x-LOS), 2) + FMath::Pow(float(y-LOS), 2)));
			UE_LOG(LogTemp, Display, TEXT("INPUT - targeting range: %d, distance: %d"), targetingRange, dist);
			if (dist < targetingRange || targetingRange <= 0) {
				writeCommandQueued("a");
				writeCommandQueued(letter);
				if (targetingRange >= 1) {
					writeCommandQueued("r");
					int currentX = LOS;
					int currentY = LOS;
					while (currentX != x) {
						if (currentX < x) {
							writeCommandQueued("l");
							currentX++;
						} else {
							writeCommandQueued("h");
							currentX--;
						}
					}
					while (currentY != y) {
						if (currentY < y) {
							writeCommandQueued("j");
							currentY++;
						} else {
							writeCommandQueued("k");
							currentY--;
						}
					}
				}
				if (!noEnter) {
					writeCommandQueued("enter");
					writeCommandQueued("enter");
					writeCommandQueued("enter");
				}
			}

		// If trying to show/hide the inventory
		} else if (selected.thingIs == "OpenButton") {
			toggleInventory();

		// If the settings open button
		} else if (selected.thingIs == "ButtonSettings") {
			settingsOpen = !settingsOpen;
			if (refToSettingsActor != nullptr) {
				refToSettingsActor->SetActorHiddenInGame(!settingsOpen);
				refToSettingsActor->SetActorEnableCollision(settingsOpen);
			}

		// If the settings close button
		} else if (selected.thingIs == "ButtonSettingsClose") {
			settingsOpen = false;
			if (refToSettingsActor != nullptr) {
				refToSettingsActor->SetActorHiddenInGame(!settingsOpen);
				refToSettingsActor->SetActorEnableCollision(settingsOpen);
			}

		// If it's the main play button
		} else if (selected.thingIs == "ButtonMainPlay") {

			// Don't allow until loaded
			if (!crawlHasStarted) {
				UE_LOG(LogTemp, Display, TEXT("INPUT - Main menu play button clicked, but not loaded yet"));
				return;
			}

			// Get the main menu widget
			UWidgetComponent* WidgetComponentMain = Cast<UWidgetComponent>(refToMainMenuActor->GetComponentByClass(UWidgetComponent::StaticClass()));
			if (WidgetComponentMain != nullptr) {
				UUserWidget* UserWidget = WidgetComponentMain->GetUserWidgetObject();
				if (UserWidget != nullptr) {
					UE_LOG(LogTemp, Display, TEXT("INPUT - Main menu play clicked"));
					refToSaveActor->SetActorHiddenInGame(false);
					refToSaveActor->SetActorEnableCollision(true);
					refToMainMenuActor->SetActorHiddenInGame(true);
					refToMainMenuActor->SetActorEnableCollision(false);
					refToMainInfoActor->SetActorHiddenInGame(true);
					refToMainInfoActor->SetActorEnableCollision(false);
					
					// Populate the save list
					savesPage = 0;
					int perPage = 6;
					int startIndex = savesPage * perPage;
					int endIndex = FMath::Min(saveNames.Num(), (savesPage+1) * perPage);
					int pagesNeeded = FMath::CeilToInt((float)saveNames.Num() / (float)perPage);
					pagesNeeded = FMath::Max(pagesNeeded, 1);
					UWidgetComponent* WidgetComponentSaves = Cast<UWidgetComponent>(refToSaveActor->GetComponentByClass(UWidgetComponent::StaticClass()));
					if (WidgetComponentSaves != nullptr) {
						UUserWidget* UserWidgetSaves = WidgetComponentSaves->GetUserWidgetObject();
						if (UserWidgetSaves != nullptr) {
							UTextBlock* PageText = Cast<UTextBlock>(UserWidgetSaves->GetWidgetFromName(TEXT("TextSavePage")));
							if (PageText != nullptr) {
								PageText->SetText(FText::FromString(FString::FromInt(savesPage+1) + "/" + FString::FromInt(pagesNeeded)));
							}
							for (int i = 0; i < perPage; i++) {
								FString textName = "TextSave" + FString::FromInt(i+1);
								UTextBlock* SaveText = Cast<UTextBlock>(UserWidgetSaves->GetWidgetFromName(*textName));
								if (SaveText != nullptr) {
									if (startIndex + i < saveNames.Num()) {
										SaveText->SetText(FText::FromString(saveNames[startIndex + i]));
									} else {
										SaveText->SetText(FText::FromString(TEXT("--------------------")));
									}
								}
								
							}
						}
					}

				}
			}

		// If it's something in the save select
		} else if (selected.thingIs == "ButtonSaveBack" || selected.thingIs == "ButtonNewGame" || selected.thingIs.Contains(TEXT("ButtonSave")) || selected.thingIs.Contains(TEXT("ButtonDelete"))) {
			UWidgetComponent* WidgetComponentSaves = Cast<UWidgetComponent>(refToSaveActor->GetComponentByClass(UWidgetComponent::StaticClass()));
			if (WidgetComponentSaves != nullptr) {
				UUserWidget* UserWidget = WidgetComponentSaves->GetUserWidgetObject();
				if (UserWidget != nullptr) {

					// For each of the buttons
					if (selected.thingIs == "ButtonSaveBack") {
						UE_LOG(LogTemp, Display, TEXT("Saves back button clicked"));
						refToSaveActor->SetActorHiddenInGame(true);
						refToSaveActor->SetActorEnableCollision(false);
						refToMainMenuActor->SetActorHiddenInGame(false);
						refToMainMenuActor->SetActorEnableCollision(true);
						refToMainInfoActor->SetActorHiddenInGame(false);
						refToMainInfoActor->SetActorEnableCollision(true);
					} else if (selected.thingIs.Contains(TEXT("ButtonNewGame"))) {
						refToBackgroundActor->SetActorHiddenInGame(false);
						refToBackgroundActor->SetActorEnableCollision(true);
						refToSpeciesActor->SetActorHiddenInGame(false);
						refToSpeciesActor->SetActorEnableCollision(true);
						refToNameActor->SetActorHiddenInGame(false);
						refToNameActor->SetActorEnableCollision(true);
						refToSaveActor->SetActorHiddenInGame(true);
						refToSaveActor->SetActorEnableCollision(false);
					} else if (selected.thingIs.Contains(TEXT("ButtonSave"))) {

						// Get the save info
						FString saveName = selected.thingIs.Replace(TEXT("ButtonSave"), TEXT(""));
						int saveLoc = FCString::Atoi(*saveName)-1 + savesPage*6;
						if (saveLoc < 0 || saveLoc >= saveNames.Num() || !saveLocToIndex.Contains(saveLoc)) {
							UE_LOG(LogTemp, Warning, TEXT("INPUT - Save location out of bounds: %i"), saveLoc);
							return;
						}
						int saveIndex = saveLocToIndex[saveLoc];
						UE_LOG(LogTemp, Display, TEXT("INPUT - Save button clicked: %s"), *saveName);

						// Send the commands
						UE_LOG(LogTemp, Display, TEXT("INPUT - Writing %i down commands"), 9+saveIndex);
						for (int i=0; i<9+saveIndex; i++) {
							writeCommandQueued("down");
						}
						writeCommandQueued("enter");
						writeCommandQueued("enter");
						writeCommandQueued("enter");

						// Load the VR save game
						saveFile = saveNames[saveLoc];
						saveFile = saveFile.Left(saveFile.Find(TEXT(","))).Replace(TEXT(" "), TEXT(""));
						saveGame = Cast<UDCSSSaveGame>(UGameplayStatics::LoadGameFromSlot(saveFile, 0));
						if (saveGame != nullptr) {
							UE_LOG(LogTemp, Display, TEXT("INPUT - Save file found, loading: %s"), *saveFile);
							loadEverything();
						} else {
							UE_LOG(LogTemp, Display, TEXT("INPUT - Save file not found, creating: %s"), *saveFile);
							saveGame = Cast<UDCSSSaveGame>(UGameplayStatics::CreateSaveGameObject(UDCSSSaveGame::StaticClass()));
							saveEverything();
						}

						// Set up the game
						refToUIActor->SetActorHiddenInGame(false);
						refToUIActor->SetActorEnableCollision(true);
						refToSaveActor->SetActorHiddenInGame(true);
						refToSaveActor->SetActorEnableCollision(false);
						refToTutorialActor->SetActorHiddenInGame(false);
						refToTutorialActor->SetActorEnableCollision(true);
						hasBeenWelcomed = false;

					} else if (selected.thingIs.Contains(TEXT("ButtonDelete"))) {

						// Get the location of the button and save
						FString saveName = selected.thingIs.Replace(TEXT("ButtonDelete"), TEXT(""));
						int buttonLoc = FCString::Atoi(*saveName)-1;
						int saveLoc = buttonLoc + savesPage*6;
						FString buttonName = "ButtonDeleteImage" + FString::FromInt(buttonLoc+1);
						UImage* ButtonImage = Cast<UImage>(UserWidget->GetWidgetFromName(*buttonName));

						// Make sure the button exists
						if (ButtonImage != nullptr && saveLoc >= 0 && saveLoc < saveNames.Num()) {

							// If we're already at the confirmation stage
							if (buttonConfirming == buttonLoc) {

								// Remove the file if it exists
								UE_LOG(LogTemp, Display, TEXT("INPUT - Delete button clicked: %i"), saveLoc);
								FString saveFileToDelete = saveNames[saveLoc];
								saveFileToDelete = saveFileToDelete.Left(saveFileToDelete.Find(TEXT(","))).Replace(TEXT(" "), TEXT(""));
								FString savePath = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir() + TEXT("\\Content\\DCSS\\saves\\")) + saveFileToDelete + TEXT(".cs");
								UE_LOG(LogTemp, Display, TEXT("INPUT - Attempting to delete save file: %s"), *savePath);
								if(savePath.Len() > 0) {
									if (FPaths::ValidatePath(savePath) && FPaths::FileExists(savePath)) {
										UE_LOG(LogTemp, Display, TEXT("Deleting save file: %s"), *savePath);
										IFileManager& FileManager = IFileManager::Get(); FileManager.Delete(*savePath);
									}
								}

								// Remove from the list and refresh
								saveNames.RemoveAt(saveLoc);
								saveLocToIndex.RemoveAt(saveLoc);
								selected.thingIs = "ButtonMainPlay";
								keyPressed("lmb", FVector2D(0.0f, 0.0f));

								// Reset the button
								UTexture2D* tex2D = getTexture("Delete");
								if (tex2D != nullptr && ButtonImage != nullptr) {
									ButtonImage->SetBrushFromTexture(tex2D);
								}
								buttonConfirming = -1;

							// If we need to change to the confirmation stage
							} else {
								buttonConfirming = buttonLoc;
								UTexture2D* tex2D = getTexture("AreYouSure");
								if (tex2D != nullptr) {
									ButtonImage->SetBrushFromTexture(tex2D);
								} else {
									UE_LOG(LogTemp, Warning, TEXT("INPUT - Couldn't find texture for confirmation"));
								}

							}

						}

					}

				}
			}

		// If it's the close tutorial button
		} else if (selected.thingIs == "ButtonCloseTutorial") {
			UE_LOG(LogTemp, Display, TEXT("INPUT - Tutorial close button clicked"));
			refToTutorialActor->SetActorHiddenInGame(true);
			refToTutorialActor->SetActorEnableCollision(false);

		// If it's something in the species select
		} else if (selected.thingIs.Contains(TEXT("ButtonBackground"))) {
			UWidgetComponent* WidgetComponentBackground = Cast<UWidgetComponent>(refToBackgroundActor->GetComponentByClass(UWidgetComponent::StaticClass()));
			if (WidgetComponentBackground != nullptr) {
				UUserWidget* UserWidget = WidgetComponentBackground->GetUserWidgetObject();
				if (UserWidget != nullptr) {
					UWidgetComponent* WidgetComponentName = Cast<UWidgetComponent>(refToNameActor->GetComponentByClass(UWidgetComponent::StaticClass()));
					if (WidgetComponentName != nullptr) {
						UUserWidget* UserWidgetName = WidgetComponentName->GetUserWidgetObject();
						if (UserWidgetName != nullptr) {
							UTextBlock* NameText = Cast<UTextBlock>(UserWidgetName->GetWidgetFromName(TEXT("TextBackground")));
							if (NameText != nullptr) {

								// Remove the highlight from the previous
								FString borderNameOld = "Border" + currentBackground;
								UBorder* borderOld = Cast<UBorder>(UserWidget->GetWidgetFromName(*borderNameOld));
								if (borderOld != nullptr) {
									borderOld->SetBrushColor(FLinearColor(0.04f, 0.04f, 0.04f, 0.0f));
								}

								// Get the name
								FString backgroundName = selected.thingIs.Replace(TEXT("ButtonBackground"), TEXT(""));
								UE_LOG(LogTemp, Display, TEXT("INPUT - Background button clicked: %s"), *backgroundName);
								currentBackground = backgroundName;

								// Add the spaces back in
								FString backgroundRespaced = currentBackground;
								for (int i = 0; i < backgroundRespaced.Len(); i++) {
									if (backgroundRespaced[i] >= 'A' && backgroundRespaced[i] <= 'Z') {
										backgroundRespaced = backgroundRespaced.Left(i) + " " + backgroundRespaced.Mid(i, backgroundRespaced.Len() - i);
										i++;
									}
								}
								backgroundRespaced = backgroundRespaced.TrimStartAndEnd();

								// Set the background name text
								NameText->SetText(FText::FromString(backgroundRespaced));

								// Add a highlight
								FString borderName = "Border" + currentBackground;
								UBorder* border = Cast<UBorder>(UserWidget->GetWidgetFromName(*borderName));
								if (border != nullptr) {
									border->SetBrushColor(FLinearColor(0.04f, 0.04f, 0.04f, 1.0f));
								}

							}
						}
					}
				}
			}

		// If it's something in the species select
		} else if (selected.thingIs.Contains(TEXT("ButtonSpecies"))) {
			UWidgetComponent* WidgetComponentSpecies = Cast<UWidgetComponent>(refToSpeciesActor->GetComponentByClass(UWidgetComponent::StaticClass()));
			if (WidgetComponentSpecies != nullptr) {
				UUserWidget* UserWidget = WidgetComponentSpecies->GetUserWidgetObject();
				if (UserWidget != nullptr) {
					UWidgetComponent* WidgetComponentName = Cast<UWidgetComponent>(refToNameActor->GetComponentByClass(UWidgetComponent::StaticClass()));
					if (WidgetComponentName != nullptr) {
						UUserWidget* UserWidgetName = WidgetComponentName->GetUserWidgetObject();
						if (UserWidgetName != nullptr) {
							UTextBlock* NameText = Cast<UTextBlock>(UserWidgetName->GetWidgetFromName(TEXT("TextSpecies")));
							if (NameText != nullptr) {

								// Remove the highlight from the previous
								FString borderNameOld = "Border" + currentSpecies;
								UBorder* borderOld = Cast<UBorder>(UserWidget->GetWidgetFromName(*borderNameOld));
								if (borderOld != nullptr) {
									borderOld->SetBrushColor(FLinearColor(0.04f, 0.04f, 0.04f, 0.0f));
								}

								// Get the name
								FString speciesName = selected.thingIs.Replace(TEXT("ButtonSpecies"), TEXT(""));
								UE_LOG(LogTemp, Display, TEXT("INPUT - Species button clicked: %s"), *speciesName);
								currentSpecies = speciesName;

								// Add the spaces back in
								FString speciesRespaced = currentSpecies;
								for (int i = 0; i < speciesRespaced.Len(); i++) {
									if (speciesRespaced[i] >= 'A' && speciesRespaced[i] <= 'Z') {
										speciesRespaced = speciesRespaced.Left(i) + " " + speciesRespaced.Mid(i, speciesRespaced.Len() - i);
										i++;
									}
								}

								// Set the species name text
								NameText->SetText(FText::FromString(speciesRespaced));

								// Add a highlight
								FString borderName = "Border" + currentSpecies;
								UBorder* border = Cast<UBorder>(UserWidget->GetWidgetFromName(*borderName));
								if (border != nullptr) {
									border->SetBrushColor(FLinearColor(0.04f, 0.04f, 0.04f, 1.0f));
								}

							}
						}
					}
				}
			}

		// If it's something in the name select
		} else if (selected.thingIs == "ButtonCharBack" || selected.thingIs == "ButtonBegin") {
			UWidgetComponent* WidgetComponentName = Cast<UWidgetComponent>(refToNameActor->GetComponentByClass(UWidgetComponent::StaticClass()));
			if (WidgetComponentName != nullptr) {
				UUserWidget* UserWidget = WidgetComponentName->GetUserWidgetObject();
				if (UserWidget != nullptr) {

					// For each of the buttons
					if (selected.thingIs == "ButtonCharBack") {
						UE_LOG(LogTemp, Display, TEXT("INPUT - Char back button clicked"));
						refToBackgroundActor->SetActorHiddenInGame(true);
						refToBackgroundActor->SetActorEnableCollision(false);
						refToSpeciesActor->SetActorHiddenInGame(true);
						refToSpeciesActor->SetActorEnableCollision(false);
						refToNameActor->SetActorHiddenInGame(true);
						refToNameActor->SetActorEnableCollision(false);
						refToSaveActor->SetActorHiddenInGame(false);
						refToSaveActor->SetActorEnableCollision(true);
					} else if (selected.thingIs == "ButtonBegin") {

						// Get info
						FString speciesLetter = speciesToLetter[currentSpecies];
						FString backgroundLetter = backgroundToLetter[currentBackground];
						if (currentSpecies == "Demigod") {
							backgroundLetter = backgroundToLetterNoGods[currentBackground];
						}

						// Write the name
						UE_LOG(LogTemp, Display, TEXT("Putting name: %s"), *currentName);
						if (currentName.Len() == 0 || currentName == defaultNameText) {
							writeCommandQueued("*");
						} else {
							for (int i = 0; i < currentName.Len(); i++) {
								writeCommandQueued("backspace");
							}
							for (int i = 0; i < currentName.Len(); i++) {
								writeCommandQueued(currentName.Mid(i, 1));
							}
						}
						
						// Select seed run
						for (int i = 0; i < 11; i++) {
							writeCommandQueued("up");
						}
						writeCommandQueued("down");
						writeCommandQueued("enter");

						// Write the seed
						UE_LOG(LogTemp, Display, TEXT("Putting seed: %s"), *currentSeed);
						for (int i = 0; i < currentSeed.Len(); i++) {
							writeCommandQueued(currentSeed.Mid(i, 1));
						}

						// Start the run
						writeCommandQueued("enter");
						writeCommandQueued(speciesLetter);
						writeCommandQueued(backgroundLetter);

						// Set up the game
						refToSpeciesActor->SetActorHiddenInGame(true);
						refToSpeciesActor->SetActorEnableCollision(false);
						refToNameActor->SetActorHiddenInGame(true);
						refToNameActor->SetActorEnableCollision(false);
						refToBackgroundActor->SetActorHiddenInGame(true);
						refToBackgroundActor->SetActorEnableCollision(false);
						refToUIActor->SetActorHiddenInGame(false);
						refToUIActor->SetActorEnableCollision(true);
						refToKeyboardActor->SetActorHiddenInGame(true);
						refToKeyboardActor->SetActorEnableCollision(false);
						isKeyboardOpen = false;
						hasBeenWelcomed = false;

					}

				}
			}

		// If it's a ladder
		} else if (thingBeingDragged.thingIs.Contains(TEXT("Ladder"))) {
			UE_LOG(LogTemp, Display, TEXT("INPUT - Ladder clicked: (%d, %d) %s"), thingBeingDragged.x, thingBeingDragged.y, *thingBeingDragged.thingIs);
			if (thingBeingDragged.x == LOS && thingBeingDragged.y == LOS && thingBeingDragged.thingIs.Contains(TEXT("Ladder"))) {
				if (thingBeingDragged.thingIs == "LadderUp") {
					writeCommandQueued("<");
					writeCommandQueued("escape");
				} else {
					writeCommandQueued(">");
					writeCommandQueued("escape");
				}
				writeCommandQueued("&");
				writeCommandQueued("{");
				writeCommandQueued("enter");
			}

		// If clicking on a monster and we have a ranged or spell quivered
		} else if (selected.thingIs == "Enemy" && (leftText.Contains(TEXT("Fire:")) || leftText.Contains(TEXT("Cast:")))) {
			UE_LOG(LogTemp, Display, TEXT("INPUT - Enemy clicked whilst holding ranged quivered"));

			// Send the commands to target the clicked enemy
			int x = selected.x;
			int y = selected.y;
			writeCommandQueued("f");
			writeCommandQueued("r");
			int currentX = LOS;
			int currentY = LOS;
			while (currentX != x) {
				if (currentX < x) {
					writeCommandQueued("l");
					currentX++;
				} else {
					writeCommandQueued("h");
					currentX--;
				}
			}
			while (currentY != y) {
				if (currentY < y) {
					writeCommandQueued("j");
					currentY++;
				} else {
					writeCommandQueued("k");
					currentY--;
				}
			}
			writeCommandQueued("enter");

		// Any of the page buttons
		} else if (selected.thingIs.Contains(TEXT("Button")) && (selected.thingIs.Contains(TEXT("Left")) || selected.thingIs.Contains(TEXT("Right")))) {
			UE_LOG(LogTemp, Display, TEXT("INPUT - Page button clicked: %s"), *selected.thingIs);
			bool isLeft = selected.thingIs.Contains(TEXT("Left"));

			// If it's the spells page
			if (selected.thingIs.Contains(TEXT("Spells"))) {
				if (isLeft) {
					spellPage--;
				} else {
					spellPage++;
				}
				shouldRedrawSpells = true;

			// If it's the abilities page
			} else if (selected.thingIs.Contains(TEXT("Abilities"))) {
				if (isLeft) {
					abilityPage--;
				} else {
					abilityPage++;
				}
				shouldRedrawAbilities = true;

			// If it's the passives page
			} else if (selected.thingIs.Contains(TEXT("Passives"))) {
				if (isLeft) {
					passivePage--;
				} else {
					passivePage++;
				}
				shouldRedrawPassives = true;
			}

		// If clicking on an inventory button
		} else if (selected.thingIs.Contains(TEXT("Button")) && inventoryOpen) {

			// If it's something in the inventory panel
			UWidgetComponent* WidgetComponentInventory = Cast<UWidgetComponent>(refToInventoryActor->GetComponentByClass(UWidgetComponent::StaticClass()));
			if (WidgetComponentInventory != nullptr) {
				UUserWidget* UserWidget = WidgetComponentInventory->GetUserWidgetObject();
				if (UserWidget != nullptr) {

					// For each of the top buttons
					if (selected.thingIs == "ButtonInventory") {
						UE_LOG(LogTemp, Display, TEXT("INPUT - Inventory button clicked"));
						writeCommandQueued("CLEARINV");
						writeCommandQueued("i");
						writeCommandQueued(">");
						writeCommandQueued(">");
						writeCommandQueued("escape");
						UWidgetSwitcher* WidgetSwitcher = Cast<UWidgetSwitcher>(UserWidget->GetWidgetFromName(TEXT("WidgetSwitcherTop")));
						if (WidgetSwitcher != nullptr) {
							WidgetSwitcher->SetActiveWidgetIndex(0);
							currentUI = "inventory";
						}
					} else if (selected.thingIs == "ButtonSpells") {
						UE_LOG(LogTemp, Display, TEXT("INPUT - Spells button clicked"));
						writeCommandQueued("CLEARSPELLS");
						writeCommandQueued("I");
						writeCommandQueued(">");
						writeCommandQueued(">");
						writeCommandQueued(">");
						writeCommandQueued(">");
						writeCommandQueued(">");
						writeCommandQueued("escape");
						UWidgetSwitcher* WidgetSwitcher = Cast<UWidgetSwitcher>(UserWidget->GetWidgetFromName(TEXT("WidgetSwitcherTop")));
						if (WidgetSwitcher != nullptr) {
							WidgetSwitcher->SetActiveWidgetIndex(1);
							currentUI = "spells";
						}

						// We aren't memorizing
						memorizing = false;
						UTextBlock* ButtonMemorizeText = Cast<UTextBlock>(UserWidget->GetWidgetFromName(TEXT("TextMemorize")));
						if (ButtonMemorizeText != nullptr) {
							ButtonMemorizeText->SetText(FText::FromString("Memorize"));
						}
						UTextBlock* SpellLevelText = Cast<UTextBlock>(UserWidget->GetWidgetFromName(TEXT("TextSpellLevels")));
						if (SpellLevelText != nullptr) {
							SpellLevelText->SetVisibility(ESlateVisibility::Hidden);
						}

					} else if (selected.thingIs == "ButtonAbilities") {
						UE_LOG(LogTemp, Display, TEXT("INPUT - Abilities button clicked"));
						writeCommandQueued("a");
						writeCommandQueued("escape");
						UWidgetSwitcher* WidgetSwitcher = Cast<UWidgetSwitcher>(UserWidget->GetWidgetFromName(TEXT("WidgetSwitcherTop")));
						if (WidgetSwitcher != nullptr) {
							WidgetSwitcher->SetActiveWidgetIndex(2);
							currentUI = "abilities";
						}
					} else if (selected.thingIs == "ButtonSkills") {
						UE_LOG(LogTemp, Display, TEXT("INPUT - Skills button clicked"));
						writeCommandQueued("m");
						writeCommandQueued("*");
						writeCommandQueued("escape");
						UWidgetSwitcher* WidgetSwitcher = Cast<UWidgetSwitcher>(UserWidget->GetWidgetFromName(TEXT("WidgetSwitcherTop")));
						if (WidgetSwitcher != nullptr) {
							WidgetSwitcher->SetActiveWidgetIndex(3);
							currentUI = "skills";
						}
						UTextBlock* SpellLevelText = Cast<UTextBlock>(UserWidget->GetWidgetFromName(TEXT("TextSpellLevels")));
						if (SpellLevelText != nullptr) {
							SpellLevelText->SetVisibility(ESlateVisibility::Hidden);
						}
					} else if (selected.thingIs == "ButtonCharacter") {
						UE_LOG(LogTemp, Display, TEXT("INPUT - Character button clicked"));
						UWidgetSwitcher* WidgetSwitcher = Cast<UWidgetSwitcher>(UserWidget->GetWidgetFromName(TEXT("WidgetSwitcherTop")));
						if (WidgetSwitcher != nullptr) {
							WidgetSwitcher->SetActiveWidgetIndex(4);
							currentUI = "character";
						}
						UWidgetSwitcher* WidgetSwitcher2 = Cast<UWidgetSwitcher>(UserWidget->GetWidgetFromName(TEXT("WidgetSwitcherCharacter")));
						if (WidgetSwitcher2 != nullptr) {
							if (WidgetSwitcher2->GetActiveWidgetIndex() == 0) {
								currentUI = "overview";
								writeCommandQueued("%");
								writeCommandQueued("escape");
							} else if (WidgetSwitcher2->GetActiveWidgetIndex() == 1) {
								currentUI = "passives";
								writeCommandQueued("A");
								writeCommandQueued("escape");
							} else if (WidgetSwitcher2->GetActiveWidgetIndex() == 2) {
								currentUI = "religion";
								writeCommandQueued("^");
								writeCommandQueued("escape");
							}
						}
					} else if (selected.thingIs == "ButtonMap") {
						UE_LOG(LogTemp, Display, TEXT("INPUT - Map button clicked"));
						UWidgetSwitcher* WidgetSwitcher = Cast<UWidgetSwitcher>(UserWidget->GetWidgetFromName(TEXT("WidgetSwitcherTop")));
						if (WidgetSwitcher != nullptr) {
							WidgetSwitcher->SetActiveWidgetIndex(5);
							currentUI = "map";
						}
					} else if (selected.thingIs == "ButtonMenu") {
						UE_LOG(LogTemp, Display, TEXT("INPUT - Menu button clicked"));
						UWidgetSwitcher* WidgetSwitcher = Cast<UWidgetSwitcher>(UserWidget->GetWidgetFromName(TEXT("WidgetSwitcherTop")));
						if (WidgetSwitcher != nullptr) {
							WidgetSwitcher->SetActiveWidgetIndex(6);
							currentUI = "menu";
						}

					// The sub-buttons in the character menu
					} else if (selected.thingIs == "ButtonOverview") {
						UE_LOG(LogTemp, Display, TEXT("INPUT - Overview button clicked"));
						writeCommandQueued("%");
						writeCommandQueued("escape");
						UWidgetSwitcher* WidgetSwitcher = Cast<UWidgetSwitcher>(UserWidget->GetWidgetFromName(TEXT("WidgetSwitcherCharacter")));
						if (WidgetSwitcher != nullptr) {
							WidgetSwitcher->SetActiveWidgetIndex(0);
							currentUI = "overview";
						}
					} else if (selected.thingIs == "ButtonPassive") {
						UE_LOG(LogTemp, Display, TEXT("INPUT - Passives button clicked"));
						writeCommandQueued("A");
						writeCommandQueued("escape");
						UWidgetSwitcher* WidgetSwitcher = Cast<UWidgetSwitcher>(UserWidget->GetWidgetFromName(TEXT("WidgetSwitcherCharacter")));
						if (WidgetSwitcher != nullptr) {
							WidgetSwitcher->SetActiveWidgetIndex(1);
							currentUI = "passives";
						} 
					} else if (selected.thingIs == "ButtonReligion") {
						UE_LOG(LogTemp, Display, TEXT("INPUT - Religion button clicked"));
						writeCommandQueued("^");
						writeCommandQueued("escape");
						UWidgetSwitcher* WidgetSwitcher = Cast<UWidgetSwitcher>(UserWidget->GetWidgetFromName(TEXT("WidgetSwitcherCharacter")));
						if (WidgetSwitcher != nullptr) {
							WidgetSwitcher->SetActiveWidgetIndex(2);
							currentUI = "religion";
						} 

					// If swapping to/from memorize
					} else if (selected.thingIs == "ButtonMemorize") {
						UE_LOG(LogTemp, Display, TEXT("INPUT - Memorize button clicked"));
						memorizing = !memorizing;

						// Depending on the mode, write the command
						if (memorizing) {
							writeCommandQueued("CLEARSPELLS");
							writeCommandQueued("M");
							writeCommandQueued(">");
							writeCommandQueued(">");
							writeCommandQueued(">");
							writeCommandQueued(">");
							writeCommandQueued(">");
							writeCommandQueued("escape");
						} else {
							writeCommandQueued("CLEARSPELLS");
							writeCommandQueued("I");
							writeCommandQueued(">");
							writeCommandQueued(">");
							writeCommandQueued(">");
							writeCommandQueued(">");
							writeCommandQueued(">");
							writeCommandQueued("escape");
						}

						// Change the text of the button
						UTextBlock* ButtonMemorizeText = Cast<UTextBlock>(UserWidget->GetWidgetFromName(TEXT("TextMemorize")));
						if (ButtonMemorizeText != nullptr) {
							if (!memorizing) {
								ButtonMemorizeText->SetText(FText::FromString("Memorize"));
							} else {
								ButtonMemorizeText->SetText(FText::FromString("Known Spells"));
							}
						}

						// Show the spell level text
						UTextBlock* SpellLevelText = Cast<UTextBlock>(UserWidget->GetWidgetFromName(TEXT("TextSpellLevels")));
						if (SpellLevelText != nullptr) {
							if (memorizing) {
								SpellLevelText->SetVisibility(ESlateVisibility::Visible);
							} else {
								SpellLevelText->SetVisibility(ESlateVisibility::Hidden);
							}
						}

					// The skill buttons
					} else if (selected.thingIs.Contains(TEXT("ButtonSkill"))) {
						UE_LOG(LogTemp, Display, TEXT("INPUT - Skill button %s clicked"), *selected.thingIs);
						FString skillName = selected.thingIs.Replace(TEXT("ButtonSkill"), TEXT(""));
						if (skillNameToInfo.Contains(skillName)) {
							FString letter = skillNameToInfo[skillName].letter;
							writeCommandQueued("m");
							writeCommandQueued("*");
							writeCommandQueued(letter);
							writeCommandQueued("escape");
						} else {
							UE_LOG(LogTemp, Warning, TEXT("INPUT - Skill name not found: %s"), *skillName);
						}
					}

				}
			}

		// Movement based on the mouse
		} else if (!shiftOn) {
			if (selected.x != -1 && selected.y != -1) {
				if (selected.x == LOS && selected.y == LOS) {
					writeCommandQueued("5");
				} else if (selected.x < LOS && selected.y < LOS) {
					writeCommandQueued("y");
				} else if (selected.x > LOS && selected.y < LOS) {
					writeCommandQueued("u");
				} else if (selected.x < LOS && selected.y > LOS) {
					writeCommandQueued("b");
				} else if (selected.x > LOS && selected.y > LOS) {
					writeCommandQueued("n");
				} else if (selected.x < LOS) {
					writeCommandQueued("h");
				} else if (selected.x > LOS) {
					writeCommandQueued("l");
				} else if (selected.y < LOS) {
					writeCommandQueued("k");
				} else if (selected.y > LOS) {
					writeCommandQueued("j");
				}
			}
		}

	} else if (key == "rmb") {
		UE_LOG(LogTemp, Display, TEXT("INPUT - Right click: (%d, %d) %s %d"), selected.x, selected.y, *selected.thingIs, selected.thingIndex);
		rmbOn = true;

		// If dragging the inventory panel
		if (selected.x == -1 && selected.y == -1 && selected.thingIs == TEXT("Inventory") && inventoryOpen) {
			draggingInventory = true;

		// If right clicking on one of the choice slots
		} else if (selected.thingIs.Contains(TEXT("ButtonOption")) && isChoiceOpen) {

			// Get the index and make sure it's valid
			FString choiceIndex = selected.thingIs.Replace(TEXT("ButtonOption"), TEXT(""));
			int choiceNum = FCString::Atoi(*choiceIndex)-1;
			if (choiceNum >= 0 && choiceNum < choiceLetters.Num() && choiceNum < choiceNames.Num()) {
				FString letter = choiceLetters[choiceNum];

				// If it's an acquirement
				if (choiceType == "acquirement") {

					// Write the commands
					writeCommandQueued("!");
					writeCommandQueued(letter);
					writeCommandQueued(">");
					writeCommandQueued("escape");
					writeCommandQueued("!");
					thingBeingDragged = SelectedThing(-1, -1, "Choice", choiceNum, "");

					// Show the description
					currentDescription = TEXT("");
					currentUsage = TEXT("");
					UWidgetComponent* WidgetComponentDesc = Cast<UWidgetComponent>(refToDescriptionActor->GetComponentByClass(UWidgetComponent::StaticClass()));
					if (WidgetComponentDesc != nullptr) {
						UUserWidget* UserWidgetDesc = WidgetComponentDesc->GetUserWidgetObject();
						if (UserWidgetDesc != nullptr) {
							UTextBlock* TextBox = Cast<UTextBlock>(UserWidgetDesc->GetWidgetFromName(TEXT("Description")));
							if (TextBox != nullptr) {
								TextBox->SetText(FText::FromString(currentDescription));
							}
							UTextBlock* UsageBox = Cast<UTextBlock>(UserWidgetDesc->GetWidgetFromName(TEXT("Usage")));
							if (UsageBox != nullptr) {
								UsageBox->SetText(FText::FromString(currentUsage));
							}
						}
					}
					refToDescriptionActor->SetActorHiddenInGame(false);

				}

			}

		// If right clicking on one of the shop slots
		} else if (selected.thingIs.Contains(TEXT("ButtonShop")) && showingShop) {

			// Get the index and make sure it's valid
			FString choiceIndex = selected.thingIs.Replace(TEXT("ButtonShop"), TEXT(""));
			int itemNum = FCString::Atoi(*choiceIndex)-1;
			if (itemNum >= 0 && itemNum < shopItems.Num()) {
				FString letter = shopItems[itemNum].letter;
				UE_LOG(LogTemp, Display, TEXT("INPUT - Right clicking on shop item %s"), *letter);

				// Write the commands
				showNextShop = false;
				writeCommandQueued(">");
				writeCommandQueued("!");
				writeCommandQueued(letter);
				writeCommandQueued(">");
				writeCommandQueued(">");
				writeCommandQueued("escape");
				writeCommandQueued("escape");
				thingBeingDragged = SelectedThing(-1, -1, "Choice", itemNum, "");

				// Show the description
				currentDescription = TEXT("");
				currentUsage = TEXT("");
				UWidgetComponent* WidgetComponentDesc = Cast<UWidgetComponent>(refToDescriptionActor->GetComponentByClass(UWidgetComponent::StaticClass()));
				if (WidgetComponentDesc != nullptr) {
					UUserWidget* UserWidgetDesc = WidgetComponentDesc->GetUserWidgetObject();
					if (UserWidgetDesc != nullptr) {
						UTextBlock* TextBox = Cast<UTextBlock>(UserWidgetDesc->GetWidgetFromName(TEXT("Description")));
						if (TextBox != nullptr) {
							TextBox->SetText(FText::FromString(currentDescription));
						}
						UTextBlock* UsageBox = Cast<UTextBlock>(UserWidgetDesc->GetWidgetFromName(TEXT("Usage")));
						if (UsageBox != nullptr) {
							UsageBox->SetText(FText::FromString(currentUsage));
						}
					}
				}
				refToDescriptionActor->SetActorHiddenInGame(false);

			}

		// If right clicking on one of the equipped slots
		} else if (selected.thingIs.Contains(TEXT("EquippedButton"))) {

			// If there's something there
			if (equippedInfo.name.Len() >= 0) {
				bool found = false;

				// If it's an inventory item
				if (equippedInfo.type == "InventoryItem") {
					writeCommandQueued("i");
					writeCommandQueued(equippedInfo.letter);
					writeCommandQueued(">");
					writeCommandQueued(">");
					writeCommandQueued("escape");
					writeCommandQueued("escape");
					int yLoc = -1;
					int xLoc = -1;
					for (int i = 0; i < inventoryLocToLetter.Num(); i++) {
						for (int j = 0; j < inventoryLocToLetter[i].Num(); j++) {
							if (inventoryLocToLetter[i][j] == equippedInfo.letter) {
								yLoc = i;
								xLoc = j;
								found = true;
								break;
							}
						}
						if (found) {
							break;
						}
					}
					if (found) {
						thingBeingDragged = SelectedThing(xLoc, yLoc, "InventoryItem", 0, "");
					}

				// If it's an ability
				} else if (equippedInfo.type == "Ability") {
					writeCommandQueued("a");
					writeCommandQueued("?");
					writeCommandQueued(equippedInfo.letter);
					writeCommandQueued("escape");
					writeCommandQueued("escape");
					thingBeingDragged = SelectedThing(-1, -1, "Ability", 0, equippedInfo.letter);
					found = true;

				// If it's a spell
				} else if (equippedInfo.type == "Spell") {
					FString letter = equippedInfo.letter;
					int nextPagesNeeded = 0;
					if (letter.Len() > 1) {
						nextPagesNeeded = 2*(letter.Len() - 1);
						letter = letter.Left(1);
					}
					writeCommandQueued("I");
					for (int i = 0; i < nextPagesNeeded; i++) {
						writeCommandQueued(">");
					}
					writeCommandQueued(letter);
					writeCommandQueued("escape");
					writeCommandQueued("escape");
					thingBeingDragged = SelectedThing(-1, -1, "Spell", 0, equippedInfo.letter);
					found = true;

				}

				// Show the description
				if (found) {
					currentDescription = TEXT("");
					currentUsage = TEXT("");
					UWidgetComponent* WidgetComponentDesc = Cast<UWidgetComponent>(refToDescriptionActor->GetComponentByClass(UWidgetComponent::StaticClass()));
					if (WidgetComponentDesc != nullptr) {
						UUserWidget* UserWidgetDesc = WidgetComponentDesc->GetUserWidgetObject();
						if (UserWidgetDesc != nullptr) {
							UTextBlock* TextBox = Cast<UTextBlock>(UserWidgetDesc->GetWidgetFromName(TEXT("Description")));
							if (TextBox != nullptr) {
								TextBox->SetText(FText::FromString(currentDescription));
							}
							UTextBlock* UsageBox = Cast<UTextBlock>(UserWidgetDesc->GetWidgetFromName(TEXT("Usage")));
							if (UsageBox != nullptr) {
								UsageBox->SetText(FText::FromString(currentUsage));
							}
						}
					}
					refToDescriptionActor->SetActorHiddenInGame(false);
				}

			}

		// If right clicking on an ability
		} else if (selected.thingIs.Contains(TEXT("ButtonAbility")) && inventoryOpen) {

			// Get the index
			int perPage = 10;
			FString abilityIndex = selected.thingIs.Replace(TEXT("ButtonAbility"), TEXT(""));
			int abilityNum = FCString::Atoi(*abilityIndex)-1;
			abilityNum += abilityPage * perPage;

			// Check if there's something there
			if (abilityNum >= 0 && abilityNum < abilityLetters.Num()) {
				FString letter = abilityLetters[abilityNum];
				if (abilityLetterToInfo.Contains(letter)) {

					// Write the commands
					writeCommandQueued("a");
					writeCommandQueued("?");
					writeCommandQueued(letter);
					writeCommandQueued("escape");
					writeCommandQueued("escape");
					thingBeingDragged = SelectedThing(-1, -1, "Ability", abilityNum, "");

					// Show the description
					currentDescription = TEXT("");
					currentUsage = TEXT("");
					UWidgetComponent* WidgetComponentDesc = Cast<UWidgetComponent>(refToDescriptionActor->GetComponentByClass(UWidgetComponent::StaticClass()));
					if (WidgetComponentDesc != nullptr) {
						UUserWidget* UserWidgetDesc = WidgetComponentDesc->GetUserWidgetObject();
						if (UserWidgetDesc != nullptr) {
							UTextBlock* TextBox = Cast<UTextBlock>(UserWidgetDesc->GetWidgetFromName(TEXT("Description")));
							if (TextBox != nullptr) {
								TextBox->SetText(FText::FromString(currentDescription));
							}
							UTextBlock* UsageBox = Cast<UTextBlock>(UserWidgetDesc->GetWidgetFromName(TEXT("Usage")));
							if (UsageBox != nullptr) {
								UsageBox->SetText(FText::FromString(currentUsage));
							}
						}
					}
					refToDescriptionActor->SetActorHiddenInGame(false);

				}
			}

		// If right clicking on a spell
		} else if (selected.thingIs.Contains(TEXT("ButtonSpell")) && inventoryOpen) {

			// Get the index
			int perPage = 10;
			FString spellIndex = selected.thingIs.Replace(TEXT("ButtonSpell"), TEXT(""));
			int spellNum = FCString::Atoi(*spellIndex)-1;
			spellNum += spellPage * perPage;

			// Check if there's something there
			if (spellNum >= 0 && spellNum < spellLetters.Num()) {
				FString letter = spellLetters[spellNum];
				if (spellLetterToInfo.Contains(letter)) {

					// If it's not on the first page
					int nextPagesNeeded = 0;
					if (letter.Len() > 1) {
						nextPagesNeeded = 2*(letter.Len() - 1);
						letter = letter.Left(1);
					}

					// Write the commands
					if (memorizing) {
						writeCommandQueued("M");
						writeCommandQueued("!");
						for (int i = 0; i < nextPagesNeeded; i++) {
							writeCommandQueued(">");
						}
						writeCommandQueued(letter);
						writeCommandQueued(">");
						writeCommandQueued("escape");
						writeCommandQueued("escape");
						thingBeingDragged = SelectedThing(-1, -1, "Memorizing", spellNum, "");
					} else {
						writeCommandQueued("I");
						for (int i = 0; i < nextPagesNeeded; i++) {
							writeCommandQueued(">");
						}
						writeCommandQueued(letter);
						writeCommandQueued(">");
						writeCommandQueued("escape");
						writeCommandQueued("escape");
						thingBeingDragged = SelectedThing(-1, -1, "Spell", spellNum, "");
					}

					// Show the description
					currentDescription = TEXT("");
					currentUsage = TEXT("");
					UWidgetComponent* WidgetComponentDesc = Cast<UWidgetComponent>(refToDescriptionActor->GetComponentByClass(UWidgetComponent::StaticClass()));
					if (WidgetComponentDesc != nullptr) {
						UUserWidget* UserWidgetDesc = WidgetComponentDesc->GetUserWidgetObject();
						if (UserWidgetDesc != nullptr) {
							UTextBlock* TextBox = Cast<UTextBlock>(UserWidgetDesc->GetWidgetFromName(TEXT("Description")));
							if (TextBox != nullptr) {
								TextBox->SetText(FText::FromString(currentDescription));
							}
							UTextBlock* UsageBox = Cast<UTextBlock>(UserWidgetDesc->GetWidgetFromName(TEXT("Usage")));
							if (UsageBox != nullptr) {
								UsageBox->SetText(FText::FromString(currentUsage));
							}
						}
					}
					refToDescriptionActor->SetActorHiddenInGame(false);

				}
			}

		// If right clicking on one of the hotbar slots
		} else if (selected.thingIs.Contains(TEXT("SlotButton"))) {

			// Get the index
			FString hotbarSlot = selected.thingIs.Replace(TEXT("SlotButton"), TEXT(""));
			int hotbarIndex = FCString::Atoi(*hotbarSlot)-1;
			if (hotbarIndex >= 0 && hotbarIndex <= numHotbarSlots) {

				// If there's something there
				if (hotbarInfos[hotbarIndex].name.Len() >= 0) {
					bool found = false;

					// If it's an inventory item
					if (hotbarInfos[hotbarIndex].type == "InventoryItem") {
						writeCommandQueued("i");
						writeCommandQueued(hotbarInfos[hotbarIndex].letter);
						writeCommandQueued(">");
						writeCommandQueued(">");
						writeCommandQueued("escape");
						writeCommandQueued("escape");
						int yLoc = -1;
						int xLoc = -1;
						for (int i = 0; i < inventoryLocToLetter.Num(); i++) {
							for (int j = 0; j < inventoryLocToLetter[i].Num(); j++) {
								if (inventoryLocToLetter[i][j] == hotbarInfos[hotbarIndex].letter) {
									yLoc = i;
									xLoc = j;
									found = true;
									break;
								}
							}
							if (found) {
								break;
							}
						}
						if (found) {
							thingBeingDragged = SelectedThing(xLoc, yLoc, "InventoryItem", 0, "");
						}

					// If it's an ability
					} else if (hotbarInfos[hotbarIndex].type == "Ability") {
						writeCommandQueued("a");
						writeCommandQueued("?");
						writeCommandQueued(hotbarInfos[hotbarIndex].letter);
						writeCommandQueued("escape");
						writeCommandQueued("escape");
						thingBeingDragged = SelectedThing(-1, -1, "Ability", 0, hotbarInfos[hotbarIndex].letter);
						found = true;

					// If it's a spell
					} else if (hotbarInfos[hotbarIndex].type == "Spell") {
						writeCommandQueued("I");
						writeCommandQueued(hotbarInfos[hotbarIndex].letter);
						writeCommandQueued("escape");
						writeCommandQueued("escape");
						thingBeingDragged = SelectedThing(-1, -1, "Spell", 0, hotbarInfos[hotbarIndex].letter);
						found = true;

					}

					// Show the description
					if (found) {
						currentDescription = TEXT("");
						currentUsage = TEXT("");
						UWidgetComponent* WidgetComponentDesc = Cast<UWidgetComponent>(refToDescriptionActor->GetComponentByClass(UWidgetComponent::StaticClass()));
						if (WidgetComponentDesc != nullptr) {
							UUserWidget* UserWidgetDesc = WidgetComponentDesc->GetUserWidgetObject();
							if (UserWidgetDesc != nullptr) {
								UTextBlock* TextBox = Cast<UTextBlock>(UserWidgetDesc->GetWidgetFromName(TEXT("Description")));
								if (TextBox != nullptr) {
									TextBox->SetText(FText::FromString(currentDescription));
								}
								UTextBlock* UsageBox = Cast<UTextBlock>(UserWidgetDesc->GetWidgetFromName(TEXT("Usage")));
								if (UsageBox != nullptr) {
									UsageBox->SetText(FText::FromString(currentUsage));
								}
							}
						}
						refToDescriptionActor->SetActorHiddenInGame(false);
					}

				}

			}

		// If right clicking on something in the inventory
		} else if (selected.thingIs.Contains(TEXT("ItemButton")) && inventoryOpen) {

			// Get the row and column
			TArray<FString> parts;
			selected.thingIs.ParseIntoArray(parts, TEXT("_"), true);
			int32 row = FCString::Atoi(*parts[1])-1;
			int32 col = FCString::Atoi(*parts[2])-1;
			
			// If it's not an empty slot
			if (inventoryLocToLetter[row][col].Len() > 0) {
				thingBeingDragged = SelectedThing(col, row, "InventoryItem", 0, "");
				UE_LOG(LogTemp, Display, TEXT("INPUT - Thing being dragged: (%d, %d) %s %d"), thingBeingDragged.x, thingBeingDragged.y, *thingBeingDragged.thingIs, thingBeingDragged.thingIndex);

				// Send the commands to get the description
				writeCommandQueued("i");
				writeCommandQueued(inventoryLocToLetter[row][col]);
				writeCommandQueued(">");
				writeCommandQueued(">");
				writeCommandQueued("escape");
				writeCommandQueued("escape");

				// Show the description
				currentDescription = TEXT("");
				currentUsage = TEXT("");
				UWidgetComponent* WidgetComponentDesc = Cast<UWidgetComponent>(refToDescriptionActor->GetComponentByClass(UWidgetComponent::StaticClass()));
				if (WidgetComponentDesc != nullptr) {
					UUserWidget* UserWidgetDesc = WidgetComponentDesc->GetUserWidgetObject();
					if (UserWidgetDesc != nullptr) {
						UTextBlock* TextBox = Cast<UTextBlock>(UserWidgetDesc->GetWidgetFromName(TEXT("Description")));
						if (TextBox != nullptr) {
							TextBox->SetText(FText::FromString(currentDescription));
						}
						UTextBlock* UsageBox = Cast<UTextBlock>(UserWidgetDesc->GetWidgetFromName(TEXT("Usage")));
						if (UsageBox != nullptr) {
							UsageBox->SetText(FText::FromString(currentUsage));
						}
					}
				}
				refToDescriptionActor->SetActorHiddenInGame(false);

			}

		// Describing a ladder
		} else if (selected.x != -1 && selected.y != -1 && selected.thingIs.Contains("Ladder")) {

			// Show the description
			if (selected.thingIs == "LadderDown") {
				currentDescription = TEXT("A ladder going down to the next floor.");
				currentUsage = TEXT("USE when on the same tile to descend.");
			} else if (selected.thingIs == "LadderUp") {
				currentDescription = TEXT("A ladder going up to the previous floor.");
				currentUsage = TEXT("USE when on the same tile to ascend.");
			}
			thingBeingDragged = selected;
			UWidgetComponent* WidgetComponentDesc = Cast<UWidgetComponent>(refToDescriptionActor->GetComponentByClass(UWidgetComponent::StaticClass()));
			if (WidgetComponentDesc != nullptr) {
				UUserWidget* UserWidgetDesc = WidgetComponentDesc->GetUserWidgetObject();
				if (UserWidgetDesc != nullptr) {
					UTextBlock* TextBox = Cast<UTextBlock>(UserWidgetDesc->GetWidgetFromName(TEXT("Description")));
					if (TextBox != nullptr) {
						TextBox->SetText(FText::FromString(currentDescription));
					}
					UTextBlock* UsageBox = Cast<UTextBlock>(UserWidgetDesc->GetWidgetFromName(TEXT("Usage")));
					if (UsageBox != nullptr) {
						UsageBox->SetText(FText::FromString(currentUsage));
					}
				}
			}
			refToDescriptionActor->SetActorHiddenInGame(false);

		// Describing something
		} else if (selected.x != -1 && selected.y != -1 && selected.thingIs != TEXT("Floor") && selected.thingIs != TEXT("Effect")) {

			// Send the commands to get the description
			if (selected.thingIs == "Enemy") {
				writeCommandQueued("SKIP");
				writeCommandQueued("ctrl-X");
				writeCommandQueued("!");
				writeCommandQueued(levelInfo[selected.y][selected.x].enemyHotkey);
				writeCommandQueued(">");
				writeCommandQueued("escape");
				writeCommandQueued("escape");
			} else if (selected.thingIs == "Shop" && selected.thingIndex >= 0 && levelInfo[selected.y][selected.x].itemHotkeys.Num() > selected.thingIndex) {
				writeCommandQueued("SKIP");
				writeCommandQueued("ctrl-X");
				writeCommandQueued("!");
				writeCommandQueued(levelInfo[selected.y][selected.x].itemHotkeys[selected.thingIndex]);
				writeCommandQueued("escape");
				writeCommandQueued("escape");
			} else if (selected.thingIs == "Item" && selected.thingIndex >= 0 && levelInfo[selected.y][selected.x].itemHotkeys.Num() > selected.thingIndex) {
				writeCommandQueued("SKIP");
				writeCommandQueued("ctrl-X");
				writeCommandQueued("!");
				writeCommandQueued(levelInfo[selected.y][selected.x].itemHotkeys[selected.thingIndex]);
				if (!levelInfo[selected.y][selected.x].items[selected.thingIndex].Contains("altar") && !levelInfo[selected.y][selected.x].items[selected.thingIndex].Contains("shop")) {
					writeCommandQueued(">");
				}
				writeCommandQueued("escape");
				writeCommandQueued("escape");
			} else if ((selected.thingIs == "Altar" || selected.thingIs == "Gate" || selected.thingIs == "Arch") && selected.thingIndex >= 0 && levelInfo[selected.y][selected.x].itemHotkeys.Num() > selected.thingIndex) {
				writeCommandQueued("SKIP");
				writeCommandQueued("ctrl-X");
				writeCommandQueued("!");
				writeCommandQueued(levelInfo[selected.y][selected.x].itemHotkeys[selected.thingIndex]);
				writeCommandQueued("escape");
				writeCommandQueued("escape");
			}
			thingBeingDragged = selected;

			// Show the description
			currentDescription = TEXT("");
			currentUsage = TEXT("");
			UWidgetComponent* WidgetComponentDesc = Cast<UWidgetComponent>(refToDescriptionActor->GetComponentByClass(UWidgetComponent::StaticClass()));
			if (WidgetComponentDesc != nullptr) {
				UUserWidget* UserWidgetDesc = WidgetComponentDesc->GetUserWidgetObject();
				if (UserWidgetDesc != nullptr) {
					UTextBlock* TextBox = Cast<UTextBlock>(UserWidgetDesc->GetWidgetFromName(TEXT("Description")));
					if (TextBox != nullptr) {
						TextBox->SetText(FText::FromString(currentDescription));
					}
					UTextBlock* UsageBox = Cast<UTextBlock>(UserWidgetDesc->GetWidgetFromName(TEXT("Usage")));
					if (UsageBox != nullptr) {
						UsageBox->SetText(FText::FromString(currentUsage));
					}
				}
			}
			refToDescriptionActor->SetActorHiddenInGame(false);
		}

	} else if (key == "rmbUp") {

		// If we're holding an item and are looking at the ground
		UE_LOG(LogTemp, Display, TEXT("INPUT - Right click released: (%d, %d) %s %d"), selected.x, selected.y, *selected.thingIs, selected.thingIndex);
		if (selected.x == 8 && selected.y == 8 && thingBeingDragged.thingIs == "InventoryItem" && thingBeingDragged.x != -1 && thingBeingDragged.y != -1) {
			UE_LOG(LogTemp, Display, TEXT("Dropping item"));
			writeCommandQueued("d");
			writeCommandQueued(inventoryLocToLetter[thingBeingDragged.y][thingBeingDragged.x]);
			writeCommandQueued("enter");
			writeCommandQueued("CLEARINV");
			writeCommandQueued("i");
			writeCommandQueued(">");
			writeCommandQueued(">");
			writeCommandQueued("escape");
			writeCommandQueued("CLEAR");
			writeCommandQueued("ctrl-X");
			writeCommandQueued(">");
			writeCommandQueued(">");
			writeCommandQueued(">");
			writeCommandQueued("escape");

		// If we're selecting an item from the ground and dropping it into the inventory
		} else if (thingBeingDragged.x == 8 && thingBeingDragged.y == 8 
			      && thingBeingDragged.thingIs == "Item" 
				  && selected.thingIs.Contains(TEXT("ItemButton"))
				  && levelInfo[thingBeingDragged.y][thingBeingDragged.x].itemHotkeys.Num() > thingBeingDragged.thingIndex) {
			FString letter = levelInfo[thingBeingDragged.y][thingBeingDragged.x].itemHotkeys[thingBeingDragged.thingIndex];
			UE_LOG(LogTemp, Display, TEXT("Picking up item with letter %s"), *letter);
			writeCommandQueued("SKIP");
			writeCommandQueued("ctrl-X");
			writeCommandQueued("!");
			writeCommandQueued(letter);
			writeCommandQueued("g");
			writeCommandQueued("CLEARINV");
			writeCommandQueued("i");
			writeCommandQueued(">");
			writeCommandQueued(">");
			writeCommandQueued("escape");
			writeCommandQueued("CLEAR");
			writeCommandQueued("ctrl-X");
			writeCommandQueued(">");
			writeCommandQueued(">");
			writeCommandQueued(">");
			writeCommandQueued("escape");

			// Note the location so that it goes into the right spot
			TArray<FString> parts;
			selected.thingIs.ParseIntoArray(parts, TEXT("_"), true);
			int32 row = FCString::Atoi(*parts[1])-1;
			int32 col = FCString::Atoi(*parts[2])-1;
			inventoryNextSpot = FIntVector2(row, col);

		// If dragging something onto one of the equipped slot
		} else if ((thingBeingDragged.thingIs == "InventoryItem" || thingBeingDragged.thingIs == "Ability" || thingBeingDragged.thingIs == "Spell") && selected.thingIs.Contains(TEXT("EquippedButton"))) {

			// If it's an inventory item
			if (thingBeingDragged.thingIs == "InventoryItem") { 

				// Get the letter of the item
				FString letter = inventoryLocToLetter[thingBeingDragged.y][thingBeingDragged.x];
				if (thingBeingDragged.letter.Len() > 0) {
					letter = thingBeingDragged.letter;
				}

				// If the name is different
				if (!rightText.Contains(inventoryLetterToName[letter]) && !inventoryLetterToName[letter].Contains(TEXT("(weapon)"))) {
					UE_LOG(LogTemp, Display, TEXT("INPUT - Equipping item via drag onto equip slot %s"), *inventoryLetterToName[letter]);

					// Try to equip it
					writeCommandQueued("w");
					writeCommandQueued(letter);

					// Refresh the inventory
					writeCommandQueued("CLEARINV");
					writeCommandQueued("i");
					writeCommandQueued(">");
					writeCommandQueued(">");
					writeCommandQueued("escape");

				}

				// Update the info
				equippedInfo.name = inventoryLetterToName[letter];
				equippedInfo.type = thingBeingDragged.thingIs;
				equippedInfo.letter = letter;

			// If it's an ability
			} else if (thingBeingDragged.thingIs == "Ability") {

				// Get the letter of the ability
				FString letter;
				if (thingBeingDragged.letter.Len() > 0) {
					letter = thingBeingDragged.letter;
				} else if (thingBeingDragged.thingIndex >= 0 && thingBeingDragged.thingIndex < abilityLetters.Num()) {
					letter = abilityLetters[thingBeingDragged.thingIndex];
				} else {
					UE_LOG(LogTemp, Warning, TEXT("INPUT - Ability letter not found for index %d"), thingBeingDragged.thingIndex);
					return;
				}

				// Update the info
				equippedInfo.type = thingBeingDragged.thingIs;
				equippedInfo.letter = letter;
				equippedInfo.name = abilityLetterToInfo[letter].name;

				// Quiver the thing
				UE_LOG(LogTemp, Display, TEXT("INPUT - Quivering ability via drag onto equip slot %s"), *abilityLetterToInfo[abilityLetters[thingBeingDragged.thingIndex]].name);
				writeCommandQueued("Q");
				writeCommandQueued("^");
				writeCommandQueued(letter);

			// If it's a spell
			} else if (thingBeingDragged.thingIs == "Spell") {

				// Get the letter of the spell
				FString letter;
				if (thingBeingDragged.letter.Len() > 0) {
					letter = thingBeingDragged.letter;
				} else if (thingBeingDragged.thingIndex >= 0 && thingBeingDragged.thingIndex < spellLetters.Num()) {
					letter = spellLetters[thingBeingDragged.thingIndex];
				} else {
					UE_LOG(LogTemp, Warning, TEXT("INPUT - Spell letter not found for index %d"), thingBeingDragged.thingIndex);
					return;
				}

				// Update the info
				equippedInfo.type = thingBeingDragged.thingIs;
				equippedInfo.letter = letter;
				equippedInfo.name = spellLetterToInfo[letter].name;

				// Quiver the thing
				UE_LOG(LogTemp, Display, TEXT("INPUT - Quivering spell via drag onto equip slot %s"), *spellLetterToInfo[spellLetters[thingBeingDragged.thingIndex]].name);
				writeCommandQueued("Q");
				writeCommandQueued("&");
				writeCommandQueued(letter);

			}

			// We need to redraw the hotbar
			shouldRedrawHotbar = true;

		// If dragging something onto one of the hotbar slots
		} else if ((thingBeingDragged.thingIs == "InventoryItem" || thingBeingDragged.thingIs == "Ability" || thingBeingDragged.thingIs == "Spell") && selected.thingIs.Contains(TEXT("SlotButton"))) {

			// Set the hotbar thing
			FString hotbarSlot = selected.thingIs.Replace(TEXT("SlotButton"), TEXT(""));
			int hotbarIndex = FCString::Atoi(*hotbarSlot)-1;
			if (hotbarIndex >= 0 && hotbarIndex <= numHotbarSlots) {

				// If it's an inventory item
				if (thingBeingDragged.thingIs == "InventoryItem") { 
					hotbarInfos[hotbarIndex].name = inventoryLetterToName[inventoryLocToLetter[thingBeingDragged.y][thingBeingDragged.x]];
					hotbarInfos[hotbarIndex].type = thingBeingDragged.thingIs;
					hotbarInfos[hotbarIndex].letter = inventoryLocToLetter[thingBeingDragged.y][thingBeingDragged.x];

				// If it's an ability
				} else if (thingBeingDragged.thingIs == "Ability") {
					hotbarInfos[hotbarIndex].type = thingBeingDragged.thingIs;
					hotbarInfos[hotbarIndex].letter = abilityLetters[thingBeingDragged.thingIndex];
					hotbarInfos[hotbarIndex].name = abilityLetterToInfo[abilityLetters[thingBeingDragged.thingIndex]].name;

				// If it's a spell
				} else if (thingBeingDragged.thingIs == "Spell") {
					hotbarInfos[hotbarIndex].type = thingBeingDragged.thingIs;
					hotbarInfos[hotbarIndex].letter = spellLetters[thingBeingDragged.thingIndex];
					hotbarInfos[hotbarIndex].name = spellLetterToInfo[spellLetters[thingBeingDragged.thingIndex]].name;
				}

				// We need to redraw the hotbar
				shouldRedrawHotbar = true;

			}

		// If we're dragging something in the inventory
		} else if (thingBeingDragged.thingIs == "InventoryItem" && thingBeingDragged.x != -1 && thingBeingDragged.y != -1 && selected.thingIs.Contains(TEXT("ItemButton"))) {
			
			// Get the row and column of the current selected
			TArray<FString> parts;
			selected.thingIs.ParseIntoArray(parts, TEXT("_"), true);
			int32 row = FCString::Atoi(*parts[1])-1;
			int32 col = FCString::Atoi(*parts[2])-1;

			// Make the swap
			UE_LOG(LogTemp, Display, TEXT("Moving item from (%d, %d) to (%d, %d)"), thingBeingDragged.x, thingBeingDragged.y, row, col);
			FString temp = inventoryLocToLetter[row][col];
			inventoryLocToLetter[row][col] = inventoryLocToLetter[thingBeingDragged.y][thingBeingDragged.x];
			inventoryLocToLetter[thingBeingDragged.y][thingBeingDragged.x] = temp;
			shouldRedrawInventory = true;

		}

		// Reset various things
		UE_LOG(LogTemp, Display, TEXT("Right click released"));
		rmbOn = false;
		targetingRange = -1;
		draggingInventory = false;
		refToDescriptionActor->SetActorHiddenInGame(true);
		thingBeingDragged = SelectedThing();

	// The debug key
	} else if (key == "debug") {

		// show everything
		writeCommandQueued("ctrl-X");

	} else {
		writeCommandQueued(key);
	}
}

// Called every frame
void Adcss::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);

	// Make sure all of the pointers are valid
	if (refToDescriptionActor == nullptr 
		|| refToInventoryActor == nullptr
		|| refToUIActor == nullptr
		|| refToDescriptionActor == nullptr
		|| refToAltarActor == nullptr
		|| refToSettingsActor == nullptr
		|| refToDeathActor == nullptr
		|| refToChoiceActor == nullptr
		|| refToShopActor == nullptr
		|| worldRef == nullptr
		|| refToTutorialActor == nullptr
		|| refToMainInfoActor == nullptr
		|| refToKeyboardActor == nullptr) {
		UE_LOG(LogTemp, Warning, TEXT("One of the pointers is null"));
		return;
	}

	// Determine if vr is enabled
	if (GEngine != nullptr && GEngine->XRSystem != nullptr) {
		IHeadMountedDisplay* hmd = GEngine->XRSystem->GetHMDDevice();
		TSharedPtr<IStereoRendering, ESPMode::ThreadSafe> pStereo = GEngine->XRSystem->GetStereoRenderingDevice();
		vrEnabled = hmd != nullptr && hmd->IsHMDConnected() && hmd->IsHMDEnabled() && pStereo->IsStereoEnabled();
	}

	// If we don't have a server address and the queue is empty, try to find one
	serverAddress = serverAddress.TrimStartAndEnd();
	if (useServer && serverAddress.Len() == 0 && serverAddressesToTry.Num() == 0) {
		FString url;
		for (int i=0; i<30; i++) {
			serverAddressesToTry.Add("http://192.168.0." + FString::FromInt(i) + ":7777/");
			serverAddressesToTry.Add("http://192.168.1." + FString::FromInt(i) + ":7777/");
		}
		for (int i=30; i<100; i++) {
			serverAddressesToTry.Add("http://192.168.0." + FString::FromInt(i) + ":7777/");
			serverAddressesToTry.Add("http://192.168.1." + FString::FromInt(i) + ":7777/");
		}
		UE_LOG(LogTemp, Display, TEXT("No server address found, added %d addresses to try"), serverAddressesToTry.Num());
	}

	// Send server connection requests
	int maxRequests = 15;
	if (useServer && serverAddress.Len() == 0 && serverAddressesToTry.Num() > 0 && currentRequests < maxRequests) {
		int numToSend = FMath::Min(maxRequests - currentRequests, serverAddressesToTry.Num());
		for (int i=0; i<numToSend; i++) {
			FString url = serverAddressesToTry[0];
			serverAddressesToTry.RemoveAt(0);
			currentRequests++;
			UE_LOG(LogTemp, Display, TEXT("Searching for server at %s"), *url);
			TSharedRef<IHttpRequest> req = (&FHttpModule::Get())->CreateRequest();
			req->SetVerb("GET");
			req->SetURL(url);
			req->OnProcessRequestComplete().BindLambda([this](
					FHttpRequestPtr pRequest,
					FHttpResponsePtr pResponse,
					bool connectedSuccessfully) mutable {
						currentRequests--;
						if (connectedSuccessfully) {
							int code = pResponse->GetResponseCode();
							FString response = *pResponse->GetContentAsString();
							if (code == 200) {
								if (response.Contains(TEXT("===SERVER==="))) {
									serverConnected = true;
									serverAddress = pRequest->GetURL();
									nextCommand = 0;
									UE_LOG(LogTemp, Display, TEXT("Found server at %s"), *serverAddress);
								}
							}
						}
				});
			req->SetTimeout(0.5f);
			req->ProcessRequest();
			httpRequests.Add(req);
		}
	}

	// Reset highlight on everything
	for (int i = 0; i < enemyUseCount; i++) {
		enemyArray[i]->FindComponentByClass<UStaticMeshComponent>()->SetRenderCustomDepth(false);
	}
	for (int i = 0; i < itemUseCount; i++) {
		itemArray[i]->FindComponentByClass<UStaticMeshComponent>()->SetRenderCustomDepth(false);
	}
	for (int i = 0; i < gridWidth; i++) {
		for (int j = 0; j < gridWidth; j++) {
			floorArray[i][j]->FindComponentByClass<UStaticMeshComponent>()->SetRenderCustomDepth(false);
		}
	}
	for (int i = 0; i < gridWidth; i++) {
		for (int j = 0; j < gridWidth; j++) {
			wallArray[i][j][0]->FindComponentByClass<UStaticMeshComponent>()->SetRenderCustomDepth(false);
			wallArray[i][j][1]->FindComponentByClass<UStaticMeshComponent>()->SetRenderCustomDepth(false);
			wallArray[i][j][2]->FindComponentByClass<UStaticMeshComponent>()->SetRenderCustomDepth(false);
			wallArray[i][j][3]->FindComponentByClass<UStaticMeshComponent>()->SetRenderCustomDepth(false);
		}
	}
	floorArray[LOS][LOS]->FindComponentByClass<UStaticMeshComponent>()->SetRenderCustomDepth(true);

	// If in nightfall, reduce the light intensity
	if (statusText.Contains(TEXT("Nightfall"))) {
		for (int i = 0; i < refToLightsActors.Num(); i++) {
			refToLightsActors[i]->FindComponentByClass<UPointLightComponent>()->SetIntensity(1.0f);
		}
	} else {
		for (int i = 0; i < refToLightsActors.Num(); i++) {
			refToLightsActors[i]->FindComponentByClass<UPointLightComponent>()->SetIntensity(60.0f);
		}
	}

	// If the current scroll is a blink or unknown, we should show the targeting
	bool isBlinkOrUnknown = false;
	if (currentDescription.Contains(TEXT("blink")) || currentDescription.Contains(TEXT("disposable arcane formula"))) {
		isBlinkOrUnknown = true;
	}

	// If in VR, get the location and direction of the hand
	FVector handLocation = FVector(0.0f, 0.0f, 0.0f);
	FVector handForward = FVector(0.0f, 0.0f, 0.0f);
	FVector handRight = FVector(0.0f, 0.0f, 0.0f);
	FVector handUp = FVector(0.0f, 0.0f, 0.0f);
	if (vrEnabled) {
		TArray<UMotionControllerComponent*> motionControllers;
		if (worldRef != nullptr) {
			worldRef->GetFirstPlayerController()->GetPawn()->GetComponents(UMotionControllerComponent::StaticClass(), motionControllers);
		}
		for (UMotionControllerComponent* motionController : motionControllers) {
			if ((motionController->MotionSource == "RightAim" && activeHand == "right") || (motionController->MotionSource == "LeftAim" && activeHand == "left")) {
				handLocation = motionController->GetComponentLocation();
				handForward = motionController->GetForwardVector();
				handRight = motionController->GetRightVector();
				handUp = motionController->GetUpVector();
			}
		}
	}

	// Get various pointers (world, player controller, etc)
	APlayerController* playerController = worldRef->GetFirstPlayerController();
	if (playerController == nullptr) {
		UE_LOG(LogTemp, Warning, TEXT("Player controller is null"));
		return;
	}
	APlayerCameraManager* playerCameraManager = playerController->PlayerCameraManager;
	if (playerCameraManager == nullptr) {
		UE_LOG(LogTemp, Warning, TEXT("Player camera manager is null"));
		return;
	}
	APawn* playerPawn = playerController->GetPawn();
	if (playerPawn == nullptr) {
		UE_LOG(LogTemp, Warning, TEXT("Player pawn is null"));
		return;
	}

	// Quantities used for various things
	FVector playerLocation = playerCameraManager->GetCameraLocation();
	FVector playerForward = playerCameraManager->GetActorForwardVector();
	FVector playerRight = playerPawn->GetActorRightVector();
	FVector playerUp = playerPawn->GetActorUpVector();
	FVector playerForwardProjected = FVector(playerForward.X, playerForward.Y, 0.0f);
	playerForwardProjected.Normalize();
	
	// Checking what the player is looking at
	refToNameTagActor->SetActorHiddenInGame(true);
	FVector Start = playerLocation;
	FVector End = Start + playerForward * 3000.0f;
	if (vrEnabled) {
		Start = handLocation;
		End = Start + handForward * 3000.0f;
	}
	FHitResult HitResult;
	FCollisionQueryParams COQP;			
	COQP.AddIgnoredActor(worldRef->GetFirstPlayerController()->GetPawn());
	if (!selected.thingIs.Contains(TEXT("Button"))) {
		selected = SelectedThing();
	}
	FCollisionResponseParams CollRes;
	if (!inMainMenu && worldRef->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, COQP, CollRes)) {
		if (HitResult.bBlockingHit) {
			AActor* hitActor = HitResult.GetActor();
			if (hitActor != nullptr) {
				UStaticMeshComponent* hitMesh = hitActor->FindComponentByClass<UStaticMeshComponent>();
				if (hitMesh != nullptr) {

					// Set the selection
					if (meshNameToThing.Contains(HitResult.GetActor()->GetName())) {
						selected = meshNameToThing[HitResult.GetActor()->GetName()];
					} else {
						UE_LOG(LogTemp, Warning, TEXT("Mesh name not in map: %s"), *HitResult.GetActor()->GetName());
					}

					// If not describing an item
					if (!rmbOn || isBlinkOrUnknown) {

						// If it's not a wall
						if (selected.thingIs == TEXT("Wall")) {
							selected = SelectedThing();
						} else {

							// Highlight it
							hitMesh->SetRenderCustomDepth(true);

							// Enable the nametag
							if (selected.thingIs != TEXT("Wall") && selected.thingIs != TEXT("Floor") && selected.thingIs != TEXT("Effect")) {

								// Determine the text to show
								FString textToShow = selected.thingIs;
								if (selected.thingIs == TEXT("Enemy")) {
									textToShow = levelInfo[selected.y][selected.x].enemy;
								} else if (selected.thingIndex >= 0 && selected.thingIndex < levelInfo[selected.y][selected.x].items.Num()) {
									textToShow = levelInfo[selected.y][selected.x].items[selected.thingIndex];
								}

								// Update the text
								if (refToNameTagActor != nullptr) {
									UWidgetComponent* WidgetComponent = Cast<UWidgetComponent>(refToNameTagActor->GetComponentByClass(UWidgetComponent::StaticClass()));
									if (WidgetComponent != nullptr) {
										UUserWidget* UserWidget = WidgetComponent->GetUserWidgetObject();
										if (UserWidget != nullptr) {
											UTextBlock* TextBox = Cast<UTextBlock>(UserWidget->GetWidgetFromName(TEXT("TextNametag")));
											if (TextBox != nullptr) {
												TextBox->SetText(FText::FromString(textToShow));
											}
										}
									}

									// Set the location and rotation
									FVector meshLoc = hitMesh->GetComponentLocation();
									refToNameTagActor->SetActorLocation(meshLoc + playerUp * 180.0f);
									FRotator newRotation = playerForward.Rotation();
									newRotation.Pitch = 0.0f;
									newRotation.Yaw += 180.0f;
									newRotation.Roll = 0.0f;

									// Show it
									refToNameTagActor->SetActorRotation(newRotation);
									refToNameTagActor->SetActorHiddenInGame(false);
									refToNameTagActor->SetActorEnableCollision(false);

								}
							}

						}

					// Or spell targeting
					} else if (rmbOn && targetingRange >= 1) {

						// Get the x and y of the hit
						int x = selected.x;
						int y = selected.y;
						int dist = FMath::RoundToInt(FMath::Sqrt(FMath::Pow(float(x-LOS), 2) + FMath::Pow(float(y-LOS), 2)));

						// If it's within range
						if (dist <= targetingRange) {
							
							// For each grid point between the player and the target
							float currentX = LOS;
							float currentY = LOS;
							float currentDist = 0;
							float distPer = 0.4;
							float theta = FMath::Atan2(float(y-LOS), float(x-LOS));
							float deltaX = distPer*FMath::Cos(theta);
							float deltaY = distPer*FMath::Sin(theta);
							while (currentDist < dist) {
								currentDist += distPer;
								currentX += deltaX;
								currentY += deltaY;
								int roundedX = FMath::RoundToInt(currentX);
								int roundedY = FMath::RoundToInt(currentY);
								if (roundedX >= 0 && roundedX < gridWidth && roundedY >= 0 && roundedY < gridWidth) {
									floorArray[roundedY][roundedX]->FindComponentByClass<UStaticMeshComponent>()->SetRenderCustomDepth(true);
								}
							}

						}
						
					}

				// Otherwise maybe it's the inventory panel
				} else {
					if (HitResult.GetActor()->GetName() == refToInventoryActor->GetName() && !draggingInventory && !selected.thingIs.Contains(TEXT("Button"))) {
						selected = SelectedThing(-1, -1, "Inventory", -1, "");
						inventoryGrabDistance = HitResult.Distance;

						// Determine x and y hit on the plane, based on the world vec hit location
						inventoryGrabPoint.X = FVector::DotProduct(HitResult.ImpactPoint - refToInventoryActor->GetActorLocation(), refToInventoryActor->GetActorRightVector());
						inventoryGrabPoint.Y = FVector::DotProduct(HitResult.ImpactPoint - refToInventoryActor->GetActorLocation(), -refToInventoryActor->GetActorUpVector());

					}
				}
			}
		}
	}

	// Keep the pop up following the player's head 
	if (isChoiceOpen) {
		FVector choiceLocation = playerLocation + playerForwardProjected * 150.0f;
		refToChoiceActor->SetActorLocation(choiceLocation);
		FRotator choiceRotation = playerForward.Rotation();
		choiceRotation.Pitch = 0.0f;
		choiceRotation.Yaw += 180.0f;
		choiceRotation.Roll = 0.0f;
		refToChoiceActor->SetActorRotation(choiceRotation);
	}

	// Keep track of which way the player is looking
	FVector dir(0.0f, 0.0f, 0.0f);
	FString dirString = "none";
	if (playerForwardProjected.X > 0 && std::abs(playerForwardProjected.Y) < std::abs(playerForwardProjected.X)) {
		dir = FVector(1.0f, 0.0f, 0.0f);
		dirString = "right";
	} else if (playerForwardProjected.X < 0 && std::abs(playerForwardProjected.Y) < std::abs(playerForwardProjected.X)) {
		dir = FVector(-1.0f, 0.0f, 0.0f);
		dirString = "left";
	} else if (playerForwardProjected.Y > 0 && std::abs(playerForwardProjected.Y) > std::abs(playerForwardProjected.X)) {
		dir = FVector(0.0f, 1.0f, 0.0f);
		dirString = "up";
	} else if (playerForwardProjected.Y < 0 && std::abs(playerForwardProjected.Y) > std::abs(playerForwardProjected.X)) {
		dir = FVector(0.0f, -1.0f, 0.0f);
		dirString = "down";
	}

	// Only update the map if we can see it
	if (inventoryOpen && currentUI == "map") {

		// Set the map based on the player's orientation
		if (dirString == "left") {
			for (int i = 0; i < gridWidth; i++) {
				for (int j = 0; j < gridWidth; j++) {
					mapToDrawRotated[i][j] = mapToDraw[gridWidth - i - 1][gridWidth - j - 1];
				}
			}
		} else if (dirString == "right") {
			mapToDrawRotated = mapToDraw;
		} else if (dirString == "down") {
			for (int i = 0; i < gridWidth; i++) {
				for (int j = 0; j < gridWidth; j++) {
					mapToDrawRotated[i][j] = mapToDraw[gridWidth - j - 1][i];
				}
			}
		} else if (dirString == "up") {
			for (int i = 0; i < gridWidth; i++) {
				for (int j = 0; j < gridWidth; j++) {
					mapToDrawRotated[i][j] = mapToDraw[j][gridWidth - i - 1];
				}
			}
		}

		// Set the map text
		FString mapText = TEXT("");
		for (int i = 0; i < gridWidth; i++) {
			for (int j = 0; j < gridWidth; j++) {
				if (mapToDrawRotated[i][j].Len() == 0) {
					mapToDrawRotated[i][j] = TEXT(" ");
				}
				mapText += mapToDrawRotated[i][j];
			}
			if (i < gridWidth - 1) {
				mapText += TEXT("\n");
			}
		}

		// Set the direction indicator
		FString indicatorText = TEXT("");
		if (dirString == "up") {
			indicatorText =  TEXT("         \n");
			indicatorText += TEXT("  ← N    \n");
			indicatorText += TEXT("         \n");
		} else if (dirString == "down") {
			indicatorText =  TEXT("         \n");
			indicatorText += TEXT("    N →  \n");
			indicatorText += TEXT("         \n");
		} else if (dirString == "left") {
			indicatorText =  TEXT("         \n");
			indicatorText += TEXT("    N    \n");
			indicatorText += TEXT("    ↓    \n");
		} else if (dirString == "right") {
			indicatorText =  TEXT("         \n");
			indicatorText += TEXT("    ↑    \n");
			indicatorText += TEXT("    N    \n");
		}

		// Update the widget texts
		UWidgetComponent* WidgetComponentMap = Cast<UWidgetComponent>(refToInventoryActor->GetComponentByClass(UWidgetComponent::StaticClass()));
		if (WidgetComponentMap != nullptr) {	
			UUserWidget* UserWidgetMap = WidgetComponentMap->GetUserWidgetObject();
			if (UserWidgetMap != nullptr) {
				UTextBlock* MapBox = Cast<UTextBlock>(UserWidgetMap->GetWidgetFromName(TEXT("TextMap")));
				if (MapBox != nullptr) {
					MapBox->SetText(FText::FromString(mapText));
				} else {
					UE_LOG(LogTemp, Warning, TEXT("MAP - Map box not found"));
				}
				UTextBlock* DirectionBox = Cast<UTextBlock>(UserWidgetMap->GetWidgetFromName(TEXT("TextIndicator")));
				if (DirectionBox != nullptr) {
					DirectionBox->SetText(FText::FromString(indicatorText));
				} else {
					UE_LOG(LogTemp, Warning, TEXT("MAP - Direction box not found"));
				}
			}
		}

	}

	// The bottom bar should vaguely following the player
	FVector newLoc = dir * 125.0f;
	newLoc.Z = 50.0f;
	refToUIActor->SetActorLocation(newLoc);
	FRotator uiRotation = dir.Rotation();
	uiRotation.Pitch = 45.0f;
	uiRotation.Yaw += 180.0f;
	uiRotation.Roll = 0.0f;
	refToUIActor->SetActorRotation(uiRotation);

	// Keep the description panel following the player rotation
	if (rmbOn) {

		// Move it to the right location
		FVector descriptionLocation = playerLocation + playerForward * 150.0f + playerRight * 30.0f;

		// Set the rotation towards the player
		FRotator descriptionRotation = playerForward.Rotation();
		descriptionRotation.Pitch = 0.0f;
		descriptionRotation.Yaw += 190.0f;
		descriptionRotation.Roll = 0.0f;

		// If in VR
		if (vrEnabled) {

			// The vector from the player to the hand
			FVector handToHead = handLocation - playerLocation;
			double handToHeadLength = handToHead.Size();
			handToHead.Normalize();

			// Get the vectors perpendicular to this one
			FVector handToHeadRight = -FVector(handToHead.Y, -handToHead.X, 0.0f);
			if (activeHand == "left") {
				handToHeadRight *= -1.0f;
			}
			FVector handToHeadUp = FVector(0.0f, 0.0f, 1.0f);

			// The location
			descriptionLocation = playerLocation + handToHead * (100.0f + handToHeadLength * 2) + handToHeadRight * 60.0f + handToHeadUp * 70.0f;

			// The rotation
			FRotator handToHeadRotation = handToHead.Rotation();
			handToHeadRotation.Pitch = 0.0f;
			if (activeHand == "left") {
				handToHeadRotation.Yaw += 170.0f;
			} else {
				handToHeadRotation.Yaw += 190.0f;
			}
			handToHeadRotation.Roll = 0.0f;
			descriptionRotation = handToHeadRotation;

		}

		// Set the location and rotation of the description
		refToDescriptionActor->SetActorLocation(descriptionLocation);
		refToDescriptionActor->SetActorRotation(descriptionRotation);

	}

	// If dragging the inventory panel, set the relative location and rotation
	if (draggingInventory) {

		// Move it to the right location
		inventoryRelLoc = playerLocation + playerForward * inventoryGrabDistance + playerRight * inventoryGrabPoint.X + playerUp * inventoryGrabPoint.Y;
		if (vrEnabled) {
			inventoryRelLoc = handLocation + handForward * inventoryGrabDistance + handRight * inventoryGrabPoint.X + handUp * inventoryGrabPoint.Y;
		}

		// Set the rotation towards the player
		inventoryRelRot = (inventoryRelLoc - playerLocation).Rotation();
		inventoryRelRot.Pitch *= -1.0f;
		inventoryRelRot.Yaw += 180.0f;

		// These should always be as though the player is facing up
		if (dirString == "right") {
			inventoryRelLoc = inventoryRelLoc.RotateAngleAxis(90.0f, playerUp);
			inventoryRelRot.Yaw += 90.0f;
		} else if (dirString == "left") {
			inventoryRelLoc = inventoryRelLoc.RotateAngleAxis(-90.0f, playerUp);
			inventoryRelRot.Yaw -= 90.0f;
		} else if (dirString == "down") {
			inventoryRelLoc = inventoryRelLoc.RotateAngleAxis(180.0f, playerUp);
			inventoryRelRot.Yaw += 180.0f;
		}

	}

	// Inventory should snap to the correct quarter
	FVector inventoryRelLocRotated = inventoryRelLoc;
	FRotator inventoryRelRotRotated = inventoryRelRot;
	if (dirString == "right") {
		inventoryRelLocRotated = inventoryRelLoc.RotateAngleAxis(-90.0f, playerUp);
		inventoryRelRotRotated.Yaw -= 90.0f;
	} else if (dirString == "left") {
		inventoryRelLocRotated = inventoryRelLoc.RotateAngleAxis(90.0f, playerUp);
		inventoryRelRotRotated.Yaw += 90.0f;
	} else if (dirString == "down") {
		inventoryRelLocRotated = inventoryRelLoc.RotateAngleAxis(180.0f, playerUp);
		inventoryRelRotRotated.Yaw -= 180.0f;
	}
	refToInventoryActor->SetActorLocation(inventoryRelLocRotated);
	refToInventoryActor->SetActorRotation(inventoryRelRotRotated);

	// The settings panel should also snap
	FVector settingsLocation = dir * 140.0f;
	settingsLocation.Z = 200.0f;
	refToSettingsActor->SetActorLocation(settingsLocation);
	FRotator settingsRotation = dir.Rotation();
	settingsRotation.Pitch = 0.0f;
	settingsRotation.Yaw += 180.0f;
	settingsRotation.Roll = 0.0f;
	refToSettingsActor->SetActorRotation(settingsRotation);

	// The tutorial should also snap
	FVector tutorialLocation = dir * 200.0f;
	tutorialLocation.Z = 200.0f;
	refToTutorialActor->SetActorLocation(tutorialLocation);
	FRotator tutorialRotation = dir.Rotation();
	tutorialRotation.Pitch = 0.0f;
	tutorialRotation.Yaw += 180.0f;
	tutorialRotation.Roll = 0.0f;
	refToTutorialActor->SetActorRotation(tutorialRotation);

	// The keyboard should also snap
	FVector keyboardLoc = dir * 125.0f;
	keyboardLoc.Z = 120.0f;
	refToKeyboardActor->SetActorLocation(keyboardLoc);
	FRotator keyboardRotation = dir.Rotation();
	keyboardRotation.Pitch = 40.0f;
	keyboardRotation.Yaw += 180.0f;
	keyboardRotation.Roll = 0.0f;
	refToKeyboardActor->SetActorRotation(keyboardRotation);

	// The altar menu should also snap
	FVector altarLocation = dir * 100.0f;
	altarLocation.Z = 200.0f;
	refToAltarActor->SetActorLocation(altarLocation);
	FRotator altarRotation = dir.Rotation();
	altarRotation.Pitch = 0.0f;
	altarRotation.Yaw += 180.0f;
	altarRotation.Roll = 0.0f;
	refToAltarActor->SetActorRotation(altarRotation);

	// The shop menu should also snap
	FVector shopLocation = dir * 150.0f;
	shopLocation.Z = 200.0f;
	refToShopActor->SetActorLocation(shopLocation);
	FRotator shopRotation = dir.Rotation();
	shopRotation.Pitch = 0.0f;
	shopRotation.Yaw += 180.0f;
	shopRotation.Roll = 0.0f;
	refToShopActor->SetActorRotation(shopRotation);

	// If told to redraw the inventory
	if (shouldRedrawInventory) {

		// Get the widget component of the inventory actor
		UWidgetComponent* WidgetComponent = Cast<UWidgetComponent>(refToInventoryActor->GetComponentByClass(UWidgetComponent::StaticClass()));
		if (WidgetComponent != nullptr) {
			UUserWidget* UserWidget = WidgetComponent->GetUserWidgetObject();
			if (UserWidget != nullptr) {

				// For each grid
				for (int i = 0; i < inventoryLocToLetter.Num(); i++) {
					for (int j = 0; j < inventoryLocToLetter[i].Num(); j++) {

						// Look for the button image e.g. ItemImage_1_1
						FString buttonName = "ItemImage_" + FString::FromInt(i+1) + "_" + FString::FromInt(j+1);
						UImage* ButtonImage = Cast<UImage>(UserWidget->GetWidgetFromName(*buttonName));
						if (ButtonImage != nullptr) {

							// If there's an item there
							if (inventoryLocToLetter[i][j] != TEXT("") && inventoryLetterToName.Contains(inventoryLocToLetter[i][j])) {
								FString matName = itemNameToTextureName(inventoryLetterToName[inventoryLocToLetter[i][j]]);
								UTexture2D* tex2D = getTexture(matName);
								ButtonImage->SetBrushFromTexture(tex2D);
								ButtonImage->SetVisibility(ESlateVisibility::Visible);

							// Otherwise set it to empty
							} else {
								ButtonImage->SetVisibility(ESlateVisibility::Hidden);
							}

						}

					}
				}

				// Redraw the gold
				UTextBlock* GoldText = Cast<UTextBlock>(UserWidget->GetWidgetFromName(TEXT("TextGold")));
				if (GoldText != nullptr) {
					GoldText->SetText(FText::FromString(FString::FromInt(gold)));
				}

			}
		}

		// We don't need to redraw anymore
		shouldRedrawInventory = false;

		// Save everything
		if (hasLoaded) {
			saveEverything();
		}

	}

	// If told to redraw the hotbar
	if (shouldRedrawHotbar) {

		// Get the widget component of the ui actor
		UWidgetComponent* WidgetComponent = Cast<UWidgetComponent>(refToUIActor->GetComponentByClass(UWidgetComponent::StaticClass()));
		if (WidgetComponent != nullptr) {
			UUserWidget* UserWidget = WidgetComponent->GetUserWidgetObject();
			if (UserWidget != nullptr) {

				// For each hotbar element
				for (int i = 0; i < hotbarInfos.Num(); i++) {

					// Get the image
					FString buttonName = "SlotImage" + FString::FromInt(i+1);
					UImage* ButtonImage = Cast<UImage>(UserWidget->GetWidgetFromName(*buttonName));
					if (ButtonImage != nullptr) {

						// If it's empty
						if (hotbarInfos[i].letter == TEXT("")) {
							ButtonImage->SetVisibility(ESlateVisibility::Hidden);

						// Otherwise, set the texture
						} else {
							FString matName = itemNameToTextureName(hotbarInfos[i].name);
							UTexture2D* tex2D = getTexture(matName);
							ButtonImage->SetBrushFromTexture(tex2D);
							ButtonImage->SetVisibility(ESlateVisibility::Visible);	
						}
					}

				}

				// For the equip slot
				UImage* EquipImage = Cast<UImage>(UserWidget->GetWidgetFromName(TEXT("EquippedImage")));
				if (EquipImage != nullptr) {

					// If it's empty
					if (equippedInfo.letter == TEXT("")) {
						EquipImage->SetVisibility(ESlateVisibility::Hidden);

					// Otherwise, set the texture
					} else {
						FString matName = itemNameToTextureName(equippedInfo.name);
						UTexture2D* tex2D = getTexture(matName);
						EquipImage->SetBrushFromTexture(tex2D);
						EquipImage->SetVisibility(ESlateVisibility::Visible);	
					}

				}

			}
		}

		// We don't need to redraw anymore
		shouldRedrawHotbar = false;

		// Save everything
		if (hasLoaded) {
			saveEverything();
		}

	}

	// If told to redraw the passive list
	if (shouldRedrawPassives) {

		// Get the inventory widget
		UWidgetComponent* WidgetComponent = Cast<UWidgetComponent>(refToInventoryActor->GetComponentByClass(UWidgetComponent::StaticClass()));
		if (WidgetComponent != nullptr) {
			UUserWidget* UserWidget = WidgetComponent->GetUserWidgetObject();
			if (UserWidget != nullptr) {

				// Where to start and end based on the page
				int perPage = 12;
				int pagesNeeded = FMath::CeilToInt((float)passives.Num() / (float)perPage);
				passivePage = FMath::Clamp(passivePage, 0, pagesNeeded-1);
				int startIndex = passivePage * perPage;
				int endIndex = FMath::Min(passives.Num(), (passivePage+1) * perPage);
				pagesNeeded = FMath::Max(pagesNeeded, 1);

				// Set the page counter
				UTextBlock* PageCounter = Cast<UTextBlock>(UserWidget->GetWidgetFromName(TEXT("TextPassivesPage")));
				if (PageCounter != nullptr) {
					PageCounter->SetText(FText::FromString(FString::FromInt(passivePage+1) + "/" + FString::FromInt(pagesNeeded)));
				}

				// For each passive
				for (int i = startIndex; i < endIndex; i++) {
					int ind = i-startIndex+1;
					FString passiveName = passives[i];
					
					// Set the name
					FString buttonName = "TextPassive" + FString::FromInt(ind);
					UTextBlock* PassiveName = Cast<UTextBlock>(UserWidget->GetWidgetFromName(*buttonName));
					if (PassiveName != nullptr) {
						PassiveName->SetText(FText::FromString(passiveName));
					}

				}

				// If not all passives, the rest should be blank
				for (int i = endIndex-startIndex+1; i <= perPage; i++) {
					FString buttonName = "TextPassive" + FString::FromInt(i);
					UTextBlock* PassiveName = Cast<UTextBlock>(UserWidget->GetWidgetFromName(*buttonName));
					if (PassiveName != nullptr) {
						PassiveName->SetText(FText::FromString(TEXT("")));
					}
				}

			}
		}

	}

	// If told to redraw the spell list
	if (shouldRedrawSpells) {

		// Get the inventory widget
		UWidgetComponent* WidgetComponent = Cast<UWidgetComponent>(refToInventoryActor->GetComponentByClass(UWidgetComponent::StaticClass()));
		if (WidgetComponent != nullptr) {
			UUserWidget* UserWidget = WidgetComponent->GetUserWidgetObject();
			if (UserWidget != nullptr) {

				// Where to start and end based on the page
				int perPage = 10;
				int pagesNeeded = FMath::CeilToInt((float)spellLetters.Num() / (float)perPage);
				spellPage = FMath::Clamp(spellPage, 0, pagesNeeded-1);
				int startIndex = spellPage * perPage;
				int endIndex = FMath::Min(spellLetters.Num(), (spellPage+1) * perPage);
				pagesNeeded = FMath::Max(pagesNeeded, 1);

				// Set the spell level text
				UTextBlock* SpellLevelText = Cast<UTextBlock>(UserWidget->GetWidgetFromName(TEXT("TextSpellLevels")));
				if (SpellLevelText != nullptr) {
					if (memorizing) {
						SpellLevelText->SetVisibility(ESlateVisibility::Visible);
						if (spellLevels > 0) {
							SpellLevelText->SetText(FText::FromString(FString::FromInt(spellLevels) + " spell levels remaining"));
						} else {
							SpellLevelText->SetText(FText::FromString("no spell levels remaining"));
						}
					} else {
						SpellLevelText->SetVisibility(ESlateVisibility::Hidden);
					}
				}

				// Set the page counter
				UTextBlock* PageCounter = Cast<UTextBlock>(UserWidget->GetWidgetFromName(TEXT("TextSpellsPage")));
				if (PageCounter != nullptr) {
					PageCounter->SetText(FText::FromString(FString::FromInt(spellPage+1) + "/" + FString::FromInt(pagesNeeded)));
				}

				// For each spell
				for (int i = startIndex; i < endIndex; i++) {
					int ind = i-startIndex+1;
					FString spellLetter = spellLetters[i];
					SpellInfo spellInfo = spellLetterToInfo[spellLetter];
					
					// Set the name
					FString buttonName = "TextSpellName" + FString::FromInt(ind);
					UTextBlock* SpellName = Cast<UTextBlock>(UserWidget->GetWidgetFromName(*buttonName));
					if (SpellName != nullptr) {
						SpellName->SetText(FText::FromString(spellInfo.name));
					}

					// Set the school
					FString buttonName2 = "TextSpellSchool" + FString::FromInt(ind);
					UTextBlock* SpellSchool = Cast<UTextBlock>(UserWidget->GetWidgetFromName(*buttonName2));
					if (SpellSchool != nullptr) {
						SpellSchool->SetText(FText::FromString(spellInfo.school));
					}

					// Set the level
					FString buttonName3 = "TextSpellLevel" + FString::FromInt(ind);
					UTextBlock* SpellLevel = Cast<UTextBlock>(UserWidget->GetWidgetFromName(*buttonName3));
					if (SpellLevel != nullptr) {
						SpellLevel->SetText(FText::FromString(FString::FromInt(spellInfo.level)));
					}

					// Set the failure
					FString buttonName4 = "TextSpellFailure" + FString::FromInt(ind);
					UTextBlock* SpellFailure = Cast<UTextBlock>(UserWidget->GetWidgetFromName(*buttonName4));
					if (SpellFailure != nullptr) {
						SpellFailure->SetText(FText::FromString(FString::FromInt(spellInfo.failure) + "%"));
					}

					// Set the image based on the name
					FString matName = itemNameToTextureName(spellInfo.name);
					UTexture2D* tex2D = getTexture(matName);
					FString buttonName5 = "ImageSpell" + FString::FromInt(ind);
					UImage* SpellImage = Cast<UImage>(UserWidget->GetWidgetFromName(*buttonName5));
					if (SpellImage != nullptr) {
						SpellImage->SetBrushFromTexture(tex2D);
						SpellImage->SetVisibility(ESlateVisibility::Visible);
					}

				}

				// If not all spells, the rest should be blank
				for (int i = endIndex-startIndex+1; i <= perPage; i++) {
					FString buttonName = "TextSpellName" + FString::FromInt(i);
					UTextBlock* SpellName = Cast<UTextBlock>(UserWidget->GetWidgetFromName(*buttonName));
					if (SpellName != nullptr) {
						SpellName->SetText(FText::FromString(TEXT("")));
					}
					FString buttonName2 = "TextSpellSchool" + FString::FromInt(i);
					UTextBlock* SpellSchool = Cast<UTextBlock>(UserWidget->GetWidgetFromName(*buttonName2));
					if (SpellSchool != nullptr) {
						SpellSchool->SetText(FText::FromString(TEXT("")));
					}
					FString buttonName3 = "TextSpellLevel" + FString::FromInt(i);
					UTextBlock* SpellLevel = Cast<UTextBlock>(UserWidget->GetWidgetFromName(*buttonName3));
					if (SpellLevel != nullptr) {
						SpellLevel->SetText(FText::FromString(TEXT("")));
					}
					FString buttonName4 = "TextSpellFailure" + FString::FromInt(i);
					UTextBlock* SpellFailure = Cast<UTextBlock>(UserWidget->GetWidgetFromName(*buttonName4));
					if (SpellFailure != nullptr) {
						SpellFailure->SetText(FText::FromString(TEXT("")));
					}
					FString buttonName5 = "ImageSpell" + FString::FromInt(i);
					UImage* SpellImage = Cast<UImage>(UserWidget->GetWidgetFromName(*buttonName5));
					if (SpellImage != nullptr) {
						SpellImage->SetBrushFromTexture(nullptr);
						SpellImage->SetVisibility(ESlateVisibility::Hidden);
					}
				}

			}
		}

		// We don't need to redraw anymore
		shouldRedrawSpells = false;

	}

	// If told to redraw the ability list
	if (shouldRedrawAbilities) {

		// Get the widget component of the ui actor
		UWidgetComponent* WidgetComponent = Cast<UWidgetComponent>(refToInventoryActor->GetComponentByClass(UWidgetComponent::StaticClass()));
		if (WidgetComponent != nullptr) {
			UUserWidget* UserWidget = WidgetComponent->GetUserWidgetObject();
			if (UserWidget != nullptr) {

				// Where to start and end based on the page
				int perPage = 10;
				int pagesNeeded = FMath::CeilToInt((float)abilityLetters.Num() / (float)perPage);
				abilityPage = FMath::Clamp(abilityPage, 0, pagesNeeded-1);
				int startIndex = abilityPage * perPage;
				int endIndex = FMath::Min(abilityLetters.Num(), (abilityPage+1) * perPage);
				pagesNeeded = FMath::Max(pagesNeeded, 1);

				// Set the page counter
				UTextBlock* PageCounter = Cast<UTextBlock>(UserWidget->GetWidgetFromName(TEXT("TextAbilitiesPage")));
				if (PageCounter != nullptr) {
					PageCounter->SetText(FText::FromString(FString::FromInt(abilityPage+1) + "/" + FString::FromInt(pagesNeeded)));
				}

				// For each ability
				for (int i = startIndex; i < endIndex; i++) {
					int ind = i+1;
					FString abilityLetter = abilityLetters[i];
					AbilityInfo abilityInfo = abilityLetterToInfo[abilityLetter];

					// Skip some abilities from wizard mode
					FString matName = itemNameToTextureName(abilityInfo.name);
					if (matName == "BuildTerrain"
						|| matName == "SetTerrainToBuild"
						|| matName == "ClearTerrainToFloor") {
						continue;
					}
					
					// Set the name
					FString buttonName = "TextAbilityName" + FString::FromInt(ind);
					UTextBlock* AbilityName = Cast<UTextBlock>(UserWidget->GetWidgetFromName(*buttonName));
					if (AbilityName != nullptr) {
						AbilityName->SetText(FText::FromString(abilityInfo.name));
					}

					// Set the cost
					FString buttonName2 = "TextAbilityCost" + FString::FromInt(ind);
					UTextBlock* AbilityCost = Cast<UTextBlock>(UserWidget->GetWidgetFromName(*buttonName2));
					if (AbilityCost != nullptr) {
						AbilityCost->SetText(FText::FromString(abilityInfo.cost));
					}

					// Set the failure
					FString buttonName3 = "TextAbilityFailure" + FString::FromInt(ind);
					UTextBlock* AbilityFailure = Cast<UTextBlock>(UserWidget->GetWidgetFromName(*buttonName3));
					if (AbilityFailure != nullptr) {
						AbilityFailure->SetText(FText::FromString(FString::FromInt(abilityInfo.failure) + "%"));
					}

					// Set the image based on the name
					UTexture2D* tex2D = getTexture(matName);
					FString buttonName4 = "ImageAbility" + FString::FromInt(ind);
					UImage* AbilityImage = Cast<UImage>(UserWidget->GetWidgetFromName(*buttonName4));
					if (AbilityImage != nullptr) {
						AbilityImage->SetBrushFromTexture(tex2D);
						AbilityImage->SetVisibility(ESlateVisibility::Visible);
					}

				}

				// If not all abilities, the rest should be blank
				for (int i = endIndex-startIndex+1; i <= perPage; i++) {
					FString buttonName = "TextAbilityName" + FString::FromInt(i);
					UTextBlock* AbilityName = Cast<UTextBlock>(UserWidget->GetWidgetFromName(*buttonName));
					if (AbilityName != nullptr) {
						AbilityName->SetText(FText::FromString(TEXT("")));
					}
					FString buttonName2 = "TextAbilityCost" + FString::FromInt(i);
					UTextBlock* AbilityCost = Cast<UTextBlock>(UserWidget->GetWidgetFromName(*buttonName2));
					if (AbilityCost != nullptr) {
						AbilityCost->SetText(FText::FromString(TEXT("")));
					}
					FString buttonName3 = "TextAbilityFailure" + FString::FromInt(i);
					UTextBlock* AbilityFailure = Cast<UTextBlock>(UserWidget->GetWidgetFromName(*buttonName3));
					if (AbilityFailure != nullptr) {
						AbilityFailure->SetText(FText::FromString(TEXT("")));
					}
					FString buttonName4 = "ImageAbility" + FString::FromInt(i);
					UImage* AbilityImage = Cast<UImage>(UserWidget->GetWidgetFromName(*buttonName4));
					if (AbilityImage != nullptr) {
						AbilityImage->SetBrushFromTexture(nullptr);
						AbilityImage->SetVisibility(ESlateVisibility::Hidden);
					}
				}
			}
		}

		// We don't need to redraw anymore
		shouldRedrawAbilities = false;

	}

	// If told to redraw the overview
	if (shouldRedrawOverview) {

		// Set the text
		UWidgetComponent* WidgetComponent2 = Cast<UWidgetComponent>(refToInventoryActor->GetComponentByClass(UWidgetComponent::StaticClass()));
		if (WidgetComponent2 != nullptr) {
			UUserWidget* UserWidget = WidgetComponent2->GetUserWidgetObject();
			if (UserWidget != nullptr) {
				UTextBlock* OverviewText = Cast<UTextBlock>(UserWidget->GetWidgetFromName(TEXT("TextOverview")));
				if (OverviewText != nullptr) {
					OverviewText->SetText(FText::FromString(overviewText));
				}
			}
		}

	}

	// If told to redraw the religion
	if (shouldRedrawReligion) {

		// Set the text
		UWidgetComponent* WidgetComponent2 = Cast<UWidgetComponent>(refToInventoryActor->GetComponentByClass(UWidgetComponent::StaticClass()));
		if (WidgetComponent2 != nullptr) {
			UUserWidget* UserWidget = WidgetComponent2->GetUserWidgetObject();
			if (UserWidget != nullptr) {
				UTextBlock* ReligionText = Cast<UTextBlock>(UserWidget->GetWidgetFromName(TEXT("TextReligion")));
				if (ReligionText != nullptr) {
					ReligionText->SetText(FText::FromString(religionText));
				}
			}
		}

	}

	// If told to redraw the skill list
	if (shouldRedrawSkills) {

		// Get the widget component of the ui actor
		UWidgetComponent* WidgetComponent = Cast<UWidgetComponent>(refToInventoryActor->GetComponentByClass(UWidgetComponent::StaticClass()));
		if (WidgetComponent != nullptr) {
			UUserWidget* UserWidget = WidgetComponent->GetUserWidgetObject();
			if (UserWidget != nullptr) {

				// The colors
				FLinearColor focusColorStar = FLinearColor(0.0f, 1.0f, 0.0f, 1.0f);
				FLinearColor focusColorPlus = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);
				FLinearColor focusColorMinus = FLinearColor(1.0f, 0.0f, 0.0f, 1.0f);

				// For each skill
				for (auto const& skill : skillNameToInfo) {
					if (skill.Key.Len() == 0) {
						continue;
					}

					// Get the name, taking only the first word
					TArray<FString> parts;
					skill.Key.ParseIntoArray(parts, TEXT(" "), true);
					FString skillName = parts[0];

					// The color of the row
					FLinearColor rowColor = focusColorPlus;
					if (skill.Value.focussed == 1) {
						rowColor = focusColorStar;
					} else if (skill.Value.focussed == -1) {
						rowColor = focusColorMinus;
					}

					// Set the name
					FString buttonName4 = "Text" + skillName + "Name";
					UTextBlock* SkillName = Cast<UTextBlock>(UserWidget->GetWidgetFromName(*buttonName4));
					if (SkillName != nullptr) {
						SkillName->SetColorAndOpacity(rowColor);
					}

					// Set the level
					FString buttonName = "Text" + skillName + "Level";
					UTextBlock* SkillLevel = Cast<UTextBlock>(UserWidget->GetWidgetFromName(*buttonName));
					if (SkillLevel != nullptr) {
						SkillLevel->SetText(FText::FromString(FString::SanitizeFloat(skill.Value.level, 1)));
						SkillLevel->SetColorAndOpacity(rowColor);
					} else {
						UE_LOG(LogTemp, Warning, TEXT("Skill not found: %s"), *skillName);
					}

					// Set the aptitude
					FString buttonName2 = "Text" + skillName + "Apt";
					UTextBlock* SkillApt = Cast<UTextBlock>(UserWidget->GetWidgetFromName(*buttonName2));
					if (SkillApt != nullptr) {
						SkillApt->SetText(FText::FromString(FString::FromInt(skill.Value.apt)));
						SkillApt->SetColorAndOpacity(rowColor);
					}

					// Set the train percentage
					FString buttonName3 = "Text" + skillName + "Train";
					UTextBlock* SkillTrain = Cast<UTextBlock>(UserWidget->GetWidgetFromName(*buttonName3));
					if (SkillTrain != nullptr) {
						SkillTrain->SetText(FText::FromString(FString::FromInt(skill.Value.train) + "%"));
						SkillTrain->SetColorAndOpacity(rowColor);
					}
					
				}

			}
		}

		// We don't need to redraw anymore
		shouldRedrawSkills = false;

	}

	// Keep the ui panel following the player
	FVector uiLocation = playerLocation + FVector(100.0f, -20.0f, -50.0f);

	// Get the widget component of the ui actor
	UWidgetComponent* WidgetComponent = Cast<UWidgetComponent>(refToUIActor->GetComponentByClass(UWidgetComponent::StaticClass()));
	if (WidgetComponent != nullptr) {
		UUserWidget* UserWidget = WidgetComponent->GetUserWidgetObject();
		if (UserWidget != nullptr) {

			// Set the health bar value
			UProgressBar* HealthBar = Cast<UProgressBar>(UserWidget->GetWidgetFromName(TEXT("BarHP")));
			if (HealthBar != nullptr) {
				HealthBar->SetPercent(float(currentHP) / float(maxHP));
			}

			// Set the mana bar value
			UProgressBar* ManaBar = Cast<UProgressBar>(UserWidget->GetWidgetFromName(TEXT("BarMP")));
			if (ManaBar != nullptr) {
				ManaBar->SetPercent(float(currentMP) / float(maxMP));
			}

			// Set the HP text
			UTextBlock* HPText = Cast<UTextBlock>(UserWidget->GetWidgetFromName(TEXT("TextHP")));
			if (HPText != nullptr) {
				HPText->SetText(FText::FromString(FString::FromInt(currentHP) + "/" + FString::FromInt(maxHP)));
			}

			// Set the MP text
			UTextBlock* MPText = Cast<UTextBlock>(UserWidget->GetWidgetFromName(TEXT("TextMP")));
			if (MPText != nullptr) {
				MPText->SetText(FText::FromString(FString::FromInt(currentMP) + "/" + FString::FromInt(maxMP)));
			}

			// Set the left text
			UTextBlock* LeftText = Cast<UTextBlock>(UserWidget->GetWidgetFromName(TEXT("TextLeft")));
			if (LeftText != nullptr) {
				LeftText->SetText(FText::FromString(leftText));
			}

			// Set the right text
			UTextBlock* RightText = Cast<UTextBlock>(UserWidget->GetWidgetFromName(TEXT("TextRight")));
			if (RightText != nullptr) {
				RightText->SetText(FText::FromString(rightText));
			}

			// Set the status text
			UTextBlock* StatusText = Cast<UTextBlock>(UserWidget->GetWidgetFromName(TEXT("TextStatus")));
			if (StatusText != nullptr) {
				StatusText->SetText(FText::FromString(statusText));
			}

		}
	}

	// Do an instruction from the queue
	if (commandQueue.Num() > 0 && prevOutput.Contains("===READY===") && crawlHasStarted && (serverConnected || !useServer)) {
		UE_LOG(LogTemp, Display, TEXT("Doing command %s"), *commandQueue[0]);
		FString command = commandQueue[0];
		if (command == "CLEAR") {
			UE_LOG(LogTemp, Display, TEXT("Clearing things"));
			clearThings();
		} else if (command == "CLEARINV") {
			UE_LOG(LogTemp, Display, TEXT("Clearing inventory"));
			inventoryLetterToName.Empty();
		} else if (command == "CLEARSPELLS") {
			UE_LOG(LogTemp, Display, TEXT("Clearing spells"));
			spellLetters.Empty();
			spellLetterToInfo.Empty();
		} else if (command == "SKIP") {
			skipNextFullDescription = true;
		} else {
			writeCommand(command);
		}
		commandQueue.RemoveAt(0);
	}

	// Turn all enemy planes towards the player
	for (int i = 0; i < enemyUseCount; i++) {
		FVector enemyLocation = enemyArray[i]->GetActorLocation();
		FRotator newRotation = (playerLocation - enemyLocation).Rotation();
		newRotation.Pitch = 0.0f;
		newRotation.Yaw += -90.0f;
		newRotation.Roll = 90.0f;
		enemyArray[i]->SetActorRotation(newRotation);
	}

	// Turn all item planes towards the player
	for (int i = 0; i < itemUseCount; i++) {
		FVector itemLocation = itemArray[i]->GetActorLocation();

		// If it's a corpse or skeleton, keep it facing up
		if (itemArray[i]->GetActorRotation().Roll == 0.0f) {
			continue;
		}

		// Otherwise, upright facing the player
		FRotator newRotation = (playerLocation - itemLocation).Rotation();
		newRotation.Pitch = 0.0f;
		newRotation.Yaw += -90.0f;
		newRotation.Roll = 90.0f;
		itemArray[i]->SetActorRotation(newRotation);

	}

	// Turn all effect planes towards the player
	for (int i = 0; i < effectUseCount; i++) {
		FVector effectLocation = effectArray[i]->GetActorLocation();
		FRotator newRotation = (playerLocation - effectLocation).Rotation();
		newRotation.Pitch = 0.0f;
		newRotation.Yaw += -90.0f;
		newRotation.Roll = 90.0f;
		effectArray[i]->SetActorRotation(newRotation);
	}

	// Read the pipe
	outText = "";
	if (!useServer) {
		outText = FPlatformProcess::ReadPipe(StdOutReadHandle);
		if (outText.Len() > 0) {
			prevOutput = outText;
			outputBuffer += outText;
		}

	// Or read from the server
	} else if (needNewRequest && serverAddress.Len() > 0) {
		needNewRequest = false;
		TSharedRef<IHttpRequest> req = (&FHttpModule::Get())->CreateRequest();
		req->SetVerb("GET");
		FString url = serverAddress + "get";
		req->SetURL(url);
		req->OnProcessRequestComplete().BindLambda([this](
				FHttpRequestPtr pRequest,
				FHttpResponsePtr pResponse,
				bool connectedSuccessfully) mutable {
					if (connectedSuccessfully) {
						int code = pResponse->GetResponseCode();
						FString response = *pResponse->GetContentAsString();
						if (code == 200) {
							serverConnected = true;
							outText = response;
							if (outText.Len() > 0) {
								prevOutput = outText;
								outputBuffer += outText;
							}
							if (response.Len() > 0) {
								UE_LOG(LogTemp, Display, TEXT("Server responded with %i bytes"), response.Len());
							}
						} else {
							UE_LOG(LogTemp, Warning, TEXT("Server returned error code %d: %s"), code, *response);
							serverConnected = false;
						}
					} else {
						UE_LOG(LogTemp, Warning, TEXT("Failed to connect to server"));
						serverConnected = false;
					}
					FTimerHandle TimerHandle;
					if (worldRef != nullptr) {
						needNewRequest = true;
					}
			});
		req->ProcessRequest();
		httpRequests.Add(req);
	}

	// If it's the menu, set it and stop requesting the menu
	if (needMenu) {
		outText = menuOutput;
		prevOutput = outText;
		outputBuffer += outText;
		needMenu = false;
		UE_LOG(LogTemp, Display, TEXT("Menu output set"));
	}

	// If we have buffer, process it
	bool shouldRedraw = false;
	if (outputBuffer.Len() > 0) {

		// Search the buffer for the start and end markers 
		int32 start = outputBuffer.Find(TEXT("===START==="));
		int32 end = outputBuffer.Mid(start + 11).Find(TEXT("===END===")) + start + 11;
		bool isReady = outputBuffer.Contains(TEXT("===READY==="));

		// If both markers are found
		while (start != INDEX_NONE && end != INDEX_NONE) {

			// Extract the text between them
			FString extracted = outputBuffer.Mid(start + 11, end - start - 11);
			if (extracted.Len() > 0) {
				// UE_LOG(LogTemp, Display, TEXT("Extracted: \"%s\""), *extracted);
				if (!hasLoaded) {
					UE_LOG(LogTemp, Display, TEXT("Should be loaded now"));
					hasLoaded = true;
				}
			} else {
				break;
			}

			// Check if it's the menu
			isMenu = extracted.Contains(TEXT("===MENU===")) || extracted.Contains(TEXT("Linley Henzell"));

			// Remove the extracted text from the buffer
			outputBuffer = outputBuffer.Mid(end + 8);
			numProcessed++;

			// Find the next instance
			start = outputBuffer.Find(TEXT("===START==="));
			end = outputBuffer.Mid(start + 11).Find(TEXT("===END===")) + start + 11;

			// Skip if the extracted text is basically nothing
			if (extracted.Len() < 10) {
				UE_LOG(LogTemp, Display, TEXT("Skipping extracted text"));
				continue;
			}

			// Add to the log of outputs
			lastOutputs.Add(extracted);
			while (lastOutputs.Num() > 10) {
				lastOutputs.RemoveAt(0);
			}

			// Copy the extracted text to the text object
			if (refToTextActor != nullptr) {
				refToTextRender = refToTextActor->GetTextRender();
				if (refToTextRender != nullptr && extracted.Replace(TEXT(" "), TEXT("")).Len() > 0) {
					refToTextRender->SetText(FText::FromString(extracted));
				}
			}

			// If we need to press enter to read the rest
			if (extracted.Contains(TEXT("--more--"))) {
				writeCommandQueued(TEXT("enter"));
			}

			// If we have died
			if (extracted.Contains(TEXT("You die..."))) {
				writeCommandQueued(TEXT("enter"));
				writeCommandQueued(TEXT("enter"));
				writeCommandQueued(TEXT("enter"));
				writeCommandQueued(TEXT("escape"));
			}

			// If we have won
			if (extracted.Contains(TEXT("You have escaped!"))) {
				writeCommandQueued(TEXT("enter"));
				writeCommandQueued(TEXT("enter"));
				writeCommandQueued(TEXT("enter"));
				writeCommandQueued(TEXT("escape"));
			}

			// Extract the level ascii
			extracted.ParseIntoArray(charArray, TEXT("\n"), true);
			for (int i = 0; i < charArray.Num(); i++) {
				charArray[i] = charArray[i].Replace(TEXT("\n"), TEXT(""));
				charArray[i] = charArray[i].Replace(TEXT("\r"), TEXT(""));
			}
			if (useServer) {
				bool firstLineIsBlank = true;
				for (int i = 0; i < charArray[0].Len(); i++) {
					if (charArray[0][i] != ' ') {
						firstLineIsBlank = false;
						break;
					}
				}
				if (!firstLineIsBlank) {
					charArray.Insert(TEXT("      "), 0);
				}
			}
			UE_LOG(LogTemp, Display, TEXT("Extracting level ascii..."));
			int offsetX = 8;
			int offsetY = 1;
			for (int i = 0; i < gridWidth; i++) {
				for (int j = 0; j < gridWidth; j++) {
					if (i+offsetY < charArray.Num() && j+offsetX < charArray[i].Len() && i < levelAscii.Num() && j < levelAscii[i].Num()) {
						levelAscii[i][j] = charArray[i+offsetY].Mid(j+offsetX, 1);
					}
				}
			}
			if (isReady) {
				UE_LOG(LogTemp, Display, TEXT("Buffer contains ready marker"));
			}

			// Output the full charArray
			UE_LOG(LogTemp, Display, TEXT("Full charArray:"));
			for (int i = 0; i < charArray.Num(); i++) {
				UE_LOG(LogTemp, Display, TEXT("%i %s"), i, *charArray[i]);
			}

			// Output the length of each line
			for (int i = 0; i < charArray.Num(); i++) {
				if (charArray[i].Len() != 80 && charArray[i].Len() > 0) {
					UE_LOG(LogTemp, Warning, TEXT("Line %d has %s chars"), i, *charArray[i]);
				}
			}

			// If it's the main menu, get the list of saves
			bool hasSaves = false;
			for (int i = 0; i < charArray.Num(); i++) {
				if (charArray[i].Contains(TEXT("Linley Henzell"))) {
					hasSaves = true;
					break;
				}
			}
			if (hasSaves) {
				inMainMenu = true;

				// We have started
				if (!crawlHasStarted) {
					if (refToMainMenuActor != nullptr) {
						UWidgetComponent* WidgetComponentMainMenu = Cast<UWidgetComponent>(refToMainMenuActor->GetComponentByClass(UWidgetComponent::StaticClass()));
						if (WidgetComponentMainMenu != nullptr) {
							UUserWidget* UserWidgetMainMenu = WidgetComponentMainMenu->GetUserWidgetObject();
							if (UserWidgetMainMenu != nullptr) {
								UTextBlock* PlayButtonText = Cast<UTextBlock>(UserWidgetMainMenu->GetWidgetFromName(TEXT("TextMainPlay")));
								if (PlayButtonText != nullptr) {
									PlayButtonText->SetText(FText::FromString("Play"));
								}
							}
						}
					}
				}
				crawlHasStarted = true;

				// Get the saves in the current menu
				FString quickLoad = TEXT("");
				for (int i = 2; i < charArray.Num(); i++) {
					if (charArray[i].Contains(TEXT("a level"))) {
						FString name = charArray[i].Mid(17, 80-17);
						name = name.Replace(TEXT("[Seeded]"), TEXT(""));
						name = name.TrimStartAndEnd();
						if (!saveNames.Contains(name)) {
							UE_LOG(LogTemp, Display, TEXT("Found save: %s"), *name);
							saveNames.Emplace(name);
						}
					} else if (charArray[i].Contains(TEXT("quick-load"))) {
						quickLoad = charArray[i].Mid(28, 80-28);
						quickLoad = quickLoad.Replace(TEXT("[Seeded]"), TEXT(""));
						quickLoad = quickLoad.TrimStartAndEnd();
						quickLoad = quickLoad.Replace(TEXT("the"), TEXT("")).Replace(TEXT(" "), TEXT("")).Replace(TEXT(" "), TEXT(""));
						UE_LOG(LogTemp, Display, TEXT("Quick load: %s"), *quickLoad);
					}
				}

				// Sort the list as crawl does it
				saveNames.Sort([](const FString& A, const FString& B) {
					if (A.Len() == 0 || B.Len() == 0) {
						return A.Len() <= B.Len();
					}
					FString levelStringA = "";
					for (int i = 0; i < A.Len(); i++) {
						if (A[i] >= '0' && A[i] <= '9') {
							levelStringA += A[i];
						}
					}
					FString levelStringB = "";
					for (int i = 0; i < B.Len(); i++) {
						if (B[i] >= '0' && B[i] <= '9') {
							levelStringB += B[i];
						}
					}
					int levelA = FCString::Atoi(*levelStringA);
					int levelB = FCString::Atoi(*levelStringB);
					if (levelA != levelB) {
						return levelA > levelB;
					} else if (A[0] == B[0]) {
						return A < B;
					} else {
						return A[0] < B[0];
					}
				});
				saveLocToIndex.Empty();
				for (int i = 0; i < saveNames.Num(); i++) {
					saveLocToIndex.Add(i);
				}

				// Move the quick load to the top (also the save loc)
				if (quickLoad.Len() > 0) {
					int index = -1;
					for (int i = 0; i < saveNames.Num(); i++) {
						FString simpName = saveNames[i].Replace(TEXT("a level"), TEXT(""));
						simpName = simpName.Replace(TEXT("0"), TEXT("")).Replace(TEXT("1"), TEXT("")).Replace(TEXT("2"), TEXT("")).Replace(TEXT("3"), TEXT("")).Replace(TEXT("4"), TEXT("")).Replace(TEXT("5"), TEXT("")).Replace(TEXT("6"), TEXT("")).Replace(TEXT("7"), TEXT("")).Replace(TEXT("8"), TEXT("")).Replace(TEXT("9"), TEXT(""));
						simpName = simpName.Replace(TEXT(","), TEXT(""));
						simpName = simpName.Replace(TEXT(" "), TEXT(""));
						simpName = simpName.Replace(TEXT(" "), TEXT(""));
						UE_LOG(LogTemp, Display, TEXT("Comparing %s to %s"), *simpName, *quickLoad);
						if (simpName == quickLoad) {
							index = i;
							break;
						}
					}
					if (index != INDEX_NONE && index >= 0) {
						FString temp = saveNames[index];
						saveNames.RemoveAt(index);
						saveNames.Insert(temp, 0);
						saveLocToIndex.RemoveAt(index);
						saveLocToIndex.Insert(index, 0);
					}
				}

			} else {
				inMainMenu = false;
			}

			// If we just used a scroll and it's asking us where to blink to
			if (locForBlink.X != -1 && locForBlink.Y != -1) {
				bool isBlink = false;
				for (int i = 0; i < charArray.Num(); i++) {
					if (charArray[i].Contains(TEXT("Blink to where?"))) {
						isBlink = true;
						break;
					}
				}
				UE_LOG(LogTemp, Display, TEXT("Is blink: %d"), isBlink);
				if (isBlink) {
					UE_LOG(LogTemp, Display, TEXT("Blinking to %d %d"), locForBlink.X, locForBlink.Y);
					if (locForBlink.X > LOS) {
						for (int i = 0; i < locForBlink.X-LOS; i++) {
							writeCommandQueued(TEXT("right"));
						}
					} else {
						for (int i = 0; i < LOS-locForBlink.X; i++) {
							writeCommandQueued(TEXT("left"));
						}
					}
					if (locForBlink.Y > LOS) {
						for (int i = 0; i < locForBlink.Y-LOS; i++) {
							writeCommandQueued(TEXT("down"));
						}
					} else {
						for (int i = 0; i < LOS-locForBlink.Y; i++) {
							writeCommandQueued(TEXT("up"));
						}
					}
					writeCommandQueued(TEXT("enter"));
					locForBlink = FIntVector2(-1, -1);
				}
			}

			// If we have a scroll of amnesia and read isn't an option
			if (justUsedAScroll) {
				bool cantRead = false;
				for (int i = 0; i < charArray.Num(); i++) {
					if (charArray[i].Contains(TEXT("Reading this right now will have no effect:"))) {
						cantRead = true;
						break;
					}
				}
				if (cantRead) {
					justUsedAScroll = false;
					writeCommandQueued(TEXT("escape"));
					writeCommandQueued(TEXT("escape"));
				}
			}

			// When a scroll has finished
			bool scrollFinished = false;
			for (int i = 0; i < charArray.Num(); i++) {
				if (charArray[i].Contains(TEXT("crumbles to dust"))) {
					scrollFinished = true;
					break;
				}
			}
			if (scrollFinished && justUsedAScroll) {
				writeCommandQueued("CLEARINV");
				writeCommandQueued("i");
				writeCommandQueued(">");
				writeCommandQueued(">");
				writeCommandQueued("escape");
				justUsedAScroll = false;
			}

			// If asking to confirm a cancel
			bool isCancel = false;
			for (int i = 0; i < charArray.Num(); i++) {
				if (
					charArray[i].Contains(TEXT("Are you sure you want to cancel"))
				) {
					isCancel = true;
					break;
				}
			}
			if (isCancel) {
				writeCommandQueued(TEXT("Y"));
			}

			// The death screen
			// 			Goodbye, Ibuc.                                                                  
			//          137 Ibuc the Blocker (level 5, -6/46 HPs) *WIZ*                        
			//              Began as a Minotaur Fighter on Feb 1, 2025.                        
			//              Mangled by Grinder (8 damage)                                      
			//              ... on level 1 of the Dungeon on Feb 16, 2025.                     
			//              The game lasted 00:53:15 (1481 turns).                             
			// Best Crawlers -                                                                 
			//   1.       0 Bupp       GhEn-01 slain by a goblin (D:1)                         
			// You can find your morgue file in the 'C:/Users/Luke/Documents/Unreal Projects/DC
			bool isDeath = false;
			for (int i = 0; i < charArray.Num(); i++) {
				if (charArray[i].Contains(TEXT("Goodbye,"))) {
					isDeath = true;
					break;
				}
			}
			if (isDeath) {
				FString deathText = TEXT("");
				for (int i = 0; i < charArray.Num(); i++) {
					if (charArray[i].Contains(TEXT("find your morgue file"))) {
						continue;
					}
					FString toAdd = charArray[i].Replace(TEXT("*WIZ*"), TEXT("     "));
					deathText += charArray[i] + TEXT("\n");
				}
				UE_LOG(LogTemp, Display, TEXT("Death text: %s"), *deathText);
					
				// Show the death actor and set the text
				if (refToDeathActor != nullptr) {
					UWidgetComponent* WidgetComponentDeath = Cast<UWidgetComponent>(refToDeathActor->GetComponentByClass(UWidgetComponent::StaticClass()));
					if (WidgetComponentDeath != nullptr) {
						UUserWidget* UserWidgetDeath = WidgetComponentDeath->GetUserWidgetObject();
						if (UserWidgetDeath != nullptr) {
							UTextBlock* DeathText = Cast<UTextBlock>(UserWidgetDeath->GetWidgetFromName(TEXT("TextDeath")));
							if (DeathText != nullptr) {
								DeathText->SetText(FText::FromString(deathText));
							}
						}
					}
					refToDeathActor->SetActorHiddenInGame(false);
					refToDeathActor->SetActorEnableCollision(true);
				}

				// Hide everything else
				refToInventoryActor->SetActorHiddenInGame(true);
				refToInventoryActor->SetActorEnableCollision(false);
				refToUIActor->SetActorHiddenInGame(true);
				refToUIActor->SetActorEnableCollision(false);

			}

			// If it's the spell memorize list
			// Spells (Memorise)                 Type                          Failure  Level 
			//  a - Scorch                        Fire                          11%      2     
			//  b - Inner Flame                   Hexes/Fire                    34%      3     
			//  c - Volatile Blastmotes           Fire/Translocation            34%      3     
			//  d - Flame Wave                    Conjuration/Fire              71%      4     
			// 3 spell levels left                                                             
			// [!] Memorise|Describe|Hide|Show   [Ctrl-F] search   [?] help         [Esc] exit 
			bool isMemorizeList = false;
			for (int i = 0; i < charArray.Num(); i++) {
				if (charArray[i].Contains(TEXT("Spells (Mem"))) {
					isMemorizeList = true;
					break;
				}
			}
			if (isMemorizeList) {

				// Get the number of spell levels left
				TArray<FString> parts;
				UE_LOG(LogTemp, Display, TEXT("Extracting spell levels from: %s"), *charArray[charArray.Num()-2]);
				charArray[charArray.Num()-2].ParseIntoArray(parts, TEXT(" "), true);
				if (parts.Num() > 0) {
					spellLevels = FCString::Atoi(*parts[0]);
				}

				// Add spells
				for (int i = 2; i < charArray.Num()-2; i++) {
					if (charArray[i].Contains("-")) {

						// Get the letter
						FString letter = charArray[i].TrimStart().Mid(0, 1);

						// Get the name
						FString name = charArray[i].Mid(4, 30).TrimStartAndEnd();;

						// Get the school
						FString school = charArray[i].Mid(35, 30).TrimStartAndEnd();;

						// Get the failure
						FString failure = charArray[i].Mid(65, 5).TrimStartAndEnd();
						int failureInt = FCString::Atoi(*failure.Left(failure.Len()-1));

						// Get the level
						FString level = charArray[i].Mid(74, 5).TrimStartAndEnd();
						int levelInt = FCString::Atoi(*level);

						// If the letter is already in the map but with a different name, change the letter
						FString baseLetter = letter;
						if (spellLetterToInfo.Contains(letter) && spellLetterToInfo[letter].name != name) {
							letter = letter + baseLetter;
						}
						if (spellLetterToInfo.Contains(letter) && spellLetterToInfo[letter].name != name) {
							letter = letter + baseLetter;
						}
						if (spellLetterToInfo.Contains(letter) && spellLetterToInfo[letter].name != name) {
							letter = letter + baseLetter;
						}

						// Add to the map
						UE_LOG(LogTemp, Display, TEXT("Spell: {%s} %s %s %d %d"), *letter, *name, *school, failureInt, levelInt);
						SpellInfo info;
						info.name = name;
						info.school = school;
						info.level = levelInt;
						info.failure = failureInt;
						spellLetterToInfo.Add(letter, info);
						spellLetters.Add(letter);

					}
				}

				// Make sure the spell letters are unique
				TMap<FString, FString, FDefaultSetAllocator, FCaseSensitiveLookupKeyFuncs<FString>> uniqueSpellLetters;
				for (const FString& letter : spellLetters) {
					uniqueSpellLetters.Add(letter, letter);
				}
				spellLetters.Empty();
				for (const auto& pair : uniqueSpellLetters) {
					spellLetters.Add(pair.Key);
				}

				// Sort the spell letters
				// should go a-z, then A-Z, then aa-zz, then AA-ZZ, then aaa-zzz, then AAA-ZZZ
				spellLetters.Sort([](const FString& A, const FString& B) {
					if (A.Len() != B.Len()) {
						return A.Len() < B.Len();
					}
					if (A.Len() > 0 && B.Len() > 0) {
						if (FChar::IsLower(A[0]) && FChar::IsUpper(B[0])) {
							return true;
						} else if (FChar::IsUpper(A[0]) && FChar::IsLower(B[0])) {
							return false;
						}
					}
					return A < B;
				});

				// Redraw the spell list
				shouldRedrawSpells = true;

			}

			// If it's the character overview
			bool isCharOverview = false;
			for (int i = 0; i < charArray.Num(); i++) {
				if (charArray[i].Contains(TEXT("HPRegen "))) {
					isCharOverview = true;
					break;
				}
			}
			if (isCharOverview) {

				// Get the text
				overviewText = TEXT("");
				for (int i = 1; i < charArray.Num(); i++) {
					FString line = charArray[i].TrimStartAndEnd();
					int32 index = line.Find(TEXT("Turns:"));
					if (index != INDEX_NONE) {
						line = line.Left(index);
					}
					int32 index2 = line.Find(TEXT("Spells:"));
					if (index2 != INDEX_NONE) {
						line = line.Left(index2);
					}
					int32 index3 = line.Find(TEXT("  Next:"));
					if (index3 != INDEX_NONE) {
						line = line.Left(index3) + TEXT("(") + line.Mid(index3+7).TrimStartAndEnd() + TEXT(")");
					}
					line = line.Replace(TEXT("XL:   "), TEXT("XL: "));
					overviewText += line + TEXT("\n");
				}
				shouldRedrawOverview = true;

			}

			// If it's the religion overview
			bool isReligionOverview = false;
			for (int i = 0; i < charArray.Num(); i++) {
				if (charArray[i].Contains(TEXT("Granted powers:"))) {
					isReligionOverview = true;
					break;
				}
			}
			if (isReligionOverview) {

				// Get the text
				religionText = TEXT("");
				for (int i = 0; i < charArray.Num(); i++) {
					religionText += charArray[i] + TEXT("\n");
				}
				shouldRedrawReligion = true;

			}

			// If it's the passive list
			// 	Innate Abilities, Weirdness & Mutations                                         
			//      Your natural rate of healing is slightly increased.                        
			//  a - You have razor-sharp teeth.                                                
			//  b - You have supernaturally acute vision. (SInv)                               
			//  c - You are afflicted with vampirism.                                          
			//  d - [You will be able to turn into a vampire bat when bloodless.] XL 3  
			bool isPassiveList = false;
			for (int i = 0; i < charArray.Num(); i++) {
				if (charArray[i].Contains(TEXT("Innate Abilities"))) {
					isPassiveList = true;
					break;
				}
			}
			if (isPassiveList) {

				// Reset the passive list
				passives.Empty();
				passivePage = 0;
				shouldRedrawPassives = true;

				// Add passives
				for (int i = 2; i < charArray.Num()-2; i++) {

					// Get the passive and it to the list
					FString name = charArray[i].Mid(4).TrimStartAndEnd();
					if (name.Contains(TEXT("Transient mutations")) 
					|| name.Contains(TEXT("Gained at a future"))
					|| name.Contains(TEXT("Mutations|Blood properties"))) {
						continue;
					}
					if (name.Len() > 0) {
						passives.Add(name);
					}

				}

			}

			// If it's the spell list
			// Your spells (describe)            Type                          Failure  Level 
			// a - Foxfire                       Conjuration/Fire              11%      1     																
			// Select a spell to describe                         [!]/[I] toggle spell headers 
			bool isSpellList = false;
			for (int i = 0; i < charArray.Num(); i++) {
				if (charArray[i].Contains(TEXT("Your spells"))) {
					isSpellList = true;
					break;
				}
			}
			if (isSpellList) {

				// Add spells
				for (int i = 2; i < charArray.Num(); i++) {
					if (charArray[i].Contains("-") || charArray[i].Contains("+")) {

						// Get the letter
						FString letter = charArray[i].TrimStart().Mid(0, 1);

						// Get the name
						FString name = charArray[i].Mid(4, 30).TrimStartAndEnd();;

						// Get the school
						FString school = charArray[i].Mid(35, 30).TrimStartAndEnd();;

						// Get the failure
						FString failure = charArray[i].Mid(65, 5).TrimStartAndEnd();
						int failureInt = FCString::Atoi(*failure.Left(failure.Len()-1));

						// Get the level
						FString level = charArray[i].Mid(74, 5).TrimStartAndEnd();
						int levelInt = FCString::Atoi(*level);

						// If the letter is already in the map but with a different name, change the letter
						FString baseLetter = letter;
						if (spellLetterToInfo.Contains(letter) && spellLetterToInfo[letter].name != name) {
							letter = letter + baseLetter;
						}
						if (spellLetterToInfo.Contains(letter) && spellLetterToInfo[letter].name != name) {
							letter = letter + baseLetter;
						}
						if (spellLetterToInfo.Contains(letter) && spellLetterToInfo[letter].name != name) {
							letter = letter + baseLetter;
						}

						// Add to the map
						UE_LOG(LogTemp, Display, TEXT("Spell: {%s} %s %s %d %d"), *letter, *name, *school, failureInt, levelInt);
						SpellInfo info;
						info.name = name;
						info.school = school;
						info.level = levelInt;
						info.failure = failureInt;
						spellLetterToInfo.Add(letter, info);
						spellLetters.Add(letter);

					}

				}

				// Make sure the spell letters are unique
				TMap<FString, FString, FDefaultSetAllocator, FCaseSensitiveLookupKeyFuncs<FString>> uniqueSpellLetters;
				for (const FString& letter : spellLetters) {
					uniqueSpellLetters.Add(letter, letter);
				}
				spellLetters.Empty();
				for (const auto& pair : uniqueSpellLetters) {
					spellLetters.Add(pair.Key);
				}

				// Sort the spell letters
				// should go a-z, then A-Z, then aa-zz, then AA-ZZ, then aaa-zzz, then AAA-ZZZ
				spellLetters.Sort([](const FString& A, const FString& B) {
					if (A.Len() != B.Len()) {
						return A.Len() < B.Len();
					}
					if (A.Len() > 0 && B.Len() > 0) {
						if (FChar::IsLower(A[0]) && FChar::IsUpper(B[0])) {
							return true;
						} else if (FChar::IsUpper(A[0]) && FChar::IsLower(B[0])) {
							return false;
						}
					}
					return A < B;
				});

				// Redraw the spell list
				shouldRedrawSpells = true;

			}

			// If it's the ability list
			// Ability - do what?                  Cost                            Failure    
			// f - Exsanguinate                    Delay                           0%                           
			// [?] toggle between ability selection and description.
			bool isAbilityList = false;
			for (int i = 0; i < charArray.Num(); i++) {
				if (charArray[i].Contains(TEXT("Ability - do what?"))) {
					isAbilityList = true;
					break;
				}
			}
			if (isAbilityList) {

				// Reset the ability list
				abilityLetters.Empty();
				abilityLetterToInfo.Empty();
				shouldRedrawAbilities = true;
				abilityPage = 0;

				// Add abilities
				for (int i = 1; i < charArray.Num(); i++) {
					if (charArray[i].Contains("-") 
						&& !charArray[i].Contains(TEXT("do what"))
						&& !charArray[i].Contains(TEXT("Ability -"))
						&& !charArray[i].Contains(TEXT("Invocations -"))
						&& !charArray[i].Contains(TEXT("Build terrain"))
						&& !charArray[i].Contains(TEXT("Ancestor Identity"))
						&& !charArray[i].Contains(TEXT("Set terrain to build"))
						&& !charArray[i].Contains(TEXT("Clear terrain to floor"))
					) {

						// Get the letter
						FString letter = charArray[i].TrimStart().Mid(0, 1);

						// Get the name
						FString name = charArray[i].Mid(4, 30).TrimStartAndEnd();;

						// Get the cost
						FString cost = charArray[i].Mid(35, 30).TrimStartAndEnd();;

						// Get the failure
						FString failure = charArray[i].Mid(65, 5).TrimStartAndEnd();
						int failureInt = FCString::Atoi(*failure.Left(failure.Len()-1));

						UE_LOG(LogTemp, Display, TEXT("Ability: {%s} %s %s %d"), *letter, *name, *cost, failureInt);

						// Add to the map
						AbilityInfo info;
						info.name = name;
						info.cost = cost;
						info.failure = failureInt;
						abilityLetterToInfo.Add(letter, info);
						abilityLetters.Add(letter);

					}

				}

			}

			// If it's the skill list 
			//     Skill           Level Train  Apt       Skill           Level Train  Apt
			// a + Fighting         0.0         -1    o + Spellcasting     2.0  20%    -1
			// b + Maces & Flails   0.0         -2    p + Conjurations     0.6   2%    -3
			// The percentage of incoming experience used to train each skill is in brown.
			// The species aptitude is in white.
			// [?] Help                [=] set a skill target
			// [/] auto|manual mode    [*] useful|all skills    [!] training|cost|targets
			bool isSkillList = false;
			for (int i = 0; i < charArray.Num(); i++) {
				if (charArray[i].Contains(TEXT(" Level Train "))) {
					isSkillList = true;
					break;
				}
			}
			if (isSkillList) {

				// For each skill name that we have, update the info
				shouldRedrawSkills = true;
				for (int i = 2; i < charArray.Num()-5; i++) {

					// Split into two
					TArray<FString> sections;
					sections.Add(charArray[i].Mid(0, 40).TrimStartAndEnd());
					sections.Add(charArray[i].Mid(40, 40).TrimStartAndEnd());
					for (int j = 0; j < sections.Num(); j++) {

						// Get the letter
						FString letter = sections[j].Mid(0, 1);

						// Get the -, + or * for the focus
						FString focus = sections[j].Mid(2, 1);
						int focusInt = 0;
						if (focus == TEXT("*")) {
							focusInt = 1;
						} else if (focus == TEXT("-")) {
							focusInt = -1;
						}

						// Get the name
						FString name = sections[j].Mid(4, 16).TrimStartAndEnd();
						if (name.TrimStartAndEnd().Len() == 0 || name.Contains(TEXT("Skill"))) {
							continue;
						}
						TArray<FString> parts;
						name.ParseIntoArray(parts, TEXT(" "), true);
						name = parts[0];

						// Get the level
						FString level = sections[j].Mid(20, 5).TrimStartAndEnd();
						float levelFloat = FCString::Atof(*level);

						// Get the train
						FString train = sections[j].Mid(25, 5).TrimStartAndEnd();
						int trainInt = FCString::Atoi(*train);

						// Get the apt
						FString apt = sections[j].Mid(30, 5).TrimStartAndEnd();
						int aptInt = FCString::Atoi(*apt);

						UE_LOG(LogTemp, Display, TEXT("Skill: {%s} %s %f %d %d"), *letter, *name, levelFloat, trainInt, aptInt);

						// Add to the map
						SkillInfo info;
						info.letter = letter;
						info.level = levelFloat;
						info.train = trainInt;
						info.apt = aptInt;
						info.focussed = focusInt;
						skillNameToInfo.Add(name, info);

					}

				}

			}

			// If it's the shop menu
			// 			Welcome to Quirharo's Antique Armour Shop! What would you like to do?           
			//  a -   81 gold   a chain mail                                                   
			//  b -   36 gold   a leather armour                                               
			//  c -   90 gold   a runed leather armour                                         
			//  d -  468 gold   a glowing plate armour                                         
			//  e -  504 gold   a heavily runed plate armour                                   
			//  f -  414 gold   a plate armour                                                 
			//  g -   72 gold   a ring mail                                                    
			//  h -   12 gold   a robe                                                         
			//  i -   66 gold   a runed robe                                                   
			//  j -  162 gold   a masterwork scale mail                                        
			// You have 0 gold pieces.                                                         
			// [Esc] exit          [!] buy|examine items       [a-j] mark item for purchase    
			// [/] sort (type)                                 [A-J] put item on shopping list 
			bool isShopMenu = false;
			for (int i = 0; i < charArray.Num(); i++) {
				if (charArray[i].Contains(TEXT("mark item for purchase"))) {
					isShopMenu = true;
					break;
				}
			}
			if (isShopMenu && showNextShop) {

				// Reset the shop list
				UE_LOG(LogTemp, Display, TEXT("Found shop menu"));
				shopItems.Empty();
				shopTitle = TEXT("");

				// For each line in the text
				for (int i = 1; i < charArray.Num(); i++) {

					// If it's the line stating the player's gold
					if (charArray[i].Contains(TEXT("You have"))) {

						// Split into words
						TArray<FString> parts;
						charArray[i].ParseIntoArray(parts, TEXT(" "), true);
						gold = FCString::Atoi(*parts[2]);

					// If it's an item being sold
					} else if (charArray[i].Contains("gold")) {

						// Get the letter
						FString letter = charArray[i].TrimStart().Mid(0, 1);

						// Get the cost
						FString cost = charArray[i].Mid(4, 10).TrimStartAndEnd();;
						int costInt = FCString::Atoi(*cost);

						// Get the name
						FString name = charArray[i].Mid(15).TrimStartAndEnd();;

						// Add to the list
						UE_LOG(LogTemp, Display, TEXT("Shop: {%s} %s %s"), *letter, *cost, *name);
						ShopItem item;
						item.name = name;
						item.price = costInt;
						item.letter = letter;
						item.selected = false;
						shopItems.Add(item);

					// If it's the title
					} else if (charArray[i].Contains(TEXT("Welcome to"))) {
						shopTitle = charArray[i].Replace(TEXT("What would you like to do?"), TEXT("")).TrimStartAndEnd();
					}

				}

				// Show the shop actor and set the text
				if (refToShopActor != nullptr) {
					UWidgetComponent* WidgetComponentShop = Cast<UWidgetComponent>(refToShopActor->GetComponentByClass(UWidgetComponent::StaticClass()));
					if (WidgetComponentShop != nullptr) {
						UUserWidget* UserWidgetShop = WidgetComponentShop->GetUserWidgetObject();
						if (UserWidgetShop != nullptr) {

							// Set the shop title
							UTextBlock* ShopTitle = Cast<UTextBlock>(UserWidgetShop->GetWidgetFromName(TEXT("TextShopTitle")));
							if (ShopTitle != nullptr) {
								ShopTitle->SetText(FText::FromString(shopTitle));
							}

							// Set each item
							for (int i = 0; i < shopItems.Num(); i++) {

								// Set the text
								FString itemName = TEXT("TextShop") + FString::FromInt(i+1);
								UE_LOG(LogTemp, Display, TEXT("Setting shop item %s to %s"), *itemName, *shopItems[i].name);
								UTextBlock* ShopItem = Cast<UTextBlock>(UserWidgetShop->GetWidgetFromName(*itemName));
								if (ShopItem != nullptr) {
									FString itemLine = TEXT("[") + FString::FromInt(shopItems[i].price) + TEXT("g] ") + shopItems[i].name;
									ShopItem->SetText(FText::FromString(itemLine));
									ShopItem->SetColorAndOpacity(FLinearColor(1.0f, 1.0f, 1.0f, 1.0f));
								}

								// Also set the image
								FString matName = itemNameToTextureName(shopItems[i].name);
								UTexture2D* tex2D = getTexture(matName);
								FString imageName = "ImageShop" + FString::FromInt(i+1);
								UImage* ShopImage = Cast<UImage>(UserWidgetShop->GetWidgetFromName(*imageName));
								if (ShopImage != nullptr) {
									ShopImage->SetBrushFromTexture(tex2D);
									ShopImage->SetVisibility(ESlateVisibility::Visible);
								}
							}

							// Clear the rest
							for (int i = shopItems.Num(); i < 12; i++) {
								FString itemName = TEXT("TextShop") + FString::FromInt(i+1);
								UTextBlock* ShopItem = Cast<UTextBlock>(UserWidgetShop->GetWidgetFromName(*itemName));
								if (ShopItem != nullptr) {
									ShopItem->SetText(FText::FromString(TEXT("")));
								}
								FString imageName = "ImageShop" + FString::FromInt(i+1);
								UImage* ShopImage = Cast<UImage>(UserWidgetShop->GetWidgetFromName(*imageName));
								if (ShopImage != nullptr) {
									ShopImage->SetVisibility(ESlateVisibility::Hidden);
								}
							}

							// Set the total cost
							int totalCost = 0;
							UTextBlock* ShopTotal = Cast<UTextBlock>(UserWidgetShop->GetWidgetFromName(TEXT("TextShopCost")));
							if (ShopTotal != nullptr) {
								for (int i = 0; i < shopItems.Num(); i++) {
									if (shopItems[i].selected) {
										totalCost += shopItems[i].price;
									}
								}
								ShopTotal->SetText(FText::FromString(FString::Printf(TEXT("Total: %d gold"), totalCost)));
							}

							// Set the player's gold count
							UTextBlock* ShopGold = Cast<UTextBlock>(UserWidgetShop->GetWidgetFromName(TEXT("TextShopGold")));
							if (ShopGold != nullptr) {
								ShopGold->SetText(FText::FromString(FString::Printf(TEXT("(you have %dg)"), gold)));
							}

						}
					}
					refToShopActor->SetActorHiddenInGame(false);
					refToShopActor->SetActorEnableCollision(true);
					showingShop = true;
				}

				// When this is open, hide the shop itself
				FString meshName = TEXT("");
				for (auto& Elem : meshNameToThing) {
					if (Elem.Value.thingIs.Contains(TEXT("Shop")) && Elem.Value.x == LOS && Elem.Value.y == LOS) {
						meshName = Elem.Key;
						break;
					}
				}
				if (meshName.Len() > 0) {
					UE_LOG(LogTemp, Display, TEXT("Hiding shop mesh %s"), *meshName);
					for (int i=0; i<itemArray.Num(); i++) {
						if (itemArray[i]->GetName() == meshName) {
							itemArray[i]->SetActorHiddenInGame(true);
							itemArray[i]->SetActorEnableCollision(false);
							break;
						}
					}
				}

			}

			// If it's a description
			bool isItemOrEnemy = false;
			for (int i = 0; i < charArray.Num(); i++) {
				if (charArray[i].Contains(TEXT("Threat:")) 
				|| charArray[i].Contains(TEXT("prefixes"))
				|| charArray[i].Contains(TEXT("trap."))
				|| charArray[i].Contains(TEXT("Pray here with"))
				|| charArray[i].Contains(TEXT("shop in the dungeon"))
				|| charArray[i].Contains(TEXT("A stone archway that seems to"))
				|| charArray[i].Contains(TEXT("possesses the following magical abilities"))
				|| charArray[i].Contains(TEXT("Clouds of this kind"))
				|| charArray[i].Contains(TEXT("standing here, you can enter"))
				|| charArray[i].Contains(TEXT("A decorative fountain"))
				|| charArray[i].Contains(TEXT("It can be dug through"))
				|| charArray[i].Contains(TEXT("(i)nscribe"))
				|| charArray[i].Contains(TEXT("Range:"))
				) {
					isItemOrEnemy = true;
					break;
				}
			}
			if (isItemOrEnemy && !isShopMenu) {
				bool foundNew = false;
				FString typeOfThing = TEXT("Item");
				FString toAdd = TEXT("");
				for (int i = 0; i < charArray.Num(); i++) {

					// If the first line is a blank line, skip it
					if (i == 0 && charArray[i].TrimStartAndEnd().Len() == 0) {
						continue;
					}

					// Determine the type of thing
					if (charArray[i].Contains(TEXT("Threat:"))) {
						typeOfThing = TEXT("Enemy");
					} else if (charArray[i].Contains(TEXT(" shop"))) {
						typeOfThing = TEXT("Shop");
					} else if (charArray[i].Contains(TEXT("Base accuracy"))) {
						typeOfThing = TEXT("Item");
					} else if (charArray[i].Contains(TEXT("Range:"))) {
						typeOfThing = TEXT("Spell");
					} else if (charArray[i].Contains(TEXT("potion"))) {
						typeOfThing = TEXT("Potion");
					} else if (charArray[i].Contains(TEXT("To read a")) || charArray[i].Contains(TEXT("disposable arcane formula"))) {
						typeOfThing = TEXT("Scroll");
					} else if (charArray[i].Contains(TEXT(" trap"))) {
						typeOfThing = TEXT("Trap");
					} else if (charArray[i].Contains(TEXT(" altar"))) {
						typeOfThing = TEXT("Altar");
					} else if (charArray[i].Contains(TEXT(" arch"))) {
						typeOfThing = TEXT("Arch");
					} else if (charArray[i].Contains(TEXT(" statue"))) {
						typeOfThing = TEXT("Statue");
					} else if (charArray[i].Contains(TEXT(" idol"))) {
						typeOfThing = TEXT("Idol");
					} else if (charArray[i].Contains(TEXT("vanish after you do so"))) {
						typeOfThing = TEXT("Gate");
					} else if (charArray[i].Contains(TEXT(" fountain"))) {
						typeOfThing = TEXT("Fountain");
					}

					// Ignore some lines
					if (charArray[i].Contains(TEXT("[!]")) 
						|| charArray[i].Contains(TEXT("To read a")) 
						|| charArray[i].Contains(TEXT("indicates the spell")) 
						|| charArray[i].Contains(TEXT("prefixes")) 
						|| charArray[i].Contains(TEXT("(i)")) 
						|| charArray[i].Contains(TEXT("While standing here")) 
						|| charArray[i].Contains(TEXT("vanish after you do so")) 
						|| charArray[i].Contains(TEXT("armor}")) 
						|| charArray[i].Contains(TEXT("key. ")) 
						|| charArray[i].Contains(TEXT("You are here.")) 
						|| charArray[i].Contains(TEXT("Pray here with > to learn more")) 
						|| charArray[i].Contains(TEXT("(>)")) 
						|| (charArray[i].Contains(TEXT("{")) && i > 1) 
						|| (charArray[i].Contains(TEXT("}")) && i > 1) 
						|| charArray[i].Contains(TEXT("(g)")) 
						|| charArray[i].Contains(TEXT("sum of A"))) {
						continue;
					}

					// Shorten some lines
					int32 searchIndex = charArray[i].Find(TEXT(" use (s)"));
					if (searchIndex != INDEX_NONE) {
						charArray[i] = charArray[i].Left(searchIndex);
					}
					if (i == 1) {
 						searchIndex = charArray[i].Find(TEXT(" - "));
						if (searchIndex != INDEX_NONE) {
							charArray[i] = charArray[i].Mid(searchIndex+3);
						}
					}

					// Add to the description
					FString trimmed = charArray[i].TrimEnd();
					UE_LOG(LogTemp, Display, TEXT("Adding to description: %s"), *trimmed);
					toAdd += trimmed + TEXT("\n");

				}

				// Remove anything non-ascii
				for (int i = 0; i < toAdd.Len(); i++) {
					if ((toAdd[i] < 32 || toAdd[i] > 127) && toAdd[i] != TEXT('\n') && toAdd[i] != TEXT('\r') && toAdd[i] != TEXT('\t')) {
						toAdd[i] = TEXT(' ');
					}
				}

				// Get rid of some other phrases
				toAdd = toAdd.Replace(TEXT(" (target"), TEXT(""));
				toAdd = toAdd.Replace(TEXT("with [v])."), TEXT(""));

				// Remove until the first period
				// int32 firstLineEnd = toAdd.Find(TEXT("."));
				int32 firstLineEnd = 0;
				bool foundNonWhitespace = false;
				for (int i = 0; i < toAdd.Len()-1; i++) {
					if (toAdd.Mid(i, 2) != TEXT("  ") && toAdd[i] != TEXT('\n') && toAdd[i] != TEXT('\r') && toAdd[i] != TEXT('\t')) {
						foundNonWhitespace = true;
					} else if (foundNonWhitespace) {
						firstLineEnd = i;
						break;
					}
				}
				FString withoutFirst = toAdd.Mid(firstLineEnd+1);
				FString newFirst = toAdd.Left(firstLineEnd+1).TrimStartAndEnd();

				// Determine where to add the new section
				FString searchString = TEXT("");
				int addLocNew = -1;
				for (int i = 0; i < withoutFirst.Len(); i++) {
					if (withoutFirst[i] != TEXT('\n') && withoutFirst[i] != TEXT(' ')) {
						for (int j = 0; j < std::min(40, withoutFirst.Len()-i); j++) {
							if (withoutFirst[i+j] == TEXT('\n')) {
								break;
							}
							searchString += withoutFirst[i+j];
							if (addLocNew == -1) {
								addLocNew = i + j;
							}
						}
						break;
					}
				}
				int addLoc = currentDescription.Find(searchString);
				
				// Combine the descriptions
				UE_LOG(LogTemp, Display, TEXT("searchString: %s"), *searchString);
				UE_LOG(LogTemp, Display, TEXT("addLoc: %d"), addLoc);
				UE_LOG(LogTemp, Display, TEXT("newLoc: %d"), addLocNew);
				if (addLoc != INDEX_NONE && searchString.TrimStartAndEnd().Len() > 0) {
					currentDescription = currentDescription.Left(addLoc) + withoutFirst.Mid(addLocNew);
				} else {
					currentDescription += toAdd;
				}
				UE_LOG(LogTemp, Display, TEXT("Current description: \n%s\n"), *currentDescription);

				// While the last line is a blank line, remove it
				while (currentDescription.EndsWith(TEXT("\n"))) {
					currentDescription = currentDescription.Left(currentDescription.Len()-1);
				}

				// Remove any double whitelines
				FRegexPattern pattern(TEXT("\n[ \t]*\n[ \t]*\n"));
				FRegexMatcher matcher(pattern, *currentDescription);
				while (matcher.FindNext()) {
					int groupStart = matcher.GetMatchBeginning();
					int groupEnd = matcher.GetMatchEnding()-1;
					currentDescription = currentDescription.Left(groupStart) + currentDescription.Mid(groupEnd);
					matcher = FRegexMatcher(pattern, *currentDescription);
				}

				// Set the usage based on the type of object
				UE_LOG(LogTemp, Display, TEXT("Type of thing: %s"), *typeOfThing);
				currentUsage = TEXT("");
				if (typeOfThing == TEXT("Enemy")) {
					currentUsage = TEXT("Press USE on a adjacent monster to attack with your equipped weapon.");
				} else if (typeOfThing == TEXT("Item")) {
					currentUsage = TEXT("Press USE whilst HELD to equip/unequip.\nRELEASE onto a slot or the floor to move it.");
				} else if (typeOfThing == TEXT("Spell")) {
					if (memorizing) {
						int spellLevelsNeeded = 0;
						for (int i = 0; i < charArray.Num(); i++) {
							if (charArray[i].Contains(TEXT("Level:"))) {
								int levelStart = charArray[i].Find(TEXT("Level:"))+7;
								int levelEnd = charArray[i].Find(TEXT("School"));
								FString spellLevel = charArray[i].Mid(levelStart, levelEnd-levelStart).TrimStartAndEnd();
								UE_LOG(LogTemp, Display, TEXT("Spell level: %s"), *spellLevel);
								spellLevelsNeeded = FCString::Atoi(*spellLevel);
								break;
							}
						}
						currentUsage = TEXT("Press USE whilst HELD to memorize the spell.\n(if you have at least ") + FString::FromInt(spellLevelsNeeded) + TEXT(" spell levels remaining)");
					} else {
						targetingRange = 0;
						bool forceTargeting = false;
						for (int i = 0; i < charArray.Num(); i++) {
							if (charArray[i].Contains(TEXT("divine exegesis"))
								|| charArray[i].Contains(TEXT("line pass"))
								|| charArray[i].Contains(TEXT("heal other"))
								|| charArray[i].Contains(TEXT("wall jump"))
							) {
								forceTargeting = true;
							}
							if (charArray[i].Contains(TEXT("Range:"))) {
								int rangeStart = charArray[i].Find(TEXT("Range:"))+7;
								FString spellRange = charArray[i].Mid(rangeStart).TrimStartAndEnd();
								UE_LOG(LogTemp, Display, TEXT("Spell range: %s"), *spellRange);
								targetingRange = 0;
								for (int j = 0; j < spellRange.Len(); j++) {
									if (spellRange[j] == TEXT('-') || spellRange[j] == TEXT('>')) {
										targetingRange++;
									}
								}
								UE_LOG(LogTemp, Display, TEXT("Spell range: %d"), targetingRange);
								break;
							}
						}
						if (forceTargeting) {
							UE_LOG(LogTemp, Display, TEXT("Forcing targeting range to 10"));
							targetingRange = 10;
						}
						currentUsage = TEXT("Press USE whilst HELD to perform the spell/ability.\nRELEASE onto a slot to move it.");
					}
				} else if (typeOfThing == TEXT("Potion")) {
					currentUsage = TEXT("Press USE whilst HELD to drink the potion.\nRELEASE onto a slot or the floor to move it.");
				} else if (typeOfThing == TEXT("Altar")) {
					currentUsage = TEXT("Press USE whilst on the same tile to view the altar menu.");
				} else if (typeOfThing == TEXT("Gate")) {
					currentUsage = TEXT("Press USE whilst on the same tile to travel through.");
				} else if (typeOfThing == TEXT("Shop")) {
					currentUsage = TEXT("Press USE whilst on the same tile to view the shop menu.");
				} else if (typeOfThing == TEXT("Scroll")) {
					currentUsage = TEXT("Press USE whilst HELD to read the scroll.\nRELEASE onto a slot or the floor to move it.");
				}

				// Set the description
				UWidgetComponent* WidgetComponentDesc = Cast<UWidgetComponent>(refToDescriptionActor->GetComponentByClass(UWidgetComponent::StaticClass()));
				if (WidgetComponentDesc != nullptr) {
					UUserWidget* UserWidgetDesc = WidgetComponentDesc->GetUserWidgetObject();
					if (UserWidgetDesc != nullptr) {
						UTextBlock* TextBox = Cast<UTextBlock>(UserWidgetDesc->GetWidgetFromName(TEXT("Description")));
						if (TextBox != nullptr) {
							TextBox->SetText(FText::FromString(currentDescription));
						}
						UTextBlock* UsageBox = Cast<UTextBlock>(UserWidgetDesc->GetWidgetFromName(TEXT("Usage")));
						if (UsageBox != nullptr) {
							UsageBox->SetText(FText::FromString(currentUsage));
						}
					}
				}

			}

			// If it's the altar menu
			bool isAltarMenu = false;
			for (int i = 0; i < charArray.Num(); i++) {
				if (charArray[i].Contains(TEXT("Overview|Powers|Wrath"))) {
					isAltarMenu = true;
					break;
				}
			}
			if (isAltarMenu && currentUI != "religion" && showNextAltar) {
				UE_LOG(LogTemp, Display, TEXT("Found altar menu"));

				// If it's a god that I haven't bothered to implement yet, skip it
				bool skipAltar = false;
				for (int i = 0; i < charArray.Num(); i++) {
					if (charArray[i].Contains(TEXT("Nemelex"))) {
						skipAltar = true;
						UE_LOG(LogTemp, Display, TEXT("Skipping altar"));
						break;
					}
				}

				// Get the text
				FString textToSet = TEXT("");
				for (int i = 0; i < charArray.Num(); i++) {
					textToSet += charArray[i] + TEXT("\n");
				}

				// Depending on the index, add to the corresponding text
				if (nextAltar == 0) {
					altarOverview = textToSet;
					UE_LOG(LogTemp, Display, TEXT("Setting altar overview: %s"), *altarOverview);
				} else if (nextAltar == 1) {
					altarPowers = textToSet;
					UE_LOG(LogTemp, Display, TEXT("Setting altar powers: %s"), *altarPowers);
				} else if (nextAltar == 2) {
					altarWrath = textToSet;
					UE_LOG(LogTemp, Display, TEXT("Setting altar wrath: %s"), *altarWrath);
				}
				nextAltar++;
				nextAltar = nextAltar % 3;

				// If we aren't skipping
				if (!skipAltar) {

					// Show the altar actor and set the text
					if (refToAltarActor != nullptr) {
						UWidgetComponent* WidgetComponentAltar = Cast<UWidgetComponent>(refToAltarActor->GetComponentByClass(UWidgetComponent::StaticClass()));
						if (WidgetComponentAltar != nullptr) {
							UUserWidget* UserWidgetAltar = WidgetComponentAltar->GetUserWidgetObject();
							if (UserWidgetAltar != nullptr) {
								UTextBlock* AltarText = Cast<UTextBlock>(UserWidgetAltar->GetWidgetFromName(TEXT("TextAltar")));
								if (AltarText != nullptr) {
									AltarText->SetText(FText::FromString(altarOverview));
								}
							}
						}
						UE_LOG(LogTemp, Display, TEXT("Showing altar menu"));
						refToAltarActor->SetActorHiddenInGame(false);
						refToAltarActor->SetActorEnableCollision(true);
						showingAltar = true;
						currentUI = TEXT("altar");
					}

					// When this is open, hide the altar itself
					FString meshName = TEXT("");
					for (auto& Elem : meshNameToThing) {
						if (Elem.Value.thingIs.Contains(TEXT("Altar")) && Elem.Value.x == LOS && Elem.Value.y == LOS) {
							meshName = Elem.Key;
							break;
						}
					}
					if (meshName.Len() > 0) {
						UE_LOG(LogTemp, Display, TEXT("Hiding altar mesh %s"), *meshName);
						for (int i=0; i<itemArray.Num(); i++) {
							if (itemArray[i]->GetName() == meshName) {
								itemArray[i]->SetActorHiddenInGame(true);
								itemArray[i]->SetActorEnableCollision(false);
								break;
							}
						}
					}

				}

			}

			// If it's the inventory menu
			bool isInventory = false;
			for (int i = 0; i < charArray.Num(); i++) {
				if (charArray[i].Contains(TEXT("Inventory:")) && charArray[i].Contains(TEXT("slots"))) {
					isInventory = true;
					break;
				}
			}
			if (isInventory) {

				// Process the inventory lines
				inventoryLetterToNamePrev = inventoryLetterToName; 
				int numBlankLines = 0;
				for (int i = 0; i < charArray.Num(); i++) {

					// If it's a blank line
					if (charArray[i].TrimStartAndEnd().Len() == 0) {
						numBlankLines++;
						continue;
					}
					UE_LOG(LogTemp, Display, TEXT("INVENTORY -> %s"), *charArray[i]);

					// If it's a line with an item
					if (charArray[i].Contains(TEXT(" - "))) {
						TArray<FString> words;
						charArray[i].ParseIntoArray(words, TEXT(" "), true);
						FString hotkey = words[0];
						FString description = TEXT("");
						for (int j = 2; j < words.Num(); j++) {
							description += words[j] + TEXT(" ");
						}
						UE_LOG(LogTemp, Display, TEXT("INVENTORY - Added to inventory: {%s} %s"), *hotkey, *description);
						inventoryLetterToName.Add(hotkey, description);
						bool alreadyHasLoc = false;
						for (int j = 0; j < inventoryLocToLetter.Num(); j++) {
							for (int k = 0; k < inventoryLocToLetter[j].Num(); k++) {
								if (inventoryLocToLetter[j][k].Equals(hotkey, ESearchCase::CaseSensitive)) {
									alreadyHasLoc = true;
									break;
								}
							}
							if (alreadyHasLoc) {
								break;
							}
						}
						if (!inventoryLetterToNamePrev.Contains(hotkey) && !alreadyHasLoc) {
							if (inventoryNextSpot.X != -1 && inventoryNextSpot.Y != -1) {
								inventoryLocToLetter[inventoryNextSpot.X][inventoryNextSpot.Y] = hotkey;
								UE_LOG(LogTemp, Display, TEXT("INVENTORY - put %s at specific loc (%d, %d)"), *hotkey, inventoryNextSpot.X, inventoryNextSpot.Y);
								inventoryNextSpot = FIntVector2(-1, -1);
							} else {
								bool added = false;
								for (int j = 0; j < inventoryLocToLetter.Num(); j++) {
									for (int k = 0; k < inventoryLocToLetter[j].Num(); k++) {
										if (inventoryLocToLetter[j][k] == TEXT("")) {
											inventoryLocToLetter[j][k] = hotkey;
											UE_LOG(LogTemp, Display, TEXT("INVENTORY - put %s at loc (%d, %d)"), *hotkey, j, k);
											added = true;
											break;
										}
									}
									if (added) {
										break;
									}
								}
							}
						} else {
							UE_LOG(LogTemp, Display, TEXT("INVENTORY - already contains %s"), *hotkey);
						}
					}
				}
				inventoryLetterToNamePrev = inventoryLetterToName;

				// If we have at least two blank lines, we have finished reading the inv
				if (numBlankLines >= 2) {
					UE_LOG(LogTemp, Display, TEXT("INVENTORY - Finished reading inventory"));

					// Output the current state
					UE_LOG(LogTemp, Display, TEXT("INVENTORY - Current state:"));
					for (auto& Elem : inventoryLetterToName) {
						UE_LOG(LogTemp, Display, TEXT("INVENTORY - {%s} %s"), *Elem.Key, *Elem.Value);
					}
					for (int i = 0; i < inventoryLocToLetter.Num(); i++) {
						for (int j = 0; j < inventoryLocToLetter[i].Num(); j++) {
							if (inventoryLocToLetter[i][j].Len() > 0) {
								UE_LOG(LogTemp, Display, TEXT("INVENTORY - loc (%d, %d) %s"), i, j, *inventoryLocToLetter[i][j]);
							}
						}
					}
					for (int i = 0; i < numHotbarSlots; i++) {
						UE_LOG(LogTemp, Display, TEXT("INVENTORY - hotbar slot %d: %s"), i, *hotbarInfos[i].letter);
					}

					// Make sure all of locs to letter are valid
					for (int i = 0; i < inventoryLocToLetter.Num(); i++) {
						for (int j = 0; j < inventoryLocToLetter[i].Num(); j++) {
							if (inventoryLocToLetter[i][j].Len() == 0) {
								continue;
							}
							if (!inventoryLetterToName.Contains(inventoryLocToLetter[i][j])) {
								UE_LOG(LogTemp, Display, TEXT("INVENTORY - fixed, replaced %s at loc (%d, %d)"), *inventoryLocToLetter[i][j], i, j);
								inventoryLocToLetter[i][j] = TEXT("");
							}
						}
					}

					// Make sure everything in the inventory has a location
					for (auto& Elem : inventoryLetterToName) {
						FString letter = Elem.Key;
						FString name = Elem.Value;
						bool hasLoc = false;
						for (int i = 0; i < inventoryLocToLetter.Num(); i++) {
							for (int j = 0; j < inventoryLocToLetter[i].Num(); j++) {
								if (inventoryLocToLetter[i][j].Equals(letter, ESearchCase::CaseSensitive)) {
									hasLoc = true;
									break;
								}
							}
							if (hasLoc) {
								break;
							}
						}
						if (!hasLoc) {
							bool hasAdded = false;
							for (int i = 0; i < inventoryLocToLetter.Num(); i++) {
								for (int j = 0; j < inventoryLocToLetter[i].Num(); j++) {
									if (inventoryLocToLetter[i][j] == TEXT("")) {
										inventoryLocToLetter[i][j] = letter;
										UE_LOG(LogTemp, Display, TEXT("INVENTORY - fixed, added %s to loc (%d, %d)"), *letter, i, j);
										hasAdded = true;
										break;
									}
								}
								if (hasAdded) {
									break;
								}
							}
						}
					}

					// Make sure all the hotbar slots are valid
					for (int i = 0; i < numHotbarSlots; i++) {
						if (hotbarInfos[i].letter.Len() == 0) {
							continue;
						}
						if (hotbarInfos[i].type == "InventoryItem") {
							if (!inventoryLetterToName.Contains(hotbarInfos[i].letter)) {
								UE_LOG(LogTemp, Display, TEXT("INVENTORY - fixed, removed letter %s from hotbar slot %d"), *hotbarInfos[i].letter, i);
								hotbarInfos[i].name = TEXT("");
								hotbarInfos[i].type = TEXT("");
								hotbarInfos[i].letter = TEXT("");
							}
						}
					}

					// Remove any duplicates from the inventory
					for (int i = 0; i < inventoryLocToLetter.Num(); i++) {
						for (int j = 0; j < inventoryLocToLetter[i].Num(); j++) {
							if (inventoryLocToLetter[i][j].Len() == 0) {
								continue;
							}
							for (int k = 0; k < inventoryLocToLetter.Num(); k++) {
								for (int l = 0; l < inventoryLocToLetter[k].Num(); l++) {
									if (i != k || j != l) {
										if (inventoryLocToLetter[i][j].Equals(inventoryLocToLetter[k][l], ESearchCase::CaseSensitive)) {
											UE_LOG(LogTemp, Display, TEXT("INVENTORY - fixed, removed duplicate letter %s at loc (%d, %d)"), *inventoryLocToLetter[k][l], k, l);
											inventoryLocToLetter[k][l] = TEXT("");
										}
									}
								}
							}
						}
					}

					// Remove any duplicates from the hotbar
					for (int i = 0; i < numHotbarSlots; i++) {
						if (hotbarInfos[i].letter.Len() == 0) {
							continue;
						}
						for (int j = 0; j < numHotbarSlots; j++) {
							if (i != j && hotbarInfos[i].letter.Equals(hotbarInfos[j].letter, ESearchCase::CaseSensitive)) {
								UE_LOG(LogTemp, Display, TEXT("INVENTORY - fixed, removed duplicate letter %s from hotbar slot %d"), *hotbarInfos[i].letter, i);
								hotbarInfos[j].name = TEXT("");
								hotbarInfos[j].type = TEXT("");
								hotbarInfos[j].letter = TEXT("");
							}
						}
					}

					// We should redraw
					shouldRedrawInventory = true;
					shouldRedrawHotbar = true;

				}

			}

			// If it's a choice menu
			// You have a choice of weapons.                                                   
			//  a - rapier                        (+1 apt)                                     
			//  b - flail                         (+2 apt)                                     
			// + - Recommended random choice  * - Random weapon                                
			// % - List aptitudes             Bksp - Return to character menu                  
			// ? - Help                                         
			// Choose an item to acquire.                                                      
			//  a - the +1 hat of Waymoroh {Dex+7}                                             
			//  b - a staff of death                                                           
			//  c - the ring of the Butterfly {Ice Fly rN+++ Str-5 Slay-5}                     
			//  d - 842 gold pieces (you have 0 gold)                                          
			// [!] acquire|examine items  [a-d] select item for acquirement         [Esc] exit                  
			bool isChoice = false;
			FString choiceTitle = "";
			FString newChoiceType = "default";
			for (int i = 0; i < charArray.Num(); i++) {
				if (charArray[i].Contains(TEXT("You have a choice of")) 
				|| charArray[i].Contains(TEXT("Choose an item to acquire"))
				|| charArray[i].Contains(TEXT("Select a spell to forget"))
				|| charArray[i].Contains(TEXT("Brand which "))
				|| charArray[i].Contains(TEXT("Curse which "))
				|| charArray[i].Contains(TEXT("Enchant which "))
				|| charArray[i].Contains(TEXT("Uncurse and destroy which "))
				|| charArray[i].Contains(TEXT("Purchase which effect"))
				|| charArray[i].Contains(TEXT("Fund which merchant"))
				|| charArray[i].Contains(TEXT("Draw which"))
				|| charArray[i].Contains(TEXT("Do you wish to have your"))
				|| charArray[i].Contains(TEXT("Casting with Divine Exegesis"))
				|| charArray[i].Contains(TEXT("Identify which "))
				|| charArray[i].Contains(TEXT("Remove which one?"))
				|| charArray[i].Contains(TEXT("Really read "))) {
					isChoice = true;
					choiceTitle = charArray[i];
					choiceTitle = choiceTitle.Replace(TEXT("[!] toggle spell headers"), TEXT(" "));
					choiceTitle = choiceTitle.TrimStartAndEnd();
					if (i < charArray.Num()-1 && !charArray[i+1].Contains(TEXT(" - "))) {
						choiceTitle += TEXT(" ") + charArray[i+1].TrimStartAndEnd();
					}
					if (choiceTitle.Contains(TEXT("Divine Exegesis"))) {
						choiceTitle = TEXT("Cast which spell with Divine Exegesis?");
						newChoiceType = "divine";
					} else if (choiceTitle.Contains(TEXT("to acquire"))) {
						newChoiceType = "acquirement";
					} else if (choiceTitle.Contains(TEXT("purchase which effect")) 
								|| choiceTitle.Contains(TEXT("Fund which merchant"))) {
						newChoiceType = "gozag";
					} else if (choiceTitle.Contains(TEXT("Brand which"))
								|| choiceTitle.Contains(TEXT("Uncurse and destroy"))
								|| choiceTitle.Contains(TEXT("Curse which"))) {
						newChoiceType = "branding";
					} else if (choiceTitle.Contains(TEXT("Remove which one?"))) {
						newChoiceType = "removal";
					} else if (choiceTitle.Contains(TEXT("Draw which"))) {
						newChoiceType = "cards";
					} else if (choiceTitle.Contains(TEXT("Select a spell to forget"))) {
						newChoiceType = "amnesia";
					}
					break;
				}
			}
			if (isChoice) {
				choiceType = newChoiceType;
				UE_LOG(LogTemp, Display, TEXT("Found choice menu: %s"), *choiceTitle);
				UE_LOG(LogTemp, Display, TEXT("Choice type: %s"), *choiceType);
				choiceLetters.Empty();
				choiceNames.Empty();
				isChoiceOpen = true;
				for (int i = 0; i < charArray.Num(); i++) {
					if (charArray[i].Contains(TEXT(" - "))) {
						UE_LOG(LogTemp, Display, TEXT("CHOICE - Found choice: %s"), *charArray[i]);

						// Skip some lines
						if (charArray[i].Contains(TEXT("random choice")) 
							|| charArray[i].Contains(TEXT("Help"))
							|| charArray[i].Contains(TEXT("[!]"))
							|| charArray[i].Contains(TEXT("Jewelry"))
							|| charArray[i].Contains(TEXT("Casting with Divine"))
							|| charArray[i].Contains(TEXT("[Ctrl-F]"))
							|| charArray[i].Contains(TEXT("Jewelry"))
							|| charArray[i].Contains(TEXT("go to first with"))
							|| charArray[i].Contains(TEXT("List aptitudes"))) {
							continue;
						}

						// Add it
						TArray<FString> words;
						charArray[i].ParseIntoArray(words, TEXT(" "), true);
						FString hotkey = words[0];
						FString description = TEXT("");
						if (choiceType == "amnesia") {
							for (int j = 2; j < words.Num()-3; j++) {
								description += words[j] + TEXT(" ");
							}
						} else if (choiceType == "divine") {
							for (int j = 2; j < words.Num()-2; j++) {
								description += words[j] + TEXT(" ");
							}
						} else {
							for (int j = 2; j < words.Num(); j++) {
								description += words[j] + TEXT(" ");
							}
						}
						description = description.TrimStartAndEnd();
						UE_LOG(LogTemp, Display, TEXT("CHOICE - Added choice: {%s} %s"), *hotkey, *description);
						choiceLetters.Add(hotkey);
						choiceNames.Add(description);

					}
				}

			}

			// If we have a level up attribute choice
			bool isAttributeChoice = false;
			for (int i = 0; i < charArray.Num(); i++) {
				if (charArray[i].Contains(TEXT("experience leads to an increase"))) {
					bool hasAlreadyChosen = false;
					for (int j = i; j < charArray.Num(); j++) {
						if (charArray[j].Contains(TEXT("You feel"))) {
							hasAlreadyChosen = true;
							break;
						}
					}
					if (!hasAlreadyChosen) {
						isAttributeChoice = true;
					}
					break;
				}
			}
			if (isAttributeChoice) {
				UE_LOG(LogTemp, Display, TEXT("Found attribute choice"));
				choiceLetters.Empty();
				choiceNames.Empty();
				choiceLetters.Add(TEXT("S"));
				choiceNames.Add(TEXT("Strength"));
				choiceLetters.Add(TEXT("I"));
				choiceNames.Add(TEXT("Intelligence"));
				choiceLetters.Add(TEXT("D"));
				choiceNames.Add(TEXT("Dexterity"));
				choiceTitle = TEXT("Choose an attribute to increase:");
				isChoiceOpen = true;				
			}

			// If it's a Yes/No confirmation
			bool isYesNo = false;
			for (int i = 0; i < charArray.Num(); i++) {
				if (charArray[i].Contains(TEXT("Are you sure you want to"))
				|| charArray[i].Contains(TEXT("Really renounce your faith"))
				|| charArray[i].Contains(TEXT("Do you wish to receive"))
				|| charArray[i].Contains(TEXT("Do you really want to"))
			) {
					isYesNo = true;
				}
				if (isYesNo && (charArray[i].Contains(TEXT("Okay")) || charArray[i].Contains(TEXT("You have lost your religion")))) {
					isYesNo = false;
				}
			}
			if (isYesNo) {
				UE_LOG(LogTemp, Display, TEXT("Found yes/no confirmation"));
				choiceLetters.Empty();
				choiceNames.Empty();
				choiceLetters.Add(TEXT("YY"));
				choiceNames.Add(TEXT("Yes"));
				choiceLetters.Add(TEXT("N"));
				choiceNames.Add(TEXT("No"));
				choiceTitle = TEXT("Are you sure?");
				isChoiceOpen = true;
			}

			// Otherwise hide the window and reset
			if (isChoiceOpen && !isChoice && !isAttributeChoice && !isItemOrEnemy && !isYesNo) {
				UE_LOG(LogTemp, Display, TEXT("Clearing choice window"));
				choiceLetters.Empty();
				choiceNames.Empty();
				isChoiceOpen = false;
				refToChoiceActor->SetActorHiddenInGame(!isChoiceOpen);
				refToChoiceActor->SetActorEnableCollision(isChoiceOpen);

			// If we should show the choice window
			} else if (isChoiceOpen && (isChoice || isAttributeChoice || isYesNo)) {
				UE_LOG(LogTemp, Display, TEXT("Showing choice window"));

				// Set up the window
				UWidgetComponent* WidgetComponentChoice = Cast<UWidgetComponent>(refToChoiceActor->GetComponentByClass(UWidgetComponent::StaticClass()));
				if (WidgetComponentChoice != nullptr) {
					UUserWidget* UserWidgetChoice = WidgetComponentChoice->GetUserWidgetObject();
					if (UserWidgetChoice != nullptr) {

						// Set all the options
						for (int i = 0; i < 7; i++) {
							FString name = TEXT("TextOption") + FString::FromInt(i+1);
							UTextBlock* TextBox = Cast<UTextBlock>(UserWidgetChoice->GetWidgetFromName(*name));
							if (TextBox != nullptr) {
								if (i < choiceNames.Num()) {
									TextBox->SetText(FText::FromString(choiceNames[i]));
								} else {
									TextBox->SetText(FText::FromString(TEXT("")));
								}
							}
							FString imageName = name.Replace(TEXT("TextOption"), TEXT("ImageOption"));
							UImage* ButtonImage = Cast<UImage>(UserWidgetChoice->GetWidgetFromName(*imageName));
							if (ButtonImage != nullptr) {
								if (i < choiceLetters.Num()) {
									FString matName = itemNameToTextureName(choiceNames[i]);
									UTexture2D* tex2D = getTexture(matName);
									if (tex2D != nullptr) {
										ButtonImage->SetBrushFromTexture(tex2D);
										ButtonImage->SetVisibility(ESlateVisibility::Visible);
									} else {
										ButtonImage->SetVisibility(ESlateVisibility::Hidden);
									}
								} else {
									ButtonImage->SetVisibility(ESlateVisibility::Hidden);
								}
							}
						}

						// Set the title
						UTextBlock* TitleBox = Cast<UTextBlock>(UserWidgetChoice->GetWidgetFromName(TEXT("TextOptionTitle")));
						if (TitleBox != nullptr) {
							TitleBox->SetText(FText::FromString(choiceTitle));
						}

					}
				}
				
				// Show it
				isChoiceOpen = true;
				refToChoiceActor->SetActorHiddenInGame(!isChoiceOpen);
				refToChoiceActor->SetActorEnableCollision(isChoiceOpen);

			}

			// If it's the ctrl-X menu
			bool isFullDescription = false;
			for (int i = 0; i < charArray.Num(); i++) {
				if (charArray[i].Contains(TEXT("(select to target/travel, [!] to examine)"))) {
					isFullDescription = true;
					break;
				}
			}
			if (isFullDescription) {

				// If told to skip, skip
				if (skipNextFullDescription) {
					skipNextFullDescription = false;
					continue;
				}

				// Parse the full description
				for (int i = 0; i < charArray.Num(); i++) {

					// The section headings
					if (!charArray[i].Contains(TEXT("Visible"))) {
						if (charArray[i].Contains(TEXT("Items"))) {
							currentType = TEXT("Items");
						} else if (charArray[i].Contains(TEXT("Monsters"))) {
							currentType = TEXT("Monsters");
						} else if (charArray[i].Contains(TEXT("Features"))) {
							currentType = TEXT("Features");
						}
					}

					// If we have a thing
					if (charArray[i].Contains(TEXT("-"))) {
						shouldRedraw = true;

						// Extract the coordinates and description
						UE_LOG(LogTemp, Display, TEXT("FULL -> %s"), *charArray[i]);
						charArray[i] = charArray[i].Replace(TEXT("( )"), TEXT("()"));
						TArray<FString> words;
						charArray[i].ParseIntoArray(words, TEXT(" "), true);
						FString hotkey = words[0];
						FString xString = words[3].Replace(TEXT("("), TEXT("")).Replace(TEXT(","), TEXT(""));
						FString yString = words[4].Replace(TEXT(")"), TEXT(""));
						int xCoord = LOS + FCString::Atoi(*xString);
						int yCoord = LOS - FCString::Atoi(*yString);
						FString description = TEXT("");
						for (int j = 5; j < words.Num(); j++) {
							description += words[j] + TEXT(" ");
						}
						description = description.Replace(TEXT("(here)"), TEXT(""));
						description = description.TrimStartAndEnd();
						UE_LOG(LogTemp, Display, TEXT("FULL - parsed (%d, %d): %s (%s)"), xCoord, yCoord, *description, *currentType);

						// Depending on the type, update the level info
						if (xCoord >= 0 && xCoord < gridWidth && yCoord >= 0 && yCoord < gridWidth) {

							// If it's smoke
							if (description.Contains(TEXT("clouded"))) {
								FString smokeDescription = description.Mid(description.Find(TEXT("clouded in "))).TrimStartAndEnd();
								smokeDescription = smokeDescription.Replace(TEXT("clouded in "), TEXT(""));
								smokeDescription = smokeDescription.Replace(TEXT("("), TEXT(""));
								smokeDescription = smokeDescription.Replace(TEXT(")"), TEXT(""));
								levelInfo[yCoord][xCoord].effect = smokeDescription;
							}

							// If it's a monster
							if (currentType == TEXT("Monsters")) {
								levelInfo[yCoord][xCoord].enemy = description;
								levelInfo[yCoord][xCoord].enemyHotkey = hotkey;

							// If it's an item
							} else if (currentType == TEXT("Items")) {
								levelInfo[yCoord][xCoord].items.Add(description);
								levelInfo[yCoord][xCoord].itemHotkeys.Add(hotkey);

							// If it's a dungeon feature
							} else if (currentType == TEXT("Features")) {
								
								// Traps
								if (description.Contains(TEXT("trap"))) {
									levelInfo[yCoord][xCoord].items.Add(description);
									levelInfo[yCoord][xCoord].itemHotkeys.Add(hotkey);

								// Gates
								} else if (gateList.Contains(description)) {
									levelInfo[yCoord][xCoord].items.Add(description);
									levelInfo[yCoord][xCoord].itemHotkeys.Add(hotkey);

								// Statues
								} else if (description.Contains(TEXT("statue"))) {
									levelInfo[yCoord][xCoord].items.Add(description);
									levelInfo[yCoord][xCoord].itemHotkeys.Add(hotkey);

								// Idols
								} else if (description.Contains(TEXT("idol"))) {
									levelInfo[yCoord][xCoord].items.Add(description);
									levelInfo[yCoord][xCoord].itemHotkeys.Add(hotkey);

								// Fountains
								} else if (description.Contains(TEXT("fountain"))) {
									levelInfo[yCoord][xCoord].items.Add(description);
									levelInfo[yCoord][xCoord].itemHotkeys.Add(hotkey);

								// Arches
								} else if (description.Contains(TEXT("arch"))) {
									levelInfo[yCoord][xCoord].items.Add(description);
									levelInfo[yCoord][xCoord].itemHotkeys.Add(hotkey);
								
								// Altars
								} else if (description.Contains(TEXT("altar"))) {
									levelInfo[yCoord][xCoord].items.Add(description);
									levelInfo[yCoord][xCoord].itemHotkeys.Add(hotkey);

								// Open door
								} else if (description.Contains(TEXT("open door"))) {
									levelInfo[yCoord][xCoord].currentChar = TEXT("'");
									levelInfo[yCoord][xCoord].floorChar = TEXT(".");

								// Closed door
								} else if (description.Contains(TEXT("closed door"))) {
									levelInfo[yCoord][xCoord].currentChar = TEXT("+");
									levelInfo[yCoord][xCoord].floorChar = TEXT(".");

								// Stairs down
								} else if (words[2].Contains(TEXT("(>)"))) {
									levelInfo[yCoord][xCoord].currentChar = TEXT(">");
									levelInfo[yCoord][xCoord].floorChar = TEXT(".");

								// Stairs up
								} else if (words[2].Contains(TEXT("(<)"))) {
									levelInfo[yCoord][xCoord].currentChar = TEXT("<");
									levelInfo[yCoord][xCoord].floorChar = TEXT(".");

								// Tree
								} else if (description.Contains(TEXT("tree"))) {
									levelInfo[yCoord][xCoord].enemy = "a tree";
									levelInfo[yCoord][xCoord].enemyHotkey = hotkey;

								// Shallow water
								} else if (description.Contains(TEXT("shallow water"))) {
									levelInfo[yCoord][xCoord].currentChar = TEXT("~");
									levelInfo[yCoord][xCoord].floorChar = TEXT("~");

								// Shops
								} else if (description.Contains(TEXT(" shop")) 
									|| description.Contains(TEXT(" bazaar")) 
									|| description.Contains(TEXT(" store")) 
									|| description.Contains(TEXT(" antiques")) 
									|| description.Contains(TEXT(" distillery")) 
									|| description.Contains(TEXT(" boutique")) 
									|| description.Contains(TEXT(" emporium"))) {
									levelInfo[yCoord][xCoord].items.Add(description);
									levelInfo[yCoord][xCoord].itemHotkeys.Add(hotkey);

								// Deep water
								} else if (description.Contains(TEXT("deep water"))) {
									levelInfo[yCoord][xCoord].currentChar = TEXT("H");
									levelInfo[yCoord][xCoord].floorChar = TEXT("H");
								}

							}
						}

					}
				}
			}

			// Check the ascii to see if the map is being shown right now
			bool isMap = false;
			if (!isItemOrEnemy && !isFullDescription && !isInventory) {
				for (int i = 0; i < levelAscii.Num(); i++) {
					for (int j = 0; j < levelAscii[i].Num(); j++) {
						if (levelAscii[i][j] == TEXT("#") || levelAscii[i][j] == TEXT("@")) {
							isMap = true;
							break;
						}
					}
				}
			}

			// If somehow we have a map but crawl hasn't loaded, set up the game
			if (!isMenu && isMap && !crawlHasStarted) {
				UE_LOG(LogTemp, Warning, TEXT("Forcing game start"));
				crawlHasStarted = true;
				refToUIActor->SetActorHiddenInGame(false);
				refToUIActor->SetActorEnableCollision(true);
				refToSaveActor->SetActorHiddenInGame(true);
				refToSaveActor->SetActorEnableCollision(false);
				refToTutorialActor->SetActorHiddenInGame(false);
				refToTutorialActor->SetActorEnableCollision(true);
				refToInventoryActor->SetActorHiddenInGame(true);
				refToInventoryActor->SetActorEnableCollision(false);
				refToMainMenuActor->SetActorHiddenInGame(true);
				refToMainMenuActor->SetActorEnableCollision(false);
				refToMainInfoActor->SetActorHiddenInGame(true);
				refToMainInfoActor->SetActorEnableCollision(false);
				inventoryOpen = false;
				commandQueue.Empty();
			}

			// If the map is being shown, then it means we also have the status section
			if (isMap) {
				if (charArray.Num() >= 15) {

					// Extract the name and thus the filename
					FString nameLine = charArray[1].Mid(37);
					nameLine = nameLine.Replace(TEXT("*WIZARD*"), TEXT("")).TrimStartAndEnd();
					fullName = nameLine;
					saveFile = nameLine.Left(nameLine.Find(TEXT(" the "))).Replace(TEXT(" "), TEXT(""));
					UE_LOG(LogTemp, Display, TEXT("STATUS - name line -> %s"), *nameLine);
					UE_LOG(LogTemp, Display, TEXT("STATUS - setting save file -> %s"), *saveFile);

					// Extract the health
					FString healthLine = charArray[3].Mid(45);
					healthLine = healthLine.Replace(TEXT("/"), TEXT(" "));
					UE_LOG(LogTemp, Display, TEXT("STATUS - health line -> %s"), *healthLine);
					TArray<FString> wordsHP;
					healthLine.ParseIntoArray(wordsHP, TEXT(" "), true);
					if (wordsHP.Num() >= 2) {
						currentHP = FCString::Atoi(*wordsHP[0]);
						maxHP = FCString::Atoi(*wordsHP[1]);
					}

					// Extract the mana
					FString manaLine = charArray[4].Mid(45);
					UE_LOG(LogTemp, Display, TEXT("STATUS - mana line -> %s"), *manaLine);
					manaLine = manaLine.Replace(TEXT("/"), TEXT(" "));
					TArray<FString> wordsMP;
					manaLine.ParseIntoArray(wordsMP, TEXT(" "), true);
					if (wordsMP.Num() >= 2) {
						currentMP = FCString::Atoi(*wordsMP[0]);
						maxMP = FCString::Atoi(*wordsMP[1]);
					}

					// Extract the branch info
					FString branchLine = charArray[8].Mid(55);
					UE_LOG(LogTemp, Display, TEXT("STATUS - branch line -> %s"), *branchLine);
					branchLine = branchLine.Replace(TEXT("Place: "), TEXT(""));
					int32 colonIndex = branchLine.Find(TEXT(":"));
					if (colonIndex != INDEX_NONE) {
						branchLine = branchLine.Mid(0, colonIndex);
					}
					currentBranch = itemNameToTextureName(branchLine);
					if (branchLine.Len() > 0 && branchLine[branchLine.Len()-1] == TEXT('s')) {
						currentBranch += TEXT("s");
					}
					UE_LOG(LogTemp, Display, TEXT("STATUS - branch -> %s"), *currentBranch);

					// Extract the left/right/status lines
					FString prevLeftText = leftText;
					FString prevRightText = rightText;
					leftText = charArray[11].Mid(36);
					rightText = charArray[10].Mid(36);
					statusText = charArray[12].Mid(37);
					leftText = leftText.TrimStartAndEnd();
					rightText = rightText.TrimStartAndEnd();
					statusText = statusText.TrimEnd();
					statusText = statusText.Replace(TEXT("===MENU==="), TEXT(""));
					rightText = rightText.Mid(3);
					UE_LOG(LogTemp, Display, TEXT("STATUS - left line -> %s"), *leftText);
					UE_LOG(LogTemp, Display, TEXT("STATUS - right line -> %s"), *rightText);
					UE_LOG(LogTemp, Display, TEXT("STATUS - status line -> %s"), *statusText);
					if (leftText != prevLeftText || rightText != prevRightText) {
						shouldRedrawHotbar = true;
					}

				}

			}

			// If we don't know how much gold we have
			bool goldInQueue = false;
			for (int i = 0; i < commandQueue.Num(); i++) {
				if (commandQueue[i] == TEXT("$")) {
					goldInQueue = true;
					break;
				}
			}
			if (gold == -1 && !isMenu && !goldInQueue && isMap) {
				writeCommandQueued(TEXT("$"));
			}

			// If the map is being shown, then we also have the log
			if (isMap) {
				if (charArray.Num() >= 18) {
					logText.Empty();
					for (int i = 18; i < charArray.Num(); i++) {
						FString newLine = charArray[i].Replace(TEXT("\n"), TEXT("")).Replace(TEXT("_"), TEXT(""));
						if (newLine.Replace(TEXT(" "), TEXT("")).Len() > 0) {
							newLine = newLine.TrimStartAndEnd();
							if (newLine.Mid(0, 1) == TEXT("_")) {
								newLine = newLine.Mid(1);
							}

							// If told that we don't have a religion
							if (newLine.Contains(TEXT("You are not religious"))) {
								religionText = TEXT("You are not religious.");
								shouldRedrawReligion = true;
								continue;
							}

							// If told how much gold we have "You now have 899 gold pieces"
							if (newLine.Contains(TEXT("gold pieces")) && newLine.Contains(TEXT(" have"))) {
								FString goldString = "";
								for (int j = 0; j < newLine.Len(); j++) {
									if (newLine[j] >= TEXT('0') && newLine[j] <= TEXT('9')) {
										goldString += newLine[j];
									}
									if (newLine[j] == TEXT('(') && goldString.Len() > 0) {
										break;
									}
								}
								gold = FCString::Atoi(*goldString);
								UE_LOG(LogTemp, Display, TEXT("LOG - gold: %d"), gold);
								shouldRedrawInventory = true;
								continue;
							}

							// If told that we already know all spells
							if (newLine.Contains(TEXT("You already know all available spells"))) {
								if (memorizing) {
									spellLetters.Empty();
									spellLetterToInfo.Empty();
									shouldRedrawSpells = true;
								}
								continue;
							}

							// If told that we don't know any spells
							if (newLine.Contains(TEXT("You don't know any spells"))) {
								if (!memorizing) {
									spellLetters.Empty();
									spellLetterToInfo.Empty();
									shouldRedrawSpells = true;
								}
								continue;
							}

							// If told that we don't have any abilities
							if (newLine.Contains(TEXT("not good enough to have"))) {
								abilityLetters.Empty();
								abilityLetterToInfo.Empty();
								shouldRedrawAbilities = true;
								continue;
							}

							// Ignore some lines
							if (newLine.Contains(TEXT("Press ?"))
								|| newLine.Contains(TEXT("Okay, then"))
								|| newLine.Contains(TEXT("Data directory"))
								|| newLine.Contains(TEXT("DCSS"))
								|| newLine.Contains(TEXT("Unknown command"))
								|| newLine.Contains(TEXT("Blink to where?"))
								|| newLine.Contains(TEXT("Really renounce your faith"))
								|| newLine.Contains(TEXT("Are you sure?"))
								|| newLine.Contains(TEXT("Cast which spell?"))
								|| newLine.Contains(TEXT("Press <"))
								|| newLine.Contains(TEXT("Wizard Command"))
								|| newLine.Contains(TEXT("Aiming:"))
								|| newLine.Contains(TEXT("Casting:"))
								|| newLine.Contains(TEXT("No monsters, items or features"))
								|| newLine.Contains(TEXT("game will not be scored"))
								|| newLine.Contains(TEXT("enter wizard mode"))
								|| newLine.Contains(TEXT("Press:"))
								|| newLine.Contains(TEXT("Mapping level"))
							) {
								continue;
							}

							UE_LOG(LogTemp, Display, TEXT("LOG - adding: %s"), *newLine);
							logText.Add(newLine);

						}
					}
					UWidgetComponent* WidgetComponentDesc = Cast<UWidgetComponent>(refToUIActor->GetComponentByClass(UWidgetComponent::StaticClass()));
					if (WidgetComponentDesc != nullptr) {
						UUserWidget* UserWidgetDesc = WidgetComponentDesc->GetUserWidgetObject();
						if (UserWidgetDesc != nullptr) {
							UTextBlock* LogBox = Cast<UTextBlock>(UserWidgetDesc->GetWidgetFromName(TEXT("TextLog")));
							if (LogBox != nullptr) {
								FString toRender = TEXT("");
								for (int i = std::max(0,logText.Num()-maxLogShown); i < logText.Num(); i++) {
									toRender += logText[i].TrimStartAndEnd();
									if (i < logText.Num() - 1) {
										toRender += TEXT("\n");
									}
								}
								LogBox->SetText(FText::FromString(toRender));
							} else {
								UE_LOG(LogTemp, Warning, TEXT("LOG - Log box not found"));
							}
						}
					}
				}
			}

			// Update the map
			if (isMap) {
				for (int i = 0; i < gridWidth; i++) {
					for (int j = 0; j < gridWidth; j++) {
						mapToDraw[i][j] = levelInfo[i][j].currentChar;
					}
				}
				mapToDraw[LOS][LOS] = TEXT("@");
			}
			
			// If trying to attack unarmed, let them
			FString lastLine = charArray[charArray.Num()-1];
			int lastLineInd = 2;
			while (lastLine.TrimStartAndEnd().Len() == 0 && charArray.Num()-lastLineInd >= 0) {
				lastLine = charArray[charArray.Num()-lastLineInd];
				lastLineInd++;
			}
			if (lastLine.Contains(TEXT("Really attack while"))) {
				writeCommandQueued(TEXT("Y"));
				writeCommandQueued(TEXT("enter"));
			}

			// If we have the welcome text, we should pretend as though it's new
			bool hasWelcomeText = false;
			for (int i = 0; i < charArray.Num(); i++) {
				if (charArray[i].Contains(TEXT("Welcome"), ESearchCase::CaseSensitive)) {
					hasWelcomeText = true;
					break;
				}
			}
			if (!isMenu && hasWelcomeText && !hasBeenWelcomed) {
				UE_LOG(LogTemp, Display, TEXT("Getting initial list of things..."));
				writeCommandQueued("CLEAR");
				writeCommandQueued("ctrl-X");
				writeCommandQueued(">");
				writeCommandQueued(">");
				writeCommandQueued(">");
				writeCommandQueued("escape");
				writeCommandQueued("enter");
				writeCommandQueued("&");
				writeCommandQueued("{");
				writeCommandQueued("enter");
				hasBeenWelcomed = true;
			}

			// If something has changed in the level
			UE_LOG(LogTemp, Display, TEXT("Is map? %d"), isMap);
			UE_LOG(LogTemp, Display, TEXT("Level ascii are different? %d"), levelAscii != levelAsciiPrev);
			if (isMap && levelAscii != levelAsciiPrev && hasLoaded) {

				// Copy into the level info
				FString thingsThatCountAsFloors = TEXT(".H~");
				for (int i = 0; i < gridWidth; i++) {
					for (int j = 0; j < gridWidth; j++) {
						if (thingsThatCountAsWalls.Contains(levelAscii[i][j], ESearchCase::CaseSensitive) || thingsThatCountAsFloors.Contains(levelAscii[i][j], ESearchCase::CaseSensitive)) {
							levelInfo[i][j].currentChar = levelAscii[i][j];
						} else if (levelInfo[i][j].enemy.Len() == 0 && (levelAscii[i][j] == TEXT("c") || levelAscii[i][j] == TEXT("C") || levelAscii[i][j] == TEXT("7"))) {
							levelInfo[i][j].currentChar = levelAscii[i][j];
							levelInfo[i][j].floorChar = TEXT(".");
						} else if (levelAscii[i][j] == TEXT("<") || levelAscii[i][j] == TEXT(">")) {
							levelInfo[i][j].currentChar = levelAscii[i][j];
							levelInfo[i][j].floorChar = TEXT(".");
						} else if (levelAscii[i][j] != TEXT(" ")) {
							levelInfo[i][j].currentChar = TEXT(".");
							levelInfo[i][j].floorChar = TEXT(".");
						}
						if (thingsThatCountAsFloors.Contains(levelAscii[i][j], ESearchCase::CaseSensitive) && levelInfo[i][j].enemy.Len() == 0) {
							levelInfo[i][j].floorChar = levelAscii[i][j];
						}
					}
				}
				
				// If we have a space surrounded by non-spaces, set it as a floor
				for (int i = 1; i < gridWidth - 1; i++) {
					for (int j = 1; j < gridWidth - 1; j++) {
						if (levelInfo[i][j].currentChar == TEXT(" ") && levelInfo[i - 1][j].currentChar != TEXT(" ") && levelInfo[i + 1][j].currentChar != TEXT(" ") && levelInfo[i][j - 1].currentChar != TEXT(" ") && levelInfo[i][j + 1].currentChar != TEXT(" ")) {
							levelInfo[i][j].currentChar = TEXT(".");
							levelInfo[i][j].floorChar = TEXT(".");
						}
					}
				}

				// If we can't see items or enemies
				for (int k = 0; k < charArray.Num(); k++) {
					if (charArray[k].Contains(TEXT("No monsters, items or features are visible"))) {
						for (int i = 0; i < gridWidth; i++) {
							for (int j = 0; j < gridWidth; j++) {
								levelInfo[i][j].items.Empty();
								levelInfo[i][j].enemy = TEXT("");
								levelInfo[i][j].effect = TEXT("");
							}
						}
						break;
					}
				}

				// Send the commands to list everything
				if (!isMenu) {
					UE_LOG(LogTemp, Display, TEXT("Getting list of things..."));
					writeCommandQueued("CLEAR");
					writeCommandQueued("ctrl-X");
					writeCommandQueued(">");
					writeCommandQueued(">");
					writeCommandQueued(">");
					writeCommandQueued("escape");
				}

				// We should update the scene
				levelAsciiPrev = levelAscii;
				shouldRedraw = true;

			}

			// Update the visuals
			if (shouldRedraw) {
				updateLevel();
			}

		}

	}

}

