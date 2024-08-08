#pragma once

#include <unordered_map>
#include <chrono>
#include <cmath>
#include <cassert>

enum TweenTransition
{
    Linear,
    Sine,
    Cubic,
    Quint,
    Circ,
    Elastic,
    Quad,
    Quart,
    Expo,
    Back,
    Bounce
};

enum TweenEasing
{
    EaseIn,
    EaseOut,
    EaseInOut
};

typedef uint64_t TweenID;

template <typename T>
struct TweenData
{
    T* value;
    T start, end;
    float duration;
    float progress;
    TweenTransition transitionType;
    TweenEasing easingType;
};

template <typename T>
class Tween
{
public:
    inline Tween<T>() = default;

    inline TweenID startTween(T* value, T start, T end, float duration, TweenTransition transitionType, TweenEasing easingType)
    {
        TweenID id = std::chrono::system_clock::now().time_since_epoch() / std::chrono::milliseconds(1);

        assert(activeTweens.count(id) <= 0 && "Tween ID duplicate");

        TweenData<T> data;
        data.value = value;
        data.start = start;
        data.end = end;
        data.duration = duration;
        data.progress = 0.0f;
        data.transitionType = transitionType;
        data.easingType = easingType;

        activeTweens[id] = data;

        return id;
    }

    inline T getTweenValue(TweenID tween)
    {
        assert(activeTweens.count(tween) > 0 && "Tween ID does not exist (get value)");
        
        TweenData<T>& tweenData = activeTweens[tween];

        float progress = std::min(tweenData.progress / tweenData.duration, 1.0f);

        return (tweenData.end - tweenData.start) * easingFunction(tweenData.transitionType, tweenData.easingType, progress) + tweenData.start;
    }

    inline bool isTweenFinished(TweenID tween)
    {
        if (activeTweens.count(tween) <= 0)
            return true;
        
        TweenData<T>& tweenData = activeTweens[tween];
        
        if (tweenData.progress >= tweenData.duration)
            return true;
        
        return false;
    }

    inline void update(float dt)
    {
        for (auto tweenIter = activeTweens.begin(); tweenIter != activeTweens.end();)
        {
            TweenID tweenID = tweenIter->first;
            TweenData<T>& tweenData = tweenIter->second;

            tweenData.progress += dt;

            *tweenData.value = getTweenValue(tweenID);

            if (isTweenFinished(tweenID))
            {
                tweenIter = activeTweens.erase(tweenIter);
                continue;
            }

            tweenIter++;
        }
    }

private:
    inline float easingFunction(TweenTransition transitionType, TweenEasing easingType, float progress)
    {
        switch (easingType)
        {
            case TweenEasing::EaseIn:
                switch (transitionType)
                {
                    case TweenTransition::Sine: return easeInSine(progress);
                    case TweenTransition::Cubic: return easeInCubic(progress);
                    case TweenTransition::Quint: return easeInQuint(progress);
                    case TweenTransition::Circ: return easeInCirc(progress);
                    case TweenTransition::Elastic: return easeInElastic(progress);
                    case TweenTransition::Quad: return easeInQuad(progress);
                    case TweenTransition::Quart: return easeInQuart(progress);
                    case TweenTransition::Expo: return easeInExpo(progress);
                    case TweenTransition::Back: return easeInBack(progress);
                    case TweenTransition::Bounce: return easeInBounce(progress);
                }
            case TweenEasing::EaseOut:
                switch (transitionType)
                {
                    case TweenTransition::Sine: return easeOutSine(progress);
                    case TweenTransition::Cubic: return easeOutCubic(progress);
                    case TweenTransition::Quint: return easeOutQuint(progress);
                    case TweenTransition::Circ: return easeOutCirc(progress);
                    case TweenTransition::Elastic: return easeOutElastic(progress);
                    case TweenTransition::Quad: return easeOutQuad(progress);
                    case TweenTransition::Quart: return easeOutQuart(progress);
                    case TweenTransition::Expo: return easeOutExpo(progress);
                    case TweenTransition::Back: return easeOutBack(progress);
                    case TweenTransition::Bounce: return easeOutBounce(progress);
                }
            case TweenEasing::EaseInOut:
                switch (transitionType)
                {
                    case TweenTransition::Sine: return easeInOutSine(progress);
                    case TweenTransition::Cubic: return easeInOutCubic(progress);
                    case TweenTransition::Quint: return easeInOutQuint(progress);
                    case TweenTransition::Circ: return easeInOutCirc(progress);
                    case TweenTransition::Elastic: return easeInOutElastic(progress);
                    case TweenTransition::Quad: return easeInOutQuad(progress);
                    case TweenTransition::Quart: return easeInOutQuart(progress);
                    case TweenTransition::Expo: return easeInOutExpo(progress);
                    case TweenTransition::Back: return easeInOutBack(progress);
                    case TweenTransition::Bounce: return easeInOutBounce(progress);
                }
        }

        return progress;
    }

    static constexpr float PI = 3.14159265f;
    static constexpr float c1 = 1.70158f;
    static constexpr float c2 = c1 * 1.525f;
    static constexpr float c3 = c1 + 1.0f;
    static constexpr float c4 = (2.0f * PI) / 3.0f;
    static constexpr float c5 = (2.0f * PI) / 4.5f;
    static constexpr float n1 = 7.5625f;
    static constexpr float d1 = 2.75f;

