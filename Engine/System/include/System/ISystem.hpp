/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** ISystem
*/

#ifndef ISYSTEM_HPP_
#define ISYSTEM_HPP_

namespace RType {

    class ComponentManager;

    /**
     * @brief Type-erased base class for System<Component>.
     */
    class ISystem {
    public:
        /**
         * @brief Default virtual destructor.
         */
        virtual ~ISystem() = default;

        /**
         * @brief Runs this system once over every relevant component.
         * @param cm Component manager providing access to the relevant
         * ComponentArray.
         */
        virtual void update(ComponentManager &cm) = 0;
    };

} // namespace RType

#endif /* !ISYSTEM_HPP_ */
