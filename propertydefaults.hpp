/******************************  <MIT License>  ******************************
 * Copyright (c) 2021 QIZI94
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *****************************************************************************/

#pragma once
#include "property.hpp"

#include <any>
#include <string_view>

namespace nap{

namespace detail{
/**
 * Default implementation of interface which uses C++17 std::any
 *  and tries to use references for objects which sizes are greater than a pointer,
 *  taking advantage of small object optimization of std::any.
 **/
struct DefaultInterface{
	using any_type          = std::any;
	using string_type       = std::string_view;
	using string_type_ref   = std::string_view;
	template<typename T>
	static any_type make_any(const T& value){
		using type = std::remove_reference_t<T>;		
		return std::make_any<const type*>(&value);
	}
	template<typename T>
	static any_type make_any(T& value){
		using type = std::remove_reference_t<T>;		
		return std::make_any<type*>(&value);
	}

	template<typename T>
	static auto& cast_any(const any_type& any){
		using type = std::remove_reference_t<T>;
		return static_cast<const type&>(*std::any_cast<const type*>(any));
	}
	template<typename T>
	static auto& cast_any(any_type& any){
		if constexpr (std::is_reference_v<T>){
			using type = std::remove_reference_t<T>;
			return static_cast<type&>(*std::any_cast<type*>(any));
		}
		else{
			return static_cast<const T&>(*std::any_cast<const T*>(any));
		}
	}
	template<typename T>
	static bool is_any(const any_type& any){
		using type = std::remove_reference_t<T>;
		return (any.type() == typeid(type*));
	}
	template<typename T>
	static auto read(T& value){
		return make_any<const T&>(value);
	}
	template<typename T>
	static auto read(const T& value){
		return make_any<T>(value);
	}
	template<typename T>
	static void write(T& value, any_type& any){
		try{
			value = std::move(cast_any<T&>(any));
			return;
		}
		catch(const std::bad_any_cast&){/* write by move failed */}
		// try write by copy
		value = cast_any<const T&>(any);
	}

	DefaultInterface() = delete;
};

}

/**
 * Property template with default implementation of the interface using std::any and small object optimizations.
 **/
using Property = PropertyTemplate<detail::DefaultInterface>;
}