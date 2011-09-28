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

#ifndef ZILLIANS_LANGUAGE_STAGE_VISITOR_SEMANTICVERIFICATIONSTAGEVISITOR1_H_
#define ZILLIANS_LANGUAGE_STAGE_VISITOR_SEMANTICVERIFICATIONSTAGEVISITOR1_H_

#include "core/Prerequisite.h"
#include "language/tree/visitor/general/GenericVisitor.h"
#include "language/tree/visitor/general/GenericDoubleVisitor.h"
#include "language/tree/visitor/general/NameManglingVisitor.h"
#include "language/stage/transformer/context/ManglingStageContext.h"
#include "language/logging/StringTable.h"
#include "language/logging/LoggerWrapper.h"
#include "language/tree/ASTNodeHelper.h"
#include "language/logging/StringTable.h"
#include "language/stage/verifier/context/SemanticVerificationContext.h"
#include "language/resolver/Resolver.h"

using namespace zillians::language::tree;
using zillians::language::tree::visitor::GenericDoubleVisitor;
using zillians::language::tree::visitor::NameManglingVisitor;

// CHECKS IN SEMANTIC VERIFICATION STAGE 1

// ERRORS:
// ====================================
// UNEXPECTED_RETURN_VALUE
// MISSING_RETURN_VALUE
// WRITE_CONST
// INVALID_NONSTATIC_CALL
// INVALID_NONSTATIC_REF
// INVALID_ACCESS_PRIVATE
// INVALID_ACCESS_PROTECTED

// WARNINGS:
// ====================================
// MISSING_RETURN
// UNINIT_ARG
// CONTROL_REACHES_END
// MISSING_CASE

namespace zillians { namespace language { namespace stage { namespace visitor {

struct SemanticVerificationStageVisitor1 : GenericDoubleVisitor
{
	CREATE_INVOKER(verifyInvoker, verify)

	SemanticVerificationStageVisitor1()
	{
		REGISTER_ALL_VISITABLE_ASTNODE(verifyInvoker)
	}

	void verify(ASTNode& node)
	{
		revisit(node);
	}

	void verify_UNINIT_ARG(ASTNode &node)
	{
		// UNINIT_ARG
		ASTNode* decl = ResolvedSymbol::get(&node);
		if(isa<VariableDecl>(decl) && ASTNodeHelper::isOwnedByFunction(node))
		{
			VariableDecl* var_decl = cast<VariableDecl>(decl);
			if(!SemanticVerificationVariableDeclContext_HasBeenInit::get(var_decl))
				LOG_MESSAGE(UNINIT_ARG, ASTNodeHelper::getNearestAnnotatableOwner(node), _var_id = var_decl->name->toString());
		}
	}

	void verify(PrimaryExpr &node)
	{
		// UNINIT_ARG
		if(node.catagory == PrimaryExpr::Catagory::IDENTIFIER)
			verify_UNINIT_ARG(node);
	}

	void verify(MemberExpr &node)
	{
		// UNINIT_ARG
		verify_UNINIT_ARG(*node.node);

		revisit(node);
	}

	void verify(BinaryExpr &node)
	{
		if(node.isAssignment())
		{
			VariableDecl* var_decl = cast<VariableDecl>(ResolvedSymbol::get(node.left));
			std::wstring name = var_decl->name->toString();

			// WRITE_CONST
			if(var_decl->is_const)
				LOG_MESSAGE(WRITE_CONST, ASTNodeHelper::getNearestAnnotatableOwner(node), _var_id = name);

			// UNINIT_ARG
			SemanticVerificationVariableDeclContext_HasBeenInit::set(var_decl, NULL);
		}
	}

	void verify(VariableDecl &node)
	{
		// UNINIT_ARG
		if(!isa<FunctionDecl>(node.parent) && !!node.initializer)
			SemanticVerificationVariableDeclContext_HasBeenInit::get_instance(&node);

		revisit(node);
	}
};

} } } }

#endif /* ZILLIANS_LANGUAGE_STAGE_VISITOR_SEMANTICVERIFICATIONSTAGEVISITOR1_H_ */
