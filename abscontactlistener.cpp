#include "abscontactlistener.h"

ABSContactListener::ABSContactListener(){

}

void ABSContactListener::BeginContact(b2Contact* contact){
    //printf("Contact!\n");

    b2Fixture* fixtureA = contact->GetFixtureA();
    b2Body* bodyA = fixtureA->GetBody();

    b2Fixture* fixtureB = contact->GetFixtureB();
    b2Body* bodyB = fixtureB->GetBody();

    this->collisions.push_back(collision_struct(bodyA, bodyB));
}
