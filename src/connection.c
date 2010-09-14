#include "pydrizzle.h"

static int
ConnectionObject_init(ConnectionObject *self, PyObject *args, PyObject *kwargs)
{
    drizzle_st *drizzle;

    //Not support
	PyObject *ssl = NULL;
	char *host = DRIZZLE_DEFAULT_TCP_HOST;
    char *user = NULL, *passwd = NULL, *db = NULL, *unix_socket = NULL;
	unsigned int port = DRIZZLE_DEFAULT_TCP_PORT_MYSQL;
	unsigned int client_flag = 0;
	int connect_timeout = 0;

	static char *kwlist[] = { "host", "user", "passwd", "db", "port",
				  "unix_socket", "connect_timeout", 
				  NULL } ;
	
	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|ssssisi:connect",
					 kwlist,
					 &host, &user, &passwd, &db,
					 &port, &unix_socket,
					 &connect_timeout
					 )){
		return -1;
    }
	
    Py_BEGIN_ALLOW_THREADS ;
	if(!drizzle_create(&(self->drizzle))){
        //TODO set error
        return -1;
    }

    //set NON_BLOKING
    drizzle_add_options(&(self->drizzle), DRIZZLE_NON_BLOCKING);

	if (connect_timeout) {
		drizzle_set_timeput(&(self->drizzle), connect_timeout);
	}
    if(!unix_socket){
	    if(!drizzle_con_add_tcp(&(self->drizzle), &(self->con), host, port, user, passwd, db, DRIZZLE_CON_MYSQL)) {
            fprintf(stderr, "drizzle_con_add_tcp:%s\n", drizzle_error(&drizzle));
            exit(1);
        }
    }else{
	    if(!drizzle_con_add_uds(&(self->drizzle), &(self->con), unix_socket, user, passwd, db, DRIZZLE_CON_MYSQL)) {
            fprintf(stderr, "drizzle_con_add_tcp:%s\n", drizzle_error(&drizzle));
            exit(1);
        }
    
    }
    Py_END_ALLOW_THREADS ;
    
    return 1;
    
}

static inline void
ConnectionObject_dealloc(ConnectionObject *self)
{
    return;
}

static PyMethodDef ConnectionObject_methods[] = {
    {NULL, NULL} /* sentinel */
};

PyTypeObject ConnectionObjectType = {
	PyObject_HEAD_INIT(&PyType_Type)
    0,
    "_pydrizzle.connection",             /*tp_name*/
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
    0,		               /* tp_traverse */
    0,		               /* tp_clear */
    0,		               /* tp_richcompare */
    0,		               /* tp_weaklistoffset */
    0,		               /* tp_iter */
    0,		                   /* tp_iternext */
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
