#ifndef COMMON_H
#define COMMON_H

#include "types.h"
#include "AssertMacro.h"
#include "CAssetID.h"
#include "CColor.h"
#include "CFourCC.h"
#include "CScopedTimer.h"
#include "CTimer.h"
#include "EGame.h"
#include "EKeyInputs.h"
#include "EMouseInputs.h"
#include "FileIO.h"
#include "FileUtil.h"
#include "Flags.h"
#include "Log.h"
#include "TString.h"
#include "Hash/CCRC32.h"
#include "Hash/CFNV1A.h"
#include "Serialization/Binary.h"
#include "Serialization/XML.h"
#include "NBasics.h"

// temporary home for ALIGN macro, moving later
#define ALIGN(Val, Align) ((Val + (Align-1)) & ~(Align-1))

// temporary home for MEMBER_OFFSET macro
#define MEMBER_OFFSET(TypeName, MemberName) ( (int) (long long) &((TypeName*)0)->MemberName )

#endif // COMMON_H
