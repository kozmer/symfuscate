#include "Hashing.h"

/*!
 * @brief
 *  DJB2 hash function to compute a hash value for a string.
 *
 * @param str
 *  The input string to hash.
 *
 * @return
 *  The computed hash value as uint32.
 */
uint32_t HASH(const char *str)
{
    uint32_t hash = 6543;
    int c;

    while ((c = *str++))
    {
        hash = ((hash << 5) + hash) + c;
    }

    return hash;
}