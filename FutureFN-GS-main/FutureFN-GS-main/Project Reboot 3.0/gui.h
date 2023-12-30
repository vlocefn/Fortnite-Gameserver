#pragma once

// TODO: Update ImGUI

#pragma comment(lib, "d3d9.lib")

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

#define GAME_TAB 1
#define PLAYERS_TAB 2
#define GAMEMODE_TAB 3
#define THANOS_TAB 4
#define EVENT_TAB 5
#define ZONE_TAB 6
#define DUMP_TAB 7
#define UNBAN_TAB 8
#define FUN_TAB 9
#define LATEGAME_TAB 10
#define DEVELOPER_TAB 11
#define DEBUGLOG_TAB 12
#define SETTINGS_TAB 13
#define CREDITS_TAB 14

#define MAIN_PLAYERTAB 1
#define INVENTORY_PLAYERTAB 2
#define LOADOUT_PLAYERTAB 4
#define FUN_PLAYERTAB 5

extern inline int StartReverseZonePhase = 7;
extern inline int EndReverseZonePhase = 5;
extern inline float StartingShield = 0;
extern inline bool bEnableReverseZone = false;
extern inline int AmountOfPlayersWhenBusStart = 0; 
extern inline bool bHandleDeath = true;
extern inline bool bUseCustomMap = false;
extern inline std::string CustomMapName = "";
extern inline int AmountToSubtractIndex = 1;
extern inline int SecondsUntilTravel = 5;
extern inline bool bSwitchedInitialLevel = false;
extern inline bool bIsInAutoRestart = false;
extern inline float AutoBusStartSeconds = 60;
extern inline int NumRequiredPlayersToStart = 2;
extern inline bool bDebugPrintLooting = false;
extern inline bool bDebugPrintFloorLoot = false;
extern inline bool bDebugPrintSwapping = false;
extern inline bool bEnableBotTick = false;
extern inline bool bZoneReversing = false;
extern inline bool bEnableCombinePickup = false;
extern inline int AmountOfBotsToSpawn = 0;
extern inline bool bEnableRebooting = false;
extern inline bool bEngineDebugLogs = false;
extern inline bool bStartedBus = false;
extern inline int AmountOfHealthSiphon = 0;

// THE BASE CODE IS FROM IMGUI GITHUB

static inline LPDIRECT3D9              g_pD3D = NULL;
static inline LPDIRECT3DDEVICE9        g_pd3dDevice = NULL;
static inline D3DPRESENT_PARAMETERS    g_d3dpp = {};

// Forward declarations of helper functions
static inline bool CreateDeviceD3D(HWND hWnd);
static inline void CleanupDeviceD3D();
static inline void ResetDevice();
static inline LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

static inline void SetIsLategame(bool Value)
{
	Globals::bLateGame.store(Value);
	StartingShield = Value ? 100 : 0;
}

static inline void Restart() // todo move?
{
	FString LevelA = Engine_Version < 424
		? L"open Athena_Terrain" : Engine_Version >= 500 ? Engine_Version >= 501
		? L"open Asteria_Terrain"
		: Globals::bCreative ? L"open Creative_NoApollo_Terrain"
		: L"open Artemis_Terrain"
		: Globals::bCreative ? L"open Creative_NoApollo_Terrain"
		: L"open Apollo_Terrain";

	static auto BeaconClass = FindObject<UClass>(L"/Script/FortniteGame.FortOnlineBeaconHost");
	auto AllFortBeacons = UGameplayStatics::GetAllActorsOfClass(GetWorld(), BeaconClass);

	for (int i = 0; i < AllFortBeacons.Num(); ++i)
	{
		AllFortBeacons.at(i)->K2_DestroyActor();
	}

	AllFortBeacons.Free();

	Globals::bInitializedPlaylist = false;
	Globals::bStartedListening = false;
	Globals::bHitReadyToStartMatch = false;
	bStartedBus = false;
	AmountOfRestarts++;

	LOG_INFO(LogDev, "Switching!");

	if (Fortnite_Version >= 15.10) // idk what ver
	{
		((AGameMode*)GetWorld()->GetGameMode())->RestartGame();
	}
	else
	{
		UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), LevelA, nullptr);
	}

	/*

	auto& LevelCollections = GetWorld()->Get<TArray<__int64>>("LevelCollections");
	int LevelCollectionSize = FindObject<UStruct>("/Script/Engine.LevelCollection")->GetPropertiesSize();

	*(UNetDriver**)(__int64(LevelCollections.AtPtr(0, LevelCollectionSize)) + 0x10) = nullptr;
	*(UNetDriver**)(__int64(LevelCollections.AtPtr(1, LevelCollectionSize)) + 0x10) = nullptr;

	*/

	// UGameplayStatics::OpenLevel(GetWorld(), UKismetStringLibrary::Conv_StringToName(LevelA), true, FString());
}

static inline std::string wstring_to_utf8(const std::wstring& str)
{
	if (str.empty()) return {};
	const auto size_needed = WideCharToMultiByte(CP_UTF8, 0, &str[0], static_cast<int>(str.size()), nullptr, 0, nullptr, nullptr);
	std::string str_to(size_needed, 0);
	WideCharToMultiByte(CP_UTF8, 0, &str[0], static_cast<int>(str.size()), &str_to[0], size_needed, nullptr, nullptr);
	return str_to;
}

static inline void InitFont()
{
	ImFontConfig FontConfig;
	FontConfig.FontDataOwnedByAtlas = false;
	ImGui::GetIO().Fonts->AddFontFromMemoryTTF((void*)ruda_bold_data, sizeof(ruda_bold_data), 17.f, &FontConfig);
}

static inline void InitStyle()
{
	auto& mStyle = ImGui::GetStyle();
	mStyle.FramePadding = ImVec2(4, 2);
	mStyle.ItemSpacing = ImVec2(6, 2);
	mStyle.ItemInnerSpacing = ImVec2(6, 4);
	mStyle.Alpha = 0.95f;
	mStyle.WindowRounding = 4.0f;
	mStyle.FrameRounding = 2.0f;
	mStyle.IndentSpacing = 6.0f;
	mStyle.ItemInnerSpacing = ImVec2(2, 4);
	mStyle.ColumnsMinSpacing = 50.0f;
	mStyle.GrabMinSize = 14.0f;
	mStyle.GrabRounding = 16.0f;
	mStyle.ScrollbarSize = 12.0f;
	mStyle.ScrollbarRounding = 16.0f;

	ImGuiStyle& style = mStyle;
	style.Colors[ImGuiCol_Text] = ImVec4(0.7490196228027344f, 0.7490196228027344f, 0.7490196228027344f, 1.0f);
	style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.3490196168422699f, 0.3490196168422699f, 0.3490196168422699f, 1.0f);
	style.Colors[ImGuiCol_WindowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.9399999976158142f);
	style.Colors[ImGuiCol_ChildBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
	style.Colors[ImGuiCol_PopupBg] = ImVec4(0.0784313753247261f, 0.0784313753247261f, 0.0784313753247261f, 0.9399999976158142f);
	style.Colors[ImGuiCol_FrameBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.5400000214576721f);
	style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.3686274588108063f, 0.1372549086809158f, 0.1372549086809158f, 0.6700000166893005f);
	style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.3882353007793427f, 0.2000000029802322f, 0.2000000029802322f, 0.6700000166893005f);
	style.Colors[ImGuiCol_TitleBg] = ImVec4(0.03921568766236305f, 0.03921568766236305f, 0.03921568766236305f, 1.0f);
	style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.47843137383461f, 0.1568627506494522f, 0.1568627506494522f, 1.0f);
	style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.47843137383461f, 0.1568627506494522f, 0.1568627506494522f, 1.0f);
	style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.1372549086809158f, 0.1372549086809158f, 0.1372549086809158f, 1.0f);
	style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.01960784383118153f, 0.01960784383118153f, 0.01960784383118153f, 0.5299999713897705f);
	style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.3098039329051971f, 0.3098039329051971f, 0.3098039329051971f, 1.0f);
	style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.407843142747879f, 0.407843142747879f, 0.407843142747879f, 1.0f);
	style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.5098039507865906f, 0.5098039507865906f, 0.5098039507865906f, 1.0f);
	style.Colors[ImGuiCol_CheckMark] = ImVec4(0.5568627715110779f, 0.09803921729326248f, 0.09803921729326248f, 1.0f);
	style.Colors[ImGuiCol_SliderGrab] = ImVec4(1.0f, 0.1882352977991104f, 0.1882352977991104f, 0.4000000059604645f);
	style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.886274516582489f, 0.0f, 0.1882352977991104f, 1.0f);
	style.Colors[ImGuiCol_Button] = ImVec4(1.0f, 0.1882352977991104f, 0.1882352977991104f, 0.4000000059604645f);
	style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.800000011920929f, 0.168627455830574f, 0.0f, 1.0f);
	style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.886274516582489f, 0.0f, 0.1882352977991104f, 1.0f);
	style.Colors[ImGuiCol_Header] = ImVec4(0.3294117748737335f, 0.3490196168422699f, 0.3568627536296844f, 0.5299999713897705f);
	style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.7568627595901489f, 0.2784313857555389f, 0.4392156898975372f, 0.6700000166893005f);
	style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.4666666686534882f, 0.4666666686534882f, 0.4666666686534882f, 0.6700000166893005f);
	style.Colors[ImGuiCol_Separator] = ImVec4(0.3176470696926117f, 0.3176470696926117f, 0.3176470696926117f, 1.0f);
	style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.3176470696926117f, 0.3176470696926117f, 0.3176470696926117f, 1.0f);
	style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.3176470696926117f, 0.3176470696926117f, 0.3176470696926117f, 1.0f);
	style.Colors[ImGuiCol_ResizeGrip] = ImVec4(1.0f, 1.0f, 1.0f, 0.8500000238418579f);
	style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(1.0f, 1.0f, 1.0f, 0.6000000238418579f);
	style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(1.0f, 1.0f, 1.0f, 0.8999999761581421f);
	style.Colors[ImGuiCol_Tab] = ImVec4(0.06666667014360428f, 0.06666667014360428f, 0.06666667014360428f, 0.5099999904632568f);
	style.Colors[ImGuiCol_TabHovered] = ImVec4(0.8588235378265381f, 0.2274509817361832f, 0.4274509847164154f, 0.6700000166893005f);
	style.Colors[ImGuiCol_TabActive] = ImVec4(0.1882352977991104f, 0.1882352977991104f, 0.1882352977991104f, 0.5699999928474426f);
	style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.0470588244497776f, 0.0470588244497776f, 0.0470588244497776f, 0.8999999761581421f);
	style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.1294117718935013f, 0.1294117718935013f, 0.1294117718935013f, 0.7400000095367432f);
	style.Colors[ImGuiCol_PlotLines] = ImVec4(0.6078431606292725f, 0.6078431606292725f, 0.6078431606292725f, 1.0f);
	style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.0f, 0.4274509847164154f, 0.3490196168422699f, 1.0f);
	style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.8980392217636108f, 0.6980392336845398f, 0.0f, 1.0f);
	style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.0f, 0.6000000238418579f, 0.0f, 1.0f);
	style.Colors[ImGuiCol_TableHeaderBg] = ImVec4(0.1882352977991104f, 0.1882352977991104f, 0.2000000029802322f, 1.0f);
	style.Colors[ImGuiCol_TableRowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
	style.Colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.0f, 1.0f, 1.0f, 0.07000000029802322f);
	style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.2588235437870026f, 0.5882353186607361f, 0.9764705896377563f, 0.3499999940395355f);
	style.Colors[ImGuiCol_DragDropTarget] = ImVec4(1.0f, 1.0f, 0.0f, 0.8999999761581421f);
	style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.2588235437870026f, 0.5882353186607361f, 0.9764705896377563f, 1.0f);
	style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.0f, 1.0f, 1.0f, 0.699999988079071f);
	style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.800000011920929f, 0.800000011920929f, 0.800000011920929f, 0.2000000029802322f);
	style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.800000011920929f, 0.800000011920929f, 0.800000011920929f, 0.3499999940395355f);
}

