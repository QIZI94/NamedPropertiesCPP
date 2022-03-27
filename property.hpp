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

#include <functional>

/** Named property */
namespace nap{

namespace detail{

template <typename T>
inline constexpr bool is_const = std::is_const_v<typename std::remove_pointer<typename std::remove_reference<T>::type>::type>;

template <typename... Args> 
struct are_const{
  static constexpr bool value {(is_const<Args> || ...)};
};
}

template<class InterfaceImpl>
/**
 * Property template is used to interact with named members(and/or other getters/setters) of a class objects.
 * (but this is not limited to object only an intended use, this can be used also in cases of sharing local context without using any object)
 *  
 * In order to use PropertyTemplate you need to define 'interface' which is used for interaction between named properties and visitor.
 * To satisfy type requirement following specification needs to be defined:
 *   * any_type 		- this is type definition which will be used to store/read value of the property.
 *   * string_type 		- type which will be used for passing the name of the named property.
 *   * string_type_ref 	- type which will be used for passing the name of the named property by refference.
 * 
 * To satisfy function requirement following specification needs to be defined:
 * 	 ** Interface for user to use
 *   * any_type make_any(type) 				- function which will take value and returns any_type.
 * 	 * any_type make_any(const type) 		- function which will take const value and returns any_type.
 *   * type cast_any<type>(any_type&) 		- cast value contained by any_type to required type.
 *   * type cast_any<type>(const any_type&)	- cast value contained by any_type to required const type.
 *   * bool is_any<type>(const any_type&)	- checks if required type is same as one contained in any type
 *   ** Interface for PropertyTemplate to use
 *   * any_type read(type)                  - used by PropertyTemplate to read type value type into any type
 *   * any_type read(const type)            - used by PropertyTemplate to read const type type value into any type
 *   * void write(type&, any_type&)         - used by PropertyTemplate to write any type value into type value
 *  Note: type or required type is meant as input type before its contained in any_type and when its returned from any_type
 **/
class PropertyTemplate{
public: //type definitions
	class Visitor{
	public: // functions
		using PropertyVisitFunc = std::function<bool(const PropertyTemplate&)>;
		
		Visitor(const PropertyVisitFunc& propertyVistFunc) : m_visitProperty(propertyVistFunc){}
		/**
		 * Executes visitor functor and passing property as argument. 
		 * 
		 * @param property property refference which will be passed into functor as argument.
		 * 
		 * @return return passed return value of functor.
		 **/
		bool visit(const PropertyTemplate& property) const {
			return m_visitProperty(property);
		}

		template<class Callable, class PropertyArray>
		/**
		 * Run visitor functor over container that supports usage in range for loop(foreach).
		 * 
		 * @param visitor functor which will be called for each property.
		 * @param properties container which support range loop(foreach) operation.
		 * 
		 * @return true when all visitor calls returned true, otherwise false
		 * 
		 * @note First call of of visitor which returns false will also break the loop.
		 **/
		static bool visit(const Callable& visitor, const PropertyArray& properties){
			for(const auto& property : properties){
				if(visitor.visit(property) == false){
					return false;
				}
			}
			return true;
		}
		template<class Callable>
		/**
		 * Run visitor functor over initializer list of properties.
		 * 
		 * @param visitor functor which will be called for each property.
		 * @param ilProperties initializer list which takes list initialization such as e.g. {1,2,3..}.
		 * 
		 * @return true when all visitor calls returned true, otherwise false
		 * 
		 * @note First call of of visitor which returns false will also break the loop.
		 **/
		static bool visit(const Callable& visitor, std::initializer_list<PropertyTemplate> ilProperties){
			for(const auto& property : ilProperties){
				if(visitor.visit(property) == false){
					return false;
				}
			}
			return true;
		}

	private: // members
		const PropertyVisitFunc m_visitProperty;
	};

	using interface         = InterfaceImpl;
	using string_type       = typename interface::string_type;
	using string_type_ref   = typename interface::string_type_ref;
	using any_type          = typename interface::any_type;

    using WriteFunction     = std::function<void(any_type& entry)>;
	using ReadFunction      = std::function<void(any_type& entry)>;
	
public: // static functions
	// wrapers
	template<typename T>
	/**
	 * Wraper function which wraps interface's implementation of make_any.
	 * 
	 * @param value passes value to interface implementation of make_any.
	 * @return value of interface implementation of make_any
	 **/
	static auto make_any(const T& value){
		return interface::template make_any(value);
	}
	template<typename T>
	/**
	 * Wraper function which wraps interface's implementation of make_any.
	 * 
	 * @param value passes value to interface implementation of make_any.
	 * @return value of interface implementation of make_any
	 **/
	static auto make_any(T& value){
		return interface::template make_any(value);
	}

