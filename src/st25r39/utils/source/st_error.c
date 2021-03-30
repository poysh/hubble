#include <stdio.h>
#include <stdint.h>
#include "st_errno.h"

int on_st_error(int r, int f)
{
	r = 0;
    (r) = (f);
    if (ERR_NONE != (r))
    {
        return (r);
    }
    return f;
}
