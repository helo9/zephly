#include "settings.hpp"
#include <sys/printk.h>
#include <zephyr.h>

int main() {
    settings().init();

    const float petra = settings().get<float>("Petra", 0.0);
    const int peter = settings().get<int>("Peter", 0);

    printk("Hallo\t%f\t%d!\n", petra, peter);
    k_sleep(K_MSEC(5000));

    return 0;
}
