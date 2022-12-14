#pragma once
// from here: https://stackoverflow.com/a/37264642/607407

#include <iostream>

#ifndef NDEBUG
#   define assertm(Expr, Msg) \
    __M_Assert(#Expr, Expr, __FILE__, __LINE__, Msg)
#else
#   define assertm(Expr, Msg) ;
#endif

void __M_Assert(const char* expr_str, bool expr, const char* file, int line, const char* msg)
{
    if (!expr)
    {
        std::cerr << "Assert failed:\t" << msg << "\n"
            << "Expected:\t" << expr_str << "\n"
            << "Source:\t\t" << file << ":" << line << "\n";
        abort();
    }
}