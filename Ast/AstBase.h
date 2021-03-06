/******************************************************************************
 * Copyright (c) 2014-2016 Leandro T. C. Melo (ltcmelo@gmail.com)
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

#ifndef UAISO_ASTBASE_H__
#define UAISO_ASTBASE_H__

#include "Ast/AstFwd.h"
#include "Ast/AstDefs.h"
#include "Ast/AstVariety.h"
#include "Common/Assert.h"
#include "Common/Config.h"
#include "Parsing/SourceLoc.h"
#include <cstdint>
#include <memory>
#include <type_traits>

namespace uaiso {

/*!
 * \brief The Ast class
 *
 * The root of all AST nodes.
 */
class UAISO_API Ast
{
public:
    /*!
     * \brief The Kind enum
     */
    enum class Kind : uint16_t
    {
#define MAKE_CASE(AST_NODE, AST_KIND) AST_NODE##AST_KIND,

        Invalid,
        FirstNameMarker__,
NAME_AST_MIXIN(MAKE_NAME_CASE)
        LastNameMarker__,
        FirstSpecMarker__,
SPEC_AST_MIXIN(MAKE_SPEC_CASE)
        LastSpecMarker__,
        FirstAttrMarker__,
ATTR_AST_MIXIN(MAKE_ATTR_CASE)
        LastAttrMarker__,
        FirstDeclMarker__,
DECL_AST_MIXIN(MAKE_DECL_CASE)
        LastDeclMarker__,
        FirstExprMarker__,
EXPR_AST_MIXIN(MAKE_EXPR_CASE)
        LastExprMarker__,
        FirstStmtMarker__,
STMT_AST_MIXIN(MAKE_STMT_CASE)
        LastStmtMarker__,
        Program,
        Generator,
        Filter

#undef MAKE_CASE
    };

    using KindType = std::underlying_type<Kind>::type;

    /*!
     * \brief Ast
     */
    Ast() : bits_(0)
    {}

    /*!
     * \brief Ast
     * \param kind
     */
    Ast(Kind kind) : bits_(0)
    {
        bit_.kind = static_cast<KindType>(kind);
    }

    virtual ~Ast() {}

    /*!
     * \brief kind
     * \return
     *
     * Return the kind of the AST node.
     */
    Kind kind() const { return Kind(bit_.kind); }

    /*!
     * \brief kindStr
     * \return
     *
     * Return the kind of the AST node as a string.
     */
    static std::string kindStr(Kind kind)
    {
#define MAKE_CASE(AST_NODE, AST_KIND) \
    case static_cast<KindType>(Kind::AST_NODE##AST_KIND): \
        return #AST_NODE#AST_KIND;

        switch (static_cast<KindType>(kind)) {
            NAME_AST_MIXIN(MAKE_NAME_CASE)
            SPEC_AST_MIXIN(MAKE_SPEC_CASE)
            ATTR_AST_MIXIN(MAKE_ATTR_CASE)
            DECL_AST_MIXIN(MAKE_DECL_CASE)
            EXPR_AST_MIXIN(MAKE_EXPR_CASE)
            STMT_AST_MIXIN(MAKE_STMT_CASE)
        default:
            UAISO_ASSERT(false, return "");
            return "";
        }

#undef MAKE_CASE
    }

    bool isName() const
    {
        return checkKind(Kind::FirstNameMarker__, Kind::LastNameMarker__);
    }

    bool isSpec() const
    {
        return checkKind(Kind::FirstSpecMarker__, Kind::LastSpecMarker__);
    }

    bool isAttr() const
    {
        return checkKind(Kind::FirstAttrMarker__, Kind::LastAttrMarker__);
    }

    bool isDecl() const
    {
        return checkKind(Kind::FirstDeclMarker__, Kind::LastDeclMarker__);
    }

    bool isExpr() const
    {
        return checkKind(Kind::FirstExprMarker__, Kind::LastExprMarker__);
    }

    bool isStmt() const
    {
        return checkKind(Kind::FirstStmtMarker__, Kind::LastStmtMarker__);
    }

protected:
    // Bits are assigned by specific AST nodes.
    struct BitFields
    {
        uint32_t kind       : 16;
        uint32_t variety    : 4;
        uint32_t alloc      : 2;
    };
    union
    {
        BitFields bit_;
        uint32_t bits_;
    };

private:
    bool checkKind(Kind firstMarker, Kind lastMarker) const
    {
        return bit_.kind > static_cast<KindType>(firstMarker)
                && bit_.kind < static_cast<KindType>(lastMarker);
    }
};

/*
 * This was originally intended for the Bison-generated C parsers. Otherwise,
 * the AST create methods are to be preferred. I hope to eventually refactor
 * parts of the code using this "old" style.
 */

template <class AstT>
AstT* newAst()
{
    return new AstT; // TODO: Allocate on an AST pool.
}

} // namespace uaiso


/*
 * Macros for AST classes.
 */

