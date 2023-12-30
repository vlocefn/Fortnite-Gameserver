#pragma once

#include "reboot.h"
#include "Stack.h"
#include "Actor.h"
#include "hooking.h"
#include "SoftObjectPtr.h"
#include "FortGameModeAthena.h"
#include "GameplayStatics.h"
#include "FortVehicleItemDefinition.h"
#include "FortDagwoodVehicle.h"

// Vehicle class name changes multiple times across versions, so I made it it's own file.


struct FVehicleWeightedDef
{
public:
	static UStruct* GetStruct()
	{
		static auto Struct = FindObject<UStruct>(L"/Script/FortniteGame.VehicleWeightedDef");
		return Struct;
	}

	static int GetStructSize() { return GetStruct()->GetPropertiesSize(); }

	TSoftObjectPtr<UFortVehicleItemDefinition>* GetVehicleItemDef()
	{
		static auto VehicleItemDefOffset = FindOffsetStruct("/Script/FortniteGame.VehicleWeightedDef", "VehicleItemDef");
		return (TSoftObjectPtr<UFortVehicleItemDefinition>*)(__int64(this) + VehicleItemDefOffset);
	}

	FScalableFloat* GetWeight()
	{
		static auto WeightOffset = FindOffsetStruct("/Script/FortniteGame.VehicleWeightedDef", "Weight");
		return (FScalableFloat*)(__int64(this) + WeightOffset);
	}
};

static inline AActor* SpawnVehicleFromSpawner(AActor* VehicleSpawner)
{
	bool bDebugSpawnVehicles = false;

	auto GameMode = Cast<AFortGameModeAthena>(GetWorld()->GetGameMode());

	FTransform SpawnTransform{};
	SpawnTransform.Translation = VehicleSpawner->GetActorLocation();
	SpawnTransform.Rotation = VehicleSpawner->GetActorRotation().Quaternion();
	SpawnTransform.Scale3D = { 1, 1, 1 };

	static auto VehicleClassOffset = VehicleSpawner->GetOffset("VehicleClass", false);
	static auto BGAClass = FindObject<UClass>(L"/Script/Engine.BlueprintGeneratedClass");

	if (VehicleClassOffset != -1) // 10.40 and below?
	{
		auto& SoftVehicleClass = VehicleSpawner->Get<TSoftObjectPtr<UClass>>(VehicleClassOffset);
		auto StrongVehicleClass = SoftVehicleClass.Get(BGAClass, true);

		if (!StrongVehicleClass)
		{
			std::string VehicleClassObjectName = SoftVehicleClass.SoftObjectPtr.ObjectID.AssetPathName.ComparisonIndex.Value == 0 ? "InvalidName" : SoftVehicleClass.SoftObjectPtr.ObjectID.AssetPathName.ToString();
			LOG_WARN(LogVehicles, "Failed to load vehicle class: {}", VehicleClassObjectName);
			return nullptr;
		}

		if (bDebugSpawnVehicles)
			LOG_INFO(LogDev, "Spawning Vehicle: {}", StrongVehicleClass->GetPathName());

		return GetWorld()->SpawnActor<AActor>(StrongVehicleClass, SpawnTransform, CreateSpawnParameters(ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn));
	}

	static auto FortVehicleItemDefOffset = VehicleSpawner->GetOffset("FortVehicleItemDef");

	if (FortVehicleItemDefOffset == -1)
		return nullptr;

	auto& SoftFortVehicleItemDef = VehicleSpawner->Get<TSoftObjectPtr<UFortVehicleItemDefinition>>(FortVehicleItemDefOffset);
	UFortVehicleItemDefinition* VIDToSpawn = nullptr;

	if (SoftFortVehicleItemDef.SoftObjectPtr.ObjectID.AssetPathName.ComparisonIndex.Value == 0)
	{
		static auto FortVehicleItemDefVariantsOffset = VehicleSpawner->GetOffset("FortVehicleItemDefVariants");

		if (FortVehicleItemDefVariantsOffset != -1)
		{
			TArray<FVehicleWeightedDef>& FortVehicleItemDefVariants = VehicleSpawner->Get<TArray<FVehicleWeightedDef>>(FortVehicleItemDefVariantsOffset);

			if (FortVehicleItemDefVariants.size() > 0)
			{
				VIDToSpawn = FortVehicleItemDefVariants.at(0, FVehicleWeightedDef::GetStructSize()).GetVehicleItemDef()->Get(UFortVehicleItemDefinition::StaticClass(), true); // TODO (Milxnor) Implement the weight
			}
		}
	}
	else
	{
		VIDToSpawn = SoftFortVehicleItemDef.Get(UFortVehicleItemDefinition::StaticClass(), true);
	}

	if (!VIDToSpawn)
	{
		std::string FortVehicleItemDefObjectName = SoftFortVehicleItemDef.SoftObjectPtr.ObjectID.AssetPathName.ComparisonIndex.Value == 0 ? "InvalidName" : SoftFortVehicleItemDef.SoftObjectPtr.ObjectID.AssetPathName.ToString();
		LOG_WARN(LogVehicles, "Failed to load vehicle item definition: {}", FortVehicleItemDefObjectName);
		return nullptr;
	}

	UClass* StrongVehicleActorClass = VIDToSpawn->GetVehicleActorClass();

	if (!StrongVehicleActorClass)
	{
		std::string VehicleActorClassObjectName = VIDToSpawn->GetVehicleActorClassSoft()->SoftObjectPtr.ObjectID.AssetPathName.ComparisonIndex.Value == 0 ? "InvalidName" : VIDToSpawn->GetVehicleActorClassSoft()->SoftObjectPtr.ObjectID.AssetPathName.ToString();
		LOG_WARN(LogVehicles, "Failed to load vehicle actor class: {}", VehicleActorClassObjectName);
		return nullptr;
	}

	if (bDebugSpawnVehicles)
		LOG_INFO(LogDev, "Spawning Vehicle (VID): {}", StrongVehicleActorClass->GetPathName());

	auto NewVehicle = GetWorld()->SpawnActor<AActor>(StrongVehicleActorClass, SpawnTransform, CreateSpawnParameters(ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn));

	if (auto FortDagwoodVehicle = Cast<AFortDagwoodVehicle>(NewVehicle)) // carrr
	{
		FortDagwoodVehicle->SetFuel(100);
	}

	return NewVehicle;
}

static inline void SpawnVehicles2()
{
	static auto FortAthenaVehicleSpawnerClass = FindObject<UClass>(L"/Script/FortniteGame.FortAthenaVehicleSpawner");
	TArray<AActor*> AllVehicleSpawners = UGameplayStatics::GetAllActorsOfClass(GetWorld(), FortAthenaVehicleSpawnerClass);

	int AmountOfVehiclesSpawned = 0;

	for (int i = 0; i < AllVehicleSpawners.Num(); i++)
	{
		auto VehicleSpawner = AllVehicleSpawners.at(i);
		auto Vehicle = SpawnVehicleFromSpawner(VehicleSpawner);

		if (Vehicle)
		{
			AmountOfVehiclesSpawned++;
		}
	}

	auto AllVehicleSpawnersNum = AllVehicleSpawners.Num();

	AllVehicleSpawners.Free();

	LOG_INFO(LogGame, "Spawned {}/{} vehicles.", AmountOfVehiclesSpawned, AllVehicleSpawnersNum);
}