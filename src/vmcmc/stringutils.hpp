/**
 * @file
 *
 * @date 29.07.2016
 * @author marco@kleesiek.com
 */

#ifndef VMCMC_STRINGUTILS_H_
#define VMCMC_STRINGUTILS_H_

#include <vmcmc/typetraits.hpp>

#include <sstream>
#include <string>
#include <utility>

namespace vmcmc {

class Sample;

std::ostream& operator<< (std::ostream& strm, const Sample& sample);

/**
 * Join an STL style container and output to a stream with its values joined by
 * a separator.
 * @param stream The output stream.
 * @param sequence The container to be serialized. Must provide STL style
 * iterators.
 * @param separator The separator to be inserted between printed values.
 * @return Reference to the output stream.
 */
template <class SequenceT, class SeparatorT>
inline std::ostream& join(std::ostream& stream, const SequenceT& sequence, const SeparatorT& separator = "")
{
    auto itBegin = std::begin(sequence);
    auto itEnd = std::end(sequence);

    if (itBegin != itEnd) {
        stream << *itBegin;
        ++itBegin;
    }

    for (; itBegin != itEnd; ++itBegin) {
        stream << separator << *itBegin;
    }

    return stream;
}

/**
 * Join an STL style container to a string.
 * @param sequence The container to be serialized. Must provide STL style
 * iterators.
 * @param separator The separator to be inserted between serialized values.
 * @return The joined string.
 */
template <class SequenceT, class SeparatorT>
inline std::string join(const SequenceT& sequence, const SeparatorT& separator)
{
    std::ostringstream strm;
    join(strm, sequence, separator);
    return strm.str();
}

/**
 * Serialize any STL pair to an output stream.
 * @param strm An output stream.
 * @param pair A pair.
 * @return Reference to the output stream.
 */
template <class T1, class T2>
inline std::ostream& operator<< (std::ostream& strm, const std::pair<T1, T2>& pair)
{
    strm << "(" << pair.first << ", " << pair.second << ")";
    return strm;
}

/**
 * Serialize any STL container to an output stream.
 * @param strm An output stream.
 * @param container A container fulfilling the type trait #vmcmc::is_container.
 * @return Reference to the output stream.
 */
template <class ContainerT, class = typename std::enable_if<vmcmc::is_container<ContainerT>::value>::type>
inline std::ostream& operator<< (std::ostream& strm, const ContainerT& container)
{
    strm << "[" << container.size() << "](";

    auto itBegin = std::begin(container);
    auto itEnd = std::end(container);

    if (itBegin != itEnd) {
        strm << *itBegin;
        itBegin++;
    }

    for (; itBegin != itEnd; ++itBegin) {
        strm << ", " << *itBegin;
    }

    strm << ")";

    return strm;
}

}

#endif /* VMCMC_STRINGUTILS_H_ */