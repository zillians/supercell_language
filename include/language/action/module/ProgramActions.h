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

#ifndef ZILLIANS_LANGUAGE_ACTION_MODULE_PROGRAMACTIONS_H_
#define ZILLIANS_LANGUAGE_ACTION_MODULE_PROGRAMACTIONS_H_

#include "language/action/detail/SemanticActionsDetail.h"

namespace zillians { namespace language { namespace action {

struct program
{
	DEFINE_ATTRIBUTES(Program*)
	DEFINE_LOCALS()

	BEGIN_ACTION(init)
	{
//		BOOST_MPL_ASSERT(( boost::is_same<_value_t, Program*&> ));
		if(getParserContext().program)
		{
			_value = getParserContext().program;
		}
		else
		{
			_value = new Program();
			getParserContext().program = _value;
		}
	}
	END_ACTION

	BEGIN_ACTION(append_package_decl)
	{
		getParserContext().active_package = _attr(0);
	}
	END_ACTION

	BEGIN_ACTION(append_import_decl)
	{
	}
	END_ACTION

	BEGIN_ACTION(append_declaration)
	{
		printf("program::append_declaration attr(0) type = %s\n", typeid(_attr_t(0)).name());
		if(!!getParserContext().active_package)
			getParserContext().active_package->addObject(_attr(0));
	}
	END_ACTION

	BEGIN_ACTION(init2)
	{
		printf("program::init2 attr(0) type = %s\n", typeid(_attr_t(0)).name());
		printf("program::init2 attr(1) type = %s\n", typeid(_attr_t(1)).name());
		printf("program::init2 attr(2) type = %s\n", typeid(_attr_t(2)).name());
	}
	END_ACTION
};

} } }

#endif /* ZILLIANS_LANGUAGE_ACTION_MODULE_PROGRAMACTIONS_H_ */
