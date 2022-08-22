// Fill out your copyright notice in the Description page of Project Settings.


#include "SLTilemapSubsystem.h"


void USLTilemapSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
}

void USLTilemapSubsystem::Deinitialize()
{
}

void USLTilemapSubsystem::BPGeneratePatterns()
{
	GeneratePatterns();
}

void USLTilemapSubsystem::BPInitPatternCells()
{
	InitPatternCells();
}

void USLTilemapSubsystem::BPUpdateAllPatternCells()
{
	UpdateAllPatternCells();
}

bool USLTilemapSubsystem::InitializeWFC()
{
	//Bail out if initial data is no good
	if (!(USLTilemapLib::IsTilemapValid(MapData) && USLTilemapLib::IsTilemapValid(PatternData)))
	{
		return false;
	}
	GeneratePatterns();
	InitPatternCells();
	UpdateAllPatternCells();
	return true;
}

bool USLTilemapSubsystem::StepWFC()
{
	//Find unobserved PatternCell with lowest entropy
	FPatternCell* CellToObserve = nullptr;
	int32 LowestEntropy = AllPossiblePatterns.Num() + 1;
	for (auto& Cell : PatternCells)
	{
		//Reset bVisited while we're here.
		Cell.bVisited = false;
		if (!Cell.bIsObserved && Cell.Entropy < LowestEntropy && Cell.Entropy > 0)
		{
			LowestEntropy = Cell.Entropy;
			CellToObserve = &Cell;
		}
	}

	//Observe Pattern cell with lowest entropy if found
	if (CellToObserve != nullptr)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Observing cell at %d,%d and it has entropy %d"), CellToObserve -> x, CellToObserve -> y, CellToObserve -> Entropy);

		ObservePatternCell(*CellToObserve);
		UpdatePatternCellEntropy(*CellToObserve);
		CellToObserve->bVisited = true;


		//Propagate new state
		TQueue<FPatternCell*> CellsToUpdate;
		for (auto& Neighbor : CellToObserve->PointersToNeighbors)
		{
			if (!Neighbor->bIsObserved)
			{
				CellsToUpdate.Enqueue(Neighbor);
			}
		}
		while (!CellsToUpdate.IsEmpty())
		{
			//Get next cell in queue
			FPatternCell* ThisCell;
			CellsToUpdate.Dequeue(ThisCell);
			ThisCell->bVisited = true;

			//cache Entropy
			const int32 OldEntropy = ThisCell->Entropy;
			UpdatePatternCellEntropy(*ThisCell);
			const int32 NewEntropy = ThisCell->Entropy;

			//Did the cell's entropy change?
			if (OldEntropy != NewEntropy)
			{
				const FTileMap CombinedPatterns = OrCellPatternsTogether(*ThisCell);
				WritePatternToMapData(CombinedPatterns, ThisCell->X, ThisCell->Y);
				//If Entropy = 1 then it's collapsed
				if (NewEntropy == 1)
				{
					ThisCell->bIsObserved = true;
				}

				//Enque neighbors
				for (auto& Neighbor : ThisCell->PointersToNeighbors)
				{
					if (!Neighbor->bIsObserved && !Neighbor->bVisited)
					{
						CellsToUpdate.Enqueue(Neighbor);
					}
				}
			}
		}
		return false;
	}
	return true;
}

bool USLTilemapSubsystem::RunWFC()
{
	if (!InitializeWFC())
	{
		return false;
	}
	int32 StepCount = 0;
	while (StepCount < 100000)
	{
		StepWFC();
		StepCount++;
	}
	
	return true;
}



void USLTilemapSubsystem::GeneratePatterns()
{
	//Generate Patterns

	AllPossiblePatterns.Empty();
	
	for (int32 j = 0; j < PatternData.Height - PatternSize + 1; j++)
	{
		for (int32 i = 0; i < PatternData.Width - PatternSize + 1; i++)
		{
			AllPossiblePatterns.AddUnique(USLTilemapLib::GetTilemapSection(PatternData, i, j, PatternSize, PatternSize));
		}
	}
	
	//Add Rotations
	int32 InitialPatternCount = AllPossiblePatterns.Num();
	for (int32 i = 0; i < InitialPatternCount; ++i)
	{
		FTileMap Pattern = AllPossiblePatterns[i];
		Pattern = USLTilemapLib::RotateTilemap(Pattern);
		AllPossiblePatterns.AddUnique(Pattern);
		Pattern = USLTilemapLib::RotateTilemap(Pattern);
		AllPossiblePatterns.AddUnique(Pattern);
		Pattern = USLTilemapLib::RotateTilemap(Pattern);
		AllPossiblePatterns.AddUnique(Pattern);
	}
	

	//Add Reflections
	InitialPatternCount = AllPossiblePatterns.Num();
	for (int32 i = 0; i < InitialPatternCount; ++i)
	{
		FTileMap Pattern = AllPossiblePatterns[i];
		Pattern = USLTilemapLib::MirrorTilemap(Pattern);
		AllPossiblePatterns.AddUnique(Pattern);
	}
	
}

