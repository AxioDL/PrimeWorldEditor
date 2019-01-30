#ifndef NCORETESTS_H
#define NCORETESTS_H

#include "Core/Resource/EResType.h"

/** Unit tests for Core */
namespace NCoreTests
{

/** Check commandline input to see if the user is running a unit test */
bool RunTests(int argc, char *argv[]);

/** Validate all cooker output for the given resource type matches the original asset data */
bool ValidateCooker(EResourceType ResourceType, bool DumpInvalidFileContents);

}

#endif // NCORETESTS_H
