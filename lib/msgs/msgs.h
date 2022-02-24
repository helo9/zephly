#ifndef LIB_MSGS_MSGS_H
#define LIB_MSGS_MSGS_H

struct Command {
    float roll; // -1.0 to 1.0
    float pitch; // -1.0 to 1.0
    float yaw; // -1.0 to 1.0
    float thrust; // 0.0 to 1.0
};

#endif /* LIB_MSGS_MSGS_H */