void USLTilemapSubsystem::InitPatternCells()
{
	PatternCells.Empty();
	TArray<bool> PatternIsAllowed;
	PatternIsAllowed.Init(true, AllPossiblePatterns.Num());
	const int32 PatternCellsWidth = MapData.Width - PatternSize + 1;
	const int32 PatternCellsHeight = MapData.Height - PatternSize + 1;
	const FTileMap DummyTileMap = FTileMap(PatternCellsWidth, PatternCellsHeight);

	for (int32 j = 0; j < PatternCellsHeight; j++)
	{
		for (int32 i = 0; i < PatternCellsWidth; i++)
		{
			TArray<int32> Neighbors;
			for (int32 y = -1; y <= 1; y++)
			{
				for (int32 x = -1; x <= 1; x++)
				{
					const int32 NeighborX = i + x;
					const int32 NeighborY = j + y;
					if (NeighborX > -1 && NeighborX < PatternCellsWidth && NeighborY > -1 && NeighborY < PatternCellsHeight && !(x == 0 && y == 0))
					{
						Neighbors.Add(USLTilemapLib::TileMapXYToIndex(DummyTileMap, NeighborX, NeighborY));
					}
				}
			}
			PatternCells.Add(FPatternCell(i, j, Neighbors, PatternIsAllowed));
		}
	}
	for (auto& Cell : PatternCells)
	{
	//Set pointers to neighbors
	//UE_LOG(LogTemp, Warning, TEXT("Cell at %d,%d has %d neighbors"), Cell.X, Cell.Y, Cell.IndicesOfNeighbors.Num());
	for (const auto& NeighborIndex : Cell.IndicesOfNeighbors)
		{
			//UE_LOG(LogTemp, Warning, TEXT("Neighbor index is %d"), NeighborIndex);
			//UE_LOG(LogTemp, Warning, TEXT("Neighbor index is %d"), &PatternCells[NeighborIndex]);
			Cell.PointersToNeighbors.Add(&PatternCells[NeighborIndex]);
		}
	}
}

void USLTilemapSubsystem::UpdateAllPatternCells()
{
	for (auto& Cell : PatternCells)
	{
		UpdatePatternCellEntropy(Cell);
	}
}

void USLTilemapSubsystem::UpdatePatternCellEntropy(FPatternCell& Cell)
{
	int32 NumAllowedPatterns = 0;
	for (int32 i = 0; i < Cell.bAllowedPatterns.Num(); i++)
	{
		if (Cell.bAllowedPatterns[i]) //Don't check patterns already ruled out
		{
			if (CanPatternFitAtThisLocation(AllPossiblePatterns[i], Cell.X, Cell.Y)) //Does pattern still fit?
			{
				NumAllowedPatterns++;
			}
			else
			{
				Cell.bAllowedPatterns[i] = false;
			}
		}
	}
	Cell.Entropy = NumAllowedPatterns;
	if (NumAllowedPatterns < 1)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Observing cell at index %d and it has entropy %d and current bIsObserved is %d"), IndexOfPatternCellToObserve, PatternCells[IndexOfPatternCellToObserve].Entropy,PatternCells[IndexOfPatternCellToObserve].bIsObserved);
		UE_LOG(LogTemp, Warning, TEXT("FUCK SHIT ASS"));
	}
}

bool USLTilemapSubsystem::CanPatternFitAtThisLocation(const FTileMap& Pattern, const int32 x, const int32 y) const
{
	for (int32 j = 0; j < Pattern.Height; j++)
	{
		for (int32 i = 0; i < Pattern.Width; i++)
		{
			const uint8 MapTileState = USLTilemapLib::GetTileAtXY(MapData, x + i, y + j);
			const uint8 PatternTileState = USLTilemapLib::GetTileAtXY(Pattern, i, j);
			const uint8 Test = MapTileState & PatternTileState;
			if (Test != PatternTileState)
			{
				return false;
			}
		}
	}
	return true;
}

void USLTilemapSubsystem::WritePatternToMapData(const FTileMap& Pattern, const int32 x, const int32 y)
{
	for (int32 j = 0; j < Pattern.Height; j++)
	{
		for (int32 i = 0; i < Pattern.Width; i++)
		{
			const uint8 Temp = USLTilemapLib::GetTileAtXY(Pattern, i, j);
			USLTilemapLib::SetTileAtXY(MapData, Temp,  x + i, y + j);
		}
	}
}

void USLTilemapSubsystem::ObservePatternCell(FPatternCell& CellToObserve)
{
	TArray<int32> AllowedPatternIndices;
	for (int32 i = 0; i < AllPossiblePatterns.Num(); i++)
	{
		if (CellToObserve.bAllowedPatterns[i])
		{
			AllowedPatternIndices.Add(i);
		}
	}
	const int32 RandomIndex = FMath::RandRange(0, AllowedPatternIndices.Num() - 1);
	const int32 IndexOfPatternToObserve = AllowedPatternIndices[RandomIndex];
	const FTileMap ObservedPattern = AllPossiblePatterns[IndexOfPatternToObserve];
	WritePatternToMapData(ObservedPattern, CellToObserve.X, CellToObserve.Y);
	CellToObserve.bIsObserved = true;
}

FTileMap USLTilemapSubsystem::OrCellPatternsTogether(FPatternCell& Cell)
{
	FTileMap Out = FTileMap(PatternSize, PatternSize, 0);
	for (int32 i = 0; i < Cell.bAllowedPatterns.Num(); i++)
	{
		if (Cell.bAllowedPatterns[i])
		{
			for (int32 j = 0; j < Out.Data.Num(); j++)
			{
				Out.Data[j] = Out.Data[j] | AllPossiblePatterns[i].Data[j];
			}
		}
	}
	return Out;
}
