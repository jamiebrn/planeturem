#include "Core/Random.hpp"

RandInt::RandInt(unsigned long int seed)
 : gen(seed)
{}

int RandInt::generate(int low, int high)
{
    std::uniform_int_distribution<int> dist(low, high);

    return dist(gen);
}
