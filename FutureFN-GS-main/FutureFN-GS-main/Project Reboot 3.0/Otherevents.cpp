#include "events.h"

UObject* GetEventLoader()
{
	UObject* Loader = nullptr;

	if (Fortnite_Version == 14.60)
		Loader = FindObject("/Junior/Levels/JuniorLoaderLevel.JuniorLoaderLevel.PersistentLevel.BP_Junior_Loader_2");
	std::cout << "Loader: " << Loader << '\n';

	return Loader;
}
UObject* GetEventScripting()
{
	UObject* Scripting = nullptr;

	if (Fortnite_Version == 14.60)
		Scripting = FindObject("/Junior/Levels/Junior_Map.Junior_Map.PersistentLevel.BP_Junior_Scripting_Child_2");

	std::cout << "Scripting: " << Scripting << '\n';

	return Scripting;
}

void Events::LoadEvent()
{
	__int64 Condition = true;