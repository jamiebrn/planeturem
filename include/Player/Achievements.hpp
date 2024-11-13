#pragma once

#include <string>

#include <steam/isteamuserstats.h>

namespace Achievements
{

void attemptAchievementUnlock(const std::string& achievementId);

extern bool steamInitialised;

}