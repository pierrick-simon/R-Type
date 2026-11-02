/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** SyncField
*/

#ifndef SYNCFIELD_HPP_
#define SYNCFIELD_HPP_

namespace RType {
    template<typename T>
    bool syncField(T &dst, const T &src)
    {
        if (dst == src)
            return false;
        dst = src;
        return true;
    }
} // namespace RType

#endif /* !SYNCFIELD_HPP_ */
