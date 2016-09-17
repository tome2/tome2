#pragma once

#include "h-basic.h"
#include "message.hpp"

/**
 * Get the current number of messages.
 */
s16b message_num();

/**
 * Get message of given age. Age must be
 * in the half-open interval [0, message_num).
 *
 * The reference is only valid as long as
 * no messages are added.
 */
message const &message_at(int age);

/**
 * Add a message.
 */
void message_add(cptr msg, byte color);

/**
 * Add a message.
 */
void message_add(message const &);
