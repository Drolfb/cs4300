#pragma once

#include "Components.h"

class Entity
{
    // EntityManager is now a friend, it can use our private constructor
    // This ensures we can't make Entity instances outside EntityManager
    friend class EntityManager;

    bool                m_active    = true;
    size_t              m_id        = 0;
    std::string         m_tag       = "default";

    // the constructor of entity is private, only its friends can use it
    Entity(const size_t & id, const std::string & tag);

public:

    // conponent pointers
    CTransform *    cTransform  = nullptr;
    CShape *        cShape      = nullptr;
    CCollision *    cCollision  = nullptr;
    CInput *        cInput      = nullptr;
    CScore *        cScore      = nullptr;
    CLifespan *     cLifespan   = nullptr;
    
    // constructor and destructor
    ~Entity();

    // private member access functions
    bool isActive() const;
    const std::string & tag() const;
    const size_t & id() const;
    void destroy();
};

