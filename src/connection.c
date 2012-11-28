#include "connection.h"
#include "cursor.h"


static inline int
ConnectionObject_init(ConnectionObject *self, PyObject *args, PyObject *kwargs);

PyObject*
pydrizzle_connect(PyObject *obj, PyObject *args, PyObject *kwargs)
{
    ConnectionObject *con;
    

    con = (ConnectionObject *)PyObject_NEW(ConnectionObject, &ConnectionObjectType);
    if (con == NULL) {
       return NULL;
    }
    
    GDEBUG("alloc ConnectionObject %p", con);

    if (ConnectionObject_init(con, args, kwargs) < 0) {
        if (con != NULL) {
            Py_DECREF(con);
        }
        return NULL;
    }
    return (PyObject*)con;
}


static int
ConnectionObject_init(ConnectionObject *self, PyObject *args, PyObject *kwargs)
{

    drizzle_con_st *con;
    
    //Not support
    PyObject *ssl = NULL;
    char *host = DRIZZLE_DEFAULT_TCP_HOST;
    char *user = NULL, *passwd = NULL, *db = NULL, *unix_socket = NULL;
    unsigned int port = 3306;
    unsigned int client_flag = 0;
    int connect_timeout = 0;

    static char *kwlist[] = { "host", "user", "passwd", "db", "port", "unix_socket", "connect_timeout", NULL } ;
  
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|ssssisi:connect",
           kwlist,
           &host, &user, &passwd, &db,
           &port, &unix_socket,
           &connect_timeout
           )){
        return -1;
    }

    DEBUG("init %p", self);  


    //set NON_BLOKING
    /* drizzle_add_options(self->drizzle, DRIZZLE_NON_BLOCKING); */

    Py_BEGIN_ALLOW_THREADS ;
    if (connect_timeout) {
        drizzle_set_timeout(drizzle, connect_timeout);
        DEBUG("set timeout %d", connect_timeout);  
    }

    DEBUG("host %s", host);
    DEBUG("port %d", port);
    DEBUG("user %s", user);
    DEBUG("passwd %s", passwd);
    DEBUG("db %s", db);

    if (!unix_socket) {
      con = drizzle_con_add_tcp(drizzle, NULL, host, port, user, passwd, db, DRIZZLE_CON_MYSQL);
    } else {
      con = drizzle_con_add_uds(drizzle, NULL, unix_socket, user, passwd, db, DRIZZLE_CON_MYSQL);
    }
    Py_END_ALLOW_THREADS;
    DEBUG("created drizzle conn %p", con);  
    DEBUG("connect %p", con);  

    if (!con) {
        PyErr_SetString(PyExc_IOError, drizzle_error(drizzle));
        RDEBUG("drizzle connection failed");
        return -1;
    }
    self->con = con;
    if (wait_connect(con) == -1) {
        return -1;
    }

    return 0;
}

static PyObject* 
ConnectionObject_close(ConnectionObject *self)
{
    if(self->con){ 
        drizzle_con_close(self->con);
        DEBUG("close %p", self->con);  
    }
    Py_RETURN_NONE;
}

static void
ConnectionObject_dealloc(ConnectionObject *self)
{
    PyObject *ret = NULL;

    if(self->con){ 
        ret = ConnectionObject_close(self);
        Py_DECREF(ret);
        DEBUG("call con free %p", self->con);
        drizzle_con_free(self->con);
        self->con = NULL;
    }
    GDEBUG("dealloc ConnectionObject %p", self);

    PyObject_DEL(self);
}

static PyObject*
ConnectionObject_autocommit(ConnectionObject *self, PyObject *args)
{
    int flag, status = 1;
    char query[256];

    drizzle_return_t ret;
    drizzle_result_st result;

    if (!PyArg_ParseTuple(args, "i", &flag)) {
        return NULL;
    }

    DEBUG("autocommit %d", flag);
    snprintf(query, 256, "SET AUTOCOMMIT=%d;", flag);
    BDEBUG("query %s", query);
    
    while(status){
        (void)drizzle_query(self->con, &result, query, 256, &ret);
        status = io_wait(self->con, ret);
        if (status == -1){
            goto error;
        }
    }

    drizzle_result_free(&result);
    Py_RETURN_NONE;
error:
    drizzle_result_free(&result);
    PyErr_SetString(PyExc_IOError, drizzle_error(drizzle));
    return NULL;
}

