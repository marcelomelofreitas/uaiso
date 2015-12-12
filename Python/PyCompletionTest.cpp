/******************************************************************************
 * Copyright (c) 2014-2015 Leandro T. C. Melo (ltcmelo@gmail.com)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
 * USA
 *****************************************************************************/

/*--------------------------*/
/*--- The UaiSo! Project ---*/
/*--------------------------*/

#include "Semantic/CompletionTest.h"
#include "Parsing/Factory.h"
#include "Parsing/Unit.h"

using namespace uaiso;

void CompletionProposer::CompletionProposerTest::PyTestCase1()
{
    std::string code = R"raw(
class Point:
    def __init__(self, x, y):
        self.x = x
        self.y = y
    def show(self):
        print 'x' % x
        print 'y' % y
                                                 # line 8
p =
#  ^
#  |
#  complete at up-arrow
)raw";

    lineCol_ = { 9, 3 };
    auto expected = { "p", "Point" };
    runCore(FactoryCreator::create(LangName::Py), code, "/test.py", expected);
}

void CompletionProposer::CompletionProposerTest::PyTestCase2()
{
    UAISO_SKIP_TEST;

    std::string code = R"raw(
class Point:
    def __init__(self, x, y):
        self.x = x
        self.y = y
    def show(self):
        print 'x' % x
        print 'y' % y
                                                 # line 8
p = Point()
p.
# ^
# |
# complete at up-arrow
)raw";

    lineCol_ = { 10, 2 };
    auto expected = { "x", "y", "show", "__init__" };
    runCore(FactoryCreator::create(LangName::Py), code, "/test.py", expected);
}

void CompletionProposer::CompletionProposerTest::PyTestCase3()
{
    std::string code = R"raw(
class Point:
    def __init__(self, x, y):
        self.x = x
        self.y = y
                                                 # line 5

if a:
    p = Point()

p.
# ^
# |
# complete at up-arrow
)raw";

    lineCol_ = { 10, 2 };
    auto expected = { "x", "y", "__init__" };
    runCore(FactoryCreator::create(LangName::Py), code, "/test.py", expected);
}

void CompletionProposer::CompletionProposerTest::PyTestCase4()
{
    std::string code = R"raw(
class Point:
    def __init__(self, x, y):
        self.x = x
        self.y = y
                                                 # line 5

def foo():
    p = Point()
    p.
#     ^
#     |
#     complete at up-arrow
)raw";

    lineCol_ = { 9, 6 };
    auto expected = { "x", "y", "__init__" };
    runCore(FactoryCreator::create(LangName::Py), code, "/test.py", expected);
}

void CompletionProposer::CompletionProposerTest::PyTestCase5()
{
    std::string code = R"raw(
class Point:
    def __init__(self, x, y):
        self.x = x
        self.y = y
                                                 # line 5

p = Point()

def foo():
    p.
#     ^
#     |
#     complete at up-arrow
)raw";

    lineCol_ = { 10, 6 };
    auto expected = { "x", "y", "__init__" };
    runCore(FactoryCreator::create(LangName::Py), code, "/test.py", expected);
}

void CompletionProposer::CompletionProposerTest::PyTestCase6()
{
    std::string code = R"raw(
class A:
    a = 1

class B:
    b = "b"
                                                 # line 6
p = A()

def foo():
    p = B()
    p.
#     ^
#     |
#     complete at up-arrow
)raw";

    lineCol_ = { 11, 6 };
    auto expected = { "b" };
    runCore(FactoryCreator::create(LangName::Py), code, "/test.py", expected);
}

void CompletionProposer::CompletionProposerTest::PyTestCase7()
{
    std::string code = R"raw(
class A:
    a = 1

class B:
    b = "b"
                                                 # line 6

if x:
    p = A()
else:
    p = A()

p.
# ^
# |
# complete at up-arrow
)raw";

    lineCol_ = { 13, 2 };
    auto expected = { "a" };
    runCore(FactoryCreator::create(LangName::Py), code, "/test.py", expected);
}

void CompletionProposer::CompletionProposerTest::PyTestCase8()
{
    UAISO_SKIP_TEST;

    std::string code = R"raw(
class A:
    a = 1

class B:
    b = "b"
                                                 # line 6

if x:
    p = A()
else:
    p = B()

p.
# ^
# |
# complete at up-arrow
)raw";

    lineCol_ = { 13, 2 };
    // Be conservative and show completion for both types.
    auto expected = { "a", "b" };
    runCore(FactoryCreator::create(LangName::Py), code, "/test.py", expected);
}