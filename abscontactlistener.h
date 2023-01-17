#ifndef ABSCONTACTLISTENER_H
#define ABSCONTACTLISTENER_H

#include <vector>

#include "box2d/b2_contact.h"
#include "box2d/b2_world_callbacks.h"

struct collision_struct{
    b2Body* bodyA = nullptr;
    b2Body* bodyB = nullptr;

    collision_struct(b2Body* a, b2Body* b){
        this->bodyA = a;
        this->bodyB = b;
    }
};

class ABSContactListener : public b2ContactListener
{
public:
    explicit ABSContactListener();

    void BeginContact(b2Contact* contact);

    std::vector<collision_struct> collisions;
};

#endif // ABSCONTACTLISTENER_H