	template<typename T>
	/**
	 * Wraper function which wraps interface's implementation of cast_any.
	 * 
	 * @param any passes const refference to any_type parameter,
	 *  as an agument for interface implementation of cast_any.
	 * @return value of interface implementation of cast_any
	 **/
	static auto cast_any(const any_type& any){
		return interface::template cast_any<T>(any);
	}

	template<typename T>
	/**
	 * Wraper function which wraps interface's implementation of cast_any.
	 * 
	 * @param any passes non-const refference to any_type parameter,
	 *  as an agument for interface implementation of cast_any.
	 * @return value of interface implementation of cast_any
	 **/
	static auto cast_any(any_type& any){
		return interface::template cast_any<T>(any);
	}
	template<typename T>
	/**
	 * Wraper function which wraps interface's implementation of is_any.
	 * 
	 * @param any passes const refference to any_type parameter,
	 *  as an agument for interface implementation of is_any.
	 * @return true if type of T, otherwise false.
	 **/
	static bool is_any(const any_type& any){
		return interface::template is_any<T>(any);
	}

	template<typename T>
	/**
	 * Wraper function which wraps interface's implementation of read.
	 * 
	 * @param value passes value by non-const reference.
	 * @return value of interface implementation of read
	 **/
	static auto read(T& value){
		return interface:: template read<T>(value);
	}

	template<typename T>
	/**
	 * Wraper function which wraps interface's implementation of read.
	 * 
	 * @param value passes value by const reference.
	 * @return value of interface implementation of read
	 **/
	static auto read(const T& value){
		return interface:: template read<T>(value);
	}

	template<typename T>
	/**
	 * Wraper function which wraps interface's implementation of write.
	 * 
	 * @param any passes any by any_type reference.
	 * @return value of interface implementation of write
	 **/
	static void write(T& value ,any_type& any){
		interface:: template write<T>(value, any);
	}

	// helpers
	template<class Callable, class... Args>
	/**
	 * Helper function which executes callable functor,
	 *  when not in cont context.
	 * 
	 * @param callable functor to be executed.
	 * @param args arguments that will be passed to the functor.
	 **/
	static void ExecWhenNotConst(Callable callable, Args... args){
		if constexpr(!detail::are_const<Args...>::value){
			callable(std::forward<Args>(args) ...);
		}
	}

	template<typename T>
	/**
	 * Taks reference of the parameter, and returns it.
	 * This is a clean way of forcing property contruction in const context. 
	 * 
	 * @param member const refference to argument of T.
	 * @return member as const refference of T.
	 **/
	static const T& ReadOnly(const T& member){
		return member;
	}

public: // member functions
	PropertyTemplate(string_type name, const ReadFunction& readFunc, const WriteFunction& writeFunc) : m_name(name), m_read(readFunc), m_write(writeFunc) {}
	PropertyTemplate(string_type name) : m_name(name), m_read(nullptr), m_write(nullptr) {}

	template<typename T>
	PropertyTemplate(string_type name, const T& constMember) : m_name(name),
	m_read(
		[&constMember](typename interface::any_type& entry){
			entry = read<T>(constMember);
		}
	), 
	m_write({})
	{}

	template<typename T>
	PropertyTemplate(string_type name, T& member) : m_name(name), 
	m_read(
		[&member](typename interface::any_type& entry){
			entry = read<T>(member);
		}
	), 
	m_write(
		[&member](typename interface::any_type& entry){
			write<T>(member, entry);
		}
	) 
	{}
	/**
	 * Returns propert name.
	 * 
	 * @return const string type rfference of property name.
	 **/
	const string_type_ref name() const{return m_name;}
	/**
	 * Fills entry of any_type with value from corespoding property.
	 * Passes non-const refference of entry to read functor.
	 * 
	 * @param entry non-const refference to any_type variable.
	 **/
	void read(any_type& entry) const{m_read(entry);}
	/**
	 * Passes const refferenc of entry to write functor.
	 * 
	 * @param entry const refference to any_type variable.
	 **/
	void write(any_type& entry) const{m_write(entry);}
	/**
	 * Checks if read functor was provided.
	 * 
	 * @return true when when read functor was provided, otherfwise false.
	 **/
	bool isReadable() const{ return (m_read != nullptr);}
	/**
	 * Checks if write functor was provided.
	 * 
	 * @return true when when write functor was provided, otherfwise false.
	 **/
	bool isWritable() const{ return (m_write != nullptr);}
	/**
	 * Checks if any operation can be done over property.
	 * 
	 * @return true when no read and write functors were provided, otherfwise false.
	 * 
	 * @note This has use case when you need to pass static string (e.g. class name),
	 *  or to be used as a separator/category when defining properties.
	 **/
	bool isNameOnly() const{return (!isReadable() && !isWritable());}

private: // members
	const string_type_ref m_name;
	const ReadFunction m_read;
	const WriteFunction m_write;
};
}