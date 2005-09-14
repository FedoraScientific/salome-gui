//  SALOME SALOMEGUI : implementation of desktop and GUI kernel
//
//  Copyright (C) 2003  CEA/DEN, EDF R&D
//
//
//
//  File   : PyInterp_base.cxx
//  Author : Christian CAREMOLI, Paul RASCLE, EDF
//  Module : SALOME
//  $Header$


#include <string>
#include <vector>

#include "PyInterp_base.h" // this include must be first (see PyInterp_base.h)!
#include <cStringIO.h>

using namespace std;

PyLockWrapper::PyLockWrapper(PyThreadState* theThreadState): 
  myThreadState(theThreadState),
  mySaveThreadState(0)
{
  PyEval_AcquireLock();
  mySaveThreadState = PyThreadState_Swap(myThreadState); // store previous current in save,
                                                         // set local in current
}


PyLockWrapper::~PyLockWrapper(){
  PyThreadState_Swap(mySaveThreadState); // restore previous current (no need to get local,
  PyEval_ReleaseLock();                  // local thread state* already in _tstate
}


class PyReleaseLock{
public:
  ~PyReleaseLock(){
    PyEval_ReleaseLock();
  }
};


PyLockWrapper PyInterp_base::GetLockWrapper(){
  return _tstate;
}


// main python interpreter

//PyThreadState *PyInterp_base::_gtstate = 0; // force 0 before execution
int PyInterp_base::_argc = 1;
char* PyInterp_base::_argv[] = {""};

PyObject *PyInterp_base::builtinmodule = NULL;


/*!
 * basic constructor here : herited classes constructors must call initalize() method
 * defined here.
 */
PyInterp_base::PyInterp_base(): _tstate(0), _vout(0), _verr(0), _g(0), _atFirst(true)
{
}

PyInterp_base::~PyInterp_base()
{
  PyLockWrapper aLock(_tstate);
  //Py_EndInterpreter(_tstate);
}


/*!
 * Must be called by herited classes constructors. initialize() calls virtuals methods
 * initstate & initcontext, not defined here in base class. initstate & initcontext methods
 * must be implemented in herited classes, following the Python interpreter policy
 * (mono or multi interpreter...).
 */
void PyInterp_base::initialize()
{
  _history.clear();       // start a new list of user's commands 
  _ith = _history.begin();

  init_python();
  // Here the global lock is released

  // The lock will be acquired in initState. Make provision to release it on exit
  PyReleaseLock aReleaseLock;

  initState();
  initContext();

  // used to interpret & compile commands
  PyObjWrapper m(PyImport_ImportModule("codeop"));
  if(!m){
    PyErr_Print();
    return;
  }   
  
  // Create cStringIO to capture stdout and stderr
  PycString_IMPORT;
  //PycStringIO = (PycStringIO_CAPI *)xxxPyCObject_Import("cStringIO", "cStringIO_CAPI");
  _vout = PycStringIO->NewOutput(128);
  _verr = PycStringIO->NewOutput(128);
  
  // All the initRun outputs are redirected to the standard output (console)
  initRun();
}

void PyInterp_base::init_python()
{
  static PyThreadState *_gtstate = 0;

  _atFirst = false;
  if (Py_IsInitialized())
    return;

  Py_SetProgramName(_argv[0]);
  Py_Initialize(); // Initialize the interpreter
  PySys_SetArgv(_argc, _argv);
  PyEval_InitThreads(); // Create (and acquire) the interpreter lock
  _gtstate = PyEval_SaveThread(); // Release global thread state
//  if(!_gtstate){
//    PyReleaseLock aReleaseLock;
//    Py_Initialize(); // Initialize the interpreter
//    PyEval_InitThreads(); // Initialize and acquire the global interpreter lock
//    PySys_SetArgv(_argc,_argv); // initialize sys.argv
//    _gtstate = PyThreadState_Get();
//  }
}

string PyInterp_base::getbanner()
{
  string aBanner("Python ");
  aBanner = aBanner + Py_GetVersion() + " on " + Py_GetPlatform() ;
  aBanner = aBanner + "\ntype help to get general information on environment\n";
  return aBanner;
}


