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
/**
 * @date Aug 10, 2011 sdk - Initial version created.
 */

#ifndef ZILLIANS_LANGUAGE_TREE_VISITOR_ERRORMESSAGEANNOTATIONCHECKVISITOR_H_
#define ZILLIANS_LANGUAGE_TREE_VISITOR_ERRORMESSAGEANNOTATIONCHECKVISITOR_H_

#include <string>
#include "core/Prerequisite.h"
#include "language/logging/StringTable.h"
#include "core/Visitor.h"
#include "language/tree/ASTNodeFactory.h"
#include "language/tree/visitor/GenericVisitor.h"
#include "language/stage/parser/context/SourceInfoContext.h"
#include "language/context/LogInfoContext.h"

using namespace zillians::language::tree;
using zillians::language::tree::visitor::GenericVisitor;

namespace zillians { namespace language { namespace stage { namespace visitor {

struct StaticTestVerificationStageVisitor : public GenericVisitor
{
    CREATE_GENERIC_INVOKER(errorMessageAnnotationCheckInvoker);

	StaticTestVerificationStageVisitor() : mAllMatch(true)
	{
		REGISTER_ALL_VISITABLE_ASTNODE(errorMessageAnnotationCheckInvoker)
	}

	bool isAllMatch() const
	{
		return mAllMatch;
	}

	void apply(ASTNode& node)
	{
		GenericVisitor::apply(node);
	}

    void apply(Statement& node)
    {
        staticTest(node);
        GenericVisitor::apply(node); // NOTE: not sure if needed (lambda???)
    }

    void apply(Declaration& node)
    {
        staticTest(node);
        GenericVisitor::apply(node);
    }

    void apply(Package& node)
    {
        staticTest(node);
        GenericVisitor::apply(node);
    }

private:

    template<typename NodeType>
    void staticTest(NodeType& node)
    {
        // construct anno LogInfos
        std::vector<LogInfo> annotatedLogInfoVec = constructLogInfoVecFromAnnotations(node.annotations);

        // get hooked LogInfos
        std::vector<LogInfo> hookedLogInfoVec = LogInfoContext::get(&node) ? LogInfoContext::get(&node)->log_infos : std::vector<LogInfo>();

        // compare
        if(!compareLogInfoVec(&node, annotatedLogInfoVec, hookedLogInfoVec))
        {
            mAllMatch = false;
        }
    }

    std::vector<LogInfo> constructLogInfoVecFromAnnotations(zillians::language::tree::Annotations* annos);
    bool compareLogInfoVec(ASTNode* errorNode, std::vector<LogInfo> annotatedLogInfoVec, std::vector<LogInfo> hookedLogInfoVec);

private:
	bool mAllMatch;
};

} } } } // namespace zillians::language::tree::visitor

#endif /* ZILLIANS_LANGUAGE_TREE_VISITOR_ERRORMESSAGEANNOTATIONCHECKVISITOR_H_ */
