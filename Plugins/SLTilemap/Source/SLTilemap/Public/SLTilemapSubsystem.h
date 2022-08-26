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
	FTileMap InputTileMap;
	UPROPERTY(BlueprintReadWrite, Category = "SLTilemap")
	FTileMap OutputTileMap;
	
	UPROPERTY(BlueprintReadWrite, Category = "SLTilemap")
	int32 PatternSize = 3;
	UPROPERTY(BlueprintReadOnly, Category = "SLTilemap")
	TArray<FTileMap> Patterns;
	UPROPERTY(BlueprintReadOnly, Category = "SLTilemap")
	TArray<int32> Counts;
	UPROPERTY(BlueprintReadOnly, Category = "SLTilemap")
	TArray<float> P;
	UPROPERTY(BlueprintReadOnly, Category = "SLTilemap")
	TArray<float> PlogP;
	UPROPERTY(BlueprintReadOnly, Category = "SLTilemap")
	TArray<FCell> Cells;
	
	UFUNCTION(BlueprintCallable, Category = "SLTilemap")
	bool InitializeWFC();
	UFUNCTION(BlueprintCallable, Category = "SLTilemap")
	bool StepWFC();
	UFUNCTION(Blueprintcallable, Category = "SLTilemap")
	bool RunWFC();


private:
	void GeneratePatterns();
	void InitPatternCells();
	void CellUpdateAllowedPatterns(FCell& Cell);
	void AddPatternToPatterns(const FTileMap& Pattern);
	void CellUpdateEntropy(FCell& Cell);
	void WritePatternToMapData(const FTileMap& Pattern, int32 x, int32 y);
	bool CanPatternFitAtThisLocation(const FTileMap& Pattern, int32 x, int32 y) const;
	void ObservePatternCell(FCell& CellToObserve);
	FTileMap OrCellPatternsTogether(FCell& Cell);	
};
