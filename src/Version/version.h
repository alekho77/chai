#pragma once
/*
Copyright 2015 Aleksey Khozin.
This file is part of the Chai.
See prod/demo/docs/LICENSE.txt for Copyright information.

Only used by version.rc
*/

#ifndef CHAI_SRC_VERSION_VERSION_H_
#define CHAI_SRC_VERSION_VERSION_H_

#define VER_FILEVERSION 1, 0, 0, 0
#define VER_FILEVERSION_STR "1.00\0"

#define VER_PRODUCTVERSION 1, 0, 0, 0
#define VER_PRODUCTVERSION_STR "1.00\0"

// VS_FF_PATCHED - File has been modified and is not identical to the original shipping file of the same version number.
// VS_FF_PRERELEASE - File is a development version, not a commercially released product.
// VS_FF_PRIVATEBUILD - File was not built using standard release procedures (must be a PrivateBuild string).
// VS_FF_SPECIALBUILD - File was built using standard release procedures
//                      but is a variation of the standard file of the same version number (must be a SpecialBuild string).
#define VER_FLAGS (VS_FF_PRIVATEBUILD)

#define VER_PRIVATEBUILD_STR "Demo version\0"

#define IDI_MAIN_ICON 91

#endif  // CHAI_SRC_VERSION_VERSION_H_