static inline void TextCentered(const std::string& text, bool bNewLine = true) {
	if (bNewLine)
		ImGui::NewLine();

	float win_width = ImGui::GetWindowSize().x;
	float text_width = ImGui::CalcTextSize(text.c_str()).x;

	// calculate the indentation that centers the text on one line, relative
	// to window left, regardless of the `ImGuiStyleVar_WindowPadding` value
	float text_indentation = (win_width - text_width) * 0.5f;

	// if text is too long to be drawn on one line, `text_indentation` can
	// become too small or even negative, so we check a minimum indentation
	float min_indentation = 20.0f;
	if (text_indentation <= min_indentation) {
		text_indentation = min_indentation;
	}

	ImGui::SameLine(text_indentation);
	ImGui::PushTextWrapPos(win_width - text_indentation);
	ImGui::TextWrapped(text.c_str());
	ImGui::PopTextWrapPos();
}

static inline bool ButtonCentered(const std::string& text, bool bNewLine = true) {
	if (bNewLine)
		ImGui::NewLine();

	float win_width = ImGui::GetWindowSize().x;
	float text_width = ImGui::CalcTextSize(text.c_str()).x;

	// calculate the indentation that centers the text on one line, relative
	// to window left, regardless of the `ImGuiStyleVar_WindowPadding` value
	float text_indentation = (win_width - text_width) * 0.5f;

	// if text is too long to be drawn on one line, `text_indentation` can
	// become too small or even negative, so we check a minimum indentation
	float min_indentation = 20.0f;
	if (text_indentation <= min_indentation) {
		text_indentation = min_indentation;
	}

	ImGui::SameLine(text_indentation);
	ImGui::PushTextWrapPos(win_width - text_indentation);
	auto res = ImGui::Button(text.c_str());
	ImGui::PopTextWrapPos();
	return res;
}

static inline void InputVector(const std::string& baseText, FVector* vec)
{
#ifdef ABOVE_S20
	ImGui::InputDouble((baseText + " X").c_str(), &vec->X);
	ImGui::InputDouble((baseText + " Y").c_str(), &vec->Y);
	ImGui::InputDouble((baseText + " Z").c_str(), &vec->Z);
#else
	ImGui::InputFloat((baseText + " X").c_str(), &vec->X);
	ImGui::InputFloat((baseText + " Y").c_str(), &vec->Y);
	ImGui::InputFloat((baseText + " Z").c_str(), &vec->Z);
#endif
}

static int Width = 640;
static int Height = 480;

static int Tab = 1;
static int PlayerTab = -1;
static bool bIsEditingInventory = false;
static bool bInformationTab = false;
static int playerTabTab = MAIN_PLAYERTAB;

static inline void StaticUI()
{
	if (IsRestartingSupported())
	{
		ImGui::Checkbox("Auto Restart", &Globals::bAutoRestart);

		if (Globals::bAutoRestart)
		{
			ImGui::InputFloat(std::format("How long after {} players join the bus will start", NumRequiredPlayersToStart).c_str(), &AutoBusStartSeconds);
			ImGui::InputInt("Num Players required for bus auto timer", &NumRequiredPlayersToStart);
		}
	}

	ImGui::InputInt("Shield/Health for siphon", &AmountOfHealthSiphon);

#ifndef PROD
	ImGui::Checkbox("Log ProcessEvent", &Globals::bLogProcessEvent);
	// ImGui::InputInt("Amount of bots to spawn", &AmountOfBotsToSpawn);
#endif

	ImGui::Checkbox("Infinite Ammo", &Globals::bInfiniteAmmo);
	ImGui::Checkbox("Infinite Materials", &Globals::bInfiniteMaterials);

	ImGui::Checkbox("No MCP (Don't change unless you know what this is)", &Globals::bNoMCP);

	if (Addresses::ApplyGadgetData && Addresses::RemoveGadgetData && Engine_Version < 503)
	{
		ImGui::Checkbox("Enable AGIDs (Don't change unless you know what this is)", &Globals::bEnableAGIDs);
	}
}

