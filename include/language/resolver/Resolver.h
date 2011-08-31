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

#ifndef ZILLIANS_LANGUAGE_RESOLVER_H_
#define ZILLIANS_LANGUAGE_RESOLVER_H_

#include "language/tree/ASTNodeFactory.h"
#include "language/tree/visitor/general/ResolutionVisitor.h"
#include "utility/Foreach.h"

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

namespace zillians { namespace language {

struct ResolvedType
{
	explicit ResolvedType(tree::ASTNode* ref) : ref(ref)
	{ }

	static tree::ASTNode* get(tree::ASTNode* node)
	{
		ResolvedType* resolved = node->get<ResolvedType>();
		if(resolved)
			return resolved->ref;
		else
			return NULL;
	}

	static void set(tree::ASTNode* node, tree::ASTNode* ref)
	{
		return node->set<ResolvedType>(new ResolvedType(ref));
	}

	tree::ASTNode* ref;
};

struct ResolvedSymbol
{
	explicit ResolvedSymbol(tree::ASTNode* ref) : ref(ref)
	{ }

	static tree::ASTNode* get(tree::ASTNode* node)
	{
		ResolvedSymbol* resolved = node->get<ResolvedSymbol>();
		if(resolved)
			return resolved->ref;
		else
			return NULL;
	}

	static void set(tree::ASTNode* node, tree::ASTNode* ref)
	{
		return node->set<ResolvedSymbol>(new ResolvedSymbol(ref));
	}

	tree::ASTNode* ref;
};

struct Resolver
{
	Resolver()
	{ }

	void enterScope(tree::ASTNode& node)
	{
		current_scopes.insert(&node);
	}

	void leaveScope(tree::ASTNode& node)
	{
		__gnu_cxx::hash_set<tree::ASTNode*>::iterator scope = current_scopes.find(&node);
		if(scope != current_scopes.end())
			current_scopes.erase(scope);
	}

	bool resolveSymbol(tree::ASTNode& scope, tree::Identifier& node)
	{
		using namespace zillians::language::tree;

		if(!ResolvedSymbol::get(&node))
		{
			resolution_visitor.target(visitor::ResolutionVisitor::Target::SYMBOL, &node);
			resolution_visitor.visit(scope);

			return checkResolvedSymbol(node);
		}
		else
		{
			return false;
		}
	}

	bool resolveSymbol(tree::Identifier& node)
	{
		using namespace zillians::language::tree;

		if(!ResolvedSymbol::get(&node))
		{
			// set the look-for target
			resolution_visitor.target(visitor::ResolutionVisitor::Target::SYMBOL, &node);

			for(__gnu_cxx::hash_set<ASTNode*>::const_iterator scope = current_scopes.begin(); scope != current_scopes.end(); ++scope)
			{
				resolution_visitor.visit(**scope);
			}

			return checkResolvedSymbol(node);
		}
		else
		{
			return false;
		}
	}

	bool checkResolvedSymbol(tree::Identifier& node)
	{
		using namespace zillians::language::tree;

		if(resolution_visitor.candidates.size() == 1)
		{
			ASTNode* ref = resolution_visitor.candidates[0];

			// TODO does that's all types that we can resolve identifier to?
			if(isa<ClassDecl>(ref))
			{
				LOG4CXX_DEBUG(Logger::Resolver, L"resolve identifier \"" << node.toString() << L"\" to \"" << cast<ClassDecl>(ref)->name->toString() << L"\"");
				ResolvedSymbol::set(&node, ref);
			}
			else if(isa<EnumDecl>(ref))
			{
				ResolvedSymbol::set(&node, ref);
			}
			else if(isa<TypedefDecl>(ref))
			{
				ResolvedSymbol::set(&node, ref);
			}
			else
			{
				BOOST_ASSERT(false && "unknown type resolved");
			}

			resolution_visitor.reset();
			return true;
		}
		else
		{
			if(resolution_visitor.candidates.size() > 1)
			{
				// TODO
				// mode than one candidate
			}
			else
			{
				// TODO
				// no candidate
			}

			resolution_visitor.reset();
			return false;
		}
	}

	bool resolveType(tree::ASTNode& scope, tree::TypeSpecifier& node)
	{
		using namespace zillians::language::tree;

		if(node.type == TypeSpecifier::ReferredType::UNSPECIFIED)
		{
			resolution_visitor.target(visitor::ResolutionVisitor::Target::TYPE, node.referred.unspecified);
			resolution_visitor.visit(scope);

			return checkResolvedType(node);
		}
		else
		{
			return false;
		}
	}

	bool resolveType(tree::TypeSpecifier& node)
	{
		using namespace zillians::language::tree;

		if(node.type == TypeSpecifier::ReferredType::UNSPECIFIED)
		{
			// set the look-for target
			resolution_visitor.target(visitor::ResolutionVisitor::Target::TYPE, node.referred.unspecified);

			for(__gnu_cxx::hash_set<ASTNode*>::const_iterator scope = current_scopes.begin(); scope != current_scopes.end(); ++scope)
			{
				resolution_visitor.visit(**scope);
			}

			return checkResolvedType(node);
		}
		else
		{
			return false;
		}
	}

	bool checkResolvedType(tree::TypeSpecifier& node)
	{
		using namespace zillians::language::tree;

		if(resolution_visitor.candidates.size() == 1)
		{
			ASTNode* ref = resolution_visitor.candidates[0];

			if(isa<ClassDecl>(ref))
			{
				LOG4CXX_DEBUG(Logger::Resolver, L"resolve identifier \"" << node.referred.unspecified->toString() << L"\" to \"" << cast<ClassDecl>(ref)->name->toString() << L"\"");
				node.update(cast<ClassDecl>(ref));
			}
			else if(isa<EnumDecl>(ref))
			{
				node.update(cast<EnumDecl>(ref));
			}
			else if(isa<TypedefDecl>(ref))
			{
				node.update(cast<TypedefDecl>(ref));
			}
			else
			{
				BOOST_ASSERT(false && "unknown type resolved");
			}

			resolution_visitor.reset();
			return true;
		}
		else
		{
			if(resolution_visitor.candidates.size() > 1)
			{
				// TODO
				// mode than one candidate
			}
			else
			{
				// TODO
				// no candidate
			}

			resolution_visitor.reset();
			return false;
		}
	}

	__gnu_cxx::hash_set<tree::ASTNode*> current_scopes;
	tree::visitor::ResolutionVisitor resolution_visitor;
};

} }

#endif /* ZILLIANS_LANGUAGE_RESOLVER_H_ */
