#ifndef CONNECTION_H
#define CONNECTION_H

#include "pydrizzle.h"

typedef struct {
    PyObject_HEAD
    drizzle_con_st *con;
    PyObject *active_cursor;
} ConnectionObject;

extern PyTypeObject ConnectionObjectType;

PyObject* pydrizzle_connect(PyObject *obj, PyObject *args, PyObject *kwargs);


#endif
