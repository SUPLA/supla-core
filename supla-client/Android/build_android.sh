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

NDK_ROOT="/Users/AC/Devel/projekty/Android/ndk-r10c"

export OPENSSL_ANDROID=/Users/AC/Public/openssl

$NDK_ROOT/ndk-build clean
$NDK_ROOT/ndk-build -C ./ NDK_DEBUG=1

cp  libs/armeabi/libsuplaclient.so ~/AndroidStudioProjects/SUPLA/app/src/main/libs/armeabi
cp  libs/armeabi-v7a/libsuplaclient.so ~/AndroidStudioProjects/SUPLA/app/src/main/libs/armeabi-v7a
cp  libs/x86/libsuplaclient.so ~/AndroidStudioProjects/SUPLA/app/src/main/libs/x86
