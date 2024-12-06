#pragma once

#include <unordered_set>

#include "Object/ObjectReference.hpp"

class LandmarkManager
{


private:
    std::unordered_set<ObjectReference> landmarks;
};