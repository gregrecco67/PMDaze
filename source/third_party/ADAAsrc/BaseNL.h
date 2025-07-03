#pragma once

#include <numbers>

using std::numbers::inv_pi;
using std::numbers::pi;

class BaseNL
{
  public:
    BaseNL() = default;
    virtual ~BaseNL() {}

    virtual void prepare(double, int) {}
    virtual void processBlock(float *, const int) {}
};
