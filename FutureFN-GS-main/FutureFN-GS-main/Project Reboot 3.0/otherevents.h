#pragma once

#include "Object.h"

UObject* GetEventLoader();
UObject* GetEventScripting();

namespace Events
{
	inline bool bHasBeenLoaded = false;

	void LoadEvent();
	void StartEvent();
	std::string GetEventPlaylistName();

}