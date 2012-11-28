#include "result.h"

static PyObject*
get_column_desc(int i, drizzle_column_type_t *types, drizzle_column_st *column)
{

    PyObject *o = NULL;

    const char *name;
    drizzle_column_type_t type;
    drizzle_column_flags_t flags;
    drizzle_charset_t charset;
    uint32_t size;
    uint8_t decimals = 0, null_ok = 0;
    size_t max_size;

    name = drizzle_column_name(column);
    DEBUG("name:%s", name);

    type = drizzle_column_type(column);
    DEBUG("type:%u", type);

    size = drizzle_column_size(column);
    DEBUG("size:%u", size);
    
    max_size = drizzle_column_size(column);
    DEBUG("max_size:%" PRIu64, max_size);

    charset = drizzle_column_charset(column);
    DEBUG("charset:%u", charset);

    decimals = drizzle_column_decimals(column);
    DEBUG("decimals:%d", decimals);
    
    flags = drizzle_column_flags(column);
    DEBUG("flags:%d", flags);

    if (flags & DRIZZLE_COLUMN_FLAGS_NOT_NULL) {
        null_ok = 0;
    } else {
        null_ok = 1;
    }
    
    types[i] = type;
    o = Py_BuildValue("(siiiiii)",
            name, type, 0, size, size, decimals, null_ok);
    return o;
}

static PyObject* 
get_desc(result_t *r, drizzle_con_st *con, drizzle_result_st *result)
{
    int status = 1, desclen = 0;
    uint16_t size = 0;
    drizzle_return_t ret;
    drizzle_column_st column;
    drizzle_column_st *c;
    PyObject *c_info = NULL;
    PyObject *desc = NULL;
    drizzle_column_type_t *types = NULL;
    
    size = drizzle_result_column_count(result);
    DEBUG("column count:%d" , size);
    
    r->column_count = size;
    types = PyMem_Malloc(sizeof(drizzle_column_type_t) * size);
    if (types == NULL) {
        return NULL;
    }
    r->types = types;

    desc = PyTuple_New(size);
    if (desc == NULL) {
        return NULL;
    }

    while (1) {
        c = drizzle_column_read(result, &column, &ret);
        status = io_wait(con, ret);
        if (status == -1) {
            goto error;
        }
        if (c == NULL) {
            break;
        } else {
            c_info = get_column_desc(desclen, types, &column);
            if (c_info == NULL) {
                goto error;
            }
            PyTuple_SetItem(desc, desclen, c_info);
            c_info = NULL;
            drizzle_column_free(c);
            desclen++;
        }
    }
    return desc;
error:
    if (c) {
        drizzle_column_free(c);
    }
    if (c_info) {
        Py_DECREF(c_info);
    }
    if (desc) {
        Py_DECREF(desc);
    }
    return NULL;
}

static PyObject*
read_fields(drizzle_con_st *con, drizzle_result_st *result)
{
    int status = 1;    
    size_t offset = 0, size = 0, total = 0;
    drizzle_return_t ret;
    drizzle_field_t field;
    
    while(status){
        field = drizzle_field_read(result, &offset, &size, &total, &ret);
        if (ret == DRIZZLE_RETURN_ROW_END) {
            break;
        }
        status = io_wait(con, ret);
        if (status == -1){
            goto error;
        }
    }
    
    if (field == NULL) {
        Py_RETURN_NONE;
    } else if (offset > 0) {

    } else {

    }
    if (field) {
        drizzle_field_free(field);
    }
    
    Py_RETURN_NONE;
error:

    return NULL;
}

static int
read_row(drizzle_con_st *con, drizzle_result_st *result)
{
    int status = 1;    
    uint64_t read_row = 0;
    drizzle_return_t ret;
    PyObject *field = NULL;

    while(status){
        read_row = drizzle_row_read(result, &ret);
        status = io_wait(con, ret);
        if (status == -1){
            goto error;
        }
    }
    if (read_row == 0) {
        return 0;
    }
    field = read_fields(con, result);

    return 1;
error:
    return -1;
}

result_t*
new_result_t(drizzle_con_st *con, drizzle_result_st *result)
{
    result_t *r = NULL;
    PyObject *desc = NULL;

    r = PyMem_Malloc(sizeof(result_t));
    if (r == NULL) {
        return NULL;
    }
    
    memset(r, 0, sizeof(result_t));
    
    desc = get_desc(r, con, result);
    if (desc == NULL) {
        PyMem_Free(r);
        return NULL;
    }

    r->con = con;
    r->result = result;
    r->description = desc;
    return r;
}

PyObject*
fetchone(result_t *res)
{
    Py_RETURN_NONE;
}

void free_result_t(result_t *res)
{
    if (res->description) {
        Py_CLEAR(res->description);
    }

    if (res->types) {
        PyMem_Free(res->types);
    }

    drizzle_result_free(res->result);

    PyMem_Free(res);
}
