/**
 * Zillians MMO
 * Copyright (C) 2007-2010 Zillians.com, Inc.
 * For more information see http://www.zillians.com
 *
 * Zillians MMO is the library and runtime for massive multiplayer online game
 * development in utility computing model, which runs as a service for every
 * developer to build their virtual world running on our GPU-assisted machines.
 *
 * This is a close source library intended to be used solely within Zillians.com
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
 * AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef ZILLIANS_LANGUAGE_TREE_TYPESPECIFIER_H_
#define ZILLIANS_LANGUAGE_TREE_TYPESPECIFIER_H_

#include "language/tree/ASTNode.h"
#include "language/tree/basic/PrimitiveType.h"
#include "language/tree/basic/Identifier.h"
#include "language/tree/basic/FunctionType.h"

namespace zillians { namespace language { namespace tree {

class ClassDecl;
class InterfaceDecl;
class EnumDecl;
class FunctionDecl;
class TypedefDecl;

struct TypeSpecifier : public ASTNode
{
	friend class boost::serialization::access;

	DEFINE_VISITABLE();
	DEFINE_HIERARCHY(TypeSpecifier, (TypeSpecifier)(ASTNode));

	struct ReferredType
	{
		enum type {
			FUNCTION_TYPE,
			PRIMITIVE,
			UNSPECIFIED,
		};

		static const wchar_t* toString(type t)
		{
			switch(t)
			{
			case FUNCTION_TYPE:		return L"function_type";
			case PRIMITIVE:			return L"primitive";
			case UNSPECIFIED: 		return L"unspecified";
			default: UNREACHABLE_CODE(); return NULL;
			}
		}
	};

	explicit TypeSpecifier(FunctionType* function_proto)
	{
		update(function_proto);
	}

	explicit TypeSpecifier(PrimitiveType::type primitive)
	{
		update(primitive);
	}

	explicit TypeSpecifier(Identifier* unspecified)
	{
		update(unspecified);
	}

	std::wstring toString() const
	{
		switch(type)
		{
		case TypeSpecifier::ReferredType::FUNCTION_TYPE: return referred.function_type->toString();
		case TypeSpecifier::ReferredType::PRIMITIVE: return PrimitiveType::toString(referred.primitive);
		case TypeSpecifier::ReferredType::UNSPECIFIED: return referred.unspecified->toString();
		default: UNREACHABLE_CODE(); return L"";
		}
	}

	void update(FunctionType* function_type)
	{
		type = ReferredType::FUNCTION_TYPE;
		referred.function_type = function_type;
	}

	void update(PrimitiveType::type primitive)
	{
		type = ReferredType::PRIMITIVE;
		referred.primitive = primitive;
	}

	void update(Identifier* unspecified)
	{
		type = ReferredType::UNSPECIFIED;
		referred.unspecified = unspecified;
	}

	bool isFunctionType() const
	{
		return (type == ReferredType::FUNCTION_TYPE);
	}

	bool isPrimitiveType() const
	{
		return (type == ReferredType::PRIMITIVE);
	}

	bool isUnspecified() const
	{
		return (type == ReferredType::UNSPECIFIED);
	}

    virtual bool isEqualImpl(const ASTNode& rhs, ASTNodeSet& visited) const
    {
    	BEGIN_COMPARE()
		COMPARE_MEMBER(type)
        switch(type)
        {
        case TypeSpecifier::ReferredType::FUNCTION_TYPE  : COMPARE_MEMBER(referred.function_type )             ; break;
        case TypeSpecifier::ReferredType::PRIMITIVE      : if(referred.primitive != __p->referred.primitive) return false; break;
        case TypeSpecifier::ReferredType::UNSPECIFIED    : COMPARE_MEMBER(referred.unspecified   )             ; break;
        default: UNREACHABLE_CODE(); break;
        }
        END_COMPARE()
    }

    virtual bool replaceUseWith(const ASTNode& from, const ASTNode& to, bool update_parent = true)
    {
    	BEGIN_REPLACE()
		switch(type)
		{
		case TypeSpecifier::ReferredType::FUNCTION_TYPE: REPLACE_USE_WITH(referred.function_type); break;
		case TypeSpecifier::ReferredType::UNSPECIFIED: REPLACE_USE_WITH(referred.unspecified); break;
		default: UNREACHABLE_CODE(); break;
		}
    	END_REPLACE()
    }

    virtual ASTNode* clone() const
    {
        switch(type)
        {
        case TypeSpecifier::ReferredType::FUNCTION_TYPE  : return new TypeSpecifier(cast<FunctionType>(referred.function_type->clone()));
        case TypeSpecifier::ReferredType::PRIMITIVE      : return new TypeSpecifier(referred.primitive);
        case TypeSpecifier::ReferredType::UNSPECIFIED    : return new TypeSpecifier(cast<Identifier>(referred.unspecified->clone()));
        default: UNREACHABLE_CODE(); break;
        }
        return NULL;
    }

    template<typename Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
    	UNUSED_ARGUMENT(version);

    	ar & boost::serialization::base_object<ASTNode>(*this);
    	ar & (int&)type;
        switch(type)
        {
        case TypeSpecifier::ReferredType::FUNCTION_TYPE  : ar & referred.function_type; break;
        case TypeSpecifier::ReferredType::PRIMITIVE      : ar & (int&)referred.primitive; break;
        case TypeSpecifier::ReferredType::UNSPECIFIED    : ar & referred.unspecified; break;
        default: UNREACHABLE_CODE(); break;
        }
    }

	ReferredType::type type;

	union ReferredUnion
	{
		FunctionType* function_type;
		PrimitiveType::type primitive;
		Identifier* unspecified;
	} referred;

protected:
	TypeSpecifier() { }
};

} } }

#endif /* ZILLIANS_LANGUAGE_TREE_TYPESPECIFIER_H_ */
