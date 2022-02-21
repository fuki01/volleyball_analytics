#!/bin/sh
set -e
if test "$CONFIGURATION" = "Debug"; then :
  cd /Users/fuki/src/j-base-clustring
  make -f /Users/fuki/src/j-base-clustring/CMakeScripts/ReRunCMake.make
fi
if test "$CONFIGURATION" = "Release"; then :
  cd /Users/fuki/src/j-base-clustring
  make -f /Users/fuki/src/j-base-clustring/CMakeScripts/ReRunCMake.make
fi
if test "$CONFIGURATION" = "MinSizeRel"; then :
  cd /Users/fuki/src/j-base-clustring
  make -f /Users/fuki/src/j-base-clustring/CMakeScripts/ReRunCMake.make
fi
if test "$CONFIGURATION" = "RelWithDebInfo"; then :
  cd /Users/fuki/src/j-base-clustring
  make -f /Users/fuki/src/j-base-clustring/CMakeScripts/ReRunCMake.make
fi

