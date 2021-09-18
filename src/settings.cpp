#include "settings.hpp"

#include <string.h>
#include <zephyr.h>
#include <settings/settings.h>
#include <sys/printk.h>


int Settings::init() {
    int rc =  settings_subsys_init();
    
    if (rc!=0) {
        return rc;
    }

    _storage[0] = Parameter::create_parameter<float>("Petra", 1.0f);
    _storage[1] = Parameter::create_parameter<int>("Peter", 1);

    //settings_register();
    return settings_load();
}

void Parameter::set_name(const char name[]) {
    size_t input_name_len = strlen(name);
    strncpy(this->name, name, (LEN_NAME<input_name_len)?LEN_NAME:input_name_len);
}

template<>
float& Parameter::_select_member<float>() {
	return value.float_val;
}

template<>
double& Parameter::_select_member<double>() {
	return value.double_val;
}

template<>
int& Parameter::_select_member<int>() {
	return value.int_val;
}
