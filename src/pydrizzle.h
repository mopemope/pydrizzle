#ifndef PYDRIZZLE_H
#define PYDRIZZLE_H

#include <Python.h>
#include <libdrizzle/drizzle_client.h>



typedef struct {
	PyObject_HEAD
    drizzle_st* drizzle;
	drizzle_con_st* con;
}   ConnectionObject;

extern PyTypeObject ConnectionObjectType;


#endif
