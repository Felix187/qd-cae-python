
#include <Python.h>
#include <numpy/arrayobject.h>
//#include "FEMFile_py.cpp"
#include "D3plot_py.hpp"
#include "Node_py.hpp"
#include "Element_py.hpp"
#include "Part_py.hpp"
#include "KeyFile_py.hpp"
#include <limits>
#include <string.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <algorithm>
#include <set>
#include "../utility/TextUtility.hpp"
#include "../utility/QD_Time.hpp"
#include "../dyna/KeyFile.hpp"
#include "../dyna/D3plot.hpp"
#include "../db/DB_Elements.hpp"
#include "../db/DB_Nodes.hpp"
#include "../db/DB_Parts.hpp"
#include "../db/Node.hpp"
#include "../db/Part.hpp"
#include "../db/Element.hpp"

using namespace std;

// Python2 or Python3?
#if PY_MAJOR_VERSION >= 3
#define ISPY3
#endif

// function termination for error stuff
#ifdef ISPY3
#define RETURN_INIT_ERROR return NULL
#define PyInt_Check(arg) PyLong_Check(arg)
#else
#define RETURN_INIT_ERROR return
#endif

// Wrapper functions for PY3 and PY2
namespace qd{
// PY3
#ifdef ISPY3
inline int isPyStr(PyObject* obj){return PyUnicode_Check(obj);}
inline char* PyStr2char(PyObject* arg){return PyUnicode_AsUTF8(arg);}
// PY2
#else
inline int isPyStr(PyObject* obj){return PyString_Check(obj);}
inline char* PyStr2char(PyObject* arg){return PyString_AsString(arg);}
#endif
}


/** Convert a c++ 2D vector into a numpy array
 *
 * @param const vector< vector<T> >& vec : 2D vector data
 * @return PyArrayObject* array : converted numpy array
 *
 * Transforms an arbitrary 2D C++ vector into a numpy array. Throws in case of
 * unregular shape. The array may contain empty columns or something else, as
 * long as it's shape is square.
 *
 * Warning this routine makes a copy of the memory!
 */
template<typename T>
static PyArrayObject* vector_to_nparray(const vector< vector<T> >& vec, int type_num = PyArray_FLOAT){

   // rows not empty
   if( !vec.empty() ){

      // column not empty
      if( !vec[0].empty() ){

        size_t nRows = vec.size();
        size_t nCols = vec[0].size();
        npy_intp dims[2] = {nRows, nCols};
        PyArrayObject* vec_array = (PyArrayObject *) PyArray_SimpleNew(2, dims, type_num);

        T *vec_array_pointer = (T*) PyArray_DATA(vec_array);

        // copy vector line by line ... maybe could be done at one
        for (size_t iRow=0; iRow < vec.size(); ++iRow){

          if( vec[iRow].size() != nCols){
             Py_DECREF(vec_array); // delete
             throw(string("Can not convert vector<vector<T>> to np.array, since c++ matrix shape is not uniform."));
          }

          copy(vec[iRow].begin(),vec[iRow].end(),vec_array_pointer+iRow*nCols);
        }

        return vec_array;

     // Empty columns
     } else {
        npy_intp dims[2] = {vec.size(), 0};
        return (PyArrayObject*) PyArray_ZEROS(2, dims, PyArray_FLOAT, 0);
     }


   // no data at all
   } else {
      npy_intp dims[2] = {0, 0};
      return (PyArrayObject*) PyArray_ZEROS(2, dims, PyArray_FLOAT, 0);
   }

}


/** Convert a c++ vector into a numpy array
 *
 * @param const vector<T>& vec : 1D vector data
 * @return PyArrayObject* array : converted numpy array
 *
 * Transforms an arbitrary C++ vector into a numpy array. Throws in case of
 * unregular shape. The array may contain empty columns or something else, as
 * long as it's shape is square.
 *
 * Warning this routine makes a copy of the memory!
 */
template<typename T>
static PyArrayObject* vector_to_nparray(const vector<T>& vec, int type_num = PyArray_FLOAT){

   // rows not empty
   if( !vec.empty() ){

       size_t nRows = vec.size();
       npy_intp dims[1] = {nRows};

       PyArrayObject* vec_array = (PyArrayObject *) PyArray_SimpleNew(1, dims, type_num);
       T *vec_array_pointer = (T*) PyArray_DATA(vec_array);

       copy(vec.begin(),vec.end(),vec_array_pointer);
       return vec_array;

   // no data at all
   } else {
      npy_intp dims[1] = {0};
      return (PyArrayObject*) PyArray_ZEROS(1, dims, PyArray_FLOAT, 0);
   }

}


