#pragma once

#define DEBUG 0

#if DEBUG
#ifndef debug(S)
#define debug(S) UI::Dialog::IO->println(S)
#endif
#else
#ifndef debug(S)
#define debug(S)
#endif
#endif