int PyInterp_base::initRun()
{
  PySys_SetObject("stderr",_verr);
  PySys_SetObject("stdout",_vout);

  PyObjWrapper verr(PyObject_CallMethod(_verr,"reset",""));
  PyObjWrapper vout(PyObject_CallMethod(_vout,"reset",""));

  //PyObject *m = PyImport_GetModuleDict();
  
  PySys_SetObject("stdout",PySys_GetObject("__stdout__"));
  PySys_SetObject("stderr",PySys_GetObject("__stderr__"));

  return 0;
}


/*!
 * This function compiles a string (command) and then evaluates it in the dictionnary
 * context if possible.
 * Returns :
 * -1 : fatal error 
 *  1 : incomplete text
 *  0 : complete text executed with success
 */
int compile_command(const char *command,PyObject *context)
{
  PyObject *m = PyImport_AddModule("codeop");
  if(!m){ // Fatal error. No way to go on.
    PyErr_Print();
    return -1;
  }
  PyObjWrapper v(PyObject_CallMethod(m,"compile_command","s",command));
  if(!v){
    // Error encountered. It should be SyntaxError,
    //so we don't write out traceback
    PyObjWrapper exception, value, tb;
    PyErr_Fetch(&exception, &value, &tb);
    PyErr_NormalizeException(&exception, &value, &tb);
    PyErr_Display(exception, value, NULL);
    return -1;
  }else if (v == Py_None){
    // Incomplete text we return 1 : we need a complete text to execute
    return 1;
  }else{
    // Complete and correct text. We evaluate it.
#ifndef WNT
    PyObjWrapper r(PyEval_EvalCode(v,context,context));
#else
    PyObjWrapper r(PyEval_EvalCode((PyCodeObject *)(void *)v,context,context));
#endif
    if(!r){
      // Execution error. We return -1
      PyErr_Print();
      return -1;
    }
    // The command has been successfully executed. Return 0
    return 0;
  }
}


int PyInterp_base::run(const char *command)
{
  if(_atFirst){
    int ret = 0;
    ret = simpleRun("from Help import *");
    if (ret) { 
      _atFirst = false;
      return ret;
    }
    ret = simpleRun("import salome");
    if (ret) { 
      _atFirst = false;
      return ret;
    }
    ret = simpleRun("salome.salome_init()");
    if (ret) { 
      _atFirst = false;
      return ret;
    }
    _atFirst = false;
  }
  return simpleRun(command);
}


int PyInterp_base::simpleRun(const char *command)
{
  if( !_atFirst && strcmp(command,"") != 0 ) {
    _history.push_back(command);
    _ith = _history.end();
  }

  // We come from C++ to enter Python world
  // We need to acquire the Python global lock
  //PyLockWrapper aLock(_tstate); // san - lock is centralized now

  // Reset redirected outputs before treatment
  PySys_SetObject("stderr",_verr);
  PySys_SetObject("stdout",_vout);
    
  PyObjWrapper verr(PyObject_CallMethod(_verr,"reset",""));
  PyObjWrapper vout(PyObject_CallMethod(_vout,"reset",""));
  
  int ier = compile_command(command,_g);

  // Outputs are redirected on standards outputs (console)
  PySys_SetObject("stdout",PySys_GetObject("__stdout__"));
  PySys_SetObject("stderr",PySys_GetObject("__stderr__"));
  return ier;
}


const char * PyInterp_base::getPrevious()
{
  if(_ith != _history.begin()){
    _ith--;
    return (*_ith).c_str();
  }
  else
    return BEGIN_HISTORY_PY;
}


const char * PyInterp_base::getNext()
{
  if(_ith != _history.end()){
    _ith++;
  }
  if (_ith == _history.end())
    return TOP_HISTORY_PY;
  else
    return (*_ith).c_str();
}


string PyInterp_base::getverr(){ 
  //PyLockWrapper aLock(_tstate);
  PyObjWrapper v(PycStringIO->cgetvalue(_verr));
  string aRet(PyString_AsString(v));
  return aRet;
}


string PyInterp_base::getvout(){  
  //PyLockWrapper aLock(_tstate);
  PyObjWrapper v(PycStringIO->cgetvalue(_vout));
  string aRet(PyString_AsString(v));
  return aRet;
}
 
