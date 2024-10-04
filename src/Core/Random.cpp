#include "Core/Random.hpp"

RandInt::RandInt(unsigned long int seed)
 : next(seed)
{}

int RandInt::generate(int low, int high)
{
    next = next * 1102515245 + 12345;
    return ((unsigned int)(next / 65536) % 32768) % (high + 1 - low) + low;
}
