// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SLTilemapLib.h"
#include "Subsystems/WorldSubsystem.h"
#include "SLTilemapSubsystem.generated.h"



UCLASS()
class SLTILEMAP_API USLTilemapSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()


public:
	//Begin Subsystem
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	//End Subsystem
	
	UPROPERTY(BlueprintReadWrite, Category = "SLTilemap")
	FTileMap MapData;
	UPROPERTY(BlueprintReadWrite, Category = "SLTilemap")
	FTileMap PatternData;
	UPROPERTY(BlueprintReadWrite, Category = "SLTilemap")
	int32 PatternSize = 3;
	UPROPERTY(BlueprintReadOnly, Category = "SLTilemap")
	TArray<FTileMap> AllPossiblePatterns;
	UPROPERTY(BlueprintReadOnly, Category = "SLTilemap")
	TArray<FPatternCell> PatternCells;
	UPROPERTY(BlueprintReadOnly, Category = "SLTilemap")
	UTexture2D* MapDataTexture;
	UPROPERTY(BlueprintReadOnly, Category = "SLTilemap")
	UTexture2D* PatternDataTexture;

	UFUNCTION(BlueprintCallable, Category = "SLTilemap")
	void BPGeneratePatterns();
	UFUNCTION(BlueprintCallable, Category = "SLTilemap")
	void BPInitPatternCells();
	UFUNCTION(BlueprintCallable, Category = "SLTilemap")
	void BPUpdateAllPatternCells();

	
	UFUNCTION(BlueprintCallable, Category = "SLTilemap")
	bool InitializeWFC();
	UFUNCTION(BlueprintCallable, Category = "SLTilemap")
	bool StepWFC();
	UFUNCTION(Blueprintcallable, Category = "SLTilemap")
	bool RunWFC();


private:
	void GeneratePatterns();
	void InitPatternCells();
	void UpdateAllPatternCells();
	void UpdatePatternCellEntropy(FPatternCell& Cell);
	void WritePatternToMapData(const FTileMap& Pattern, int32 x, int32 y);
	bool CanPatternFitAtThisLocation(const FTileMap& Pattern, int32 x, int32 y) const;
	void ObservePatternCell(FPatternCell& CellToObserve);
	FTileMap OrCellPatternsTogether(FPatternCell& Cell);	
};
