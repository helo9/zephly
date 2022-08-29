#! /bin/sh

cppcheck \
    --error-exitcode=1 \
    --force \
    --enable=warning,style,performance,portability,information \
    \
    drivers \
    lib \
    app