    inline float easeInSine(float x) {return 1.0f - std::cos((x * PI) / 2.0f);}
    inline float easeOutSine(float x) {return std::sin((x * PI) / 2.0f);}
    inline float easeInOutSine(float x) {return -(std::cos(PI * x) - 1.0f) / 2.0f;}
    inline float easeInCubic(float x) {return x * x * x;}
    inline float easeOutCubic(float x) {return 1.0f - std::pow(1.0f - x, 3.0f);}
    inline float easeInOutCubic(float x) {return x < 0.5f ? 4.0f * x * x * x : 1.0f - std::pow(-2.0f * x + 2.0f, 3.0f) / 2.0f;}
    inline float easeInQuint(float x) {return x * x * x * x * x;}
    inline float easeOutQuint(float x) {return 1.0f - std::pow(1.0f - x, 5.0f);}
    inline float easeInOutQuint(float x) {return x < 0.5f ? 16.0f * x * x * x * x * x : 1.0f - std::pow(-2.0f * x + 2.0f, 5.0f) / 2.0f;}
    inline float easeInCirc(float x) {return 1.0f - std::sqrt(1.0f - std::pow(x, 2.0f));}
    inline float easeOutCirc(float x) {return std::sqrt(1.0f - std::pow(x - 1.0f, 2.0f));}
    inline float easeInOutCirc(float x)
    {
        return (x < 0.5f ? (1.0f - std::sqrt(1.0f - std::pow(2.0f * x, 2.0f))) / 2.0f : (std::sqrt(1.0f - std::pow(-2.0f * x + 2.0f, 2.0f)) + 1.0f) / 2.0f);
    }
    inline float easeInElastic(float x)
    {
        return (x == 0.0f ? 0.0f : x == 1.0f ? 1.0f : -std::pow(2.0f, 10.0f * x - 10.0f) * std::sin((x * 10.0f - 10.75f) * c4));
    }
    inline float easeOutElastic(float x)
    {
        return (x == 0.0f ? 0.0f : x == 1.0f ? 1.0f : std::pow(2.0f, -10.0f * x) * std::sin((x * 10.0f - 0.75f) * c4) + 1.0f);
    }
    inline float easeInOutElastic(float x)
    {
        return (x == 0.0f ? 0.0f : x == 1.0f ? 1.0f : x < 0.5f ? -(std::pow(2.0f, 20.0f * x - 10.0f) * std::sin((20.0f * x - 11.125f) * c5)) / 2.0f
        : (std::pow(2.0f, -20.0f * x + 10.0f) * std::sin((20.0f * x - 11.125f) * c5)) / 2.0f + 1.0f);
    }
    inline float easeInQuad(float x) {return x * x;}
    inline float easeOutQuad(float x) {return 1.0f - (1.0f - x) * (1.0f - x);}
    inline float easeInOutQuad(float x) {return x < 0.5f ? 2.0f * x * x : 1.0f - std::pow(-2.0f * x + 2.0f, 2.0f) / 2.0f;}
    inline float easeInQuart(float x) {return x * x * x * x;}
    inline float easeOutQuart(float x) {return 1.0f - std::pow(1.0f - x, 4.0f);}
    inline float easeInOutQuart(float x) {return x < 0.5f ? 8.0f * x * x * x * x : 1.0f - std::pow(-2.0f * x + 2.0f, 4.0f) / 2.0f;}
    inline float easeInExpo(float x) {return x == 0.0f ? 0.0f : std::pow(2.0f, 10.0f * x - 10.0f);}
    inline float easeOutExpo(float x) {return x == 1.0f ? 1.0f : 1.0f - std::pow(2.0f, -10.0f * x);}
    inline float easeInOutExpo(float x)
    {
        return (x == 0.0f ? 0.0f : x == 1.0f ? 1.0f : x < 0.5f ? std::pow(2.0f, 20.0f * x - 10.0f) / 2.0f
        : (2.0f - std::pow(2.0f, -20.0f * x + 10.0f)) / 2.0f);
    }
    inline float easeInBack(float x)
    {
        return c3 * x * x * x - c1 * x * x;
    }
    inline float easeOutBack(float x)
    {
        return 1.0f + c3 * std::pow(x - 1.0f, 3.0f) + c1 * std::pow(x - 1.0f, 2.0f);
    }
    inline float easeInOutBack(float x)
    {
        return (x < 0.5f ? (std::pow(2.0f * x, 2.0f) * ((c2 + 1.0f) * 2.0f * x - c2)) / 2.0f
        : (std::pow(2.0f * x - 2.0f, 2.0f) * ((c2 + 1.0f) * (x * 2.0f - 2.0f) + c2) + 2.0f) / 2.0f);
    }
    inline float easeOutBounce(float x)
    {
        if (x < 1.0f / d1) return n1 * x * x;
        else if (x < 2.0f / d1) return n1 * (x -= 1.5f / d1) * x + 0.75f;
        else if (x < 2.5f / d1) return n1 * (x -= 2.25f / d1) * x + 0.9375f;
        else return n1 * (x -= 2.625f / d1) * x + 0.984375f;
    }
    inline float easeInBounce(float x) {return 1.0f - easeOutBounce(1.0f - x);}
    inline float easeInOutBounce(float x)
    {
        return (x < 0.5f ? (1.0f - easeOutBounce(1.0f - 2.0f * x)) / 2.0f : (1.0f + easeOutBounce(2.0f * x - 1.0f)) / 2.0f);
    }

private:
    std::unordered_map<TweenID, TweenData<T>> activeTweens;

};