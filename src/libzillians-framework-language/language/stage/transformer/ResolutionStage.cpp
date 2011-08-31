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

#include "language/stage/transformer/ResolutionStage.h"
#include "language/stage/transformer/visitor/ResolutionStageVisitor.h"
#include "language/tree/visitor/general/NodeTypeNameVisitor.h"
#include "language/tree/visitor/general/NodeInfoVisitor.h"
#include "language/resolver/Resolver.h"
#include "language/context/ParserContext.h"

namespace zillians { namespace language { namespace stage {

ResolutionStage::ResolutionStage() : disable_type_inference(false), total_resolved_count(0)
{ }

ResolutionStage::~ResolutionStage()
{ }

const char* ResolutionStage::name()
{
	return "type_inference";
}

void ResolutionStage::initializeOptions(po::options_description& option_desc, po::positional_options_description& positional_desc)
{
    option_desc.add_options()
    ("no-type-inference", "disable type inference system so every type declaration must be made explicitly");
}

bool ResolutionStage::parseOptions(po::variables_map& vm)
{
	disable_type_inference = (vm.count("no-type-inference") > 0);

	return true;
}

bool ResolutionStage::execute()
{
	// TODO remove this
	return true;

	if(!resolveTypes()) return false;
	if(!resolveSymbols()) return false;

	return true;
}

bool ResolutionStage::resolveTypes()
{
	if(getParserContext().program)
	{
		tree::Program& program = *getParserContext().program;

		Resolver resolver;
		visitor::ResolutionStageVisitor visitor(visitor::ResolutionStageVisitor::Target::TYPE_RESOLUTION, program, resolver);

		std::size_t last_unresolved_count = 0;
		while(true)
		{
			visitor.reset();
			visitor.visit(program);

			total_resolved_count += visitor.get_resolved_count();
			std::size_t unresolved_count = visitor.get_unresolved_count();

			if(unresolved_count == 0 || unresolved_count == last_unresolved_count)
			{
				break;
			}
			else
			{
				BOOST_ASSERT(unresolved_count < last_unresolved_count && "make sure we are making progress");
				last_unresolved_count = unresolved_count;
			}
		}

		if(last_unresolved_count > 0)
		{
			total_unresolved_count += last_unresolved_count;
			tree::visitor::NodeTypeNameVisitor name;
			tree::visitor::NodeInfoVisitor info;
			for(__gnu_cxx::hash_set<ASTNode*>::iterator it = visitor.unresolved_nodes.begin(); it != visitor.unresolved_nodes.end(); ++it)
			{
				if(tree::isa<tree::TypeSpecifier>(*it))
				{
					tree::TypeSpecifier* specifier = tree::cast<tree::TypeSpecifier>(*it);
					if(specifier->type == tree::TypeSpecifier::ReferredType::UNSPECIFIED)
					{
						name.visit(**it);
						info.visit(**it);
						LOG4CXX_ERROR(Logger::TransformerStage, L"failed to resolve type specifier: " << specifier->referred.unspecified->toString() << ", full qualified id: " << info.stream.str());
						info.reset();
					}
				}
			}
			return false;
		}
	}
	else
	{
		LOG4CXX_ERROR(Logger::TransformerStage, L"empty program node");
	}

	return true;
}

bool ResolutionStage::resolveSymbols()
{
	if(getParserContext().program)
	{
		tree::Program& program = *getParserContext().program;

		Resolver resolver;
		visitor::ResolutionStageVisitor visitor(visitor::ResolutionStageVisitor::Target::SYMBOL_RESOLUTION, program, resolver);

		std::size_t last_unresolved_count = 0;
		while(true)
		{
			visitor.reset();
			visitor.visit(program);

			total_resolved_count += visitor.get_resolved_count();
			std::size_t unresolved_count = visitor.get_unresolved_count();

			if(unresolved_count == 0 || unresolved_count == last_unresolved_count)
			{
				break;
			}
			else
			{
				BOOST_ASSERT(unresolved_count < last_unresolved_count && "make sure we are making progress");
				last_unresolved_count = unresolved_count;
			}
		}

		if(last_unresolved_count > 0)
		{
			total_unresolved_count += last_unresolved_count;
			tree::visitor::NodeTypeNameVisitor name;
			tree::visitor::NodeInfoVisitor info;
			for(__gnu_cxx::hash_set<ASTNode*>::iterator it = visitor.unresolved_nodes.begin(); it != visitor.unresolved_nodes.end(); ++it)
			{
				if(tree::isa<tree::Identifier>(*it))
				{
					tree::Identifier* identifier = tree::cast<tree::Identifier>(*it);
//					if(specifier->type == tree::TypeSpecifier::ReferredType::UNSPECIFIED)
//					{
//						name.visit(**it);
//						info.visit(**it);
//						LOG4CXX_ERROR(Logger::TransformerStage, L"failed to resolve type specifier: " << specifier->referred.unspecified->toString() << ", full qualified id: " << info.stream.str());
//						info.reset();
//					}
				}
			}
			return false;
		}
	}
	else
	{
		LOG4CXX_ERROR(Logger::TransformerStage, L"empty program node");
	}

	return true;
}

std::size_t ResolutionStage::get_resolved_count()
{
	return total_resolved_count;
}

std::size_t ResolutionStage::get_unresolved_count()
{
	return total_unresolved_count;
}

} } }
