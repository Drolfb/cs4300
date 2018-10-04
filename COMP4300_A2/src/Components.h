#pragma once

#include "Common.h"

class Component
{
public:
    virtual ~Component() {}
};

class CTransform : public Component
{
public:
    Vec2 pos    = { 0.0, 0.0 };
    Vec2 speed  = { 0.0, 0.0 };
    float angle = 0;

    CTransform() {}
    CTransform(const Vec2 & p, const Vec2 & s, float a) : pos(p), speed(s), angle(a) {}
};

class CShape : public Component
{
public:
    sf::CircleShape circle;

    CShape(float radius, int points, const sf::Color & fill, const sf::Color & outline, float thickness)
        : circle(radius, points)
    {
        circle.setFillColor(fill);
        circle.setOutlineColor(outline);
        circle.setOutlineThickness(thickness);
        circle.setOrigin(radius, radius);
    }
};

class CCollision : public Component
{
public:
    float radius = 0;
    CCollision(const float & r) : radius(r) {}
};

class CScore : public Component
{
public:
    int score = 0;
    CScore(int s) : score(s) {}
};

class CLifespan : public Component
{
public:
    sf::Clock clock;
    int lifespan = 0;
    CLifespan(int ms) : lifespan(ms) {}
};

class CInput : public Component
{
public:
    bool up = false;
    bool left = false;
    bool right = false;
    bool down = false;

    CInput() {}
};
