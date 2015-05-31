//
//  nbfuncs.h
//  nbcross
//
//  Created by Philip Koch on 11/28/14.
//

#ifndef __nbcross__nbfuncs__
#define __nbcross__nbfuncs__

#include <stdio.h>
#include <iostream>
#include <vector>

#include "Log.h"
#include "nbdebug.h"

#define NBCROSS_VERSION 6

class CrossFunc {
    
public:
    std::string name;
    int (*func)(void);
    std::vector<std::string> args;
    
    CrossFunc(std::string n,  int (*f)(void), std::vector<std::string> a) :
    name(n),
    func(f),
    args(a)
    { }
};

// Defined in pkoch_defs.cpp
int test_func();
int arg_test_func();
int CrossBright_func();

// Defined in misc_defs.cpp

// Defined in vision_defs.cpp
int ImageConverter_func();
int Edges_func();

/* add all functions to publish to this vector */
static const std::vector<CrossFunc> FUNCS = {
    CrossFunc("test", test_func, {}),
    CrossFunc("arg_test", arg_test_func, {"YUVImage", "YUVImage"}),
    CrossFunc("CrossBright", CrossBright_func, {"YUVImage"}),
    CrossFunc("ImageConverter", ImageConverter_func, {"YUVImage"}),
    CrossFunc("Edges", Edges_func, {"YUVImage"})
};

extern std::vector<nblog::Log *> args;
extern std::vector<nblog::Log *> rets;

#endif /* defined(__nbcross__nbfuncs__) */
