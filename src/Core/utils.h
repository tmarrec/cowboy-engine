#pragma once

#include <iostream>

#define INFO(m) \
    {\
    std::cerr << "   \033[44m\033[1m[INFO]\033[49m\033[0m "; \
    std::cerr << m << '\n'; \
    }

#define OK(m) \
    {\
    std::cerr << "   \033[42m\033[1m[ OK ]\033[49m\033[0m "; \
    std::cerr << m << '\n'; \
    }

#define WARNING(m) \
    {\
    std::cerr << "\033[43m\033[1m[WARNING]\033[49m\033[0m "; \
    std::cerr << m << '\n'; \
    }

#define WARNING_FILE(m) \
    {\
    std::cerr << '\n'; \
    std::cerr << "\033[43m\033[1m[WARNING]\033[49m\033[0m " << '\n'; \
    std::cerr << "\033[1mFILE\033[0m    : " << __FILE__ << '\n'; \
    std::cerr << "\033[1mFUNCTION\033[0m: " << __func__ << '\n'; \
    std::cerr << "\033[1mLINE\033[0m    : " << __LINE__ << '\n'; \
    std::cerr << "\033[1mMESSAGE\033[0m : " << m << '\n'; \
    std::cerr << '\n'; \
    }

#define ERROR(m) \
    {\
    std::cerr << "  \033[41m\033[1m[ERROR]\033[49m\033[0m "; \
    std::cerr << m << '\n'; \
    }

#define ERROR_EXIT(m) \
    {\
    std::cerr << '\n'; \
    std::cerr << "\033[41m\033[1m[ERROR]\033[49m\033[0m " << '\n'; \
    std::cerr << "\033[1mFILE\033[0m    : " << __FILE__ << '\n'; \
    std::cerr << "\033[1mFUNCTION\033[0m: " << __func__ << '\n'; \
    std::cerr << "\033[1mLINE\033[0m    : " << __LINE__ << '\n'; \
    std::cerr << "\033[1mMESSAGE\033[0m : " << m << '\n'; \
    std::cerr << '\n'; \
    exit(2);\
    }

#ifndef NDEBUG
#define ASSERT(c, m) \
    {\
    if (!(c)) { \
        std::cerr << '\n'; \
        std::cerr << "\033[41m\033[1m[ASSERT ERROR]\033[49m\033[0m " << '\n'; \
        std::cerr << "\033[1mFILE\033[0m    : " << __FILE__ << '\n'; \
        std::cerr << "\033[1mFUNCTION\033[0m: " << __func__ << '\n'; \
        std::cerr << "\033[1mLINE\033[0m    : " << __LINE__ << '\n'; \
        std::cerr << "\033[1mMESSAGE\033[0m : " << m << '\n'; \
        std::cerr << '\n'; \
        exit(3); \
    }\
    }
#else
#define ASSERT(c, m) \
    {\
        do \
        { \
        } while(0) \
    }
#endif

#define PRINT_TITLE() \
    std::cout << \
        "vulkan-testings - Tristan Marrec 2021"\
        << '\n';
