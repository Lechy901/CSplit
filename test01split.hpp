
#include <string>
#include <sstream>
#include <tuple>
#include <iostream>
#include <utility>
#include <type_traits>

namespace splitter {
	template<typename ... TLL>
	class SplitC {
		// pomocne funkce na ziskavani charu z tuple
		char assign_to_char(char ch) {
			return ch;
		}
		template<typename T>
		char assign_to_char(T ch) {
			throw 42;
		}
		std::tuple<TLL ...> tup;
	public:
		SplitC(std::tuple<TLL ...> &t) : tup(t) {};

		// pro I == 0 nesmime sahat na I - 1 index v tuple
		// firstChar urcuje jestli jsme v minule iteraci nacetli lvalue referenci
		template< std::size_t I = 0, typename ... TL>
		typename std::enable_if<I == 0, void>::type
		split(std::istream & ss, bool firstChar) {
			bool b0 = std::is_lvalue_reference<typename std::tuple_element<I, std::tuple<TL...>>::type>::value;
			if (b0) {
				split<I + 1, TL...>(ss, true);
			} else {
				if (!firstChar) {
					std::string s;
					if (ss.eof())
						throw std::logic_error("necekany eof");
					char ch = ss.get();
					char delim = assign_to_char(std::get<I>(tup));
					if (ch != delim)
						throw std::logic_error("prvni znak neodpovida prvnimu delimiteru");
					split<I + 1, TL...>(ss, false);
				}
			}
		}

		template< std::size_t I = 0, typename ... TL>
		typename std::enable_if<I < sizeof...(TL) - 1 && I != 0, void>::type
		split(std::istream & ss, bool firstChar) {
			bool b0 = std::is_lvalue_reference<typename std::tuple_element<I, std::tuple<TL...>>::type>::value;
			if (b0) {
				split<I + 1, TL...>(ss, true);
			} else {
				if (firstChar) {
					std::string s;
					char delim = assign_to_char(std::get<I>(tup));
					std::getline(ss, s, delim);
					std::stringstream sss(s);
					auto& a = std::get<I - 1>(tup);
					sss >> a;
					// check jestli promenna sezrala cely stream spravne
					if (sss.fail())
						throw std::logic_error("promenna nevybrala cely stream");
					sss.get();
					if (!sss.fail())
						throw std::logic_error("promenna nevybrala cely stream");

				} else {
					// zkusime nacist char ze vstupu a porovname
					if (ss.eof())
						throw std::logic_error("necekany eof");
					char ch = ss.get();
					char delim = assign_to_char(std::get<I>(tup));
					if (ch != delim)
						throw std::logic_error("vstupni znak neodpovida zadanemu charu");
				}
				split<I + 1, TL...>(ss, false);
			}
		}
		
		// pro posledni iteraci se dal nezarekurzime
		template< std::size_t I = 0, typename ... TL>
		typename std::enable_if<I == sizeof...(TL) - 1, void>::type
		split(std::istream & ss, bool firstChar) {
			bool b0 = std::is_lvalue_reference<typename std::tuple_element<I, std::tuple<TL...>>::type>::value;
			if (b0) {
				std::string s;
				std::getline(ss, s, '\n');
				std::stringstream sss(s);
				auto& a = std::get<I>(tup);
				sss >> a;
				//check jestli promenna vyzrala cely stream
				if (sss.fail())
					throw std::logic_error("promenna nevybrala cely stream");
				sss.get();
				if (!sss.fail())
					throw std::logic_error("promenna nevybrala cely stream");
			} else {
				if (firstChar) {
					//nacist vstup a ulozit do promenne na I - 1
					std::string s;
					char delim = assign_to_char(std::get<I>(tup));
					std::getline(ss, s, delim);
					std::stringstream sss(s);
					auto& a = std::get<I - 1>(tup);
					sss >> a;
					if (sss.fail())
						throw std::logic_error("promenna nevybrala cely stream");
					sss.get();
					if (!sss.fail())
						throw std::logic_error("promenna nevybrala cely stream");
				} else {
					// nacist jeden znak a porovnat
					if (ss.eof())
						throw std::logic_error("necekany eof");
					char ch = ss.get();
					char delim = assign_to_char(std::get<I>(tup));
					if (ch != delim)
						throw std::logic_error("vstupni znak neodpovida zadanemu charu");
				}
			}
		}
	};
	template<typename ... TL>
	inline std::istream &operator>>(std::istream &s, SplitC<TL ...> &&c) {
		c.template split<0, TL...>(s, false);
		return s;
	}

	// check jestli argumenty typove odpovidaji zadani
	template<std::size_t I = 0, typename ... TL>
	inline typename std::enable_if<I == sizeof...(TL)-1, void>::type check_args();

	template< std::size_t I = 0, typename ... TL>
	inline typename std::enable_if<I < sizeof...(TL)-1, void>::type check_args() {
		// check jestli nejsou dve lvalue vedle sebe
		static_assert(!(std::is_lvalue_reference<typename std::tuple_element<I, std::tuple<TL...>>::type>::value == std::is_lvalue_reference<typename std::tuple_element<I + 1, std::tuple<TL...>>::type>::value && std::is_lvalue_reference<typename std::tuple_element<I, std::tuple<TL...>>::type>::value), "Nesmi byt 2 lvalue za sebou");
		// kdyz je rvalue, check jestli je char
		static_assert(std::is_lvalue_reference<typename std::tuple_element<I, std::tuple<TL...>>::type>::value || std::is_same<typename std::tuple_element<I, std::tuple<TL...>>::type, char>::value, "rvalue musi byt char");
		check_args<I + 1, TL ...>();
	}

	template<std::size_t I = 0, typename ... TL>
	inline typename std::enable_if<I == sizeof...(TL)-1, void>::type check_args() {
		// v posledni iteraci checkujeme jen jestli je lvalue / rvalue char
		static_assert(std::is_lvalue_reference<typename std::tuple_element<I, std::tuple<TL...>>::type>::value || std::is_same<typename std::tuple_element<I, std::tuple<TL...>>::type, char>::value, "rvalue musi byt char");
	}
	template<typename ... TL>
	inline SplitC<TL ...> split(TL && ... plist) {
			std::tuple<TL ...> tup(std::forward<TL>(plist) ...);
			check_args< 0 , TL ...>();
			return SplitC<TL ...>(tup);
	}

}

