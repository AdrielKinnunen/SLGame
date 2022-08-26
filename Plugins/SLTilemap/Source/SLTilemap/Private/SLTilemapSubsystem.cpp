// Fill out your copyright notice in the Description page of Project Settings.


#include "SLTilemapSubsystem.h"


void USLTilemapSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
}

void USLTilemapSubsystem::Deinitialize()
{
}

bool USLTilemapSubsystem::InitializeWFC()
{
	const double StartTime = FPlatformTime::Seconds();
	if (!(USLTilemapLib::IsTilemapValid(OutputTileMap) && USLTilemapLib::IsTilemapValid(InputTileMap)))
	{
		return false;
	}
	GeneratePatterns();
	InitPatternCells();
	for (auto& Cell : Cells)
	{
		CellUpdateAllowedPatterns(Cell);
		CellUpdateEntropy(Cell);
	}
	const double EndTime = FPlatformTime::Seconds();
	const double TotalTimems = 1000*(EndTime - StartTime);
	UE_LOG(LogTemp, Warning, TEXT("Initialization took %f ms"), TotalTimems);

	return true;
}

bool USLTilemapSubsystem::StepWFC()
{

	const double StartTime = FPlatformTime::Seconds();

	//Find unobserved PatternCell with lowest entropy
	FCell* CellToObserve = nullptr;
	float LowestEntropy = 1000000000.0;
	for (auto& Cell : Cells)
	{
		if (!Cell.bIsObserved && Cell.Entropy < LowestEntropy)
		{
			LowestEntropy = Cell.Entropy;
			CellToObserve = &Cell;
		}
	}

	//Check for bad entropy found during search
	if (LowestEntropy < 1)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cell at %d, %d has bad entropy %f and was found during lowest entropy search"), CellToObserve -> X, CellToObserve -> Y, CellToObserve -> Entropy);
		return false;
	}
	
	if (CellToObserve == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("No unobserved cell was found"));
		return false;
	}

	//Observe Pattern cell with lowest entropy if found
	{
		UE_LOG(LogTemp, Warning, TEXT("Observing cell at %d,%d and it has entropy %d"), CellToObserve -> X, CellToObserve -> Y, CellToObserve -> Entropy);
		ObservePatternCell(*CellToObserve);
		//UpdatePatternCellEntropy(*CellToObserve);
		
		//Propagate new state
		//Enqueue unobserved neighbors
		TQueue<FCell*> CellsToUpdate;
		for (auto& Neighbor : CellToObserve->PointersToNeighbors)
		{
			if (!Neighbor->bIsObserved)
			{
				CellsToUpdate.Enqueue(Neighbor);
			}
		}

		//Update Cells in queue, enquing their unobserved neighbors if cell changes
		while (!CellsToUpdate.IsEmpty())
		{
			//Get next cell in queue
			FCell* ThisCell;
			CellsToUpdate.Dequeue(ThisCell);

			//cache Entropy
			const int32 OldEntropy = ThisCell->Entropy;
			CellUpdateAllowedPatterns(*ThisCell);
			CellUpdateEntropy(*ThisCell);
			const int32 NewEntropy = ThisCell->Entropy;

			//Fail if cell has bad entropy
			if (NewEntropy < 0)
			{
				UE_LOG(LogTemp, Warning, TEXT("Cell at %d, %d has bad entropy %f during propagation"), ThisCell -> X, ThisCell -> Y, ThisCell -> Entropy);
				return false;
			}
			
			//Did the cell's entropy change?
			if (OldEntropy != NewEntropy)
			{
				//Update Map Data with new cell state
				const FTileMap CombinedPatterns = OrCellPatternsTogether(*ThisCell);
				WritePatternToMapData(CombinedPatterns, ThisCell->X, ThisCell->Y);

				//Enque unobserved neighbors
				for (auto& Neighbor : ThisCell->PointersToNeighbors)
				{
					if (!Neighbor->bIsObserved)
					{
						CellsToUpdate.Enqueue(Neighbor);
					}
				}
			}
		}
	}
	const double EndTime = FPlatformTime::Seconds();
	const double TotalTimems = 1000*(EndTime - StartTime);
	UE_LOG(LogTemp, Warning, TEXT("Step took %f ms"), TotalTimems);

	return true;
}

bool USLTilemapSubsystem::RunWFC()
{
	if (!InitializeWFC())
	{
		return false;
	}
	while (StepWFC())	{}
	return true;
}


void USLTilemapSubsystem::AddPatternToPatterns(const FTileMap& Pattern)
{
	const int Index = Patterns.Find(Pattern);
	if (Index > -1)
	{
		Counts[Index]++;
	}
	else
	{
		Patterns.Add(Pattern);
		Counts.Add(1);
	}
}