static inline void MainTabs()
{
	// std::ofstream bannedStream(Moderation::Banning::GetFilePath());

	if (ImGui::BeginTabBar(""))
	{
		if (ImGui::BeginTabItem("Game"))
		{
			Tab = GAME_TAB;
			PlayerTab = -1;
			bInformationTab = false;
			ImGui::EndTabItem();
		}

		// if (serverStatus == EServerStatus::Up)
		{
			/* if (ImGui::BeginTabItem("Players"))
			{
				Tab = PLAYERS_TAB;
				ImGui::EndTabItem();
			} */
		}

		if (false && ImGui::BeginTabItem("Gamemode"))
		{
			Tab = GAMEMODE_TAB;
			PlayerTab = -1;
			bInformationTab = false;
			ImGui::EndTabItem();
		}

		// if (Events::HasEvent())
		if (Globals::bGoingToPlayEvent)
		{
			if (ImGui::BeginTabItem(("Event")))
			{
				Tab = EVENT_TAB;
				PlayerTab = -1;
				bInformationTab = false;
				ImGui::EndTabItem();
			}
		}

		if (ImGui::BeginTabItem(("Zone")))
		{
			Tab = ZONE_TAB;
			PlayerTab = -1;
			bInformationTab = false;
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Dump"))
		{
			Tab = DUMP_TAB;
			PlayerTab = -1;
			bInformationTab = false;
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Fun"))
		{
			Tab = FUN_TAB;
			PlayerTab = -1;
			bInformationTab = false;
			ImGui::EndTabItem();
		}

		if (Globals::bLateGame.load() && ImGui::BeginTabItem("Lategame"))
		{
			Tab = LATEGAME_TAB;
			PlayerTab = -1;
			bInformationTab = false;
			ImGui::EndTabItem();
		}

#if 0
		if (bannedStream.is_open() && ImGui::BeginTabItem("Unban")) // skunked
		{
			Tab = UNBAN_TAB;
			PlayerTab = -1;
			bInformationTab = false;
			ImGui::EndTabItem();
		}
#endif

		/* if (ImGui::BeginTabItem(("Settings")))
		{
			Tab = SETTINGS_TAB;
			PlayerTab = -1;
			bInformationTab = false;
			ImGui::EndTabItem();
		} */

		// maybe a Replication Stats for >3.3?

		if (ImGui::BeginTabItem("Developer"))
		{
			Tab = DEVELOPER_TAB;
			PlayerTab = -1;
			bInformationTab = false;
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Debug Logs"))
		{
			Tab = DEBUGLOG_TAB;
			PlayerTab = -1;
			bInformationTab = false;
			ImGui::EndTabItem();
		}

		if (false && ImGui::BeginTabItem(("Credits")))
		{
			Tab = CREDITS_TAB;
			PlayerTab = -1;
			bInformationTab = false;
			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();
	}
}

static inline void PlayerTabs()
{
	if (ImGui::BeginTabBar(""))
	{
		if (ImGui::BeginTabItem("Main"))
		{
			playerTabTab = MAIN_PLAYERTAB;
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem(("Inventory")))
		{
			playerTabTab = INVENTORY_PLAYERTAB;
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem(("Cosmetics")))
		{
			playerTabTab = LOADOUT_PLAYERTAB;
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem(("Fun")))
		{
			playerTabTab = FUN_PLAYERTAB;
			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();
	}
}

static inline DWORD WINAPI LateGameThread(LPVOID)
{
	float MaxTickRate = 30;

	auto GameMode = Cast<AFortGameModeAthena>(GetWorld()->GetGameMode());
	auto GameState = Cast<AFortGameStateAthena>(GameMode->GetGameState());

	auto GetAircrafts = [&]() -> std::vector<AActor*>
	{
		static auto AircraftsOffset = GameState->GetOffset("Aircrafts", false);
		std::vector<AActor*> Aircrafts;

		if (AircraftsOffset == -1)
		{
			// GameState->Aircraft

			static auto FortAthenaAircraftClass = FindObject<UClass>(L"/Script/FortniteGame.FortAthenaAircraft");
			auto AllAircrafts = UGameplayStatics::GetAllActorsOfClass(GetWorld(), FortAthenaAircraftClass);

			for (int i = 0; i < AllAircrafts.Num(); i++)
			{
				Aircrafts.push_back(AllAircrafts.at(i));
			}

			AllAircrafts.Free();
		}
		else
		{
			const auto& GameStateAircrafts = GameState->Get<TArray<AActor*>>(AircraftsOffset);

			for (int i = 0; i < GameStateAircrafts.Num(); i++)
			{
				Aircrafts.push_back(GameStateAircrafts.at(i));
			}
		}

		return Aircrafts;
	};

	GameMode->StartAircraftPhase();

	while (GetAircrafts().size() <= 0)
	{
		Sleep(1000 / MaxTickRate);
	}

	static auto SafeZoneLocationsOffset = GameMode->GetOffset("SafeZoneLocations");
	const TArray<FVector>& SafeZoneLocations = GameMode->Get<TArray<FVector>>(SafeZoneLocationsOffset);

	if (SafeZoneLocations.Num() < 4)
	{
		LOG_WARN(LogLateGame, "Unable to find SafeZoneLocation! Disabling lategame..");
		SetIsLategame(false);
		return 0;
	}

	const FVector ZoneCenterLocation = SafeZoneLocations.at(3);

	FVector LocationToStartAircraft = ZoneCenterLocation;
	LocationToStartAircraft.Z += 10000;

	auto Aircrafts = GetAircrafts();

	float DropStartTime = GameState->GetServerWorldTimeSeconds() + 5.f;
	float FlightSpeed = 0.0f;

	for (int i = 0; i < Aircrafts.size(); ++i)
	{
		auto CurrentAircraft = Aircrafts.at(i);
		CurrentAircraft->TeleportTo(LocationToStartAircraft, FRotator());

		static auto FlightInfoOffset = CurrentAircraft->GetOffset("FlightInfo", false);

		if (FlightInfoOffset == -1)
		{
			static auto FlightStartLocationOffset = CurrentAircraft->GetOffset("FlightStartLocation");
			static auto FlightSpeedOffset = CurrentAircraft->GetOffset("FlightSpeed");
			static auto DropStartTimeOffset = CurrentAircraft->GetOffset("DropStartTime");

			CurrentAircraft->Get<FVector>(FlightStartLocationOffset) = LocationToStartAircraft;
			CurrentAircraft->Get<float>(FlightSpeedOffset) = FlightSpeed;
			CurrentAircraft->Get<float>(DropStartTimeOffset) = DropStartTime;
		}
		else
		{
			auto FlightInfo = CurrentAircraft->GetPtr<FAircraftFlightInfo>(FlightInfoOffset);

			FlightInfo->GetFlightSpeed() = FlightSpeed;
			FlightInfo->GetFlightStartLocation() = LocationToStartAircraft;
			FlightInfo->GetTimeTillDropStart() = DropStartTime;
		}
	}

	while (GameState->GetGamePhase() != EAthenaGamePhase::Aircraft)
	{
		Sleep(1000 / MaxTickRate);
	}

	/*
	static auto MapInfoOffset = GameState->GetOffset("MapInfo");
	auto MapInfo = GameState->Get(MapInfoOffset);

	if (MapInfo)
	{
		static auto FlightInfosOffset = MapInfo->GetOffset("FlightInfos", false);

		if (FlightInfosOffset != -1)
		{
			auto& FlightInfos = MapInfo->Get<TArray<FAircraftFlightInfo>>(FlightInfosOffset);

			for (int i = 0; i < FlightInfos.Num(); i++)
			{
				auto FlightInfo = FlightInfos.AtPtr(i, FAircraftFlightInfo::GetStructSize());

				FlightInfo->GetFlightSpeed() = FlightSpeed;
				FlightInfo->GetFlightStartLocation() = LocationToStartAircraft;
				FlightInfo->GetTimeTillDropStart() = DropStartTime;
			}
		}
	}
	*/

	while (GameState->GetGamePhase() == EAthenaGamePhase::Aircraft)
	{
		Sleep(1000 / MaxTickRate);
	}

	static auto World_NetDriverOffset = GetWorld()->GetOffset("NetDriver");
	auto WorldNetDriver = GetWorld()->Get<UNetDriver*>(World_NetDriverOffset);
	auto& ClientConnections = WorldNetDriver->GetClientConnections();

	for (int z = 0; z < ClientConnections.Num(); z++)
	{
		auto ClientConnection = ClientConnections.at(z);
		auto FortPC = Cast<AFortPlayerController>(ClientConnection->GetPlayerController());

		if (!FortPC)
			continue;

		auto WorldInventory = FortPC->GetWorldInventory();

		if (!WorldInventory)
			continue;

		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_int_distribution<> dist(0, 7);
		//Yes I know this is Skunked But I can't think Today.
		UFortItemDefinition* Ar = nullptr;
		UFortItemDefinition* Shotgun = nullptr;
		UFortItemDefinition* Sniper = nullptr;
		UFortItemDefinition* Shields = nullptr;
		UFortItemDefinition* Other = nullptr;
		UFortItemDefinition* TrapPlaced = nullptr;

		int randomIndex = dist(gen);
		switch (randomIndex)
		{
		case 0:
			Ar = FindObject<UFortItemDefinition>(L"/HighTower/Items/Tomato/Tomato_Rifle/WID_Assault_Stark_Athena_R_Ore_T03.WID_Assault_Stark_Athena_R_Ore_T03");
			break;
		case 1:
			Ar = FindObject<UFortItemDefinition>(L"/HighTower/Items/Tomato/Tomato_Rifle/WID_Assault_Stark_Athena_R_Ore_T03.WID_Assault_Stark_Athena_R_Ore_T03");
			break;
		case 2:
			Ar = FindObject<UFortItemDefinition>(L"/HighTower/Items/Tomato/Tomato_Rifle/WID_Assault_Stark_Athena_R_Ore_T03.WID_Assault_Stark_Athena_R_Ore_T03");
			break;
		case 3:
			Ar = FindObject<UFortItemDefinition>(L"/Game/Athena/Items/Weapons/WID_Assault_AutoHigh_Athena_VR_Ore_T03.WID_Assault_AutoHigh_Athena_VR_Ore_T03");
			break;
		case 4:
			Ar = FindObject<UFortItemDefinition>(L"/Game/Athena/Items/Weapons/WID_Assault_AutoHigh_Athena_SR_Ore_T03.WID_Assault_AutoHigh_Athena_SR_Ore_T03");
			break;
		case 5:
			Ar = FindObject<UFortItemDefinition>(L"/Game/Athena/Items/Weapons/WID_Assault_AutoHigh_Athena_VR_Ore_T03.WID_Assault_AutoHigh_Athena_VR_Ore_T03");
			break;
		case 6:
			Ar = FindObject<UFortItemDefinition>(L"/HighTower/Items/Tomato/Tomato_Rifle/WID_Assault_Stark_Athena_R_Ore_T03.WID_Assault_Stark_Athena_R_Ore_T03");
			break;
		case 7:
			Ar = FindObject<UFortItemDefinition>(L"/HighTower/Items/Tomato/Tomato_Rifle/WID_Assault_Stark_Athena_R_Ore_T03.WID_Assault_Stark_Athena_R_Ore_T03");
			break;
		default:
			break;
		}

		randomIndex = dist(gen); // Changed variable name to avoid redeclaration
		switch (randomIndex)
		{
		case 0:
			Shotgun = FindObject<UFortItemDefinition>(L"/Game/Athena/Items/Weapons/WID_Shotgun_Standard_Athena_VR_Ore_T03.WID_Shotgun_Standard_Athena_VR_Ore_T03");
			break;
		case 1:
			Shotgun = FindObject<UFortItemDefinition>(L"/Game/Athena/Items/Weapons/WID_Shotgun_Standard_Athena_UC_Ore_T03.WID_Shotgun_Standard_Athena_UC_Ore_T03");
			break;
		case 2:
			Shotgun = FindObject<UFortItemDefinition>(L"/Game/Athena/Items/Weapons/WID_Shotgun_Standard_Athena_SR_Ore_T03.WID_Shotgun_Standard_Athena_SR_Ore_T03");
			break;
		case 3:
			Shotgun = FindObject<UFortItemDefinition>(L"/Game/Athena/Items/Weapons/WID_Shotgun_Standard_Athena_Common.WID_Shotgun_Standard_Athena_Common");
			break;
		case 4:
			Shotgun = FindObject<UFortItemDefinition>(L"/Game/Athena/Items/Weapons/WID_Shotgun_Charge_Athena_UR_Ore_T03.WID_Shotgun_Charge_Athena_UR_Ore_T03");
			break;
		case 5:
			Shotgun = FindObject<UFortItemDefinition>(L"/Game/Athena/Items/Weapons/WID_Shotgun_Standard_Athena_C_Ore_T03.WID_Shotgun_Standard_Athena_C_Ore_T03");
			break;
		case 6:
			Shotgun = FindObject<UFortItemDefinition>(L"/Game/Athena/Items/Weapons/WID_Shotgun_Standard_Athena_UC_Ore_T03.WID_Shotgun_Standard_Athena_UC_Ore_T03");
			break;
		case 7:
			Shotgun = FindObject<UFortItemDefinition>(L"/Game/Athena/Items/Weapons/WID_Shotgun_Standard_Athena_C_Ore_T03.WID_Shotgun_Standard_Athena_C_Ore_T03");
			break;
		default:
			break;
		}

		randomIndex = dist(gen);
		switch (randomIndex)
		{
		case 0:
			Sniper = FindObject<UFortItemDefinition>(L"/Game/Athena/Items/Weapons/WID_Sniper_NoScope_Athena_R_Ore_T03.WID_Sniper_NoScope_Athena_R_Ore_T03");
			break;
		case 1:
			Sniper = FindObject<UFortItemDefinition>(L"/Game/Athena/Items/Weapons/WID_Sniper_Heavy_Athena_VR_Ore_T03.WID_Sniper_Heavy_Athena_VR_Ore_T03");
			break;
		case 2:
			Sniper = FindObject<UFortItemDefinition>(L"/FlipperGameplay/Items/Weapons/CoreSMG/WID_SMG_CoreSMG_Athena_VR.WID_SMG_CoreSMG_Athena_VR");
			break;
		case 3:
			Sniper = FindObject<UFortItemDefinition>(L"/Game/Athena/Items/Weapons/WID_Sniper_BoltAction_Scope_Athena_VR_Ore_T03.WID_Sniper_BoltAction_Scope_Athena_VR_Ore_T03");
			break;
		case 4:
			Sniper = FindObject<UFortItemDefinition>(L"/FlipperGameplay/Items/Weapons/CoreSniper/WID_Sniper_CoreSniper_Athena_R.WID_Sniper_CoreSniper_Athena_R");
			break;
		case 5:
			Sniper = FindObject<UFortItemDefinition>(L"/FlipperGameplay/Items/Weapons/CoreSMG/WID_SMG_CoreSMG_Athena_UC.WID_SMG_CoreSMG_Athena_UC");
			break;
		case 6:
			Sniper = FindObject<UFortItemDefinition>(L"/FlipperGameplay/Items/Weapons/CoreSMG/WID_SMG_CoreSMG_Athena_UR_IOBrute.WID_SMG_CoreSMG_Athena_UR_IOBrute");
			break;
		case 7:
			Sniper = FindObject<UFortItemDefinition>(L"/Game/Athena/Items/Weapons/WID_Sniper_Standard_Scope_Athena_SR_Ore_T03.WID_Sniper_Standard_Scope_Athena_SR_Ore_T03");
			break;
		default:
			break;
		}

		randomIndex = dist(gen);
		switch (randomIndex)
		{
		case 0:
			Shields = FindObject<UFortItemDefinition>(L"/Game/Athena/Items/Consumables/ShieldSmall/Athena_ShieldSmall.Athena_ShieldSmall");
			break;
		case 1:
			Shields = FindObject<UFortItemDefinition>(L"/Game/Athena/Items/Consumables/ShockwaveGrenade/Athena_ShockGrenade.Athena_ShockGrenade");
			break;
		case 2:
			Shields = FindObject<UFortItemDefinition>(L"/Game/Athena/Items/Consumables/StickyGrenade/Athena_StickyGrenade.Athena_StickyGrenade");
			break;
		case 3:
			Shields = FindObject<UFortItemDefinition>(L"/Game/Athena/Items/Consumables/GasGrenade/Athena_GasGrenade.Athena_GasGrenade");
			break;
		case 4:
			Shields = FindObject<UFortItemDefinition>(L"/Game/Athena/Items/Consumables/SuperTowerGrenade/Levels/GiftBox/HolidayGiftBox/Athena_HolidayGiftBox.Athena_HolidayGiftBox");
			break;
		case 5:
			Shields = FindObject<UFortItemDefinition>(L"/Game/Athena/Items/Consumables/ShieldSmall/Athena_ShieldSmall.Athena_ShieldSmall");
			break;
		case 6:
			Shields = FindObject<UFortItemDefinition>(L"/Game/Athena/Items/Consumables/Grenade/Athena_Grenade.Athena_Grenade");
			break;
		case 7:
			Shields = FindObject<UFortItemDefinition>(L"/Game/Athena/Items/Consumables/ShieldSmall/Athena_ShieldSmall.Athena_ShieldSmall");
			break;
		default:
			break;
		}

		randomIndex = dist(gen);
		switch (randomIndex)
		{
		case 0:
			Other = FindObject<UFortItemDefinition>(L"/Game/Athena/Items/Weapons/WID_Hook_Gun_VR_Ore_T03.WID_Hook_Gun_VR_Ore_T03");
			break;
		case 1:
			Other = FindObject<UFortItemDefinition>(L"/Game/Athena/Items/Weapons/WID_Launcher_Shockwave_Athena_UR_Ore_T03.WID_Launcher_Shockwave_Athena_UR_Ore_T03");
			break;
		case 2:
			Other = FindObject<UFortItemDefinition>(L"/Game/Athena/Items/Weapons/WID_Launcher_Shockwave_Athena_UR_Ore_T03.WID_Launcher_Shockwave_Athena_UR_Ore_T03");
			break;
		case 3:
			Other = FindObject<UFortItemDefinition>(L"/Game/Athena/Items/Consumables/ChillBronco/Athena_ChillBronco_NPC.Athena_ChillBronco_NPC");
			break;
		case 4:
			Other = FindObject<UFortItemDefinition>(L"/Game/Athena/Items/Weapons/WID_Launcher_Shockwave_Athena_UR_Ore_T03.WID_Launcher_Shockwave_Athena_UR_Ore_T03");
			break;
		case 5:
			Other = FindObject<UFortItemDefinition>(L"/Game/Athena/Items/Weapons/WID_Launcher_Shockwave_Athena_UR_Ore_T03.WID_Launcher_Shockwave_Athena_UR_Ore_T03");
			break;
		case 6:
			Other = FindObject<UFortItemDefinition>(L"/Game/Athena/Items/Weapons/WID_Launcher_Shockwave_Athena_UR_Ore_T03.WID_Launcher_Shockwave_Athena_UR_Ore_T03");
			break;
		case 7:
			Other = FindObject<UFortItemDefinition>(L"/Game/Athena/Items/Weapons/WID_Hook_Gun_VR_Ore_T03.WID_Hook_Gun_VR_Ore_T03");
			break;
		default:
			break;
		}

		randomIndex = dist(gen);
		switch (randomIndex)
		{
		case 0:
			TrapPlaced = FindObject<UFortItemDefinition>(L"");
			break;
		case 1:
			TrapPlaced = FindObject<UFortItemDefinition>(L"");
			break;
		case 2:
			TrapPlaced = FindObject<UFortItemDefinition>(L"");
			break;
		case 3:
			TrapPlaced = FindObject<UFortItemDefinition>(L"");
			break;
		case 4:
			TrapPlaced = FindObject<UFortItemDefinition>(L"");
			break;
		case 5:
			TrapPlaced = FindObject<UFortItemDefinition>(L"");
			break;
		case 6:
			TrapPlaced = FindObject<UFortItemDefinition>(L"");
			break;
		case 7:
			TrapPlaced = FindObject<UFortItemDefinition>(L"");
			break;
		default:
			break;
		}

		static auto WoodItemData = FindObject<UFortItemDefinition>(L"/Game/Items/ResourcePickups/WoodItemData.WoodItemData");
		static auto StoneItemData = FindObject<UFortItemDefinition>(L"/Game/Items/ResourcePickups/StoneItemData.StoneItemData");
		static auto MetalItemData = FindObject<UFortItemDefinition>(L"/Game/Items/ResourcePickups/MetalItemData.MetalItemData");
		static auto Shells = FindObject<UFortItemDefinition>(L"/Game/Athena/Items/Ammo/AthenaAmmoDataShells.AthenaAmmoDataShells");
		static auto Medium = FindObject<UFortItemDefinition>(L"/Game/Athena/Items/Ammo/AthenaAmmoDataBulletsMedium.AthenaAmmoDataBulletsMedium");
		static auto Light = FindObject<UFortItemDefinition>(L"/Game/Athena/Items/Ammo/AthenaAmmoDataBulletsLight.AthenaAmmoDataBulletsLight");
		static auto Heavy = FindObject<UFortItemDefinition>(L"/Game/Athena/Items/Ammo/AthenaAmmoDataBulletsHeavy.AthenaAmmoDataBulletsHeavy");

		WorldInventory->AddItem(WoodItemData, nullptr, 500);
		WorldInventory->AddItem(StoneItemData, nullptr, 500);
		WorldInventory->AddItem(MetalItemData, nullptr, 500);
		WorldInventory->AddItem(Shotgun, nullptr, 1);
		WorldInventory->AddItem(Sniper, nullptr, 1);
		WorldInventory->AddItem(Ar, nullptr, 1);
		WorldInventory->AddItem(Other, nullptr, 1);
		WorldInventory->AddItem(Shields, nullptr, 6);
		WorldInventory->AddItem(TrapPlaced, nullptr, 3);
		WorldInventory->AddItem(Shells, nullptr, 999);
		WorldInventory->AddItem(Medium, nullptr, 999);
		WorldInventory->AddItem(Light, nullptr, 999);
		WorldInventory->AddItem(Heavy, nullptr, 999);

		WorldInventory->Update();
	}

	static auto SafeZonesStartTimeOffset = GameState->GetOffset("SafeZonesStartTime");
	GameState->Get<float>(SafeZonesStartTimeOffset) = 0.001f;

	return 0;
}

static inline void MainUI()
{
	bool bLoaded = true;

	if (PlayerTab == -1)
	{
		MainTabs();

		if (Tab == GAME_TAB)
		{
			if (bLoaded)
			{
				StaticUI();

				if (!bStartedBus)
				{
					bool bWillBeLategame = Globals::bLateGame.load();
					ImGui::Checkbox("Lategame", &bWillBeLategame);
					SetIsLategame(bWillBeLategame);
				}

				ImGui::Text(std::format("Joinable {}", Globals::bStartedListening).c_str());

				static std::string ConsoleCommand;

				ImGui::InputText("Console command", &ConsoleCommand);

				if (ImGui::Button("Execute console command"))
				{
					auto wstr = std::wstring(ConsoleCommand.begin(), ConsoleCommand.end());

					auto aa = wstr.c_str();
					FString cmd = aa;

					UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), cmd, nullptr);
				}

				 if (ImGui::Button("Spawn BGAs"))
				{
					SpawnBGAs();
				} 

				
				if (ImGui::Button("New"))
				{
					static auto NextFn = FindObject<UFunction>("/Game/Athena/Prototype/Blueprints/Cube/CUBE.CUBE_C.Next");
					static auto NewFn = FindObject<UFunction>("/Game/Athena/Prototype/Blueprints/Cube/CUBE.CUBE_C.New");					
					auto Loader = GetEventLoader("/Game/Athena/Prototype/Blueprints/Cube/CUBE.CUBE_C");

					LOG_INFO(LogDev, "Loader: {}", __int64(Loader));

					if (Loader)
					{
						int32 NewParam = 1;
						// Loader->ProcessEvent(NextFn, &NewParam);
						Loader->ProcessEvent(NewFn, &NewParam);
					}
				}

				if (ImGui::Button("Next"))
				{
					static auto NextFn = FindObject<UFunction>("/Game/Athena/Prototype/Blueprints/Cube/CUBE.CUBE_C.Next");
					static auto NewFn = FindObject<UFunction>("/Game/Athena/Prototype/Blueprints/Cube/CUBE.CUBE_C.New");
					auto Loader = GetEventLoader("/Game/Athena/Prototype/Blueprints/Cube/CUBE.CUBE_C");

					LOG_INFO(LogDev, "Loader: {}", __int64(Loader));

					if (Loader)
					{
						int32 NewParam = 1;
						Loader->ProcessEvent(NextFn, &NewParam);
						// Loader->ProcessEvent(NewFn, &NewParam);
					}
				}
				

				if (!bIsInAutoRestart && Engine_Version < 500 && ImGui::Button("Restart"))
				{
					if (Engine_Version < 503)
					{
						Restart();
						LOG_INFO(LogGame, "Restarting!");
					}
					else
					{
						LOG_ERROR(LogGame, "Restarting is not supported on chapter 2 and above!");
					}
				}

				/*
				if (ImGui::Button("Test bruh"))
				{
					__int64 bruh;
					__int64* (*sub_7FF7476F4458)(__int64* a1, UWorld* a2, __int64 a3) = decltype(sub_7FF7476F4458)(Addresses::GetSessionInterface);

					sub_7FF7476F4458(&bruh, GetWorld(), 0);

					LOG_INFO(LogDev, "bruh: 0x{:x}", bruh);
					auto VFT = *(__int64*)bruh;
					LOG_INFO(LogDev, "VFT: 0x{:x}", VFT - __int64(GetModuleHandleW(0)));
				}
				*/

				if (!bStartedBus)
				{
					if (Globals::bLateGame.load() || Fortnite_Version >= 11)
					{
						if (ImGui::Button("Start Bus"))
						{
							bStartedBus = true;

							auto GameMode = (AFortGameModeAthena*)GetWorld()->GetGameMode();
							auto GameState = Cast<AFortGameStateAthena>(GameMode->GetGameState());
							/*
							if (Fortnite_Version == 14.60)
							{
								static auto OverrideBattleBusSkin = FindObject(L"/Game/Athena/Items/Cosmetics/BattleBuses/BBID_WinterBus.BBID_WinterBus");
								LOG_INFO(LogDev, "OverrideBattleBusSkin: {}", __int64(OverrideBattleBusSkin));

								if (OverrideBattleBusSkin)
								{
									static auto AssetManagerOffset = GetEngine()->GetOffset("AssetManager");
									auto AssetManager = GetEngine()->Get(AssetManagerOffset);

									if (AssetManager)
									{
										static auto AthenaGameDataOffset = AssetManager->GetOffset("AthenaGameData");
										auto AthenaGameData = AssetManager->Get(AthenaGameDataOffset);

										if (AthenaGameData)
										{
											static auto DefaultBattleBusSkinOffset = AthenaGameData->GetOffset("DefaultBattleBusSkin");
											AthenaGameData->Get(DefaultBattleBusSkinOffset) = OverrideBattleBusSkin;
										}
									}

									static auto DefaultBattleBusOffset = GameState->GetOffset("DefaultBattleBus");
									GameState->Get(DefaultBattleBusOffset) = OverrideBattleBusSkin;

									static auto FortAthenaAircraftClass = FindObject<UClass>("/Script/FortniteGame.FortAthenaAircraft");
									auto AllAircrafts = UGameplayStatics::GetAllActorsOfClass(GetWorld(), FortAthenaAircraftClass);

									for (int i = 0; i < AllAircrafts.Num(); i++)
									{
										auto Aircraft = AllAircrafts.at(i);

										static auto DefaultBusSkinOffset = Aircraft->GetOffset("DefaultBusSkin");
										Aircraft->Get(DefaultBusSkinOffset) = OverrideBattleBusSkin;

										static auto SpawnedCosmeticActorOffset = Aircraft->GetOffset("SpawnedCosmeticActor");
										auto SpawnedCosmeticActor = Aircraft->Get<AActor*>(SpawnedCosmeticActorOffset);

										if (SpawnedCosmeticActor)
										{
											static auto ActiveSkinOffset = SpawnedCosmeticActor->GetOffset("ActiveSkin");
											SpawnedCosmeticActor->Get(ActiveSkinOffset) = OverrideBattleBusSkin;
										}
									}
								}
							}
							*/

							AmountOfPlayersWhenBusStart = GameState->GetPlayersLeft();

							if (Globals::bLateGame.load())
							{
								CreateThread(0, 0, LateGameThread, 0, 0, 0);
							}
							else
							{
								GameMode->StartAircraftPhase();
							}
						}
					}
					else
					{
						if (ImGui::Button("Start Bus Countdown"))
						{
							bStartedBus = true;

							auto GameMode = (AFortGameMode*)GetWorld()->GetGameMode();
							auto GameState = Cast<AFortGameStateAthena>(GameMode->GetGameState());

							AmountOfPlayersWhenBusStart = GameState->GetPlayersLeft(); // scuffed!!!!

							if (Fortnite_Version == 1.11)
							{
								static auto OverrideBattleBusSkin = FindObject(L"/Game/Athena/Items/Cosmetics/BattleBuses/BBID_WinterBus.BBID_WinterBus");
								LOG_INFO(LogDev, "OverrideBattleBusSkin: {}", __int64(OverrideBattleBusSkin));

								if (OverrideBattleBusSkin)
								{
									static auto AssetManagerOffset = GetEngine()->GetOffset("AssetManager");
									auto AssetManager = GetEngine()->Get(AssetManagerOffset);

									if (AssetManager)
									{
										static auto AthenaGameDataOffset = AssetManager->GetOffset("AthenaGameData");
										auto AthenaGameData = AssetManager->Get(AthenaGameDataOffset);

										if (AthenaGameData)
										{
											static auto DefaultBattleBusSkinOffset = AthenaGameData->GetOffset("DefaultBattleBusSkin");
											AthenaGameData->Get(DefaultBattleBusSkinOffset) = OverrideBattleBusSkin;
										}
									}

									static auto DefaultBattleBusOffset = GameState->GetOffset("DefaultBattleBus");
									GameState->Get(DefaultBattleBusOffset) = OverrideBattleBusSkin;

									static auto FortAthenaAircraftClass = FindObject<UClass>("/Script/FortniteGame.FortAthenaAircraft");
									auto AllAircrafts = UGameplayStatics::GetAllActorsOfClass(GetWorld(), FortAthenaAircraftClass);

									for (int i = 0; i < AllAircrafts.Num(); i++)
									{
										auto Aircraft = AllAircrafts.at(i);

										static auto DefaultBusSkinOffset = Aircraft->GetOffset("DefaultBusSkin");
										Aircraft->Get(DefaultBusSkinOffset) = OverrideBattleBusSkin;

										static auto SpawnedCosmeticActorOffset = Aircraft->GetOffset("SpawnedCosmeticActor");
										auto SpawnedCosmeticActor = Aircraft->Get<AActor*>(SpawnedCosmeticActorOffset);

										if (SpawnedCosmeticActor)
										{
											static auto ActiveSkinOffset = SpawnedCosmeticActor->GetOffset("ActiveSkin");
											SpawnedCosmeticActor->Get(ActiveSkinOffset) = OverrideBattleBusSkin;
										}
									}
								}
							}

							static auto WarmupCountdownEndTimeOffset = GameState->GetOffset("WarmupCountdownEndTime");
							// GameState->Get<float>(WarmupCountdownEndTimeOffset) = UGameplayStatics::GetTimeSeconds(GetWorld()) + 10;

							float TimeSeconds = GameState->GetServerWorldTimeSeconds(); // UGameplayStatics::GetTimeSeconds(GetWorld());
							float Duration = 10;
							float EarlyDuration = Duration;

							static auto WarmupCountdownStartTimeOffset = GameState->GetOffset("WarmupCountdownStartTime");
							static auto WarmupCountdownDurationOffset = GameMode->GetOffset("WarmupCountdownDuration");
							static auto WarmupEarlyCountdownDurationOffset = GameMode->GetOffset("WarmupEarlyCountdownDuration");

							GameState->Get<float>(WarmupCountdownEndTimeOffset) = TimeSeconds + Duration;
							GameMode->Get<float>(WarmupCountdownDurationOffset) = Duration;

							// GameState->Get<float>(WarmupCountdownStartTimeOffset) = TimeSeconds;
							GameMode->Get<float>(WarmupEarlyCountdownDurationOffset) = EarlyDuration;
						}
					}
				}
			}
		}

		else if (Tab == PLAYERS_TAB)
		{
			
		}

		else if (Tab == EVENT_TAB)
		{
			if (ImGui::Button(std::format("Start {}", GetEventName()).c_str()))
			{
				StartEvent();
			}

			if (Fortnite_Version == 8.51)
			{
				if (ImGui::Button("Unvault DrumGun"))
				{
					static auto SetUnvaultItemNameFn = FindObject<UFunction>(L"/Game/Athena/Prototype/Blueprints/White/BP_SnowScripting.BP_SnowScripting_C.SetUnvaultItemName");
					auto EventScripting = GetEventScripting();

					if (EventScripting)
					{
						FName Name = UKismetStringLibrary::Conv_StringToName(L"DrumGun");
						EventScripting->ProcessEvent(SetUnvaultItemNameFn, &Name);

						static auto PillarsConcludedFn = FindObject<UFunction>(L"/Game/Athena/Prototype/Blueprints/White/BP_SnowScripting.BP_SnowScripting_C.PillarsConcluded");
						EventScripting->ProcessEvent(PillarsConcludedFn, &Name);
					}
				}
			}
		}

		else if (Tab == ZONE_TAB)
		{
			if (ImGui::Button("Start Safe Zone"))
			{
				UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), L"startsafezone", nullptr);
			}

			if (ImGui::Button("Pause Safe Zone"))
			{
				UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), L"pausesafezone", nullptr);
			}

			if (ImGui::Button("Skip Zone"))
			{
				UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), L"skipsafezone", nullptr);
			}

			if (ImGui::Button("Start Shrink Safe Zone"))
			{
				UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), L"startshrinksafezone", nullptr);
			}

			if (ImGui::Button("Skip Shrink Safe Zone"))
			{
				auto GameMode = Cast<AFortGameModeAthena>(GetWorld()->GetGameMode());
				auto SafeZoneIndicator = GameMode->GetSafeZoneIndicator();

				if (SafeZoneIndicator)
				{
					SafeZoneIndicator->SkipShrinkSafeZone();
				}
			}
		}

		else if (Tab == DUMP_TAB)
		{
			ImGui::Text("These will all be in your Win64 folder!");

			static std::string FortniteVersionStr = std::format("Fortnite Version {}\n\n", std::to_string(Fortnite_Version));

			if (ImGui::Button("Dump Objects"))
			{
				auto ObjectNum = ChunkedObjects ? ChunkedObjects->Num() : UnchunkedObjects ? UnchunkedObjects->Num() : 0;

				std::ofstream obj("ObjectsDump.txt");

				obj << FortniteVersionStr;

				for (int i = 0; i < ObjectNum; i++)
				{
					auto CurrentObject = GetObjectByIndex(i);

					if (!CurrentObject)
						continue;

					obj << CurrentObject->GetFullName() << '\n';
				}
			}

			if (ImGui::Button("Dump Skins (Skins.txt)"))
			{
				std::ofstream SkinsFile("Skins.txt");

				if (SkinsFile.is_open())
				{
					SkinsFile << FortniteVersionStr;

					static auto CIDClass = FindObject<UClass>("/Script/FortniteGame.AthenaCharacterItemDefinition");

					auto AllObjects = GetAllObjectsOfClass(CIDClass);

					for (int i = 0; i < AllObjects.size(); i++)
					{
						auto CurrentCID = AllObjects.at(i);

						static auto DisplayNameOffset = CurrentCID->GetOffset("DisplayName");

						FString DisplayNameFStr = UKismetTextLibrary::Conv_TextToString(CurrentCID->Get<FText>(DisplayNameOffset));

						if (!DisplayNameFStr.Data.Data)
							continue;

						SkinsFile << std::format("[{}] {}\n", DisplayNameFStr.ToString(), CurrentCID->GetPathName());
					}
				}
			}

			if (ImGui::Button("Dump Playlists (Playlists.txt)"))
			{
				std::ofstream PlaylistsFile("Playlists.txt");

				if (PlaylistsFile.is_open())
				{
					PlaylistsFile << FortniteVersionStr;
					static auto FortPlaylistClass = FindObject<UClass>("/Script/FortniteGame.FortPlaylist");
					// static auto FortPlaylistClass = FindObject("Class /Script/FortniteGame.FortPlaylistAthena");

					auto AllObjects = GetAllObjectsOfClass(FortPlaylistClass);

					for (int i = 0; i < AllObjects.size(); i++)
					{
						auto Object = AllObjects.at(i);

						static auto UIDisplayNameOffset = Object->GetOffset("UIDisplayName");
						FString PlaylistNameFStr = UKismetTextLibrary::Conv_TextToString(Object->Get<FText>(UIDisplayNameOffset));

						if (!PlaylistNameFStr.Data.Data)
							continue;

						std::string PlaylistName = PlaylistNameFStr.ToString();

						PlaylistsFile << std::format("[{}] {}\n", PlaylistName, Object->GetPathName());
					}
				}
				else
					std::cout << "Failed to open playlist file!\n";
			}

			if (ImGui::Button("Dump Weapons (Weapons.txt)"))
			{
				std::ofstream WeaponsFile("Weapons.txt");

				if (WeaponsFile.is_open())
				{
					WeaponsFile << FortniteVersionStr;

					auto DumpItemDefinitionClass = [&WeaponsFile](UClass* Class) {
						auto AllObjects = GetAllObjectsOfClass(Class);

						for (int i = 0; i < AllObjects.size(); i++)
						{
							auto Object = AllObjects.at(i);

							static auto DisplayNameOffset = Object->GetOffset("DisplayName");
							FString ItemDefinitionFStr = UKismetTextLibrary::Conv_TextToString(Object->Get<FText>(DisplayNameOffset));

							if (!ItemDefinitionFStr.Data.Data)
								continue;

							std::string ItemDefinitionName = ItemDefinitionFStr.ToString();

							// check if it contains gallery or playset and just ignore?

							WeaponsFile << std::format("[{}] {}\n", ItemDefinitionName, Object->GetPathName());
						}
					};

					DumpItemDefinitionClass(UFortWeaponItemDefinition::StaticClass());
					DumpItemDefinitionClass(UFortGadgetItemDefinition::StaticClass());
					DumpItemDefinitionClass(FindObject<UClass>("/Script/FortniteGame.FortAmmoItemDefinition"));
				}
				else
					std::cout << "Failed to open playlist file!\n";
			}
		}
		else if (Tab == UNBAN_TAB)
		{

		}
		else if (Tab == FUN_TAB)
		{
			static std::string ItemToGrantEveryone;
			static int AmountToGrantEveryone = 1;

			ImGui::InputFloat("Starting Shield", &StartingShield);
			ImGui::InputText("Item to Give", &ItemToGrantEveryone);
			ImGui::InputInt("Amount to Give", &AmountToGrantEveryone);

			if (ImGui::Button("Destroy all player builds"))
			{
				auto AllBuildingSMActors = UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABuildingSMActor::StaticClass());

				for (int i = 0; i < AllBuildingSMActors.Num(); i++)
				{
					auto CurrentBuildingSMActor = (ABuildingSMActor*)AllBuildingSMActors.at(i);

					if (CurrentBuildingSMActor->IsDestroyed() || CurrentBuildingSMActor->IsActorBeingDestroyed() || !CurrentBuildingSMActor->IsPlayerPlaced()) continue;

					CurrentBuildingSMActor->SilentDie();
					// CurrentBuildingSMActor->K2_DestroyActor();
				}

				AllBuildingSMActors.Free();
			}

			if (ImGui::Button("Give Item to Everyone"))
			{
				auto ItemDefinition = FindObject<UFortItemDefinition>(ItemToGrantEveryone, nullptr, ANY_PACKAGE);
				
				if (ItemDefinition)
				{
					static auto World_NetDriverOffset = GetWorld()->GetOffset("NetDriver");
					auto WorldNetDriver = GetWorld()->Get<UNetDriver*>(World_NetDriverOffset);
					auto& ClientConnections = WorldNetDriver->GetClientConnections();

					for (int i = 0; i < ClientConnections.Num(); i++)
					{
						auto PlayerController = Cast<AFortPlayerController>(ClientConnections.at(i)->GetPlayerController());

						if (!PlayerController->IsValidLowLevel())
							continue;

						auto WorldInventory = PlayerController->GetWorldInventory();

						if (!WorldInventory->IsValidLowLevel())
							continue;

						bool bShouldUpdate = false;
						WorldInventory->AddItem(ItemDefinition, &bShouldUpdate, AmountToGrantEveryone);

						if (bShouldUpdate)
							WorldInventory->Update();
					}
				}
				else
				{
					ItemToGrantEveryone = "";
					LOG_WARN(LogUI, "Invalid Item Definition!");
				}
			}

			auto GameState = Cast<AFortGameStateAthena>(GetWorld()->GetGameState());

			if (GameState)
			{
				static auto DefaultGliderRedeployCanRedeployOffset = FindOffsetStruct("/Script/FortniteGame.FortGameStateAthena", "DefaultGliderRedeployCanRedeploy", false);
				static auto DefaultParachuteDeployTraceForGroundDistanceOffset = GameState->GetOffset("DefaultParachuteDeployTraceForGroundDistance", false);

				if (Globals::bStartedListening) // it resets accordingly to ProHenis b4 this
				{
					if (DefaultParachuteDeployTraceForGroundDistanceOffset != -1)
					{
						ImGui::InputFloat("Automatic Parachute Pullout Distance", GameState->GetPtr<float>(DefaultParachuteDeployTraceForGroundDistanceOffset));
					}
				}

				if (DefaultGliderRedeployCanRedeployOffset != -1)
				{
					bool EnableGliderRedeploy = (bool)GameState->Get<float>(DefaultGliderRedeployCanRedeployOffset);

					if (ImGui::Checkbox("Enable Glider Redeploy", &EnableGliderRedeploy))
					{
						GameState->Get<float>(DefaultGliderRedeployCanRedeployOffset) = EnableGliderRedeploy;
					}
				}

				GET_PLAYLIST(GameState);

				if (CurrentPlaylist)
				{
					bool bRespawning = CurrentPlaylist->GetRespawnType() == EAthenaRespawnType::InfiniteRespawn || CurrentPlaylist->GetRespawnType() == EAthenaRespawnType::InfiniteRespawnExceptStorm;

					if (ImGui::Checkbox("Respawning", &bRespawning))
					{
						CurrentPlaylist->GetRespawnType() = (EAthenaRespawnType)bRespawning;
					}
				}
			}
		}
		else if (Tab == LATEGAME_TAB)
		{
			if (bEnableReverseZone)
				ImGui::Text(std::format("Currently {}eversing zone", bZoneReversing ? "r" : "not r").c_str());

			ImGui::Checkbox("Enable Reverse Zone (EXPERIMENTAL)", &bEnableReverseZone);

			if (bEnableReverseZone)
			{
				ImGui::InputInt("Start Reversing Phase", &StartReverseZonePhase);
				ImGui::InputInt("End Reversing Phase", &EndReverseZonePhase);
			}
		}
		else if (Tab == DEVELOPER_TAB)
		{
			static std::string ClassNameToDump;
			static std::string FunctionNameToDump;
			static std::string ObjectToDump;
			static std::string FileNameToSaveTo;
			static bool bExcludeUnhandled = true;

			ImGui::Checkbox("Handle Death", &bHandleDeath);
			ImGui::Checkbox("Fill Vending Machines", &Globals::bFillVendingMachines);
			ImGui::Checkbox("Enable Bot Tick", &bEnableBotTick);
			ImGui::Checkbox("Enable Rebooting", &bEnableRebooting);
			ImGui::Checkbox("Enable Combine Pickup", &bEnableCombinePickup);
			ImGui::Checkbox("Exclude unhandled", &bExcludeUnhandled);
			ImGui::InputInt("Amount To Subtract Index", &AmountToSubtractIndex);
			ImGui::InputText("Class Name to mess with", &ClassNameToDump);
			ImGui::InputText("Object to dump", &ObjectToDump);
			ImGui::InputText("File to save to", &FileNameToSaveTo);

			ImGui::InputText("Function Name to mess with", &FunctionNameToDump);

			if (ImGui::Button("Print Gamephase Step"))
			{
				auto GameState = Cast<AFortGameStateAthena>(GetWorld()->GetGameState());

				if (GameState)
				{
					LOG_INFO(LogDev, "GamePhaseStep: {}", (int)GameState->GetGamePhaseStep());
				}
			}

			if (ImGui::Button("Dump Object Info"))
			{
				ObjectViewer::DumpContentsToFile(ObjectToDump, FileNameToSaveTo, bExcludeUnhandled);
			}

			if (ImGui::Button("Print all instances of class"))
			{
				auto ClassToScuff = FindObject<UClass>(ClassNameToDump);

				if (ClassToScuff)
				{
					auto ObjectNum = ChunkedObjects ? ChunkedObjects->Num() : UnchunkedObjects ? UnchunkedObjects->Num() : 0;

					for (int i = 0; i < ObjectNum; i++)
					{
						auto CurrentObject = GetObjectByIndex(i);

						if (!CurrentObject)
							continue;

						if (!CurrentObject->IsA(ClassToScuff))
							continue;

						LOG_INFO(LogDev, "Object Name: {}", CurrentObject->GetPathName());
					}
				}
			}

			if (ImGui::Button("Load BGA Class"))
			{
				static auto BlueprintGeneratedClassClass = FindObject<UClass>(L"/Script/Engine.BlueprintGeneratedClass");
				auto Class = LoadObject(ClassNameToDump, BlueprintGeneratedClassClass);

				LOG_INFO(LogDev, "New Class: {}", __int64(Class));
			}

			if (ImGui::Button("Find all classes that inherit"))
			{
				auto ClassToScuff = FindObject<UClass>(ClassNameToDump);

				if (ClassToScuff)
				{
					auto ObjectNum = ChunkedObjects ? ChunkedObjects->Num() : UnchunkedObjects ? UnchunkedObjects->Num() : 0;

					for (int i = 0; i < ObjectNum; i++)
					{
						auto CurrentObject = GetObjectByIndex(i);

						if (!CurrentObject || CurrentObject == ClassToScuff)
							continue;

						if (!CurrentObject->IsA(ClassToScuff))
							continue;

						LOG_INFO(LogDev, "Class Name: {}", CurrentObject->GetPathName());
					}
				}
			}

			if (ImGui::Button("Print Class VFT"))
			{
				auto Class = FindObject<UClass>(ClassNameToDump);

				if (Class)
				{
					auto ClassToDump = Class->CreateDefaultObject();

					if (ClassToDump)
					{
						LOG_INFO(LogDev, "{} VFT: 0x{:x}", ClassToDump->GetName(), __int64(ClassToDump->VFTable) - __int64(GetModuleHandleW(0)));
					}
				}
			}

			if (ImGui::Button("Print Function Exec Addr"))
			{
				auto Function = FindObject<UFunction>(FunctionNameToDump);

				if (Function)
				{
					LOG_INFO(LogDev, "{} Exec: 0x{:x}", Function->GetName(), __int64(Function->GetFunc()) - __int64(GetModuleHandleW(0)));
				}
			}

			/* if (ImGui::Button("Load BGA Class (and spawn so no GC)"))
			{
				static auto BGAClass = FindObject<UClass>("/Script/Engine.BlueprintGeneratedClass");
				auto Class = LoadObject<UClass>(ClassNameToDump, BGAClass);

				if (Class)
				{
					GetWorld()->SpawnActor<AActor>(Class, FVector());
				}
			} */

			/* 
			ImGui::Text(std::format("Amount of hooks {}", AllFunctionHooks.size()).c_str());

			for (auto& FunctionHook : AllFunctionHooks)
			{
				if (ImGui::Button(std::format("{} {} (0x{:x})", (FunctionHook.IsHooked ? "Unhook" : "Hook"), FunctionHook.Name, (__int64(FunctionHook.Original) - __int64(GetModuleHandleW(0)))).c_str()))
				{
					if (FunctionHook.IsHooked)
					{
						if (!FunctionHook.VFT || FunctionHook.Index == -1)
						{
							Hooking::MinHook::Unhook(FunctionHook.Original);
						}
						else
						{
							VirtualSwap(FunctionHook.VFT, FunctionHook.Index, FunctionHook.Original);
						}
					}
					else
					{
						Hooking::MinHook::Hook(FunctionHook.Original, FunctionHook.Detour, nullptr, FunctionHook.Name);
					}

					FunctionHook.IsHooked = !FunctionHook.IsHooked;
				}
			} 
			*/
		}
		else if (Tab == DEBUGLOG_TAB)
		{
			ImGui::Checkbox("Floor Loot Debug Log", &bDebugPrintFloorLoot);
			ImGui::Checkbox("Looting Debug Log", &bDebugPrintLooting);
			ImGui::Checkbox("Swapping Debug Log", &bDebugPrintSwapping);
			ImGui::Checkbox("Engine Debug Log", &bEngineDebugLogs);
		}
		else if (Tab == SETTINGS_TAB)
		{
			// ImGui::Checkbox("Use custom lootpool (from Win64/lootpool.txt)", &Defines::bCustomLootpool);
		}
	}
}

