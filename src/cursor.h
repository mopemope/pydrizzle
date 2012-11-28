#ifndef CURSOR_H
#define CURSOR_H

#include "pydrizzle.h"
#include "result.h"

typedef struct {
    PyObject_HEAD
    PyObject *connection;
    drizzle_con_st *con;
    result_t *result;
} CursorObject;

extern PyTypeObject CursorObjectType;

PyObject* CursorObject_new(PyObject *conObj, drizzle_con_st *con);

#endif


