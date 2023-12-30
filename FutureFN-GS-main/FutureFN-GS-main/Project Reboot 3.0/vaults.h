#pragma once
#include "reboot.h"
#include "World.h"
#include <Windows.h>
#include <dxgi.h>
#include <d3d11.h>
#include <d3d9.h>

#include <ImGui/imgui.h>
#include <ImGui/imgui_impl_dx9.h>

#include <string>
#include <imgui/imgui_impl_win32.h>
#include <imgui/imgui_stdlib.h>
#include <vector>
#include <format>
#include <imgui/imgui_internal.h>
#include <set>
#include <fstream>
#include <olectl.h>

#include "Object.h"
#include "objectviewer.h"
#include "FortAthenaMutator_Disco.h"
#include "globals.h"
#include "Fonts/ruda-bold.h"
#include "Vector.h"
#include "reboot.h"
#include "FortGameModeAthena.h"
#include "UnrealString.h"
#include "KismetTextLibrary.h"
#include "KismetSystemLibrary.h"
#include "GameplayStatics.h"
#include "Text.h"
#include <Images/reboot_icon.h>
#include "FortGadgetItemDefinition.h"
#include "FortWeaponItemDefinition.h"
#include "events.h"
#include "FortAthenaMutator_Heist.h"
#include "BGA.h"
#include "vendingmachine.h"
#include "die.h"

namespace FortAI
{
	void OpenVaults()
	{
		auto AgencyVault = FindObject("PersistentLevel.BGA_Athena_Keycard_Lock_TheAgency_");
		auto SharkVault = FindObject("PersistentLevel.BGA_Athena_Keycard_Lock_SharkIsland_");
		auto OilRigVault = FindObject("PersistentLevel.BGA_Athena_Keycard_Lock_OilRig_");
		auto UndergroundBaseVault = FindObject("PersistentLevel..BGA_Athena_Keycard_Lock_UndergroundBase_");
		auto YachtVault = FindObject("PersistentLevel.BGA_Athena_Keycard_Lock_Yacht");

		static auto OpenVaultFn = FindObject("Function /Game/Athena/Items/EnvironmentalItems/Locks/Keycard/Actors/Locks/BGA_Athena_Keycard_Lock_Parent.BGA_Athena_Keycard_Lock_Parent_C.CallOpenVault");
		
		/*
		if (AgencyVault)
		{
			AgencyVault->ProcessEvent(OpenVaultFn);
		}

		if (YachtVault)
			YachtVault->ProcessEvent(OpenVaultFn);

		if (SharkVault)
			SharkVault->ProcessEvent(OpenVaultFn);

		if (UndergroundBaseVault)
			UndergroundBaseVault->ProcessEvent(OpenVaultFn);

		if (OilRigVault)
			OilRigVault->ProcessEvent(OpenVaultFn);
		*/
	}
}