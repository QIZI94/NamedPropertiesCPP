#pragma once

#include <functional>
#include <any>
#include <string_view>
#include <array>
/** Universal property */
namespace unip{

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
	static any_type make_any(T value){
		if constexpr(sizeof(T) > sizeof(void*)){
			return std::make_any<const T&>(value);
		}
		else{
			return std::make_any<T>(value);
		}
	}
	template<typename T>
	static auto cast_any(const any_type& any){
		if constexpr(sizeof(T) > sizeof(void*)){
			return std::any_cast<const T&>(any);
		}
		else{
			return std::any_cast<T>(any);
		}
	}
	template<typename T>
	static T& cast_any(any_type& any){
		return std::any_cast<T&>(any);
	}
	template<typename T>
	static bool is_any(const any_type& any){
		return (any.type() == typeid(T));
	}

	DefaultInterface() = delete;
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
 * 	 * any_type make_any(type) 				- function which will take value and returns any_type.
 *   * type cast_any<type>(any_type&) 		- cast value contained by any_type to required type.
 *   * type cast_any<type>(const any_type&)	- cast value contained by any_type to required const type.
 *   * bool is_any<type>(const any_type&)	- checks if required type is same as one contained in any type
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

	using WriteFunction     = std::function<void(const std::any& entry)>;
	using ReadFunction      = std::function<void(std::any& entry)>;
	
	using interface         = InterfaceImpl;
	using string_type       = typename interface::string_type;
	using string_type_ref   = typename interface::string_type_ref;
	using any_type          = typename interface::any_type;
	
public: // static functions
	// wrapers
	template<typename T>
	/**
	 * Wraper function which wraps interface's implementation of make_any.
	 * 
	 * @param value passes value to interface implementation of make_any.
	 * @return value of interface implementation of make_any
	 **/
	static auto make_any(T value){
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

	// helpers
	template<class Callable, class... Args>
	/**
	 * Helper function which executes callable functor,
	 *  when not in cont context.
	 * 
	 * @param callable functor to be executed.
	 * @param args arguments that will be passed to the functor.
	 **/
	static void ExecWhenNotConst(const Callable& callable, Args&... args){
		callable(std::forward<Args>(args)...);
	}
	/**
	 * Helper function which does not executes callable functor,
	 *  when in cont context.
	 **/
	template<class Callable, class... Args>
	static void ExecWhenNotConst(const Callable&, const Args&...){}

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
	PropertyTemplate(string_type name, const ReadFunction& readFunc) : m_name(name), m_read(readFunc), m_write(nullptr) {}
	PropertyTemplate(string_type name, const WriteFunction& writeFunc) : m_name(name), m_read(nullptr), m_write(writeFunc) {}
	PropertyTemplate(string_type name) : m_name(name), m_read(nullptr), m_write(nullptr) {}

	template<typename T>
	PropertyTemplate(string_type name, T& member, const ReadFunction& readFunc) : m_name(name),
	m_read(readFunc),
	m_write(
		[&member](const typename interface::any_type& entry){
			member = cast_any<T>(entry);
		}
	)
	{}
	template<typename T>
	PropertyTemplate(string_type name, T& member, const WriteFunction& writeFunc) : m_name(name),
	m_read(
		[&member](typename interface::any_type& entry){
			entry = make_any<T>(member);
		}
	),
	m_write(writeFunc)
	{}

	template<typename T>
	PropertyTemplate(string_type name, const T& constMember) : m_name(name),
	m_read(
		[&constMember](typename interface::any_type& entry){
			entry = make_any<T>(constMember);
		}
	), 
	m_write({})
	{}

	template<typename T>
	PropertyTemplate(string_type name, T& member) : m_name(name), 
	m_read(
		[&member](typename interface::any_type& entry){
			entry = make_any<T>(member);
		}
	), 
	m_write(
		[&member](const typename interface::any_type& entry){
			member = cast_any<T>(entry);
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
	void write(const any_type& entry) const{m_write(entry);}
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

/**
 * Property template with default implementation of the interface using std::any and small object optimizations.
 **/
using Property = PropertyTemplate<detail::DefaultInterface>;
}