static inline void PregameUI()
{
	StaticUI();

	if (Engine_Version >= 422 && Engine_Version < 424)
	{
		ImGui::Checkbox("Creative", &Globals::bCreative);
	}

	if (Addresses::SetZoneToIndex)
	{
		bool bWillBeLategame = Globals::bLateGame.load();
		ImGui::Checkbox("Lategame", &bWillBeLategame);
		SetIsLategame(bWillBeLategame);
	}

	if (HasEvent())
	{
		ImGui::Checkbox("Play Event", &Globals::bGoingToPlayEvent);
	}

	if (!bSwitchedInitialLevel)
	{
		// ImGui::Checkbox("Use Custom Map", &bUseCustomMap);

		if (bUseCustomMap)
		{
			// ImGui::InputText("Custom Map", &CustomMapName);
		}

		ImGui::SliderInt("Seconds until load into map", &SecondsUntilTravel, 1, 100);
	}
		
	if (!Globals::bCreative)
		ImGui::InputText("Playlist", &PlaylistName);
}

static inline HICON LoadIconFromMemory(const char* bytes, int bytes_size, const wchar_t* IconName) {
	HANDLE hMemory = CreateFileMappingW(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, bytes_size, IconName);
	if (hMemory == NULL) {
		return NULL;
	}

	LPVOID lpBuffer = MapViewOfFile(hMemory, FILE_MAP_READ, 0, 0, bytes_size);

	if (lpBuffer == NULL) {
		CloseHandle(hMemory);
		return NULL;
	}

	ICONINFO icon_info;

	if (!GetIconInfo((HICON)lpBuffer, &icon_info)) {
		UnmapViewOfFile(lpBuffer);
		CloseHandle(hMemory);
		return NULL;
	}

	HICON hIcon = CreateIconIndirect(&icon_info);
	UnmapViewOfFile(lpBuffer);
	CloseHandle(hMemory);
	return hIcon;
}

