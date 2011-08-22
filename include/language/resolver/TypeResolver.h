/**
 * Zillians MMO
 * Copyright (C) 2007-2011 Zillians.com, Inc.
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

#ifndef ZILLIANS_LANGUAGE_TYPERESOLVER_H_
#define ZILLIANS_LANGUAGE_TYPERESOLVER_H_

#include "language/tree/ASTNodeFactory.h"
#include "language/tree/visitor/general/TypeResolutionVisitor.h"
#include "utility/TemplateTricks.h"

namespace __gnu_cxx {

template<typename T>
struct hash<T*>
{
	std::size_t operator() (T* value)
	{
		return reinterpret_cast<std::size_t>(value);
	}
};

}

namespace zillians { namespace language { namespace resolver {

struct TypeResolver
{
	TypeResolver()
	{ }

	void enterScope(const tree::ASTNode& node)
	{
		current_scopes.insert(&node);
	}

	void leaveScope(const tree::ASTNode& node)
	{
		__gnu_cxx::hash_set<const tree::ASTNode*>::iterator scope = current_scopes.find(&node);
		if(scope != current_scopes.end())
			current_scopes.erase(scope);
	}

	bool resolve(const tree::TypeSpecifier& node)
	{
		if(node.type == tree::TypeSpecifier::ReferredType::UNSPECIFIED)
		{
			type_visitor.target(node.referred.unspecified);

			for(__gnu_cxx::hash_set<const tree::ASTNode*>::const_iterator scope = current_scopes.begin(); scope != current_scopes.end(); ++scope)
			{
				type_visitor.visit(**scope);
			}

			if(type_visitor.candidates.size() == 1)
			{
				// TODO update the link
				type_visitor.reset();
				return true;
			}
			else
			{
				if(type_visitor.candidates.size() > 1)
				{
					// TODO
					// mode than one candidate
				}
				else
				{
					// TODO
					// no candidate
				}

				type_visitor.reset();
				return false;
			}
		}
		else
		{
			return false;
		}
	}

	__gnu_cxx::hash_set<const tree::ASTNode*> current_scopes;
	tree::visitor::TypeResolutionVisitor type_visitor;
};

} } }

#endif /* ZILLIANS_LANGUAGE_TYPERESOLVER_H_ */