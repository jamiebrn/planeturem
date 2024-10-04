#pragma once

class RandInt
{
public:
    RandInt(unsigned long int seed);

    int generate(int low, int high);

private:
    unsigned long int next = 1;

};