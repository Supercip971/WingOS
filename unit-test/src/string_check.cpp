#include "string_check.h"
#include "unit_test.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
LIB(libc_string)
{

    SECTION("string length function verification")
    {

        CHECK("strlen test")
        {

            size_t str_length = 5;
            const char *str = "hello";
            REQUIRE_EQUAL(strlen(str), str_length);
            size_t str2_length = 30;
            const char *str2 = "012345679012345678901234567890";
            REQUIRE_EQUAL(strlen(str2), str2_length);
        }

        CHECK("strnlen no limit test")
        {

            size_t str_length = 5;
            const char *str = "hello";
            REQUIRE_EQUAL(strnlen(str, 999), str_length);
            size_t str2_length = 30;
            const char *str2 = "012345679012345678901234567890";
            REQUIRE_EQUAL(strnlen(str2, 999), str2_length);
        }
        CHECK("strnlen with limit test")
        {

            const char *str2 = "012345679012345678901234567890";
            REQUIRE_EQUAL(strnlen(str2, 3), 3);
            REQUIRE_EQUAL(strnlen(str2, 7), 7);
            REQUIRE_EQUAL(strnlen(str2, 30), 30);
        }
    }

    SECTION("string compare function verification")
    {

        CHECK("strcmp positive result test")
        {

            const char *a1 = "hello";
            const char *a2 = "hello";
            REQUIRE(strcmp(a1, a2) == 0);

            const char *b1 = "a";
            const char *b2 = "a";
            REQUIRE(strcmp(b1, b2) == 0);

            const char *c1 = "gnu/linux";
            const char *c2 = "gnu/linux";
            REQUIRE(strcmp(c1, c2) == 0);
        }
        CHECK("strcmp negative result test")
        {

            const char *a1 = "hello";
            const char *a2 = "heLlO";
            REQUIRE(strcmp(a1, a2) != 0);

            const char *b1 = "testing";
            const char *b2 = "testingtest";
            REQUIRE(strcmp(b1, b2) != 0);

            const char *c1 = "c++ powaaaa";
            const char *c2 = "ansi c powaaaa";
            REQUIRE(strcmp(c1, c2) != 0);
        }
    }

    SECTION("memory compare function verification")
    {

        CHECK("memcmp positive result test")
        {

            uint8_t a1[] = {0, 0, 0, 10, 10, 20};
            uint8_t a2[] = {0, 0, 0, 10, 10, 20};
            REQUIRE(memcmp(a1, a2, sizeof(a1)));

            uint8_t b1[] = {1};
            uint8_t b2[] = {1};
            REQUIRE(memcmp(b1, b2, sizeof(b1)));

            uint8_t c1[] = {0, 5, 0, 0, 10, 10};
            uint8_t c2[] = {0, 5, 0, 0, 10, 10};
            REQUIRE(memcmp(c1, c2, sizeof(c1)));
        }

        CHECK("memcmp negative result test")
        {

            uint8_t a1[] = {0, 0, 0, 10, 10, 20};
            uint8_t a2[] = {0, 0, 0, 10, 10, 30};
            REQUIRE(!memcmp(a1, a2, sizeof(a1)));

            uint8_t b1[] = {0};
            uint8_t b2[] = {1};
            REQUIRE(!memcmp(b1, b2, sizeof(b1)));

            uint8_t c1[] = {1, 0, 0, 0, 10, 10};
            uint8_t c2[] = {0, 0, 0, 0, 10, 10};
            REQUIRE(!memcmp(c1, c2, sizeof(c1)));
        }
    }
    SECTION("memory set function verification")
    {

        CHECK("memset test")
        {
            constexpr int size_to_check = 512;
            uint8_t buffer[size_to_check];
            memset(buffer, 8, size_to_check);
            memset(buffer, 64, size_to_check / 2);
            for (int i = 0; i < size_to_check; i++)
            {
                if (i < size_to_check / 2)
                {
                    REQUIRE_EQUAL(buffer[i], 64);
                }
                else
                {
                    REQUIRE_EQUAL(buffer[i], 8);
                }
            }
        }
    }
    SECTION("split string into tokens verification") {
        CHECK("strtoken test") 
        {
            const char *s = "This is a cool test!";
            char *e = strtok(const_cast<char*>(s), ' ');

            for (int i = 0; s[i] != ' '; i++) {
                REQUIRE(s[i] == *(e+i)) 
            }
            for (int i = 4; s[i] != ' '; i++) {
                REQUIRE(s[i] == *(e+i)) 
            }
        }
    }
}
END_LIB(libc_string)
