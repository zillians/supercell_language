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
 * @date Aug 5, 2011 sdk - Initial version created.
 */

#ifndef ZILLIANS_LANGUAGE_TREE_PACKAGE_H_
#define ZILLIANS_LANGUAGE_TREE_PACKAGE_H_

#include "language/tree/ASTNode.h"
#include "language/tree/basic/Identifier.h"
#include "utility/TemplateTricks.h"

namespace zillians { namespace language { namespace tree {

/**
 * Package is used to represent the hierarchical structure of a program.
 * Every ASTNode, except Package and Program, must be contained by a Package.
 */
struct Package : public ASTNode
{
	DEFINE_VISITABLE();
	DEFINE_HIERARCHY(Package, (Package)(ASTNode));

	explicit Package(SimpleIdentifier* _id) : id(_id)
	{
		id->parent = this;
	}

	Package* findPackage(const std::wstring& name)
	{
		foreach(i, children)
		{
			if((*i)->id->toString().compare(name) == 0)
				return (*i);
		}
		return NULL;
	}

	void addPackage(Package* package)
	{
		package->parent = this;
		children.push_back(package);
	}

	void addObject(ASTNode* object)
	{
		object->parent = this;
		objects.push_back(object);
	}

	SimpleIdentifier* id;
	std::vector<Package*> children;
	std::vector<ASTNode*> objects;
};

} } }

#endif /* ZILLIANS_LANGUAGE_TREE_PACKAGE_H_ */