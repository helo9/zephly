#ifndef SETTINGS_HPP
#define SETTINGS_HPP

#include <zephyr.h>
#include <settings/settings.h>
#include <sys/printk.h>

class Settings {
public:
	static Settings& getInstance(){
		static Settings instance;

		return instance;
	}
	Settings(Settings const&)				= delete;
	void operator=(Settings const&) = delete;

	int init() {
		int rc =  settings_subsys_init();
		
		if (rc!=0) {
			return rc;
		}
		//settings_register();
		return settings_load();
	}

private:
	Settings() {}
};

#endif // SETTINGS_HPP
