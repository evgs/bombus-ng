/*
	Copyright (C) 2005 Cory Nelson

	This software is provided 'as-is', without any express or implied
	warranty.  In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

	1. The origin of this software must not be misrepresented; you must not
		claim that you wrote the original software. If you use this software
		in a product, an acknowledgment in the product documentation would be
		appreciated but is not required.
	2. Altered source versions must be plainly marked as such, and must not be
		misrepresented as being the original software.
	3. This notice may not be removed or altered from any source distribution.

	Contact Info:
		email:   phrosty@gmail.com
		website: http://www.int64.org

	Caveat:
		Expects wchar_t to be UTF-16 or UTF-32.
		Does not support UTF-16 surrogate pairs.
*/

#ifndef __UTF8_HPP__
#define __UTF8_HPP__

#include <string>
#include <iterator>
#include <stdexcept>

namespace utf8 {
	class utf8_error : public std::runtime_error {
	public:
		utf8_error(const char *msg) : runtime_error(msg) {}
	};

	namespace detail {
		template<typename InputIterator>
		static wchar_t decode_utf8(InputIterator &iter, InputIterator &end) {
			wchar_t ret;

			if(((*iter)&0x80) == 0) {
				ret=*iter;
				++iter;
			}
			else if(((*iter)&0x20) == 0) {
				ret=((wchar_t)((*iter)&0x1F)) << 6;

				if(++iter == end) throw utf8_error("utf-8 sequence incomplete");
				ret|=(*iter)&0x3F;
				
				++iter;
			}
			else if(((*iter)&0x10) == 0) {
				ret=((wchar_t)((*iter)&0x0F)) << 12;
				
				if(++iter == end) throw utf8_error("utf-8 sequence incomplete");
				ret|=((wchar_t)((*iter)&0x3F)) << 6;
				
				if(++iter == end) throw utf8_error("utf-8 sequence incomplete");
				ret|=(*iter)&0x3F;

				++iter;
			}
			else throw utf8_error("utf-8 not convertable to utf-16");

			return ret;
		}

		template<typename OutputIterator>
		static void encode_wchar(wchar_t ch, OutputIterator &dest) {
			if(ch <= 0x007F) {
				*dest=(char)ch;
				++dest;
			}
			else if(ch <= 0x07FF) {
				*dest = (char)(
					0xC0 |
					((ch & 0x07C0) >> 6)
				);
				++dest;

				*dest = (char)(
					0x80 |
					(ch & 0x003F)
				);
				++dest;
			}
			else {
				*dest = (char)(
					0xE0 |
					((ch & 0xF000) >> 12)
				);
				++dest;

				*dest = (char)(
					0x80 |
					((ch & 0x0FC0) >> 6)
				);
				++dest;

				*dest = (char)(
					0x80 |
					(ch & 0x003F)
				);
				++dest;
			}
		}
	}

	template<typename InputIterator, typename OutputIterator>
	static OutputIterator utf8_wchar(InputIterator first, InputIterator last, OutputIterator dest) {
		for(; first!=last; ++dest)
			*dest=detail::decode_utf8(first, last);
		return dest;
	}

	template<typename InputIterator, typename OutputIterator>
	static OutputIterator wchar_utf8(InputIterator first, InputIterator last, OutputIterator dest) {
		for(; first!=last; ++first)
			detail::encode_wchar(*first, dest);
		return dest;
	}

	static void utf8_wchar(const std::string &utf8, std::wstring &wchar) {
		wchar.clear();
		utf8_wchar(utf8.begin(), utf8.end(), std::back_inserter(wchar));
	}

	static std::wstring utf8_wchar(const std::string &str) {
		std::wstring ret;
		utf8_wchar(str, ret);
		return ret;
	}

	static void wchar_utf8(const std::wstring &wchar, std::string &utf8) {
		utf8.clear();
		wchar_utf8(wchar.begin(), wchar.end(), std::back_inserter(utf8));
	}

	static std::string wchar_utf8(const std::wstring &str) {
		std::string ret;
		wchar_utf8(str, ret);
		return ret;
	}

	template<typename InputIterator>
	class iutf8_iterator : public std::iterator<std::input_iterator_tag,wchar_t> {
	private:
		InputIterator it, e;
		wchar_t c;
		bool d;

		void getval() {
			if(!(d=(it==e)))
				c=detail::decode_utf8(it);
		}

	public:
		iutf8_iterator() : d(true) {}

		iutf8_iterator(InputIterator iter, InputIterator end) : it(iter),e(end),d(false) {
			if(iter!=end && *it == '\xEF') {
				if(++it == end) throw utf8_error("utf-8 bom incomplete");
				if(*it != '\xBB') throw utf8_error("utf-8 bom invalid");

				if(++it == end) throw utf8_error("utf-8 bom incomplete");
				if(*it != '\xBF') throw utf8_error("utf-8 bom invalid");

				++it;
			}

			getval();
		}

		iutf8_iterator& operator++() {
			getval();
			return *this;
		}

		iutf8_iterator operator++(int) {
			iutf8_iterator ret=*this;
			++*this;
			return ret;
		}

		bool operator==(const iutf8_iterator &iter) const { return (d == iter.d); }
		bool operator!=(const iutf8_iterator &iter) const { return (d != iter.d); }

		wchar_t operator*() const { return c; }
	};

	template<typename OutputIterator>
	class outf8_iterator : public std::iterator<std::output_iterator_tag,wchar_t> {
	private:
		OutputIterator it;

	public:
		outf8_iterator(OutputIterator iter) : it(iter) {
			*it='\xEF'; ++it;
			*it='\xBB'; ++it;
			*it='\xBF'; ++it;
		}

		outf8_iterator& operator=(wchar_t ch) { detail::encode_wchar(ch, it); return *this; }

		outf8_iterator& operator++() { return *this; }
		outf8_iterator operator++(int) { return *this; }

		outf8_iterator& operator*() { return *this; }
	};
}

#endif
