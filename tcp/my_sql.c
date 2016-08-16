#include <Python.h>
#include "my_types.h"
#include "my_sql.h"

void userLogin(char *addr, char *user, struct sys_conf *conf)
{
    Py_Initialize();

    PyObject *info, *sql_conf;
    PyObject *p_module, *p_func;
    PyObject *res;

    p_module = PyImport_Import(PyString_FromString("login"));
    p_func = PyObject_GetAttrString(p_module, "user_login");

    info = Py_BuildValue("(ss)", addr, user);
    sql_conf = Py_BuildValue("{s:s,s:i,s:s,s:s,s:s}",
            "host", conf->host, "port", conf->port,
            "user", conf->user, "passwd", conf->passwd,
            "db", conf->db);

    PyObject_Call(p_func, info, sql_conf);

    Py_Finalize();
}

void userLogout(char *user, struct sys_conf *conf)
{
    Py_Initialize();
    PyObject *info, *sql_conf;
    PyObject *p_module, *p_func;
    PyObject *res;
    p_module = PyImport_Import(PyString_FromString("logout"));
    p_func = PyObject_GetAttrString(p_module, "user_logout");

    info = PyString_FromString(user);
    sql_conf = Py_BuildValue("{s:s,s:i,s:s,s:s,s:s}",
            "host", conf->host, "port", conf->port,
            "user", conf->user, "passwd", conf->passwd,
            "db", conf->db);

    res = PyObject_Call(p_func, info, sql_conf);
    debug("++++++++++++\n");
    //debug("%s\n", PyString_AsString(res));
    //PyString_AsString(res);

    Py_Finalize();
}
