#if (defined(LINUX_USER_MODE))
#include <Python.h>
#include <libobject/core/try.h>
#include <libobject/core/utils/registry/registry.h>

static int test_python()
{
    int ret;
    PyObject *pArgs = NULL;
    PyObject *pResult = NULL;
    PyObject *pfunc = NULL;
    PyObject *pmodule = NULL;

    TRY {
        Py_Initialize(); //初始化
        if (!Py_IsInitialized()) {
            return -1; //init python failed
        }
        //相当于在python中的import sys语句。这个函数是宏，相当于直接运行python语句
        PyRun_SimpleString("import sys");
        PyRun_SimpleString("sys.path.append('./3rd/test_python/')");//是将搜索路径设置为当前目录。
        pmodule = PyImport_ImportModule("test_python");  //导入hello.py
        THROW_IF(pmodule == NULL, -1);
    
        pfunc = PyObject_GetAttrString(pmodule, "func1"); //导入func1函数
        THROW_IF(pfunc == NULL, -1);
        
        // 向Python传参数是以元组（tuple）的方式传过去的，
        // 因此我们实际上就是构造一个合适的Python元组就可以了
        // 要用到PyTuple_New，Py_BuildValue，PyTuple_SetItem等几个函数
        /*这个元组其实只是传参的载体
        创建几个元素python函数就是几个参数,这里创建的元组是作为1个参数传递的*/
        pArgs = PyTuple_New(3);
        PyObject *pVender = Py_BuildValue("i", 2);     //构建参数1
        PyObject *pDataID = Py_BuildValue("i", 10001); //构建参数2
        PyObject *pyTupleList = PyTuple_New(2);        //构建参数3,这里创建的元组是作为c的数组
        float arr_f[2];
        arr_f[0] = 78;
        arr_f[1] = 3.41;
        for (int i = 0; i < 2; i++) {
            //这里是把c数组构建成python的元组
            PyTuple_SetItem(pyTupleList, i, Py_BuildValue("f", arr_f[i])); 
        }
    
        //参数入栈
        PyTuple_SetItem(pArgs, 0, pVender);
        PyTuple_SetItem(pArgs, 1, pDataID);
        PyTuple_SetItem(pArgs, 2, pyTupleList);
        
        //调用python脚本函数
        pResult = PyObject_CallObject(pfunc, pArgs);
        int a;
        float b;
        // PyArg_Parse(pResult, "i", &a);
        PyArg_ParseTuple(pResult,"if",&a,&b);
        printf("%d %f\n", a,b);
    } CATCH (ret) {

    } FINALLY {
        if (!pfunc) {
            Py_XDECREF(pmodule);
        }
        if (!pfunc) {
            Py_XDECREF(pfunc);
        }
        if (!pArgs) {
            Py_XDECREF(pArgs);
        }
        if (!pResult) {
            Py_XDECREF(pResult);
        }
        Py_Finalize();
    }
    
    return ret;
}
REGISTER_TEST_FUNC(test_python);
#endif
