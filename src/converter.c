#include "converter.h"


static int
convert_object(PyObject *obj, buffer_t *buf)
{
    return 0;
}

buffer_t*
get_converted_query(char *query, size_t len, PyObject *args)
{
    buffer_t *qbuf;
    char *buf, *st;
    size_t buflen;
    int c, qlen, ret;
    buffer_result bret;
    PyObject *iter = NULL, *item = NULL;


    qbuf = new_buffer(1024 * 4, 0);
    if ( qbuf == NULL) {
        return NULL;
    }
    buf = st = query;
    buflen = len; 
    
    if (args != NULL) {
        iter = PyObject_GetIter(args);
        if (iter == NULL || PyErr_Occurred()) {
            free_buffer(qbuf);
            return NULL;
        }
    }

    while(buflen > 0){
        c = *buf++;
        if (c == '?'){
            qlen = buf - st - 1;
            bret = write2buf(qbuf, st, qlen);
            if (bret != WRITE_OK) {
                goto error;
            }
            if (iter) {
                item =  PyIter_Next(iter);
                if (item == NULL) {
                    //TODO Set Error
                    goto error;
                }
                //ret = convert_object(item);
                if (ret == -1) {
                    goto error;
                }


            }

            st = buf;
        }
        buflen--;

    }
    DEBUG("%s", query);
    qlen = buf - st;
    bret = write2buf(qbuf, st, qlen);
    if (bret != WRITE_OK) {
        goto error;
    }
    
    return qbuf;
error:
    free_buffer(qbuf);
    Py_XDECREF(item);
    Py_XDECREF(iter);
    return NULL;

}
