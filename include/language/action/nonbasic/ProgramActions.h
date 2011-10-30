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
	DEFINE_ATTRIBUTES(void)
	DEFINE_LOCALS(LOCATION_TYPE)

	BEGIN_ACTION(define_active_package)
	{
		SimpleIdentifier *simple_ident = cast<SimpleIdentifier>(_param(1));

		Package *package = getParserContext().parent_package->findPackage(simple_ident->toString());
		if(!package)
		{
			BIND_CACHED_LOCATION(package = new Package(simple_ident));
			getParserContext().parent_package->addPackage(package);
		}

		getParserContext().active_package = package;

		if(_param(0).is_initialized())
		{
			if(getParserContext().active_package->annotations)
			{
				getParserContext().active_package->annotations->merge(**_param(0));
			}
			else
			{
				getParserContext().active_package->setAnnotations(*_param(0));
			}
		}
	}
	END_ACTION

	BEGIN_ACTION(append_import)
	{
#ifdef DEBUG
		printf("program::append_import_decl param(0) type = %s\n", typeid(_param_t(0)).name());
#endif
		if(getParserContext().program)
		{
			Import* import = new Import(_param(0)); BIND_CACHED_LOCATION(import);
			getParserContext().program->addImport(import);
		}
	}
	END_ACTION

	BEGIN_ACTION(append_alias)
	{
	}
	END_ACTION

	BEGIN_ACTION(append_global_decl)
	{
#ifdef DEBUG
		printf("program::append_global_decl param(0) type = %s\n", typeid(_param_t(0)).name());
#endif
		if(getParserContext().active_package)
			getParserContext().active_package->addObject(_param(0));
	}
	END_ACTION
};

} } }

#endif /* ZILLIANS_LANGUAGE_ACTION_MODULE_PROGRAMACTIONS_H_ */
