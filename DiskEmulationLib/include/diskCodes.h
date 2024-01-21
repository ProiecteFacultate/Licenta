#ifndef DISKEMULATIONLIB_DISKCODES_H
#define DISKEMULATIONLIB_DISKCODES_H

//Error codes (we add 1000 to each error code in order to not mismatch with other response values)
#define EC_NO_ERROR 1000
#define EC_SECTOR_NOT_FOUND 1004
#define EC_READ_ERROR 1016
#define EC_ERROR_IN_DISK_CONTROLLER 1032



//Special disk services related
#define MULTIPLE_SECTORS_VERIFY_UNEQUAL_DATA 1



//Helper methods related
#define METADATA_SECTOR_WRITE_FAILED (-1)
#define METADATA_SECTOR_WRITE_SUCCESS 0

#define SECTOR_READ_FAILED (-1)
#define SECTOR_READ_SUCCESS 0

#define SECTOR_WRITE_FAILED (-1)
#define SECTOR_WRITE_SUCCESS 0

#define SECTOR_VERIFY_FAILED (-1)
#define SECTOR_VERIFY_EQUAL_DATA 0
#define SECTOR_VERIFY_UNEQUAL_DATA 1




#endif //DISKEMULATIONLIB_DISKCODES_H