#define AST_CLASS(AST_NODE, AST_KIND) \
    using Self = AST_NODE##AST_KIND##Ast; \
    static std::unique_ptr<Self> create() \
    { \
        return std::unique_ptr<Self>(newAst<Self>()); \
    } \

#define CREATE_WITH_LOC(LOC_MEMBER) \
    static std::unique_ptr<Self> create(const SourceLoc& loc) \
    { \
        auto ast = create(); \
        ast->set##LOC_MEMBER##Loc(loc); \
        return ast; \
    }

#define CREATE_WITH_AST(AST_MEMBER, AST_KIND) \
    static std::unique_ptr<Self> create(std::unique_ptr<AST_KIND##Ast> p) \
    { \
        auto ast = create(); \
        ast->set##AST_MEMBER(std::move(p)); \
        return ast; \
    }


/*
 * The AST interface is a bit messed up. The methods using raw pointers were
 * originally created to make it convenient for Bison parsers. Otherwise, the
 * smart-pointer versions should be used.
 */

#define NAMED_AST_PARAM(NAME, MEMBER, PARAM_TYPE) \
    Self* set##NAME(PARAM_TYPE* param) \
    { \
        MEMBER##_.reset(param); \
        return this; \
    } \
    Self* set##NAME(std::unique_ptr<PARAM_TYPE> param) \
    { \
        MEMBER##_ = std::move(param); \
        return this; \
    } \
    PARAM_TYPE* MEMBER() const { return MEMBER##_.get(); }

#define NAMED_AST_PARAM__BASE__(NAME, PARAM_TYPE) \
    virtual Self* set##NAME(PARAM_TYPE*) { return nullptr; }

#define NAMED_AST_PARAM__(NAME, TEMPLATE, PARAM_TYPE) \
    Self* set##NAME(PARAM_TYPE* param) override \
    { \
        TEMPLATE::set##NAME##__(param); \
        return this; \
    }

#define NAMED_AST_LIST_PARAM(NAME, MEMBER, PARAM_TYPE) \
    Self* add##NAME(PARAM_TYPE* param) \
    { \
        MEMBER##_ ? \
            MEMBER##_->append(std::unique_ptr<PARAM_TYPE>(param)) : \
            (void)(MEMBER##_ = PARAM_TYPE##List::create(std::unique_ptr<PARAM_TYPE>(param))); \
        return this; \
    } \
    Self* merge##NAME##s(std::unique_ptr<PARAM_TYPE##List> param) \
    { \
        MEMBER##_ ? \
            MEMBER##_->merge(std::move(param)) : \
            (void)(MEMBER##_ = std::move(param)); \
        return this; \
    } \
    Self* set##NAME##s(std::unique_ptr<PARAM_TYPE##List> param) \
    { \
        MEMBER##_ = std::move(param); \
        return this; \
    } \
    Self* add##NAME(std::unique_ptr<PARAM_TYPE> param) \
    { \
        MEMBER##_ ? \
            MEMBER##_->append(std::move(param)) : \
            (void)(MEMBER##_ = PARAM_TYPE##List::create(std::move(param))); \
        return this; \
    } \
    Self* set##NAME##s##SR(PARAM_TYPE##List* param) \
    { \
        MEMBER##_.reset(param->finishSR()); \
        return this; \
    } \
    PARAM_TYPE##List* MEMBER() const { return MEMBER##_.get(); }

#define NAMED_AST_LIST_PARAM__BASE__(NAME, PARAM_TYPE) \
    virtual Self* set##NAME##s(PARAM_TYPE##List*) { return nullptr; } \
    virtual Self* set##NAME##s##SR(PARAM_TYPE##List*) { return nullptr; } \
    virtual Self* set##NAME##s(std::unique_ptr<PARAM_TYPE##List>) { return nullptr; } \

#define NAMED_AST_LIST_PARAM__(NAME, TEMPLATE, PARAM_TYPE) \
    Self* set##NAME##s(PARAM_TYPE##List* param) override \
    { \
        TEMPLATE::set##NAME##s##__(param); \
        return this; \
    } \
    Self* set##NAME##s##SR(PARAM_TYPE##List* param) override \
    { \
        TEMPLATE::set##NAME##s##__(param->finishSR()); \
        return this; \
    } \
    Self* set##NAME##s(std::unique_ptr<PARAM_TYPE##List> param) override \
    { \
        TEMPLATE::set##NAME##s##__(param.release()); \
        return this; \
    }

#define NAMED_LOC_PARAM(NAME, MEMBER) \
    Self* set##NAME##Loc(const SourceLoc& param) \
    { \
        MEMBER##Loc_ = param; \
        return this; \
    } \
    const SourceLoc& MEMBER##Loc() const { return MEMBER##Loc_; }

#define NAMED_LOC_PARAM__BASE__(NAME) \
    virtual Self* set##NAME(const SourceLoc&) { return nullptr; } \

#define NAMED_LOC_PARAM__(NAME, BASE_TEMPLATE) \
    Self* set##NAME(const SourceLoc& param) override \
    { \
        BASE_TEMPLATE::set##NAME##__(param); \
        return this; \
    }

#endif
