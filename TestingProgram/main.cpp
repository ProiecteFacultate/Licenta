#include <iostream>

#include "include/time_tests/fat32TimeTests.h"
#include "include/time_tests/ext2TimeTests.h"
#include "include/time_tests/hfsTimeTests.h"
#include "include/space_tests/fat32SpaceTests.h"
#include "include/space_tests/ext2SpaceTests.h"
#include "include/space_tests/hfsSpaceTests.h"
#include "include/commonTests.h"

int main()
{
    common_time_test_2();

    return 0;
}
