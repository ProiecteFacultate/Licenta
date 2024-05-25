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
    hfs_space_test_5();

    return 0;
}
