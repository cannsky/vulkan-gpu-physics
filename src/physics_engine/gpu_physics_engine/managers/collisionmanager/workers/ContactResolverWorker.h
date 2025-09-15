#pragma once

#include "../Contact.h"
#include <vector>
#include <iostream>

class ContactResolverWorker {
public:
    ContactResolverWorker() = default;
    ~ContactResolverWorker() = default;
    
    // Contact resolution (runs on CPU for now, can be ported to GPU)
    void resolveContacts(const std::vector<Contact>& contacts, float deltaTime);

private:
    // Log contact resolution for debugging
    void logContactResolution(const Contact& contact) const;
};