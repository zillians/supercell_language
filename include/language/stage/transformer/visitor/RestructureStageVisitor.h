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

#ifndef ZILLIANS_LANGUAGE_STAGE_VISITOR_RESTRUCTURESTAGEVISITOR_H_
#define ZILLIANS_LANGUAGE_STAGE_VISITOR_RESTRUCTURESTAGEVISITOR_H_

#include "core/Prerequisite.h"
#include "language/tree/ASTNodeHelper.h"
#include "language/context/TransformerContext.h"
#include "language/tree/visitor/general/GenericDoubleVisitor.h"
#include "language/tree/visitor/general/NameManglingVisitor.h"
#include "language/stage/transformer/context/ManglingStageContext.h"

using namespace zillians::language::tree;
using zillians::language::tree::visitor::GenericDoubleVisitor;
using zillians::language::tree::visitor::NameManglingVisitor;

namespace zillians { namespace language { namespace stage { namespace visitor {

/**
 * RestructureStageVisitor is the visitation helper for RestructureStage
 *
 * @see RestructureStage
 */
struct RestructureStageVisitor : GenericDoubleVisitor
{
	CREATE_INVOKER(restructInvoker, restruct)

	RestructureStageVisitor()
	{
		REGISTER_ALL_VISITABLE_ASTNODE(restructInvoker)
	}

	void restruct(ASTNode& node)
	{
		revisit(node);
	}

	void restruct(BinaryExpr& node)
	{
		revisit(node);

		// transform all arithmetic assignment into separate arithmetic expression and assignment expression
		// for example (a += b):
		//
		//      +=
		//     /  \
		//    a    b
		//
		// will be transformed into:
		//
		//      =
		//     / \
		//    a   +
		//       / \
		//      a   b
		//
		// for a more advanced example (a += b += c):
		//
		//      +=
		//     /  \
		//    a    +=
		//        /  \
		//       b    c
		//
		// will be transformed into:
		//
		//      =
		//     / \
		//    a   +
		//       / \
		//      a   =
		//         / \
		//        b   +
		//           / \
		//          b   c
		//
		if(node.isAssignment() && node.opcode != BinaryExpr::OpCode::ASSIGN)
		{
			transforms.push_back([&](){
				BinaryExpr::OpCode::type decomposed_op = BinaryExpr::OpCode::decomposeAssignment(node.opcode);
				BOOST_ASSERT(decomposed_op != BinaryExpr::OpCode::INVALID && "invalid decomposed binary operator");
				BinaryExpr* new_rhs = new BinaryExpr(decomposed_op, cast<Expression>(node.left->clone()), node.right);
				node.replaceUseWith(*node.right, *new_rhs, false);
				new_rhs->parent = &node;
				node.opcode = BinaryExpr::OpCode::ASSIGN;
			});
		}
	}

	void restruct(VariableDecl& node)
	{
		revisit(node);

		// transform all initializer into a separate assignment binary expression
		// for example (var a = ...):
		//
		//       block
		//      /  |  \
		//     ......  var_decl
		//            /        \
		//           a         (initializer)
		//
		// will be transformed into:
		//
		//       block------------------\
		//      /  |  \                  \
		//     ......  var_decl           =
		//            /        \         / \
		//           a         (null)	a   (initializer)
		//
		if(node.initializer && ASTNodeHelper::isOwnedByFunction(node))
		{
			transforms.push_back([&](){
				DeclarativeStmt* anchor = cast<DeclarativeStmt>(node.parent);
				Block* parent = cast<Block>(anchor->parent);
				SimpleIdentifier* name = cast<SimpleIdentifier>(node.name);
				BOOST_ASSERT(parent != NULL && name != NULL && anchor != NULL && "variable declaration has incorrect hierarchy");
				if(parent && name && anchor)
				{
					ExpressionStmt* append = new ExpressionStmt(
							new BinaryExpr(
									BinaryExpr::OpCode::ASSIGN,
									new PrimaryExpr(cast<Identifier>(node.name->clone())),
									node.initializer));

					SplitReferenceContext::set(append, anchor);

					parent->insertObject(anchor, append, false);

					node.initializer = NULL;
				}
			});
		}
	}

public:
	bool hasTransforms()
	{
		return (transforms.size() > 0);
	}

	void applyTransforms()
	{
		foreach(i, transforms)
		{
			(*i)();
		}
		transforms.clear();
	}

private:
	std::vector<std::function<void()>> transforms;
};

} } } }

#endif /* ZILLIANS_LANGUAGE_STAGE_VISITOR_RESTRUCTURESTAGEVISITOR_H_ */
