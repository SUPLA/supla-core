/*
 Copyright (C) AC SOFTWARE SP. Z O.O.

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include "ipc/TakeOcrPhotoCommandTest.h"

namespace testing {

void TakeOcrPhotoCommandTest::SetUp() {
  IpcCommandTest::SetUp();
  cmd = new TakeOcrPhotoCommandMock(socketAdapter);
}

void TakeOcrPhotoCommandTest::TearDown() {
  IpcCommandTest::TearDown();
  delete cmd;
}

supla_abstract_ipc_command *TakeOcrPhotoCommandTest::getCommand(void) {
  return cmd;
}

TEST_F(TakeOcrPhotoCommandTest, noData) {
  EXPECT_FALSE(cmd->process_command(buffer, sizeof(buffer), 0));
}

TEST_F(TakeOcrPhotoCommandTest, TakeOcrPhotoWithSuccess) {
  EXPECT_CALL(*cmd, take_ocr_photo(10, 20, 30)).WillOnce(Return(true));

  commandProcessingTest("TAKE-OCR-PHOTO:10,20,30\n", "OK:30\n");
}

TEST_F(TakeOcrPhotoCommandTest, TakeOcrPhotoWithFilure) {
  EXPECT_CALL(*cmd, take_ocr_photo).WillOnce(Return(false));
  commandProcessingTest("TAKE-OCR-PHOTO:10,20,30\n", "UNKNOWN:30\n");
}

TEST_F(TakeOcrPhotoCommandTest, noParams) {
  EXPECT_CALL(*cmd, take_ocr_photo).Times(0);
  commandProcessingTest("TAKE-OCR-PHOTO:\n", "UNKNOWN:0\n");
}

TEST_F(TakeOcrPhotoCommandTest, paramsWithZeros) {
  EXPECT_CALL(*cmd, take_ocr_photo).Times(0);
  commandProcessingTest("TAKE-OCR-PHOTO:0,0,0\n", "UNKNOWN:0\n");
}

TEST_F(TakeOcrPhotoCommandTest, badParams) {
  EXPECT_CALL(*cmd, take_ocr_photo).Times(0);
  commandProcessingTest("TAKE-OCR-PHOTO:a,10,c\n", "UNKNOWN:0\n");
}

} /* namespace testing */
