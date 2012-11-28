#ifndef CONVERTER_H
#define CONVERTER_H

#include "pydrizzle.h"
#include "buffer.h"

buffer_t*
get_converted_query(char *query, size_t len, PyObject *args);

#endif

