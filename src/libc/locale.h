#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

// extracted from manpage
#define LC_ALL 1            // All of the locale
#define LC_ADDRESS 2        // Formatting of addresses and geography-related items (*)
#define LC_COLLATE 3        // String collation
#define LC_CTYPE 4          // Character classification
#define LC_IDENTIFICATION 5 // Metadata describing the locale (*)
#define LC_MEASUREMENT 6    // Settings related to measurements (metric versus US customary) (*)
#define LC_MESSAGES 7       // Localizable natural-language messages
#define LC_MONETARY 8       // Formatting of monetary values
#define LC_NAME 9           // Formatting of salutations for persons (*)
#define LC_NUMERIC 10       // Formatting of nonmonetary numeric values
#define LC_PAPER 11         // Settings related to the standard paper size (*)
#define LC_TELEPHONE 12     // Formats to be used with telephone services (*)
#define LC_TIME 13          // Formatting of date and time values

    char *setlocale(int category, const char *locale);

#define LC_WOS_UTF8 1

    int getencoding();
#ifdef __cplusplus
}
#endif