void USLTilemapSubsystem::GeneratePatterns()
{
	//Generate Patterns
	Patterns.Empty();
	Counts.Empty();
	for (int32 j = 0; j < InputTileMap.Height - PatternSize + 1; j++)
	{
		for (int32 i = 0; i < InputTileMap.Width - PatternSize + 1; i++)
		{
			FTileMap Pattern = USLTilemapLib::GetTilemapSection(InputTileMap, i, j, PatternSize, PatternSize);
			AddPatternToPatterns(Pattern);
			Pattern = USLTilemapLib::RotateTilemap(Pattern);
			AddPatternToPatterns(Pattern);
			Pattern = USLTilemapLib::RotateTilemap(Pattern);
			AddPatternToPatterns(Pattern);
			Pattern = USLTilemapLib::RotateTilemap(Pattern);
			AddPatternToPatterns(Pattern);
			Pattern = USLTilemapLib::MirrorTilemap(Pattern);
			AddPatternToPatterns(Pattern);
			Pattern = USLTilemapLib::RotateTilemap(Pattern);
			AddPatternToPatterns(Pattern);
			Pattern = USLTilemapLib::RotateTilemap(Pattern);
			AddPatternToPatterns(Pattern);
			Pattern = USLTilemapLib::RotateTilemap(Pattern);
			AddPatternToPatterns(Pattern);
		}
	}
	
	float SumCounts = 0;
	for (const auto& Count : Counts)
	{
		SumCounts += Count;
	}

	P.SetNum(Counts.Num());
	PlogP.SetNum(Counts.Num());
	for (int32 i = 0; i < Counts.Num(); i++)
	{
		const float Probability = Counts[i] / SumCounts;
		P[i] =  Probability;
		PlogP[i] = Probability * log2(Probability);
	}
}

void USLTilemapSubsystem::InitPatternCells()
{
	Cells.Empty();
	const int32 PatternCellsWidth = OutputTileMap.Width - PatternSize + 1;
	const int32 PatternCellsHeight = OutputTileMap.Height - PatternSize + 1;
	const int32 NeighborDistance = PatternSize - 1;
	
	TArray<int32> AllowedPatternIndices;
	for (int32 i = 0; i < Patterns.Num(); i++)
	{
		AllowedPatternIndices.Add(i);
	}
	
	//Create PatternCells and initialize them
	for (int32 j = 0; j < PatternCellsHeight; j++)
	{
		for (int32 i = 0; i < PatternCellsWidth; i++)
		{
			Cells.Add(FCell(i, j, AllowedPatternIndices));
		}
	}

	//Set each PatternCell's Neighbors array
	for (auto& Cell : Cells)
	{
		TArray<FCell*> Neighbors;
		for (int32 Y = -NeighborDistance; Y <= NeighborDistance; Y++)
		{
			for (int32 X = -NeighborDistance; X <= NeighborDistance; X++)
			{
				const int32 NeighborX = Cell.X + X;
				const int32 NeighborY = Cell.Y + Y;
				const int32 NeighborIndex = USLTilemapLib::XYToIndex(PatternCellsWidth, NeighborX, NeighborY);
				if (NeighborX > -1 && NeighborX < PatternCellsWidth && NeighborY > -1 && NeighborY < PatternCellsHeight && !(X == 0 && Y == 0))
				{
					Neighbors.Add(&Cells[NeighborIndex]);
				}
			}
		}
		Cell.PointersToNeighbors = Neighbors;
	}
}

void USLTilemapSubsystem::CellUpdateAllowedPatterns(FCell& Cell)
{
	for (int32 i = Cell.AllowedPatternIndices.Num() - 1; i >= 0; i--)
	{
		//Does pattern still fit?
		const int32 PatternIndex = Cell.AllowedPatternIndices[i];
		if (!CanPatternFitAtThisLocation(Patterns[PatternIndex], Cell.X, Cell.Y))
		{
			Cell.AllowedPatternIndices.RemoveAt(i);
		}
	}
	if (Cell.AllowedPatternIndices.Num() == 1)
	{
		Cell.bIsObserved = true;
	}
}

void USLTilemapSubsystem::CellUpdateEntropy(FCell& Cell)
{
	/*
	float Entropy = 0;
	for (const auto& i : Cell.AllowedPatternIndices)
	{
		Entropy -= PlogP[i];
	}
	//float Entropy = 0;
	//TempEntropy -= Probability * log2(Probability);
	*/
	
	Cell.Entropy = Cell.AllowedPatternIndices.Num();
}

bool USLTilemapSubsystem::CanPatternFitAtThisLocation(const FTileMap& Pattern, const int32 x, const int32 y) const
{
	for (int32 j = 0; j < Pattern.Height; j++)
	{
		for (int32 i = 0; i < Pattern.Width; i++)
		{
			const uint8 MapTileState = USLTilemapLib::GetTileAtXY(OutputTileMap, x + i, y + j);
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
			USLTilemapLib::SetTileAtXY(OutputTileMap, Temp,  x + i, y + j);
		}
	}
}

void USLTilemapSubsystem::ObservePatternCell(FCell& CellToObserve)
{
	const int32 RandomIndex = FMath::RandRange(0, CellToObserve.AllowedPatternIndices.Num() - 1);
	const int32 IndexOfPatternToObserve = CellToObserve.AllowedPatternIndices[RandomIndex];
	const FTileMap ObservedPattern = Patterns[IndexOfPatternToObserve];
	WritePatternToMapData(ObservedPattern, CellToObserve.X, CellToObserve.Y);
	CellToObserve.bIsObserved = true;
}

FTileMap USLTilemapSubsystem::OrCellPatternsTogether(FCell& Cell)
{
	FTileMap Out = FTileMap(PatternSize, PatternSize, 0);
	for (const auto& Index : Cell.AllowedPatternIndices)
	{
		for (int32 i = 0; i < Out.Data.Num(); i++)
		{
			Out.Data[i] = Out.Data[i] | Patterns[Index].Data[i];
		}
	}
	return Out;
}
