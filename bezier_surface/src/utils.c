#include "utils.h"

#include <math.h>

unsigned long long fact[] = {
    1ULL,                    // 0!
    1ULL,                    // 1!
    2ULL,                    // 2!
    6ULL,                    // 3!
    24ULL,                   // 4!
    120ULL,                  // 5!
    720ULL,                  // 6!
    5040ULL,                 // 7!
    40320ULL,                // 8!
    362880ULL,               // 9!
    3628800ULL,              // 10!
    39916800ULL,             // 11!
    479001600ULL,            // 12!
    6227020800ULL,           // 13!
    87178291200ULL,          // 14!
    1307674368000ULL,        // 15!
    20922789888000ULL,       // 16!
    355687428096000ULL,      // 17!
    6402373705728000ULL,     // 18!
    121645100408832000ULL,   // 19!
    2432902008176640000ULL   // 20!
};

double degree_to_radian(double degree)
{
	return degree * M_PI / 180.0;
}

