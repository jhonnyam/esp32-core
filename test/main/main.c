/* Example test application for testable component.

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include <string.h>
#include "unity.h"

static void print_banner(const char* text);

void app_main()
{
    print_banner("Running tests without [fails] tag");
    UNITY_BEGIN();
    unity_run_tests_by_tag("[fails]", true);
    UNITY_END();

    print_banner("Starting interactive test menu");
    unity_run_menu();
}

static void print_banner(const char* text)
{
    printf("\n#### %s #####\n\n", text);
}

