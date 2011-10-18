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

#include "language/stage/parser/ASTDeserializationStage.h"
#include "language/stage/parser/visitor/ASTDeserializationStageVisitor.h"
#include "language/context/ParserContext.h"
#include "language/tree/ASTNodeSerialization.h"
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/filesystem.hpp>

namespace zillians { namespace language { namespace stage {

ASTDeserializationStage::ASTDeserializationStage() : enabled_load(false)
{ }

ASTDeserializationStage::~ASTDeserializationStage()
{ }

const char* ASTDeserializationStage::name()
{
	return "AST Deserialization Stage";
}

std::pair<shared_ptr<po::options_description>, shared_ptr<po::options_description>> ASTDeserializationStage::getOptions()
{
	shared_ptr<po::options_description> option_desc_public(new po::options_description());
	shared_ptr<po::options_description> option_desc_private(new po::options_description());

	option_desc_public->add_options()
		("load-ast", po::value<std::string>(), "load serialized AST file as root");

	foreach(i, option_desc_public->options()) option_desc_private->add(*i);

	option_desc_private->add_options();

	return std::make_pair(option_desc_public, option_desc_private);
}

bool ASTDeserializationStage::parseOptions(po::variables_map& vm)
{
	if(vm.count("load-ast") > 0)
	{
		enabled_load = true;
		ast_file_to_load = vm["load-ast"].as<std::string>();
	}

	inputs = vm["input"].as<std::vector<std::string>>();

	return true;
}

bool ASTDeserializationStage::execute(bool& continue_execution)
{
	// prepare the global parser context
	if(!hasParserContext())
		setParserContext(new ParserContext());

	if(enabled_load)
	{
		ASTNode* deserialized = tryDeserialize(ast_file_to_load);
		if(!deserialized) return false;

		getParserContext().program->root = tree::cast<tree::Package>(deserialized);

	}

	foreach(i, inputs)
	{
		boost::filesystem::path p(*i);
		if(strcmp(p.extension().c_str(), ".ast") == 0)
		{
			ASTNode* deserialized = tryDeserialize(*i);
			if(!deserialized) return false;

			// TODO merge with imported root ast
			//getParserContext().program->imported_root
		}
	}

	return true;
}

tree::ASTNode* ASTDeserializationStage::tryDeserialize(const std::string& s)
{
	std::ifstream ifs(s);
	if(!ifs.good()) return NULL;

	boost::archive::text_iarchive ia(ifs);
	tree::ASTNode* from_serialize = NULL;
	ia >> from_serialize;

	visitor::ASTDeserializationStageVisitor<boost::archive::text_iarchive> deserialzer(ia);
	deserialzer.visit(*from_serialize);

	return from_serialize;
}

} } }