/** Test if a PyObject is an integral (int or long)
 * @param PyObject* obj
 * @return bool isIntegral
 */
static bool
PyObject_isIntegral(PyObject* obj){

  if( PyLong_Check(obj) ){
    return true;

  #ifndef ISPY3
  } else if(PyInt_Check(obj)){
    return true;
  #endif

  } else {
    return false;
  }

}

/** convert_obj_to_int: cast python object to long with checks
 *
 * @param PyObject* item : python object to convert
 * @return int ret : converted long
 *
 * The checks usually are done in python > 3 automatically but in python27
 * one has to deal with dirty stuff oneself.
 */
static int
convert_obj_to_int(PyObject* item){

  // check type
  PyObject_isIntegral(item);

  // convert
  long nodeID_long = -1;
  if( PyLong_Check(item) ){
    nodeID_long = PyLong_AsLong(item); 

  #ifndef ISPY3
  } else if(PyInt_Check(item)){
    nodeID_long = PyInt_AsLong(item);
  #endif
  } 

  // Overflow cast check
  //if(nodeID_long > (long) std::numeric_limits<int>::max){
  if(nodeID_long > (long) INT_MAX){
    throw(string("Integer overflow error."));
  //} else if (nodeID_long < (long) std::numeric_limits<int>::min){
  } else if (nodeID_long < (long) INT_MIN){
    throw(string("Integer underflow error."));
  }

  return (int) PyLong_AsLong(item);

}



extern "C" {


  // Include all the source implementations code
  //
  // This is a little weird, since they are just copied in here,
  // but who cares as long as it works fine.
  #include "FEMFile_py.cpp"
  #include "KeyFile_py.cpp"
  #include "D3plot_py.cpp"
  #include "Node_py.cpp"
  #include "Element_py.cpp"
  #include "Part_py.cpp"

  /*******************************************************/
  /*                                                     */
  /*                   Q D _ M O D U L E                 */
  /*                                                     */
  /*******************************************************/

  static PyObject *
  test_codie(PyObject *self, PyObject *args)
  {
   return Py_None;
  }

  /* MODULE codie function table */
  static PyMethodDef QDMethods[] = {
    {"test_codie",  test_codie, METH_VARARGS,"Used for debugging sometimes."},
    {NULL, NULL, 0, NULL}        /* Sentinel */
  };

  /* MODULE (Python3 only) */
  #ifdef ISPY3
  static PyModuleDef dyna_cpp_module = {
    PyModuleDef_HEAD_INIT,
    "dyna_cpp",
    "qd cae routines for LS-DYNA.",
    -1,
	  QDMethods
    //NULL, NULL, NULL, NULL, NULL
  };
  #endif

  /* MODULE INIT */
  #ifdef ISPY3
  PyMODINIT_FUNC
  PyInit_dyna_cpp(void)
  #else
  void
  initdyna_cpp(void)
  #endif
  {
    PyObject* m;

    // Constructor
    if (PyType_Ready(&QD_D3plot_Type) < 0)
      RETURN_INIT_ERROR;
      // PY3 return NULL;
      //return;

    if (PyType_Ready(&QD_Node_Type) < 0)
      RETURN_INIT_ERROR;

    if (PyType_Ready(&QD_Element_Type) < 0)
      RETURN_INIT_ERROR;

    if (PyType_Ready(&QD_Part_Type) < 0)
      RETURN_INIT_ERROR;

   if (PyType_Ready(&QD_KeyFile_Type) < 0)
      RETURN_INIT_ERROR;

    // Init Module
    #ifdef ISPY3
    m = PyModule_Create(&dyna_cpp_module);
    #else
    m = Py_InitModule3("dyna_cpp", QDMethods,
                       "qd cae routines for LS-DYNA.");
    #endif

    import_array();

    if (m == NULL) 
      RETURN_INIT_ERROR;

    Py_INCREF(&QD_D3plot_Type);
    PyModule_AddObject(m, "QD_D3plot", (PyObject *)&QD_D3plot_Type);

    Py_INCREF(&QD_KeyFile_Type);
    PyModule_AddObject(m, "KeyFile", (PyObject *)&QD_KeyFile_Type);

    Py_INCREF(&QD_Node_Type);
    PyModule_AddObject(m, "Node", (PyObject *)&QD_Node_Type);

    Py_INCREF(&QD_Element_Type);
    PyModule_AddObject(m, "Element", (PyObject *)&QD_Element_Type);

    Py_INCREF(&QD_Element_Type);
    PyModule_AddObject(m, "Part", (PyObject *)&QD_Part_Type);

    #ifdef ISPY3
    return m;
    #endif
  }

}