#ifndef DVIDEINTO_H
#define DVIDEINTO_H

#include <vector>
#include <iterator>

template<typename Iterator>
std::vector<Iterator> divideInto(int n, Iterator begin, Iterator end)
{
    std::vector<Iterator> result;
    const int             length         = std::distance(begin, end);
    const int             partial_length = (length + n - 1) / n;

    for(Iterator i = begin; i < end; std::advance(i, partial_length))
    {
        result.push_back(i);
    }
    result.push_back(end);

    return result;
}

#endif//DVIDEINTO_H
