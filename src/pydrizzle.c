#include "pydrizzle.h"

static PyMethodDef _pydrizzle_methods[] = {
	{NULL, NULL} /* sentinel */
};

PyMODINIT_FUNC
init_pydrizzle(void)
{
    PyObject *m;
	m = Py_InitModule3("_pydrizzle", _pydrizzle_methods, NULL);
	if (m == NULL){
		return;
    }

    if(PyType_Ready(&ConnectionObjectType) < 0){ 
        return;
    }


}

