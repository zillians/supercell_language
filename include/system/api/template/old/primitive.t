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
 
package api.system;

@native
class Array<T>
{
	// TODO define more array methods
}

@native
class Vector<T>
{
	@native
	public function new();
	
	@native 
	public function size():uint32;
	
	@native
	public function front():T

	@native
	public function back():T
	
	@native 
	public function pushBack(element:T):void
	
	@native
	public function popBack():void
}

// every user defined object is derived from Object
@native 
class Object
{
	@native
	public function clone():Object;
	
	@native
	public function hash():uint32;
	
	@native
	public function domain():uint32;
}

// define the null constant 
@native 
const null:Object;

// define the convention type conversion routine
@native 
function cast<X,Y>(Y value):X;

// define the convention type conversion check routine
@native 
function isa<X,Y>(Y value):boolean;
