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
    Poc::TestList multiSystems = Poc::multiSystemTests();
    Poc::TestList views = Poc::viewTests();
    Poc::TestList viewVectors = Poc::viewVectorTests();
    Poc::TestList frameSystems = Poc::frameSystemTests();
    Poc::TestList gmSystems = Poc::gmSystemTests();
    Poc::TestList recycling = Poc::recyclingTests();
    Poc::TestList tricky = Poc::trickyTests();
    Poc::TestList simulation = Poc::simulationTests();
    Poc::TestList duplicates = Poc::duplicateTests();

    tests.insert(tests.end(), duplicates.begin(), duplicates.end());
    tests.insert(tests.end(), systems.begin(), systems.end());
    tests.insert(tests.end(), multiSystems.begin(), multiSystems.end());
    tests.insert(tests.end(), views.begin(), views.end());
    tests.insert(tests.end(), viewVectors.begin(), viewVectors.end());
    tests.insert(tests.end(), frameSystems.begin(), frameSystems.end());
    tests.insert(tests.end(), gmSystems.begin(), gmSystems.end());
    tests.insert(tests.end(), recycling.begin(), recycling.end());
    tests.insert(tests.end(), tricky.begin(), tricky.end());
    tests.insert(tests.end(), simulation.begin(), simulation.end());
    return Poc::runTests(tests) == 0 ? 0 : 84;
}
