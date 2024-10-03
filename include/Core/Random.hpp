#pragma once

#include <random>

class RandInt
{
public:
    RandInt(unsigned long int seed);

    int generate(int low, int high);

private:
    std::default_random_engine gen;

};