static inline DWORD WINAPI GuiThread(LPVOID)
{
	WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, L"RebootClass", NULL };
	::RegisterClassEx(&wc);
	HWND hwnd = ::CreateWindowExW(0L, wc.lpszClassName, L"Future GS", (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX), 100, 100, Width, Height, NULL, NULL, wc.hInstance, NULL);

	if (false) // idk why this dont work
	{
		auto hIcon = LoadIconFromMemory((const char*)reboot_icon_data, strlen((const char*)reboot_icon_data), L"RebootIco");
		SendMessageW(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
		SendMessageW(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
	}

	// SetWindowLongPtrW(hwnd, GWL_STYLE, WS_POPUP); // Disables windows title bar at the cost of dragging and some quality

	// Initialize Direct3D
	if (!CreateDeviceD3D(hwnd))
	{
		CleanupDeviceD3D();
		::UnregisterClass(wc.lpszClassName, wc.hInstance);
		return 1;
	}

	// Show the window
	::ShowWindow(hwnd, SW_SHOWDEFAULT);
	::UpdateWindow(hwnd);

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	io.IniFilename = NULL; // Disable imgui.ini generation.
	io.DisplaySize = ImGui::GetMainViewport()->Size;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	// io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Dear ImGui style
	InitFont();
	InitStyle();

	// Setup Platform/Renderer backends
	ImGui_ImplWin32_Init(hwnd);
	ImGui_ImplDX9_Init(g_pd3dDevice);

	// Our state
	bool show_demo_window = true;
	bool show_another_window = false;
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	ImFontConfig config;
	config.MergeMode = true;
	config.GlyphMinAdvanceX = 13.0f; // Use if you want to make the icon monospaced
	// static const ImWchar icon_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
	// io.Fonts->AddFontFromFileTTF("Reboot Resources/fonts/fontawesome-webfont.ttf", 13.0f, &config, icon_ranges);

	bool done = false;

	while (!done)
	{
		MSG msg;
		while (::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
			if (msg.message == WM_QUIT)
			{
				// done = true;
				break;
			}
		}

		ImGui_ImplDX9_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		auto WindowSize = ImGui::GetMainViewport()->Size;
		// ImGui::SetNextWindowPos(ImVec2(WindowSize.x * 0.5f, WindowSize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f)); // Center
		ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);

		tagRECT rect;

		if (GetWindowRect(hwnd, &rect))
		{
			int width = rect.right - rect.left;
			int height = rect.bottom - rect.top;
			ImGui::SetNextWindowSize(ImVec2(width, height), ImGuiCond_Always);
		}

		if (!ImGui::IsWindowCollapsed())
		{
			ImGui::Begin("FutureIScool", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);

			Globals::bInitializedPlaylist ? MainUI() : PregameUI();

			ImGui::End();
		}

		// Rendering
		ImGui::EndFrame();
		g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
		g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
		g_pd3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
		D3DCOLOR clear_col_dx = D3DCOLOR_RGBA((int)(clear_color.x * clear_color.w * 255.0f), (int)(clear_color.y * clear_color.w * 255.0f), (int)(clear_color.z * clear_color.w * 255.0f), (int)(clear_color.w * 255.0f));
		g_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, clear_col_dx, 1.0f, 0);

		if (g_pd3dDevice->BeginScene() >= 0)
		{
			ImGui::Render();
			ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
			g_pd3dDevice->EndScene();
		}

		HRESULT result = g_pd3dDevice->Present(NULL, NULL, NULL, NULL);

		// Handle loss of D3D9 device
		if (result == D3DERR_DEVICELOST && g_pd3dDevice->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
			ResetDevice();
	}

	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	CleanupDeviceD3D();
	::DestroyWindow(hwnd);
	::UnregisterClass(wc.lpszClassName, wc.hInstance);

	return 0;
}

