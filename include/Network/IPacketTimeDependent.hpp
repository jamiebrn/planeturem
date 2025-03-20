#pragma once

#include <string>
#include <extlib/steam/steam_api.h>

// If used, remember to also serialise hostPingLocation
struct IPacketTimeDependent
{
    // std::string hostPingLocation;

    // Sets ping location to this computer's ping location (call if host)
    // inline bool setHostPingLocation()
    // {
    //     SteamNetworkPingLocation_t pingLocation;
    //     if (!SteamNetworkingUtils()->GetLocalPingLocation(pingLocation))
    //     {
    //         return false;
    //     }
        
    //     char pingLocationStrBuffer[k_cchMaxSteamNetworkingPingLocationString];
    //     SteamNetworkingUtils()->ConvertPingLocationToString(pingLocation, pingLocationStrBuffer, k_cchMaxSteamNetworkingPingLocationString);
    //     // hostPingLocation = pingLocationStrBuffer;
        
    //     return true;
    // }
    
    // Parses host ping location from string and corrects time values
    inline bool applyPingEstimate(const std::string& senderPingLocation)
    {
        SteamNetworkPingLocation_t pingLocation;
        if (!SteamNetworkingUtils()->ParsePingLocationString(senderPingLocation.c_str(), pingLocation))
        {
            return false;
        }
        
        float pingTimeSecs = SteamNetworkingUtils()->EstimatePingTimeFromLocalHost(pingLocation) / 1000.0f;

        applyPingCorrection(pingTimeSecs);

        return true;
    }

protected:
    virtual void applyPingCorrection(float pingTimeSecs) = 0;

};