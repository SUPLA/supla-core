#!/bin/bash

# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

[ -z "$NDK_ROOT" ] && NDK_ROOT=$(echo ~/Library/Android/ndk-r14b)
[ -z "$SUPLA_ANDROID" ] && SUPLA_ANDROID=$(echo ~/StudioProjects/supla-android)
[ -z "$OPENSSL_ANDROID" ] && export OPENSSL_ANDROID=$(echo ~/CProjects/openssl)

$NDK_ROOT/ndk-build clean
$NDK_ROOT/ndk-build V=1 -C ./ NDK_DEBUG=1

cp  libs/armeabi/libsuplaclient.so $SUPLA_ANDROID/app/src/main/libs/armeabi/
cp  libs/armeabi-v7a/libsuplaclient.so $SUPLA_ANDROID/app/src/main/libs/armeabi-v7a/
cp  libs/x86/libsuplaclient.so $SUPLA_ANDROID/app/src/main/libs/x86/
