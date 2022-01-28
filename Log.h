//------------------------------------------------------------------------|
// Copyright (c) 2016-2018, 2022 by Raymond M. Foulk IV
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
// CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
// TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
// SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//------------------------------------------------------------------------|

//------------------------------------------------------------------------|
// http://stackoverflow.com/questions/19415845/
//     a-better-log-macro-using-template-metaprogramming
// http://stackoverflow.com/questions/7398310/
//     how-to-get-only-file-name-in-preprocessor

#include <stdint.h>
#include <iostream>
#include <libgen.h>
#include <string.h>

struct None { };

//------------------------------------------------------------------------|
template <typename First, typename Second> struct Pair
{
    First first;
    Second second;
};

//------------------------------------------------------------------------|
template <typename List> struct LogData
{
    List list;
};

//------------------------------------------------------------------------|
template <typename Begin, typename Value>
		LogData< Pair<Begin, const Value &> >
    	operator<<(LogData<Begin> begin, const Value &value)
{
    return {{begin.list, value}};
}

//------------------------------------------------------------------------|
template <typename Begin, size_t n>
LogData< Pair<Begin, const char *> >
operator<<(LogData<Begin> begin, const char (&value)[n])
{
    return {{begin.list, value}};
}

//------------------------------------------------------------------------|
inline void printList(std::ostream &os, None)
{
}

//------------------------------------------------------------------------|
template <typename Begin, typename Last>
void printList(std::ostream &os, const Pair<Begin, Last> &data)
{
    printList(os, data.first);
    os << data.second;
}

//------------------------------------------------------------------------|
template <typename List>
void log(const char *file, const char *func, const LogData<List> &data)
{
    char * fileName = strdup(basename((char *) file));
    char * dontCare = NULL;
    fileName = strtok_r(fileName, ".", &dontCare);
    std::cout << fileName << ": " << func << "(): ";
    free(fileName);
    printList(std::cout, data.list);
    std::cout << "\n";
}

//------------------------------------------------------------------------|
#define LOG(x) (log(__FILE__, __FUNCTION__, LogData<None>() << x))
