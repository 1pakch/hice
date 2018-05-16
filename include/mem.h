#pragma once

#include <stdlib.h>


#define MALLOC(T) ((T*)(malloc(sizeof(T))))
#define FREE(p) (free(p))
