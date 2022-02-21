#!/bin/sh
set -e
if test "$CONFIGURATION" = "Debug"; then :
  cd /Users/fuki/src/j-base-clustring
  echo Build\ all\ projects
fi
if test "$CONFIGURATION" = "Release"; then :
  cd /Users/fuki/src/j-base-clustring
  echo Build\ all\ projects
fi
if test "$CONFIGURATION" = "MinSizeRel"; then :
  cd /Users/fuki/src/j-base-clustring
  echo Build\ all\ projects
fi
if test "$CONFIGURATION" = "RelWithDebInfo"; then :
  cd /Users/fuki/src/j-base-clustring
  echo Build\ all\ projects
fi

