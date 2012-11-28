#include "buffer.h"

#define LIMIT_MAX 1024 * 1024 * 1024


buffer_t*
new_buffer(size_t buf_size, size_t limit)
{
    buffer_t *buf;

    buf = PyMem_Malloc(sizeof(buffer_t));
    if (buf == NULL) {
        return NULL;
    }
    
    memset(buf, 0, sizeof(buffer_t));

    buf->buf = PyMem_Malloc(sizeof(char) * buf_size);
    if (buf->buf == NULL) {
        PyMem_Free(buf);
        return buf;
    }

    buf->buf_size = buf_size;
    if(limit){
        buf->limit = limit;
    }else{
        buf->limit = LIMIT_MAX;
    }
    return buf;
}

buffer_result
write2buf(buffer_t *buf, const char *c, size_t l) {

    size_t newl;
    char *newbuf;
    buffer_result ret = WRITE_OK;
    newl = buf->len + l;

    if (l == 0) {
        return ret;
    }

    if (newl >= buf->buf_size) {
        buf->buf_size *= 2;
        if(buf->buf_size <= newl) {
            buf->buf_size = (int)(newl + 1);
        }
        if(buf->buf_size > buf->limit){
            buf->buf_size = buf->limit + 1;
        }
        newbuf = (char*)PyMem_Realloc(buf->buf, buf->buf_size);
        if (!newbuf) {
            PyErr_SetString(PyExc_MemoryError,"out of memory");
            PyMem_Free(buf->buf);
            buf->buf = 0;
            buf->buf_size = buf->len = 0;
            return MEMORY_ERROR;
        }
        buf->buf = newbuf;
    }
    if(newl >= buf->buf_size){
        l = buf->buf_size - buf->len -1;
        ret = LIMIT_OVER;
    }
    memcpy(buf->buf + buf->len, c , l);
    buf->len += (int)l;
    return ret;
}

void
free_buffer(buffer_t *buf)
{
    PyMem_Free(buf->buf);
    PyMem_Free(buf);
}

/*
PyObject *
getPyString(buffer_t *buf)
{
    PyObject *o;
    o = PyBytes_FromStringAndSize(buf->buf, buf->len);
    free_buffer(buf);
    return o;
}

char *
getString(buffer_t *buf)
{
    buf->buf[buf->len] = '\0';
    return buf->buf;
}
*/




