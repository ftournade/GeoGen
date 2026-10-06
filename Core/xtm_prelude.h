//GeoGen: minimal replacement for xtm's Core/stdafx.h (not part of this repo).
//Provides what the vendored xtm sources expect to be force-included, without <windows.h>
//so it can be included after the MFC headers.
#pragma once

#include <stdlib.h>
#include <math.h>
#include <vector>
#include <map>
#include <string>

#include <Core/BasicTypes.h>
#include <Core/MathConstants.h>
#include <Core/MemoryManager.h>
