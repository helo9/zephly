#ifndef SETTINGS_HPP
#define SETTINGS_HPP

#include <sys/printk.h>
#include <cstring>
#include <optional>

struct Parameter {
	static constexpr size_t LEN_NAME = 15;
	char name[LEN_NAME] = "empty";
	union {
		int int_val;
		float float_val;
		double double_val;
	} value {0};

	template<class T>
	static Parameter create_parameter(const char name[], T val) {

		Parameter p{};
		
		p.set_name(name);

		p._select_member<T>() = val;

		return p;
	}

	template<class T>
	const T& select_member(){
		return this->_select_member<T>();
	}

private:
	void set_name(const char name[]);

	template<class T>
	T& _select_member();
};


class Settings {
public:
	static Settings& getInstance(){
		static Settings instance;

		return instance;
	}
	Settings(Settings const&)		= delete;
	void operator=(Settings const&) = delete;

	int init();
	
	template<class T>
	const T get(const char str[], T default_value) {
		for (auto param : _storage) {
			printk("%s - %s -- %d\n", param.name, str, std::strcmp(str, param.name));
			if (std::strcmp(str, param.name) == 0) {
				return param.select_member<T>();
			}
		}
		return default_value;
		
	}
private:
	Settings() {}

	float param1=1.0f;

	Parameter _storage[15];
};

constexpr auto settings = Settings::getInstance;

#endif // SETTINGS_HPP
