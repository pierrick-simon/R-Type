/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Main
*/

#include "Test.hpp"

int main()
{
    Poc::TestList tests = Poc::edgeCaseTests();
    Poc::TestList systems = Poc::systemTests();
    Poc::TestList simulation = Poc::simulationTests();

    tests.insert(tests.end(), systems.begin(), systems.end());
    tests.insert(tests.end(), simulation.begin(), simulation.end());
    return Poc::runTests(tests) == 0 ? 0 : 84;
}
