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
// UNINIT_REF
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

	void verify(PrimaryExpr &node)
	{
		if(node.catagory == PrimaryExpr::Catagory::IDENTIFIER && ASTNodeHelper::isOwnedByFunction(node))
		{
			ASTNode* decl = ResolvedSymbol::get(&node);
			if(isa<VariableDecl>(decl))
			{
				VariableDecl* var_decl = cast<VariableDecl>(decl);
				std::wstring name = var_decl->name->toString();
				ASTNode* attachment_point = ASTNodeHelper::getOwnerAnnotationAttachPoint(node);

				// UNINIT_REF
				if(ASTNodeHelper::isOwnedByRValue(node))
					if(!SemanticVerificationVariableDeclContext_HasBeenInit::get(var_decl))
						LOG_MESSAGE(UNINIT_REF, attachment_point, _var_id = name);

				// INVALID_NONSTATIC_REF
				if(!isa<FunctionDecl>(node.parent))
					if(ASTNodeHelper::getOwnerFunction(node)->is_static && !var_decl->is_static)
						LOG_MESSAGE(INVALID_NONSTATIC_REF, attachment_point, _var_id = name);
			}
		}

		revisit(node);
	}

	void verify(CallExpr &node)
	{
		// INVALID_NONSTATIC_CALL
		ASTNode* decl = ResolvedSymbol::get(node.node);
		FunctionDecl* owner_func_decl = ASTNodeHelper::getOwnerFunction(node);
		ASTNode* attachment_point = ASTNodeHelper::getOwnerAnnotationAttachPoint(node);
		if(isa<FunctionDecl>(decl))
		{
			FunctionDecl* func_decl = cast<FunctionDecl>(decl);
			if(owner_func_decl->is_static && !func_decl->is_static)
				LOG_MESSAGE(INVALID_NONSTATIC_CALL, attachment_point, _func_id = func_decl->name->toString());
		}
		if(isa<VariableDecl>(decl))
		{
			VariableDecl* var_decl = cast<VariableDecl>(decl);
			if(owner_func_decl->is_static && !var_decl->is_static)
				LOG_MESSAGE(INVALID_NONSTATIC_CALL, attachment_point, _func_id = var_decl->name->toString());
		}

		revisit(node);
	}

	void verify(BinaryExpr &node)
	{
		if(node.isAssignment())
		{
			VariableDecl* var_decl = cast<VariableDecl>(ResolvedSymbol::get(node.left));

			// WRITE_CONST
			if(var_decl->is_const)
				LOG_MESSAGE(WRITE_CONST, ASTNodeHelper::getOwnerAnnotationAttachPoint(node), _var_id = var_decl->name->toString());

			// UNINIT_REF
			SemanticVerificationVariableDeclContext_HasBeenInit::set(var_decl, NULL);
		}

		revisit(node);
	}

	void verify(BranchStmt &node)
	{
		if(node.opcode == BranchStmt::OpCode::RETURN)
		{
			FunctionDecl* func_decl = ASTNodeHelper::getOwnerFunction(node);

			// MISSING_RETURN
			SemanticVerificationFunctionDeclContext_HasVisitedReturn::get_instance(func_decl);

			TypeSpecifier* return_param = func_decl->type;
			TypeSpecifier* return_arg = cast<TypeSpecifier>(ResolvedType::get(node.result));
			ASTNode* attachment_point = ASTNodeHelper::getOwnerAnnotationAttachPoint(node);

			// UNEXPECTED_RETURN_VALUE
			if(ASTNodeHelper::isVoidType(return_param) && !ASTNodeHelper::isVoidType(return_arg))
				LOG_MESSAGE(UNEXPECTED_RETURN_VALUE, attachment_point);

			// MISSING_RETURN_VALUE
			if(!ASTNodeHelper::isVoidType(return_param) && ASTNodeHelper::isVoidType(return_arg))
				LOG_MESSAGE(MISSING_RETURN_VALUE, attachment_point, _type = return_param->toString());
		}

		revisit(node);
	}

	void verify(VariableDecl &node)
	{
		// UNINIT_REF
		if(ASTNodeHelper::isOwnedByFunction(node) && ASTNodeHelper::isOwnedByBlock(node) && !!node.initializer)
			SemanticVerificationVariableDeclContext_HasBeenInit::get_instance(&node);

		revisit(node);
	}

	void verify(FunctionDecl &node)
	{
		revisit(node);

		// MISSING_RETURN
		if(!ASTNodeHelper::isVoidType(node.type) && !SemanticVerificationFunctionDeclContext_HasVisitedReturn::get(&node))
			LOG_MESSAGE(MISSING_RETURN, &node);
	}
};

} } } }

#endif /* ZILLIANS_LANGUAGE_STAGE_VISITOR_SEMANTICVERIFICATIONSTAGEVISITOR1_H_ */
