#include "Player/Achievements.hpp"

bool Achievements::steamInitialised = false;

void Achievements::attemptAchievementUnlock(const std::string& achievementId)
{
    if (!steamInitialised)
    {
        return;
    }

    SteamUserStats()->SetAchievement(achievementId.c_str());
    SteamUserStats()->StoreStats();
}