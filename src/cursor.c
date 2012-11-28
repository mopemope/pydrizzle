#include "cursor.h"
#include "converter.h"
#include "buffer.h"


static int
execute_simple_query(CursorObject *self, char *query, size_t size)
{
    int status = 1;
    drizzle_return_t ret;
    drizzle_result_st *res = NULL;
    result_t *result = NULL;

    DEBUG("query:%s size:%d", query, (int)size);
    DEBUG("use drizzle_con_st:%p", self->con);
    while(status){
        res = drizzle_query(self->con, NULL, query, size, &ret);
        DEBUG("execute:%.*s", (int)size, query);
        DEBUG("execute result:%p:ret:%d", res, ret);
        status = io_wait(self->con, ret);
        DEBUG("status:%d", status);
        if (status == -1){
            goto error;
        }
    }
    
    result = new_result_t(self->con, res);
    if (result == NULL) {
        goto error;
    }
    self->result = result;
    
    return 1;
error:
    PyErr_SetString(PyExc_IOError, drizzle_error(drizzle));
    if (res) {
        drizzle_result_free(res);
    }
    return -1;
}

PyObject*
CursorObject_new(PyObject *conObj, drizzle_con_st *con)
{

    CursorObject *cursor;

    cursor = (CursorObject *)PyObject_NEW(CursorObject, &CursorObjectType);
    if (cursor == NULL) {
       return NULL;
    }
    
    GDEBUG("alloc CursorObject %p", cursor);
    cursor->connection = conObj;
    cursor->con = con;
    cursor->result = NULL;

    return (PyObject*)cursor;
}

static void
CursorObject_dealloc(CursorObject *self)
{
    GDEBUG("dealloc CursorObject %p", self);

    PyObject_DEL(self);
}

static PyObject*
CursorObject_close(CursorObject *self)
{
    Py_RETURN_NONE;
}


static PyObject*
CursorObject_execute(CursorObject *self, PyObject *args, PyObject *kwargs)
{

    PyObject *qargs = NULL;
    char *query = NULL;
    int len, ret;
    buffer_t *buf;

    if (!PyArg_ParseTuple(args, "s#|O", &query, &len, &qargs)) {
        return NULL;
    }
    //TODO qargs type check
    //
    if (self->result) {
        free_result_t(self->result);
        self->result = NULL;
    }
    DEBUG("query:%s", query);
    DEBUG("qargs:%p", qargs);

    buf = get_converted_query(query, len, qargs);
    if (buf == NULL) {
        return NULL;
    }
    
    DEBUG("converted query:%.*s", (int)buf->len, buf->buf);
    ret = execute_simple_query(self, buf->buf, buf->len);
    free_buffer(buf);
    if (ret == -1) {
        return NULL;
    }

    Py_RETURN_NONE;
}



static PyObject*
CursorObject_fetchone(CursorObject *self)
{

    if (!self->result){
        //TODO Error
        return NULL;
    }

    Py_RETURN_NONE;
}

static PyMethodDef CursorObject_methods[] = {
    {"close", (PyCFunction)CursorObject_close, METH_NOARGS, 0},
    {"execute", (PyCFunction)CursorObject_execute, METH_VARARGS, 0},
    {"fetchone", (PyCFunction)CursorObject_fetchone, METH_NOARGS, 0},
    {NULL, NULL} /* sentinel */
};

static PyMemberDef cursor_members[] = {
    {0},
};

PyTypeObject CursorObjectType = {
#ifdef PY3
    PyVarObject_HEAD_INIT(NULL, 0)
#else
    PyObject_HEAD_INIT(NULL)
    0,                    /* ob_size */
#endif
    MODULE_NAME ".Cursor",             /*tp_name*/
    sizeof(CursorObject), /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)CursorObject_dealloc, /*tp_dealloc*/
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
    CursorObject_methods,        /* tp_methods */
    cursor_members,                         /* tp_members */
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