// Helper functions

static inline bool CreateDeviceD3D(HWND hWnd)
{
	if ((g_pD3D = Direct3DCreate9(D3D_SDK_VERSION)) == NULL)
		return false;

	// Create the D3DDevice
	ZeroMemory(&g_d3dpp, sizeof(g_d3dpp));
	g_d3dpp.Windowed = TRUE;
	g_d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	g_d3dpp.BackBufferFormat = D3DFMT_UNKNOWN; // Need to use an explicit format with alpha if needing per-pixel alpha composition.
	g_d3dpp.EnableAutoDepthStencil = TRUE;
	g_d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
	g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;           // Present with vsync
	//g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;   // Present without vsync, maximum unthrottled framerate
	if (g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &g_d3dpp, &g_pd3dDevice) < 0)
		return false;

	return true;
}

static inline void CleanupDeviceD3D()
{
	if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = NULL; }
	if (g_pD3D) { g_pD3D->Release(); g_pD3D = NULL; }
}

static inline void ResetDevice()
{
	ImGui_ImplDX9_InvalidateDeviceObjects();
	HRESULT hr = g_pd3dDevice->Reset(&g_d3dpp);
	if (hr == D3DERR_INVALIDCALL)
		IM_ASSERT(0);
	ImGui_ImplDX9_CreateDeviceObjects();
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

static inline LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	// my implementation of window dragging..
	/* {
		static int dababy = 0;
		if (dababy > 100) // wait until gui is initialized ig?
		{
			if (ImGui::IsMouseDragging(ImGuiMouseButton(0)))
			{
				// if (LOWORD(lParam) > 255 && HIWORD(lParam) > 255)
				{
					POINT p;
					GetCursorPos(&p);

					SetWindowPos(hWnd, nullptr, p.x, p.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
				}
			}
		}
		dababy++;
	} */

	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
		return true;

	switch (msg)
	{
	case WM_SIZE:
		if (g_pd3dDevice != NULL && wParam != SIZE_MINIMIZED)
		{
			g_d3dpp.BackBufferWidth = LOWORD(lParam);
			g_d3dpp.BackBufferHeight = HIWORD(lParam);
			ResetDevice();
		}
		return 0;
	case WM_SYSCOMMAND:
		if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
			return 0;
		break;
	case WM_DESTROY:
		::PostQuitMessage(0);
		return 0;
	}
	return ::DefWindowProc(hWnd, msg, wParam, lParam);
}