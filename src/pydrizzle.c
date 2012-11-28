#include "pydrizzle.h"
#include "connection.h"
#include "cursor.h"

drizzle_st *drizzle;

static PyObject *external_io_wait = NULL;

static int
init_drizzle(void)
{
    drizzle = drizzle_create(NULL);
    if (drizzle == NULL) {
        PyErr_SetString(PyExc_IOError, "drizzle_st create failed");
        RDEBUG("drizzle_st create failed");
        return -1;
    }
    drizzle_add_options(drizzle, DRIZZLE_NON_BLOCKING);
    return 1;
}

int
wait_connect(drizzle_con_st *con)
{
    drizzle_return_t ret;
    int fd = 0, events = 0;
    PyObject *fileno, *state, *args, *res;

    while (1) {
        ret = drizzle_con_connect(con);
        if (ret == DRIZZLE_RETURN_OK) {
            return 1;
        } else if(ret == DRIZZLE_RETURN_IO_WAIT) {
            events = con->events;
            YDEBUG("IO_WAIT con:%p events:%d", con, events);
            if (external_io_wait) {
                fd = drizzle_con_fd(con);
                if (fd == -1){
                    return -1;
                }

                fileno = PyLong_FromLong((long)fd);
                if (fileno == NULL) {
                    return -1;
                }
                state = PyLong_FromLong((long)events);
                if (state == NULL) {
                    Py_DECREF(fileno);
                    return -1;
                }

                args = PyTuple_Pack(2, fileno, state);
                if (args == NULL) {
                    Py_DECREF(fileno);
                    Py_DECREF(state);
                    return -1;
                }
                
                YDEBUG("call external_io_wait ...");
                res = PyObject_CallObject(external_io_wait, args);
                Py_DECREF(args);

                if (res == NULL) {
                    return -1;
                }
                Py_XDECREF(res);
                ret = drizzle_con_set_revents(con, events);
                if (ret != DRIZZLE_RETURN_OK){
                    RDEBUG("ret %d:%s", ret, drizzle_error(drizzle));
                    return -1;
                }

                return 1;
            } else {
                YDEBUG("call drizzle_con_wait ...");
                ret = drizzle_con_wait(drizzle);
                if (ret != DRIZZLE_RETURN_OK){
                    RDEBUG("ret %d:%s", ret, drizzle_error(drizzle));
                    return -1;
                }
            }
        } else {
            RDEBUG("ret %d:%s", ret, drizzle_error(drizzle));
            return -1;
        }
    }
    return 1;
}

int
io_wait(drizzle_con_st *con, drizzle_return_t ret)
{
    drizzle_return_t dret;
    int fd = 0, events = 0;
    PyObject *fileno, *state, *args, *res;

    if (ret == DRIZZLE_RETURN_OK) {
        return 0;
    }else if (ret == DRIZZLE_RETURN_IO_WAIT) {
        events = con->events;

        YDEBUG("IO_WAIT con:%p events:%d", con, events);
        if (external_io_wait) {
            fd = drizzle_con_fd(con);
            if (fd == -1){
                return -1;
            }

            fileno = PyLong_FromLong((long)fd);
            if (fileno == NULL) {
                return -1;
            }
            state = PyLong_FromLong((long)events);
            if (state == NULL) {
                Py_DECREF(fileno);
                return -1;
            }

            args = PyTuple_Pack(2, fileno, state);
            if (args == NULL) {
                Py_DECREF(fileno);
                Py_DECREF(state);
                return -1;
            }

            YDEBUG("call external_io_wait ...");
            res = PyObject_CallObject(external_io_wait, args);
            Py_DECREF(args);

            if (res == NULL) {
                return -1;
            }
            Py_XDECREF(res);
            dret = drizzle_con_set_revents(con, events);
            if (dret != DRIZZLE_RETURN_OK){
                RDEBUG("ret %d:%s", dret, drizzle_error(drizzle));
                return -1;
            }
            return 1;
        } else {
            DEBUG("call drizzle_con_wait ...");
            dret = drizzle_con_wait(drizzle);

            if (dret != DRIZZLE_RETURN_OK){
                RDEBUG("ret %d:%s", dret, drizzle_error(drizzle));
                return -1;
            }
            return 1;
        }
    }else{
        RDEBUG("ret %d:%s", ret, drizzle_error(drizzle));
        return -1;
    }
    return 0;
}

static PyObject*
pydrizzle_set_wait_callback(PyObject *obj, PyObject *args)
{
    PyObject *temp = NULL;

    if (!PyArg_ParseTuple(args, "O", &temp)) {
        return NULL;
    }

    if (!PyCallable_Check(temp)) {
        PyErr_SetString(PyExc_TypeError, "must be callable");
        return NULL;
    }

    if (external_io_wait) {
        Py_DECREF(external_io_wait);
    }
    
    external_io_wait = temp;
    Py_INCREF(external_io_wait);

    Py_RETURN_NONE;
}

static PyMethodDef pydrizzle_methods[] = {
    {"connect", (PyCFunction)pydrizzle_connect, METH_VARARGS|METH_KEYWORDS, "connect"},
    {"set_wait_callback", (PyCFunction)pydrizzle_set_wait_callback, METH_VARARGS, "set_wait_callback"},
    {NULL, NULL, 0, NULL}        /* Sentinel */
};

#ifdef PY3
#define INITERROR return NULL

static struct PyModuleDef pydrizzle_module_def = {
    PyModuleDef_HEAD_INIT,
    MODULE_NAME,
    NULL,
    -1,
    pydrizzle_methods,
};

PyObject *
PyInit_pydrizzle(void)
#else
#define INITERROR return

PyMODINIT_FUNC
init_pydrizzle(void)
#endif
{
    PyObject *m;
#ifdef PY3
    m = PyModule_Create(&pydrizzle_module_def);
#else
    m = Py_InitModule3(MODULE_NAME, pydrizzle_methods, "");
#endif
    if (m == NULL){
        INITERROR;
    }

    if (PyType_Ready(&ConnectionObjectType) < 0){
        INITERROR;
    }

    if (PyType_Ready(&CursorObjectType) < 0){
        INITERROR;
    }

    PyModule_AddIntConstant(m, "POLLIN", POLLIN );
    PyModule_AddIntConstant(m, "POLLOUT", POLLOUT );

    if (init_drizzle() < 0){
        INITERROR;
    }

#ifdef PY3
    return m;
#endif
}
