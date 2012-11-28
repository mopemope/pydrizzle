#ifndef RESULT_H
#define RESULT_H

#include "pydrizzle.h"

typedef struct _result {
    PyObject *description;
    PyObject *converter;
    drizzle_con_st *con;
    drizzle_result_st *result;
    drizzle_column_type_t *types;
    uint16_t column_count;
} result_t;

result_t* new_result_t(drizzle_con_st *con, drizzle_result_st *result);

PyObject* fetchone(result_t *result);

void free_result_t(result_t *result);


#endif
