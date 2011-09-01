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

#ifndef ZILLIANS_LANGUAGE_ACTION_BASIC_IDENTIFIERACTIONS_H_
#define ZILLIANS_LANGUAGE_ACTION_BASIC_IDENTIFIERACTIONS_H_

#include "language/action/detail/SemanticActionsDetail.h"

namespace zillians { namespace language { namespace action {

struct identifier
{
	DEFINE_ATTRIBUTES(SimpleIdentifier*)
	DEFINE_LOCALS()

	BEGIN_ACTION(init)
	{
#ifdef DEBUG
		printf("identifier attr(0) type = %s\n", typeid(_attr_t(0)).name());
#endif
		_value = new SimpleIdentifier(_attr(0));
	}
	END_ACTION
};

struct nested_identifier
{
	DEFINE_ATTRIBUTES(Identifier*)
	DEFINE_LOCALS()

	BEGIN_ACTION(init)
	{
#ifdef DEBUG
		printf("nested_identifier attr(0) type = %s\n", typeid(_attr_t(0)).name());
		printf("nested_identifier attr(1) type = %s\n", typeid(_attr_t(1)).name());
#endif
		if(_attr(1).size() == 1)
			_value = _attr(0);
		else
		{
			_value = new NestedIdentifier();
			cast<NestedIdentifier>(_value)->appendIdentifier(_attr(0));
			deduced_foreach_value(i, _attr(1))
				cast<NestedIdentifier>(_value)->appendIdentifier(i);
		}
	}
	END_ACTION
};

struct template_arg_identifier
{
	DEFINE_ATTRIBUTES(Identifier*)
	DEFINE_LOCALS()

	BEGIN_ACTION(init)
	{
#ifdef DEBUG
		printf("template_arg_identifier attr(0) type = %s\n", typeid(_attr_t(0)).name());
		printf("template_arg_identifier attr(1) type = %s\n", typeid(_attr_t(1)).name());
#endif
		if(_attr(1).is_initialized())
		{
			_value =  new TemplatedIdentifier(TemplatedIdentifier::Usage::ACTUAL_ARGUMENT, _attr(0));
			deduced_foreach_value(i, *_attr(1))
				cast<TemplatedIdentifier>(_value)->appendArgument(i);
		}
		else
			_value = _attr(0);
	}
	END_ACTION
};

struct template_param_identifier
{
	DEFINE_ATTRIBUTES(Identifier*)
	DEFINE_LOCALS()

	BEGIN_ACTION(init)
	{
#ifdef DEBUG
		printf("template_param_identifier attr(0) type = %s\n", typeid(_attr_t(0)).name());
		printf("template_param_identifier attr(1) type = %s\n", typeid(_attr_t(1)).name());
#endif
		if(_attr(1).is_initialized())
		{
			_value =  new TemplatedIdentifier(TemplatedIdentifier::Usage::FORMAL_PARAMETER, _attr(0));
			deduced_foreach_value(i, *(_attr(1)))
			{
				switch(i.which())
				{
				case 0:
					cast<TemplatedIdentifier>(_value)->appendParameter(boost::get<SimpleIdentifier*>(i));
					break;
				case 1:
					cast<TemplatedIdentifier>(_value)->appendParameter(new SimpleIdentifier(L"..."));
					break;
				}
			}
		}
		else
			_value = _attr(0);
	}
	END_ACTION
};

} } }

#endif /* ZILLIANS_LANGUAGE_ACTION_BASIC_IDENTIFIERACTIONS_H_ */