static PyObject*
ConnectionObject_ping(ConnectionObject *self, PyObject *args)
{

    int status = 1;
    drizzle_return_t ret;
    drizzle_result_st result;

    drizzle_con_ping(self->con, &result, &ret);

    status = io_wait(self->con, ret);
    if (status == -1) {
        goto error;
    }

    Py_RETURN_NONE;
error:
    PyErr_SetString(PyExc_IOError, drizzle_error(drizzle));
    return NULL;
}

static PyObject*
ConnectionObject_commit(ConnectionObject *self)
{
    int status = 1;
    drizzle_return_t ret;
    drizzle_result_st result;
    
    while(status){
        (void)drizzle_query(self->con, &result, "COMMIT;", 7, &ret);
        status = io_wait(self->con, ret);
        if (status == -1){
            goto error;
        }
    }

    drizzle_result_free(&result);
    Py_RETURN_NONE;
error:
    drizzle_result_free(&result);
    PyErr_SetString(PyExc_IOError, drizzle_error(drizzle));
    return NULL;
}

static PyObject*
ConnectionObject_rollback(ConnectionObject *self, PyObject *args)
{
    int status = 1;
    drizzle_return_t ret;
    drizzle_result_st result;
    
    while(status){
        (void)drizzle_query(self->con, &result, "ROLLBACK;", 9, &ret);
        status = io_wait(self->con, ret);
        if (status == -1){
            goto error;
        }
    }

    drizzle_result_free(&result);
    Py_RETURN_NONE;
error:
    drizzle_result_free(&result);
    PyErr_SetString(PyExc_IOError, drizzle_error(drizzle));
    return NULL;
}


static PyObject*
ConnectionObject_cursor(ConnectionObject *self, PyObject *args, PyObject *kwargs)
{
    PyObject *cursor;
    
    if (self->active_cursor) {
        //cleat cursor
    }

    cursor = CursorObject_new((PyObject*)self, self->con);
    if (cursor == NULL){
        return NULL;
    }
    self->active_cursor = cursor;
    return cursor;
}

static PyMethodDef ConnectionObject_methods[] = {
    {"autocommit", (PyCFunction)ConnectionObject_autocommit, METH_VARARGS, 0},
    {"ping", (PyCFunction)ConnectionObject_ping, METH_NOARGS, 0},
    {"close", (PyCFunction)ConnectionObject_close, METH_NOARGS, 0},
    {"commit", (PyCFunction)ConnectionObject_commit, METH_NOARGS, 0},
    {"rollback", (PyCFunction)ConnectionObject_rollback, METH_NOARGS, 0},
    {"cursor", (PyCFunction)ConnectionObject_cursor, METH_VARARGS|METH_KEYWORDS, 0},
    {NULL, NULL} /* sentinel */
};

PyTypeObject ConnectionObjectType = {
#ifdef PY3
    PyVarObject_HEAD_INIT(NULL, 0)
#else
    PyObject_HEAD_INIT(NULL)
    0,                    /* ob_size */
#endif
    MODULE_NAME ".Connection",             /*tp_name*/
    sizeof(ConnectionObject), /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)ConnectionObject_dealloc, /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    0,                         /*tp_repr*/
    0,                         /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    0,                         /*tp_as_mapping*/
    0,                         /*tp_hash */
    0,                         /*tp_call*/
    0,                         /*tp_str*/
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_GC | Py_TPFLAGS_BASETYPE, /* tp_flags */
    "",                 /* tp_doc */
    0,                   /* tp_traverse */
    0,                   /* tp_clear */
    0,                   /* tp_richcompare */
    0,                   /* tp_weaklistoffset */
    0,                   /* tp_iter */
    0,                       /* tp_iternext */
    ConnectionObject_methods,        /* tp_methods */
    0,                         /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    0,                      /* tp_init */
    0,                         /* tp_alloc */
    0,                           /* tp_new */
};
