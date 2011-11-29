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

#ifndef ZILLIANS_LANGUAGE_TREE_VISITOR_GENERAL_NAMEMANGLINGVISITOR_H_
#define ZILLIANS_LANGUAGE_TREE_VISITOR_GENERAL_NAMEMANGLINGVISITOR_H_

#include "core/Prerequisite.h"
#include "utility/UnicodeUtil.h"
#include "core/Visitor.h"
#include "language/tree/visitor/GenericVisitor.h"
#include "language/tree/basic/PrimitiveType.h"
#include "language/tree/ASTNodeHelper.h"
#include <ctype.h>

namespace zillians { namespace language { namespace tree { namespace visitor {

struct NameManglingVisitor : Visitor<ASTNode, void, VisitorImplementation::recursive_dfs>
{
	CREATE_INVOKER(mangleInvoker, mangle)

	NameManglingVisitor() : named_scoped(false), dependent_component(false)
	{
		REGISTER_ALL_VISITABLE_ASTNODE(mangleInvoker)
	}

	void mangle(ASTNode& node)
	{ }

	void mangle(Package& node)
	{
		if(ASTNodeHelper::isRootPackage(&node))
		{
			stream << "_Z"; // always begin with "_Z"
			if(named_scoped)
				stream << "N"; // name involves a Package/ClassDecl
		}
		else
		{
			named_scoped = true;
			visit(*node.id);
		}
	}

	void mangle(SimpleIdentifier& node)
	{
		if(node.name == L"ptr_")        stream << "P";
		else if(node.name == L"ref_")   stream << "R";
		else if(node.name == L"const_") stream << "C";
		else if(node.name == L"void_")  stream << "v";
		else
			stream << node.name.length() << encode(node.name);
	}

	void mangle(TemplatedIdentifier& node)
	{
		visit(*node.id);
		foreach(i, node.templated_type_list)
            resolveAndVisit(*i);
	}

	void mangle(TypeSpecifier& node)
	{
		switch(node.type)
		{
		case TypeSpecifier::ReferredType::FUNCTION_TYPE:
			visit(*node.referred.function_type);
			break;
		case TypeSpecifier::ReferredType::PRIMITIVE:
			switch(node.referred.primitive)
			{
			case PrimitiveType::VOID:    stream << "v"; break;
			case PrimitiveType::BOOL:    stream << "b"; break;
			case PrimitiveType::INT8:    stream << "c"; break;
			case PrimitiveType::INT16:   stream << "s"; break;
			case PrimitiveType::INT32:   stream << "l"; /* or 'i' ? */ break;
			case PrimitiveType::INT64:   stream << "x"; break;
			case PrimitiveType::FLOAT32: stream << "f"; break;
			case PrimitiveType::FLOAT64: stream << "d"; break;
			default: UNREACHABLE_CODE();
			}
			break;
		case TypeSpecifier::ReferredType::UNSPECIFIED:
			resolveAndVisit(&node);
			break;
		}
	}

	// NOTE: if there are repeating arg "function types", use "S{digit}_" to represent repeated arg
	//       with digit being the index, instead of explicitly repeating the same mangled info
	void mangle(FunctionType& node)
	{
		if(node.parent) visit(*node.parent); // up-trace to get complete mangled-name
		stream << "PF"; // function pointer is always a pointer
		visit(*node.return_type);
		foreach(i, node.argument_types)
        	resolveAndVisit(*i);
		stream << "E"; // always end with "E"
	}

	void mangle(ClassDecl& node)
	{
		if(!dependent_component)
		{
			if(node.parent) visit(*node.parent); // up-trace to get complete mangled-name
			named_scoped = true;
		}
		visitIdentifier(node.name);
	}

	void mangle(FunctionDecl& node)
	{
#if 1 // NOTE: temporary work-around for c-tor bug
		if(isa<ClassDecl>(node.parent) && node.name->toString() == L"new")
			return;
#endif
		if(node.parent) visit(*node.parent); // up-trace to get complete mangled-name
		visitIdentifier(node.name);
		std::vector<VariableDecl*>::iterator p = node.parameters.begin();
		if(ASTNodeHelper::hasOwnerNamedScope(&node))
		{
			stream << "E"; // if mangled name begins with "N", end with "E"
			if(isa<ClassDecl>(node.parent))
			{
				BOOST_ASSERT(node.parameters.size() >= 1 && "methods must have 1st parameter \"this\"");
				p++; // skip "this" parameter for methods
			}
		}
		if(p == node.parameters.end())
			stream << "v"; // empty param-list equivalent to "void" param type
		else
		{
			for(; p != node.parameters.end(); p++)
                visit(*(*p)->type);
		}
	}

	const std::string& encode(const std::wstring& ucs4)
	{
		ucs4_to_utf8_temp.clear();
		utf8_to_llvm_temp.clear();

		// first we should covert UCS-4 to UTF-8
		ucs4_to_utf8(ucs4, ucs4_to_utf8_temp);

		// because LLVM only accept identifier of the form: '[%@][a-zA-Z$._][a-zA-Z$._0-9]*'
		// we have to convert illegal identifier into legal one
		for(std::string::const_iterator i = ucs4_to_utf8_temp.begin(), e = ucs4_to_utf8_temp.end(); i != e; ++i)
		{
			char c = *i;
#if 1 // NOTE: truncate llvm-rewrite for now -- not useful
			if(c == '<') break;
#endif
			if( ((i == ucs4_to_utf8_temp.begin()) ? false : isdigit(c)) || isalpha(c) || (c == '_') || (c == '.') )
			{
				utf8_to_llvm_temp.push_back(c);
			}
			else
			{
				utf8_to_llvm_temp.push_back('$');
				utf8_to_llvm_temp.append(toAsciiNumber(c));
				utf8_to_llvm_temp.push_back('$');
			}
		}

		return utf8_to_llvm_temp;
	}

	void reset()
	{
#if 1 // NOTE: for debugging only
		std::cout << "NameManglingVisitor: " << stream.str() << std::endl;
#endif
		stream.str("");
		named_scoped        = false;
		dependent_component = false;
	}

	std::stringstream stream;

private:
	static const char* toAsciiNumber(char c)
	{
		static char buffer[4];
		snprintf(buffer, 3, "%d", c);
		return buffer;
	}

	void resolveAndVisit(ASTNode* node)
	{
        ASTNode* resolved_type = ASTNodeHelper::findUniqueTypeResolution(node);
        if(resolved_type)
        {
			dependent_component = true;
			visitResolvedType(resolved_type);
			dependent_component = false;
        }
	}

	void visitResolvedType(ASTNode* node)
	{
		if(isa<TypeSpecifier>(node))
			visit(*cast<TypeSpecifier>(node));
		else if(isa<ClassDecl>(node))
			visit(*cast<ClassDecl>(node));
		else
			UNREACHABLE_CODE();
	}

	void visitIdentifier(Identifier* ident)
	{
		if(isa<SimpleIdentifier>(ident))
			visit(*cast<SimpleIdentifier>(ident));
		else if(isa<TemplatedIdentifier>(ident))
			visit(*cast<TemplatedIdentifier>(ident));
		else
			UNREACHABLE_CODE();
	}

	std::string ucs4_to_utf8_temp;
	std::string utf8_to_llvm_temp;

	bool named_scoped;
	bool dependent_component;
};

} } } }

#endif /* ZILLIANS_LANGUAGE_TREE_VISITOR_GENERAL_NAMEMANGLINGVISITOR_H_ */
