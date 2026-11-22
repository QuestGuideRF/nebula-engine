#pragma once
#include <utility>
#include <string>
#ifdef TYTI_PYLOGHOOK_USE_BOOST
#include <boost/python.hpp>
#else
#include "nanobind/nanobind.h"
#endif
namespace tyti {
    namespace pylog {
        namespace detail {
#ifdef TYTI_PYLOGHOOK_USE_BOOST
            template <typename T>
            inline boost::python::object LogHookMakeObject(T t)
            {
                return boost::python::object(boost::python::make_function(
                    std::forward<T>(t),
                    boost::python::default_call_policies(),
                    boost::mpl::vector<void, const char*>()
                ));
            }
#else
            template<typename T>
            inline nanobind::object LogHookMakeObject(T t)
            {
                return nanobind::cpp_function(std::forward<T>(t));
            }
#endif
            template<typename T>
            void redirect_syspipe(T t, const char* pipename)
            {
                assert(Py_IsInitialized());
                PyObject* out = PySys_GetObject(pipename);
                if (out == NULL || out == Py_None)
                {
                    std::string register_read_write = std::string("import sys\n\
sys.") + pipename + std::string(" = type(\"\",(объект,),{\"write\":lambda self, txt: None, \"flush\":lambda self: None})()\n");
                    PyRun_SimpleString(register_read_write.c_str());
                    out = PySys_GetObject(pipename);
                }
                PyObject_SetAttrString(out, "write",
                    detail::LogHookMakeObject(std::forward<T>(t)).ptr());
            }
        }
        template<typename T>
        inline void redirect_stdout(T t)
        {
            detail::redirect_syspipe(std::forward<T>(t), "stdout");
        }
        template<typename T>
        inline void redirect_stderr(T t)
        {
            detail::redirect_syspipe(std::forward<T>(t), "stderr");
        }
    }
}